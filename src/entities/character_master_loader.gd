class_name CharacterMasterLoader
extends RefCounted

const BASE_MODEL_PATH: String = "res://char_human/rig/makehuman/model normal.fbx"
const BLEND_MESH_PATH: String = "res://char_human/rig/makehuman/makehuman_blend_mesh.res"
const EYE_TEXTURE_PATH: String = "res://char_human/rig/makehuman/brown_eye.png"
const SKIN_TEXTURE_PATH: String = "res://char_human/rig/makehuman/skin_diffuse.png"

static var _cached_model: PackedScene = null
static var _cached_blend_mesh: Mesh = null
static var _eye_texture: Texture2D = null
static var _skin_texture: Texture2D = null

static func load_character_model() -> Node3D:
	if not _cached_model:
		if not ResourceLoader.exists(BASE_MODEL_PATH):
			push_error("CharacterMasterLoader: Cannot find base model at " + BASE_MODEL_PATH)
			return null
		_cached_model = load(BASE_MODEL_PATH)
		
	var root: Node3D = _cached_model.instantiate()
	if not root:
		return null
		
	if not _cached_blend_mesh and ResourceLoader.exists(BLEND_MESH_PATH):
		_cached_blend_mesh = load(BLEND_MESH_PATH)
		
	var body_mesh_inst: MeshInstance3D = null
	for mi in root.find_children("*", "MeshInstance3D", true, false):
		if "normal" in mi.name.to_lower() or "body" in mi.name.to_lower():
			body_mesh_inst = mi
			break
			
	if body_mesh_inst and _cached_blend_mesh:
		body_mesh_inst.mesh = _cached_blend_mesh
		
	_setup_materials(root)
	return root

static func apply_morph_weights(model_root: Node3D, fat_pct: float, muscle_pct: float) -> void:
	if not model_root:
		return
		
	var mesh_inst: MeshInstance3D = null
	for mi in model_root.find_children("*", "MeshInstance3D", true, false):
		if "normal" in mi.name.to_lower() or "body" in mi.name.to_lower():
			mesh_inst = mi
			break
			
	if not mesh_inst or not mesh_inst.mesh or mesh_inst.mesh.get_blend_shape_count() == 0:
		return

	var skinny_weight: float = 0.0
	var fat_weight: float = 0.0
	var muscle_weight: float = clamp(muscle_pct, 0.0, 1.0)
	
	if fat_pct <= 0.18:
		skinny_weight = clamp((0.18 - fat_pct) / 0.18, 0.0, 1.0)
		fat_weight = 0.0
	else:
		skinny_weight = 0.0
		fat_weight = clamp((fat_pct - 0.18) / 0.82, 0.0, 1.0)
		
	var idx_skinny = mesh_inst.find_blend_shape_by_name(&"Skinny")
	if idx_skinny >= 0:
		mesh_inst.set_blend_shape_value(idx_skinny, skinny_weight)
		
	var idx_muscle = mesh_inst.find_blend_shape_by_name(&"Muscle")
	if idx_muscle >= 0:
		mesh_inst.set_blend_shape_value(idx_muscle, muscle_weight)
		
	var idx_fat = mesh_inst.find_blend_shape_by_name(&"Fat")
	if idx_fat >= 0:
		mesh_inst.set_blend_shape_value(idx_fat, fat_weight)

static func _setup_materials(root: Node3D) -> void:
	if _eye_texture == null and ResourceLoader.exists(EYE_TEXTURE_PATH):
		_eye_texture = load(EYE_TEXTURE_PATH)
	if _skin_texture == null and ResourceLoader.exists(SKIN_TEXTURE_PATH):
		_skin_texture = load(SKIN_TEXTURE_PATH)
		
	for mi in root.find_children("*", "MeshInstance3D", true, false):
		if not mi is MeshInstance3D or not mi.mesh:
			continue
			
		var mi_name = mi.name.to_lower()
		var is_eye = "high-poly" in mi_name or "eye" in mi_name
		
		var mesh: Mesh = mi.mesh
		for s in range(mesh.get_surface_count()):
			if is_eye:
				var eye_mat = StandardMaterial3D.new()
				eye_mat.resource_name = "EyeMaterial"
				if _eye_texture:
					eye_mat.albedo_texture = _eye_texture
				eye_mat.albedo_color = Color(1.0, 1.0, 1.0, 1.0)
				eye_mat.roughness = 0.12
				eye_mat.metallic = 0.0
				eye_mat.metallic_specular = 0.6
				eye_mat.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA_DEPTH_PRE_PASS
				eye_mat.cull_mode = BaseMaterial3D.CULL_BACK
				mi.set_surface_override_material(s, eye_mat)
			else:
				var skin_mat = StandardMaterial3D.new()
				skin_mat.resource_name = "SkinMaterial"
				if _skin_texture:
					skin_mat.albedo_texture = _skin_texture
					skin_mat.albedo_color = Color(1.0, 1.0, 1.0, 1.0)
				else:
					skin_mat.albedo_color = Color(0.91, 0.803, 0.730, 1.0)
				skin_mat.roughness = 0.58
				skin_mat.metallic = 0.0
				skin_mat.diffuse_mode = BaseMaterial3D.DIFFUSE_BURLEY
				skin_mat.cull_mode = BaseMaterial3D.CULL_BACK
				mi.set_surface_override_material(s, skin_mat)
