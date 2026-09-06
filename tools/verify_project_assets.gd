extends SceneTree

# Run after a fresh editor import; catches silent missing-animation fallbacks.
func _initialize() -> void:
	call_deferred("_verify")

func _verify() -> void:
	var scene = load("res://scenes/biology_showroom.tscn") as PackedScene
	if scene == null:
		_fail("Showroom scene missing")
		return
	var showroom = scene.instantiate()
	root.add_child(showroom)
	await process_frame
	await process_frame
	var character = showroom.find_child("HumanCharacter", true, false)
	if character == null or character.current_model == null or character.skeleton == null:
		_fail("Character model or skeleton missing")
		return
	if character.skeleton.get_bone_count() == 0:
		_fail("Skeleton has no bones")
		return
	var expected = ["Idle", "Walking", "Running", "Sprint", "Crouch_Down", "Crouch_Up", "Fall_Flat", "Death_Forward", "Death_Forward_React", "Death_Backward"]
	for animation_name in expected:
		if not character.anim_player.has_animation(animation_name):
			_fail("Missing animation: " + animation_name)
			return
		var animation = character.anim_player.get_animation(animation_name)
		if animation.get_track_count() == 0 or animation.length <= 0.0:
			_fail("Empty animation: " + animation_name)
			return
	var blend_mesh = load("res://char_human/rig/makehuman/makehuman_blend_mesh.res") as ArrayMesh
	if blend_mesh == null or blend_mesh.get_blend_shape_count() < 3:
		_fail("Missing body morphs")
		return
	var textures = ["char_human/rig/makehuman/brown_eye.png", "char_human/rig/makehuman/skin_diffuse.png"]
	for part in ["silhouette_normal", "silhouette_skinny", "silhouette_muscle", "silhouette_fat", "body_muscular", "body_skeletal", "body_nervous", "body_circulatory"]:
		textures.append("assets/ui/paperdoll/" + part + ".jpg")
	for path in textures:
		if load("res://" + path) as Texture2D == null:
			_fail("Missing texture: " + path)
			return
	print("ASSET_CHECK_OK: skeleton, 10 non-empty animations, body morphs and 10 textures")
	showroom.queue_free()
	await process_frame
	quit(0)

func _fail(message: String) -> void:
	push_error("ASSET_CHECK_FAILED: " + message)
	quit(1)
