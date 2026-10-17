// Night vision (an image intensifier, PVS-14 style) as a post-process pass over the finished frame.
// Barrel distortion, round tube mask, gain, bloom around bright sources, green P43 phosphor, scintillation,
// honeycomb fibre plate, scanlines and a HUD (crosshair, HH:MM:SS clock, blinking REC dot).
//
// Ported from God's Eye View, src/styles/surveillance.js (MIT, Copyright (c) 2026 Bilawal Sidhu;
// see LICENSE.gods-eye-view next to this file). Changes: GLSL 450 for Vulkan, push constants instead of
// uniforms, the size read from the texture, the HUD placed for Vulkan's top-left origin, the clock from
// clock_seconds instead of the effect's time, the HUD drawn over the tube mask, intensity 0 leaving the whole
// frame untouched.
//
// Inputs: set 0 binding 0 is the frame (display-referred 0..1 colour, a linear clamp-to-edge sampler);
// the push constants below. Vertex shader: fullscreen.vert.
#version 450

layout(set = 0, binding = 0) uniform sampler2D scene;

layout(push_constant) uniform Params {
  float intensity;      // 0 = the frame untouched, 1 = full effect
  float time;           // seconds, for the noise, scanline drift and the REC blink
  float gain;           // 0..1, amplification: brighter, noisier (default 0.55)
  float bloom;          // 0..1, halo around bright sources (default 0.3)
  float scanlines;      // 0..1 (default 1)
  float pixelation;     // 1..6, tube cell size in pixels (default 2.5)
  float clock_seconds;  // the time of day shown in the HUD, seconds since midnight
} pc;

layout(location = 0) in vec2 v_uv;
layout(location = 0) out vec4 out_color;

const vec3 kPhosphor = vec3(0.16, 1.0, 0.22);  // P43, 530 nm

float hash(vec2 p) {
  vec3 p3 = fract(vec3(p.xyx) * 0.1031);
  p3 += dot(p3, p3.yzx + 33.33);
  return fract((p3.x + p3.y) * p3.z);
}

float valueNoise(vec2 p) {
  vec2 i = floor(p);
  vec2 f = fract(p);
  f = f * f * (3.0 - 2.0 * f);
  float a = hash(i);
  float b = hash(i + vec2(1.0, 0.0));
  float c = hash(i + vec2(0.0, 1.0));
  float d = hash(i + vec2(1.0, 1.0));
  return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

vec2 barrelDistort(vec2 uv, float strength) {
  vec2 c = uv * 2.0 - 1.0;
  float r2 = dot(c, c);
  c *= 1.0 + r2 * strength * 0.5 + r2 * r2 * strength * 0.15;
  return c * 0.5 + 0.5;
}

// The fibre-optic plate's hex cells: 1 on a cell edge.
float honeycomb(vec2 uv, vec2 dims) {
  vec2 p = uv * dims * (min(dims.x, dims.y) * 0.008);
  vec2 r = vec2(1.0, 1.732);
  vec2 h = r * 0.5;
  vec2 a = mod(p, r) - h;
  vec2 b = mod(p - h, r) - h;
  vec2 gv = dot(a, a) < dot(b, b) ? a : b;
  float d = max(abs(gv.x), abs(gv.y * 0.577 + abs(gv.x) * 0.5));
  return smoothstep(0.4, 0.45, d);
}

// Seven-segment digits; p is 0..1 in the cell with y up.
float segment(vec2 p, int seg) {
  float s = 0.0;
  if (seg == 0) s = step(0.2, p.x) * step(p.x, 0.8) * step(0.85, p.y) * step(p.y, 1.0);
  if (seg == 1) s = step(0.7, p.x) * step(p.x, 0.9) * step(0.5, p.y) * step(p.y, 0.95);
  if (seg == 2) s = step(0.7, p.x) * step(p.x, 0.9) * step(0.05, p.y) * step(p.y, 0.5);
  if (seg == 3) s = step(0.2, p.x) * step(p.x, 0.8) * step(0.0, p.y) * step(p.y, 0.15);
  if (seg == 4) s = step(0.1, p.x) * step(p.x, 0.3) * step(0.05, p.y) * step(p.y, 0.5);
  if (seg == 5) s = step(0.1, p.x) * step(p.x, 0.3) * step(0.5, p.y) * step(p.y, 0.95);
  if (seg == 6) s = step(0.2, p.x) * step(p.x, 0.8) * step(0.42, p.y) * step(p.y, 0.58);
  return s;
}

float digit(vec2 p, int d) {
  const int masks[10] = int[10](0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B);
  int m = masks[d];
  float s = 0.0;
  for (int i = 0; i < 7; i++) {
    if (((m >> (6 - i)) & 1) == 1) s += segment(p, i);
  }
  return clamp(s, 0.0, 1.0);
}

float colon(vec2 p) {
  return step(0.3, p.x) * step(p.x, 0.7) * (step(0.2, p.y) * step(p.y, 0.4) + step(0.6, p.y) * step(p.y, 0.8));
}

// HH:MM:SS at the bottom left.
float renderTimestamp(vec2 hudUV) {
  vec2 p = (hudUV - vec2(0.02, 0.03)) / vec2(0.22, 0.035);
  if (p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0) return 0.0;

  int totalSec = int(mod(pc.clock_seconds, 86400.0));
  int hours = totalSec / 3600;
  int minutes = (totalSec % 3600) / 60;
  int seconds = totalSec % 60;

  float charWidth = 1.0 / 8.5;
  int charIdx = int(p.x / charWidth);
  vec2 localP = vec2(mod(p.x, charWidth) / charWidth, p.y);

  int dv = -1;
  if (charIdx == 0) dv = hours / 10;
  else if (charIdx == 1) dv = hours % 10;
  else if (charIdx == 2) return colon(localP);
  else if (charIdx == 3) dv = minutes / 10;
  else if (charIdx == 4) dv = minutes % 10;
  else if (charIdx == 5) return colon(localP);
  else if (charIdx == 6) dv = seconds / 10;
  else if (charIdx == 7) dv = seconds % 10;
  if (dv < 0 || dv > 9) return 0.0;
  return digit(localP, dv);
}

float crosshair(vec2 uv) {
  vec2 c = uv - 0.5;
  float h = smoothstep(0.001, 0.0004, abs(c.y)) * step(0.01, abs(c.x)) * step(abs(c.x), 0.025);
  float v = smoothstep(0.001, 0.0004, abs(c.x)) * step(0.01, abs(c.y)) * step(abs(c.y), 0.025);
  return clamp(h + v, 0.0, 1.0);
}

float luma(vec3 c) { return dot(c, vec3(0.299, 0.587, 0.114)); }

// The HUD over the finished, masked image (the original drew it under the tube mask, which hid its corners).
// hudUV has y up from the bottom of the screen, as in the original's layout.
vec3 drawHud(vec3 color, vec2 hudUV) {
  float intensity = pc.intensity;

  // Top left: label and gain markers (bars, not text).
  vec2 labelArea = (hudUV - vec2(0.03, 0.92)) / vec2(0.06, 0.03);
  if (labelArea.x >= 0.0 && labelArea.x <= 1.0 && labelArea.y >= 0.0 && labelArea.y <= 1.0)
    color += kPhosphor * step(0.1, labelArea.x) * step(labelArea.x, 0.9) * step(0.2, labelArea.y) *
             step(labelArea.y, 0.8) * 0.3 * intensity;
  vec2 gainArea = (hudUV - vec2(0.03, 0.88)) / vec2(0.05, 0.025);
  if (gainArea.x >= 0.0 && gainArea.x <= 1.0 && gainArea.y >= 0.0 && gainArea.y <= 1.0)
    color += kPhosphor * step(0.1, gainArea.x) * step(gainArea.x, 0.9) * step(0.2, gainArea.y) *
             step(gainArea.y, 0.8) * 0.2 * intensity;

  color += kPhosphor * crosshair(hudUV) * 0.4 * intensity;
  color += kPhosphor * renderTimestamp(hudUV) * 0.6 * intensity;

  // Top right: REC, blinking.
  float recDot = smoothstep(0.008, 0.004, length(hudUV - vec2(0.95, 0.94)));
  color += vec3(0.3, 1.0, 0.2) * recDot * step(0.5, fract(pc.time * 0.8)) * intensity;

  return clamp(color, 0.0, 1.0);
}

void main() {
  vec2 uv = v_uv;
  vec2 dims = vec2(textureSize(scene, 0));
  vec2 texel = 1.0 / dims;
  // Where drawHud places things: y up from the bottom of the screen, as in the original.
  vec2 hudUV = vec2(v_uv.x, 1.0 - v_uv.y);
  float intensity = pc.intensity;

  vec2 distUV = barrelDistort(uv, 0.5 * intensity);

  // Round tube: black outside, brightest in the centre.
  vec2 centered = uv * 2.0 - 1.0;
  centered.x *= dims.x / dims.y;
  float radius = length(centered);
  float tubeMask = pow(1.0 - smoothstep(0.6, 1.05, radius), 0.7);
  float tubeShading = max(1.0 - radius * radius * 0.3, 0.0);
  if (tubeMask < 0.001 || distUV.x < 0.0 || distUV.x > 1.0 || distUV.y < 0.0 || distUV.y > 1.0) {
    out_color = vec4(drawHud(texture(scene, v_uv).rgb * (1.0 - intensity), hudUV), 1.0);
    return;
  }

  // Tube resolution.
  float pixSize = mix(1.0, pc.pixelation, intensity);
  vec2 snappedUV = floor(distUV * dims / pixSize) * pixSize / dims;
  distUV = mix(distUV, snappedUV, intensity);

  vec4 original = texture(scene, distUV);

  // Gain: more light, a steeper curve.
  float gainLevel = mix(0.8, 2.5, pc.gain);
  float amplified = clamp(luma(original.rgb) * gainLevel, 0.0, 1.0);
  amplified = pow(amplified, mix(1.2, 0.7, pc.gain));

  // Halo around bright sources (11x11, wide taps).
  float bloomAccum = 0.0;
  float bloomW = 0.0;
  for (int y = -5; y <= 5; y++) {
    for (int x = -5; x <= 5; x++) {
      vec2 offset = vec2(float(x), float(y)) * texel * 4.0;
      float bright = smoothstep(0.4, 0.9, luma(texture(scene, distUV + offset).rgb) * gainLevel);
      float w = exp(-float(x * x + y * y) / 18.0);
      bloomAccum += bright * w;
      bloomW += w;
    }
  }
  float corona = bloomAccum / bloomW * pc.bloom * 1.5;

  vec3 nvgColor = kPhosphor * (amplified + corona);

  // Scintillation: slow tube grain, plus sparkle that grows in the dark parts.
  float tubeGrain = valueNoise(uv * 120.0 + vec2(pc.time * 0.5, pc.time * 0.3));
  nvgColor += kPhosphor * (tubeGrain - 0.5) * mix(0.06, 0.2, pc.gain) * intensity;
  float darkNoise = (1.0 - amplified) * hash(uv * dims + vec2(pc.time * 200.0, pc.time * 300.0));
  nvgColor += kPhosphor * darkNoise * 0.08 * pc.gain * intensity;

  nvgColor *= 1.0 - honeycomb(distUV, dims) * 0.04 * intensity;

  float scanline = pow(sin(distUV.y * dims.y * 1.2 + pc.time * 2.0) * 0.5 + 0.5, 2.5);
  nvgColor *= 1.0 - scanline * pc.scanlines * 0.15 * intensity;

  nvgColor *= tubeShading * tubeMask;

  nvgColor = clamp(nvgColor, 0.0, 1.0);
  out_color = vec4(drawHud(mix(original.rgb, nvgColor * tubeMask, intensity), hudUV), 1.0);
}
