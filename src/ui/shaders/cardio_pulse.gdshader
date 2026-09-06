shader_type canvas_item;
render_mode blend_add;

// =============================================================================
// SHADER CARDIOVASCULAR DINÁMICO (2D CANVASITEM)
// Sincroniza el latido cardíaco visual con heart_rate_bpm, atenúa por
// hipovolemia (pérdida de sangre) y hace destellar alertas de hemorragia.
// =============================================================================

uniform float heart_rate_bpm : hint_range(0.0, 240.0) = 75.0;
uniform float blood_volume_pct : hint_range(0.0, 1.0) = 1.0;
uniform float bleed_intensity : hint_range(0.0, 1.0) = 0.0;
uniform vec4 vascular_color : source_color = vec4(0.95, 0.20, 0.20, 0.90);
uniform vec4 bleed_color : source_color = vec4(1.0, 0.05, 0.05, 1.0);
uniform float overall_alpha : hint_range(0.0, 1.0) = 1.0;

void fragment() {
	vec4 tex_color = texture(TEXTURE, UV);
	if (tex_color.a < 0.01) {
		discard;
	}

	// 1. Ritmo Cardíaco (BPS: Beams / Beats Per Second)
	float bps = heart_rate_bpm / 60.0;
	float pulse = 1.0;
	if (bps > 0.05) {
		// Onda de pulso sistólico con rebote diastólico suave
		float t = mod(TIME * bps, 1.0);
		float systolic = exp(-t * 6.0);
		pulse = 0.80 + 0.35 * systolic;
	} else {
		// Paro Cardíaco / Asistolia
		pulse = 0.15;
	}

	// 2. Pérdida de volemia (vasoconstricción periférica)
	float volume_factor = clamp(0.25 + 0.75 * blood_volume_pct, 0.2, 1.0);

	// 3. Destello de alerta de hemorragia activa
	float bleed_flash = (sin(TIME * 9.0) * 0.5 + 0.5) * bleed_intensity;
	vec3 final_color = mix(vascular_color.rgb, bleed_color.rgb, bleed_flash);

	COLOR.rgb = tex_color.rgb * final_color * pulse * volume_factor;
	COLOR.a = tex_color.a * overall_alpha;
}
