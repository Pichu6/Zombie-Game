shader_type canvas_item;
render_mode blend_add;

// =============================================================================
// SHADER ESQUELÉTICO Y TRAUMATISMOS (2D CANVASITEM)
// Renderiza el sistema óseo en estilo rayos X holográficos e ilumina en
// alerta roja/ámbar pulsante cuando se detectan fracturas o fisuras activas.
// =============================================================================

uniform float fracture_intensity : hint_range(0.0, 1.0) = 0.0;
uniform vec4 bone_color : source_color = vec4(0.45, 0.82, 0.92, 0.80);
uniform vec4 fracture_alert_color : source_color = vec4(1.0, 0.35, 0.15, 1.0);
uniform float overall_alpha : hint_range(0.0, 1.0) = 1.0;

void fragment() {
	vec4 tex_color = texture(TEXTURE, UV);
	if (tex_color.a < 0.01) {
		discard;
	}

	// 1. Resplandor base de rayos X
	float bone_luminance = 0.90 + 0.10 * sin(TIME * 2.0);

	// 2. Destello de alerta de fractura ósea
	float fracture_flash = (sin(TIME * 6.0) * 0.5 + 0.5) * fracture_intensity;
	vec3 final_bone_col = mix(bone_color.rgb, fracture_alert_color.rgb, fracture_flash);

	COLOR.rgb = tex_color.rgb * final_bone_col * bone_luminance;
	COLOR.a = tex_color.a * overall_alpha;
}
