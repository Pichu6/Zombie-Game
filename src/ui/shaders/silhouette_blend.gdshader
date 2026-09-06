shader_type canvas_item;

// =============================================================================
// SHADER DE SILUETA MÉDICA MULTI-SOMATOTIPO (2D CANVASITEM)
// Mezcla continua y sin saltos bruscos entre:
//   - Normal (silueta de referencia)
//   - Skinny (flaco / bajo peso)
//   - Muscle (musculoso / hipertrofia)
//   - Fat (grasa corporal / sobrepeso)
// Elimina el fondo negro mediante recorte suave de luminancia y colorea en
// estilo ciberpunk médico / holográfico.
// =============================================================================

uniform sampler2D tex_skinny : filter_linear, repeat_disable;
uniform sampler2D tex_muscle : filter_linear, repeat_disable;
uniform sampler2D tex_fat : filter_linear, repeat_disable;

uniform float weight_skinny : hint_range(0.0, 1.0) = 0.0;
uniform float weight_muscle : hint_range(0.0, 1.0) = 0.0;
uniform float weight_fat : hint_range(0.0, 1.0) = 0.0;
uniform float overall_alpha : hint_range(0.0, 1.0) = 0.85;

void fragment() {
	vec4 c_norm = texture(TEXTURE, UV);
	vec4 c_skin = texture(tex_skinny, UV);
	vec4 c_musc = texture(tex_muscle, UV);
	vec4 c_fat  = texture(tex_fat, UV);

	vec3 blended = c_norm.rgb;
	if (weight_fat > 0.001) {
		blended = mix(blended, c_fat.rgb, weight_fat);
	}
	if (weight_skinny > 0.001) {
		blended = mix(blended, c_skin.rgb, weight_skinny);
	}
	if (weight_muscle > 0.001) {
		blended = mix(blended, c_musc.rgb, weight_muscle);
	}

	float brightness = max(blended.r, max(blended.g, blended.b));
	float alpha = smoothstep(0.06, 0.18, brightness) * overall_alpha;

	COLOR = vec4(blended, alpha);
}
