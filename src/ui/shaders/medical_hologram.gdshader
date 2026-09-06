shader_type spatial;
render_mode blend_add, depth_draw_never, cull_back, unshaded;

// =============================================================================
// SHADER ESPACIAL HOLOGRÁFICO MÉDICO (3D BIOMEDICAL SCAN)
// Produce un cuerpo traslúcido cian/teal con efecto Fresnel en los bordes,
// scanlines dinámicas y modulación de alerta clínica.
// =============================================================================

uniform vec4 base_color : source_color = vec4(0.02, 0.22, 0.32, 0.45);
uniform vec4 rim_color : source_color = vec4(0.25, 0.85, 1.0, 1.0);
uniform float rim_power : hint_range(0.5, 6.0) = 2.2;
uniform float scanline_frequency : hint_range(0.0, 200.0) = 100.0;
uniform float scanline_speed : hint_range(0.0, 10.0) = 1.2;
uniform float scanline_intensity : hint_range(0.0, 1.0) = 0.25;
uniform float glow_intensity : hint_range(0.5, 4.0) = 1.5;

// Tinte adicional de estado clínico
uniform vec4 status_tint : source_color = vec4(0.0, 0.0, 0.0, 0.0);

void fragment() {
	// 1. Detección Fresnel de contorno
	float v_dot_n = clamp(dot(VIEW, NORMAL), 0.0, 1.0);
	float fresnel = pow(1.0 - v_dot_n, rim_power);

	// 2. Scanline holográfica animada
	float scanline = sin((SCREEN_UV.y * scanline_frequency) - (TIME * scanline_speed)) * 0.5 + 0.5;
	float scan_mod = 1.0 - (scanline * scanline_intensity);

	// 3. Composición de color
	vec3 col = mix(base_color.rgb, rim_color.rgb, fresnel);
	col += status_tint.rgb * status_tint.a;
	col *= scan_mod * glow_intensity;

	ALBEDO = col;
	ALPHA = clamp(base_color.a + fresnel * 0.8, 0.0, 1.0);
}
