shader_type canvas_item;
render_mode blend_add;

// =============================================================================
// SHADER SISTEMA NERVIOSO Y DOLOR (2D CANVASITEM)
// Modula impulsos bioeléctricos sinusoidales que viajan por las vías neurales
// y aumenta el jitter/parpadeo de alarma según el nivel de dolor y shock.
// =============================================================================

uniform float pain_level : hint_range(0.0, 100.0) = 0.0;
uniform vec4 base_nerve_color : source_color = vec4(0.90, 0.75, 0.25, 0.85);
uniform vec4 acute_pain_color : source_color = vec4(1.0, 0.25, 0.15, 1.0);
uniform float overall_alpha : hint_range(0.0, 1.0) = 1.0;

void fragment() {
	vec4 tex_color = texture(TEXTURE, UV);
	if (tex_color.a < 0.01) {
		discard;
	}

	// 1. Impulso sináptico que se propaga verticalmente por la médula y nervios
	float wave = sin((UV.y * 14.0) - (TIME * 4.5)) * 0.5 + 0.5;
	float baseline_pulse = 0.85 + (0.25 * wave);

	// 2. Jitter eléctrico modulado por dolor (dolor severo = espasmos de alta frecuencia)
	float pain_factor = clamp(pain_level / 100.0, 0.0, 1.0);
	float jitter = sin(TIME * (15.0 + pain_factor * 25.0)) * 0.5 + 0.5;
	float pain_boost = 1.0 + (jitter * pain_factor * 0.65);

	// 3. Coloración: Amarillo dorado saludable -> Rojo/Magenta en agonía
	vec3 nerve_col = mix(base_nerve_color.rgb, acute_pain_color.rgb, pain_factor * 0.85);

	COLOR.rgb = tex_color.rgb * nerve_col * baseline_pulse * pain_boost;
	COLOR.a = tex_color.a * overall_alpha;
}
