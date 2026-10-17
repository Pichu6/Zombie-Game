// Thermal camera (FLIR) as a post-process pass over the finished frame.
// White-hot / black-hot, optional ironbow palette, sensor pixelation, soft IR blur, temperature bands,
// hot-spot bloom, drifting noise, round lens mask and a HUD (crosshair, readout, frame counter, scale bar).
// The "temperature" is the frame's luminance: the scene is not asked how hot anything is.
//
// Ported from God's Eye View, src/styles/thermal.js (MIT, Copyright (c) 2026 Bilawal Sidhu;
// see LICENSE.gods-eye-view next to this file). Changes: GLSL 450 for Vulkan, push constants instead of
// uniforms, the size read from the texture, the HUD placed for Vulkan's top-left origin and drawn over the
// lens mask, intensity 0 leaving the whole frame untouched.
//
// Inputs: set 0 binding 0 is the frame (display-referred 0..1 colour, a linear clamp-to-edge sampler);
// the push constants below. Vertex shader: fullscreen.vert.
#version 450

layout(set = 0, binding = 0) uniform sampler2D scene;

layout(push_constant) uniform Params {
  float intensity;    // 0 = the frame untouched, 1 = full effect
  float time;         // seconds, for the noise and the frame counter
  float sensitivity;  // 0..1, the range of luminance mapped to temperature (default 0.75)
  float bloom;        // 0..1, hot-spot bloom (default 0.65)
  float mode;         // < 0.5 white-hot, >= 0.5 black-hot (default 0)
  float pixelation;   // 1..6, sensor cell size in pixels (default 1.5)
  float palette;      // 0 monochrome, 1 ironbow (default 0)
} pc;

layout(location = 0) in vec2 v_uv;
layout(location = 0) out vec4 out_color;

// Black, purple, magenta, red, orange, yellow, white: cold to hot.
vec3 ironbow(float t) {
  t = clamp(t, 0.0, 1.0);
  const vec3 c0 = vec3(0.0, 0.0, 0.0);
  const vec3 c1 = vec3(0.13, 0.0, 0.30);
  const vec3 c2 = vec3(0.49, 0.0, 0.45);
  const vec3 c3 = vec3(0.86, 0.10, 0.18);
  const vec3 c4 = vec3(1.0, 0.55, 0.0);
  const vec3 c5 = vec3(1.0, 0.91, 0.32);
  const vec3 c6 = vec3(1.0, 1.0, 1.0);
  float s = t * 6.0;
  if (s < 1.0) return mix(c0, c1, s);
  if (s < 2.0) return mix(c1, c2, s - 1.0);
  if (s < 3.0) return mix(c2, c3, s - 2.0);
  if (s < 4.0) return mix(c3, c4, s - 3.0);
  if (s < 5.0) return mix(c4, c5, s - 4.0);
  return mix(c5, c6, s - 5.0);
}

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

float fbm(vec2 p) {
  float v = 0.0;
  float a = 0.5;
  for (int i = 0; i < 4; i++) {
    v += a * valueNoise(p);
    p = p * 2.0 + vec2(100.0);
    a *= 0.5;
  }
  return v;
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

// A digit 0-9, 10 for '.', 11 for the degree sign.
float renderChar(vec2 p, int ch) {
  if (ch == 10) return smoothstep(0.15, 0.0, length(p - vec2(0.5, 0.08)));
  if (ch == 11) return smoothstep(0.04, 0.02, abs(length(p - vec2(0.5, 0.82)) - 0.08));
  if (ch >= 0 && ch <= 9) return digit(p, ch);
  return 0.0;
}

// One character of the HUD, origin and size in HUD coordinates.
float hudChar(vec2 hudUV, vec2 origin, vec2 size, int ch) {
  vec2 p = (hudUV - origin) / size;
  if (p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0) return 0.0;
  return renderChar(p, ch);
}

float crosshair(vec2 uv) {
  vec2 c = uv - 0.5;
  float h = smoothstep(0.0015, 0.0005, abs(c.y)) * step(0.015, abs(c.x)) * step(abs(c.x), 0.04);
  float v = smoothstep(0.0015, 0.0005, abs(c.x)) * step(0.015, abs(c.y)) * step(abs(c.y), 0.04);
  return clamp(h + v, 0.0, 1.0);
}

float scaleBar(vec2 uv) {
  return step(0.94, uv.x) * step(uv.x, 0.955) * step(0.15, uv.y) * step(uv.y, 0.85);
}

float luma(vec3 c) { return dot(c, vec3(0.299, 0.587, 0.114)); }

// The HUD over the finished, masked image (the original drew it under the lens mask, which hid its corners).
// hudUV has y up from the bottom of the screen, as in the original's layout.
vec3 drawHud(vec3 color, vec2 hudUV) {
  float intensity = pc.intensity;
  float isBlackHot = step(0.5, pc.mode);
  float hud = 0.0;

  // Top left: label and mode markers (bars, not text).
  vec2 labelArea = (hudUV - vec2(0.02, 0.92)) / vec2(0.08, 0.04);
  if (labelArea.x >= 0.0 && labelArea.x <= 1.0 && labelArea.y >= 0.0 && labelArea.y <= 1.0)
    hud += step(0.1, labelArea.x) * step(labelArea.x, 0.9) * step(0.3, labelArea.y) * step(labelArea.y, 0.7) * 0.6;
  vec2 modeArea = (hudUV - vec2(0.02, 0.88)) / vec2(0.06, 0.03);
  if (modeArea.x >= 0.0 && modeArea.x <= 1.0 && modeArea.y >= 0.0 && modeArea.y <= 1.0)
    hud += step(0.1, modeArea.x) * step(modeArea.x, 0.9) * step(0.2, modeArea.y) * step(modeArea.y, 0.8) * 0.4;

  hud += crosshair(hudUV) * 0.7;

  // Top right: a readout of 20-50 degrees from the luminance at the centre.
  float tempC = 20.0 + luma(texture(scene, vec2(0.5)).rgb) * 30.0;
  int tempInt = int(tempC);
  int tempDec = int(fract(tempC) * 10.0);
  vec2 tempOrigin = vec2(0.88, 0.92);
  vec2 charSize = vec2(0.018, 0.035);
  float spacing = 0.02;
  float tempHud = hudChar(hudUV, tempOrigin, charSize, tempInt / 10)
                + hudChar(hudUV, tempOrigin + vec2(spacing, 0.0), charSize, tempInt % 10)
                + hudChar(hudUV, tempOrigin + vec2(spacing * 2.0, 0.0), charSize, 10)
                + hudChar(hudUV, tempOrigin + vec2(spacing * 2.6, 0.0), charSize, tempDec)
                + hudChar(hudUV, tempOrigin + vec2(spacing * 3.5, 0.0), charSize, 11);
  hud += tempHud * 0.8;

  // Bottom right: frame counter at 30 frames a second.
  const int place[4] = int[4](1000, 100, 10, 1);
  int frame = int(mod(pc.time * 30.0, 10000.0));
  float frameHud = 0.0;
  for (int i = 0; i < 4; i++)
    frameHud += hudChar(hudUV, vec2(0.88, 0.04) + vec2(float(i) * spacing, 0.0), charSize, (frame / place[i]) % 10);
  hud += frameHud * 0.5;

  // Right edge: the temperature scale.
  float bar = scaleBar(hudUV);
  if (bar > 0.0) {
    float barGrad = (hudUV.y - 0.15) / 0.7;
    color = mix(color, vec3(mix(barGrad, 1.0 - barGrad, isBlackHot)), bar * 0.9 * intensity);
  }

  return clamp(color + hud * 0.6 * intensity, 0.0, 1.0);
}

void main() {
  vec2 uv = v_uv;
  vec2 dims = vec2(textureSize(scene, 0));
  vec2 texel = 1.0 / dims;
  // Where drawHud places things: y up from the bottom of the screen, as in the original.
  vec2 hudUV = vec2(v_uv.x, 1.0 - v_uv.y);
  float intensity = pc.intensity;

  // Round lens: black outside, brightest in the centre.
  vec2 centered = uv * 2.0 - 1.0;
  centered.x *= dims.x / dims.y;
  float radius = length(centered);
  float lensMask = pow(1.0 - smoothstep(0.6, 1.05, radius), 0.7);
  float lensShading = max(1.0 - radius * radius * 0.25, 0.0);
  if (lensMask < 0.001) {
    out_color = vec4(drawHud(texture(scene, v_uv).rgb * (1.0 - intensity), hudUV), 1.0);
    return;
  }

  // Sensor resolution.
  float pixSize = mix(1.0, pc.pixelation, intensity);
  vec2 snappedUV = floor(uv * dims / pixSize) * pixSize / dims;
  uv = mix(uv, snappedUV, intensity);

  // Soft IR blur (5x5 Gaussian).
  vec3 blurred = vec3(0.0);
  float totalWeight = 0.0;
  for (int y = -2; y <= 2; y++) {
    for (int x = -2; x <= 2; x++) {
      float w = exp(-0.5 * float(x * x + y * y) / 2.0);
      blurred += texture(scene, uv + vec2(float(x), float(y)) * texel * 1.5).rgb * w;
      totalWeight += w;
    }
  }
  blurred /= totalWeight;
  vec4 original = texture(scene, uv);
  vec3 src = mix(original.rgb, blurred, 0.6 * intensity);

  // Luminance to temperature; sensitivity widens or narrows the range around mid grey.
  float sens = mix(0.25, 1.0, pc.sensitivity);
  float temp = clamp((luma(src) - (0.5 - sens * 0.5)) / sens, 0.0, 1.0);

  // Temperature bands.
  float bandLine = abs(fract(temp * 12.0) - 0.5);
  float contour = smoothstep(0.04, 0.06, bandLine);
  temp *= mix(1.0, contour * 0.85 + 0.15, 0.3 * intensity);

  float isBlackHot = step(0.5, pc.mode);
  float thermal = mix(temp, 1.0 - temp, isBlackHot);
  // The ironbow follows the true temperature, whatever the white-hot / black-hot switch says.
  vec3 thermalColor = mix(vec3(thermal), ironbow(temp), pc.palette);

  // Hot-spot bloom (9x9, wide taps).
  float bloomSample = 0.0;
  float bloomWeight = 0.0;
  for (int y = -4; y <= 4; y++) {
    for (int x = -4; x <= 4; x++) {
      vec2 offset = vec2(float(x), float(y)) * texel * 3.0;
      float sMapped = clamp((luma(texture(scene, uv + offset).rgb) - (0.5 - sens * 0.5)) / sens, 0.0, 1.0);
      float sFinal = mix(sMapped, 1.0 - sMapped, isBlackHot);
      float w = exp(-0.5 * float(x * x + y * y) / 8.0);
      bloomSample += smoothstep(0.6, 1.0, sFinal) * w;
      bloomWeight += w;
    }
  }
  thermalColor += bloomSample / bloomWeight * pc.bloom * 0.8;

  // Slow, cloud-like sensor noise.
  float noise = fbm(uv * 80.0 + vec2(pc.time * 0.3, pc.time * 0.2));
  thermalColor += (noise - 0.5) * 0.08 * intensity;

  thermalColor *= lensShading * lensMask;
  thermalColor = clamp(thermalColor, 0.0, 1.0);

  vec3 finalColor = mix(original.rgb, thermalColor, intensity);
  finalColor *= mix(1.0, lensMask, intensity);
  out_color = vec4(drawHud(finalColor, hudUV), 1.0);
}
