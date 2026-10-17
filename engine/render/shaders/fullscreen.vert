// One triangle that covers the screen, for the post-process passes (thermal.frag, night_vision.frag).
// Draw 3 vertices with no vertex buffer: vkCmdDraw(cmd, 3, 1, 0, 0).
// v_uv runs 0..1 across the screen, (0,0) at the top left (Vulkan's framebuffer origin).
#version 450

layout(location = 0) out vec2 v_uv;

void main() {
  v_uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
  gl_Position = vec4(v_uv * 2.0 - 1.0, 0.0, 1.0);
}
