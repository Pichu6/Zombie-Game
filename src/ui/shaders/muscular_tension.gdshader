shader_type canvas_item;
render_mode blend_add;

// =============================================================================
// SHADER SISTEMA MUSCULAR ANATÓMICO Y TENSIÓN / DOLOR (2D CANVASITEM)
// Renderiza el sistema muscular profundo en estilo holográfico médico y
// modula pulsaciones de esfuerzo, fatiga láctica y dolor en las fibras.
// =============================================================================

uniform float muscle_fatigue : hint_range(0.0, 1.0) = 0.0;
uniform float muscle_pain : hint_range(0.0, 100.0) = 0.0;
uniform vec4 base_muscle_color : source_color = vec4(0.92, 0.45, 0.28, 0.85);
uniform vec4 strain_alert_color : source_color = vec4(1.0, 0.20, 0.12, 1.0);
uniform float overall_alpha : hint_range(0.0, 1.0) = 1.0;

void fragment() {
	vec4 tex_color = texture(TEXTURE, UV);
	if (tex_color.a < 0.01) {
		discard;
	}

	// 1. Pulso basal sutil de tono muscular en reposo
	float basal_tone = 0.88 + 0.12 * sin(TIME * 2.5);

	// 2. Tensión fibrilar y fatiga (pulsación más rápida cuando hay sobrecarga)
	float strain_factor = clamp((muscle_fatigue * 0.5) + (muscle_pain / 100.0 * 0.5), 0.0, 1.0);
	float fibril_pulse = sin(TIME * (3.0 + strain_factor * 8.0)) * 0.5 + 0.5;
	float tension_boost = 1.0 + (fibril_pulse * strain_factor * 0.55);

	// 3. Mezcla cromática: Ámbar cobrizo saludable -> Rojo carmesí intenso en lesión/sobrecarga
	vec3 final_muscle_col = mix(base_muscle_color.rgb, strain_alert_color.rgb, strain_factor * 0.85);

	COLOR.rgb = tex_color.rgb * final_muscle_col * basal_tone * tension_boost;
	COLOR.a = tex_color.a * overall_alpha;
}
