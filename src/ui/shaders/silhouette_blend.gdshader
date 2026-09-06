shader_type canvas_item;
uniform sampler2D tex_skinny : filter_linear, repeat_disable;
uniform sampler2D tex_muscle : filter_linear, repeat_disable;
uniform sampler2D tex_fat : filter_linear, repeat_disable;
uniform sampler2D scan_bones : filter_linear, repeat_disable;
uniform sampler2D scan_muscles : filter_linear, repeat_disable;
uniform sampler2D scan_vessels : filter_linear, repeat_disable;
uniform sampler2D scan_nerves : filter_linear, repeat_disable;
uniform float weight_skinny = 0.0;
uniform float weight_muscle = 0.0;
uniform float weight_fat = 0.0;
uniform float overall_alpha = 1.0;
uniform vec4 scan_regions[17];
uniform float damage_levels[17];
uniform int hover_region = -1;
uniform int selected_region = -1;
uniform int scan_mode = 0;
uniform bool scan_hold = false;
float region_mask(vec2 uv, vec4 region) {
	vec2 local = (uv - region.xy) / max(region.zw, vec2(0.001));
	return 1.0 - smoothstep(0.35, 1.05, length(local));
}
void fragment() {
	vec3 skin = texture(TEXTURE, UV).rgb;
	skin = mix(skin, texture(tex_fat, UV).rgb, weight_fat);
	skin = mix(skin, texture(tex_skinny, UV).rgb, weight_skinny);
	skin = mix(skin, texture(tex_muscle, UV).rgb, weight_muscle);
	float brightness = max(skin.r, max(skin.g, skin.b));
	float body = smoothstep(0.06, 0.18, brightness);
	float hover = 0.0;
	float selected = 0.0;
	float injury = 0.0;
	for (int i = 0; i < 17; i++) {
		float mask = region_mask(UV, scan_regions[i]);
		if (i == hover_region) { hover = mask; }
		if (i == selected_region) { selected = mask; }
		injury = max(injury, mask * damage_levels[i]);
	}
	float lens = scan_hold ? 1.0 : hover;
	vec3 interior = texture(scan_bones, UV).rgb * vec3(0.46, 0.86, 1.0);
	if (scan_mode == 1) { interior = texture(scan_muscles, UV).rgb * vec3(1.0, 0.55, 0.38); }
	if (scan_mode == 3) { interior = texture(scan_vessels, UV).rgb * vec3(1.0, 0.3, 0.38); }
	if (scan_mode == 4) { interior = texture(scan_nerves, UV).rgb * vec3(0.9, 0.76, 1.0); }
	if (scan_mode == 5) {
		interior += texture(scan_vessels, UV).rgb * vec3(0.7, 0.14, 0.22);
	}
	vec3 surface = skin * (1.0 + selected * 0.65);
	surface = mix(surface, skin * 0.15 + interior * 0.95, lens * 0.92);
	float pulse = 0.72 + 0.28 * sin(TIME * 3.2);
	surface += vec3(1.0, 0.22, 0.09) * injury * pulse * 0.7;
	COLOR = vec4(surface, body * overall_alpha);
}
