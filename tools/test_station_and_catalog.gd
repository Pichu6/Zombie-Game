extends SceneTree
const Catalog = preload("res://src/items/item_catalog.gd")
const Visuals = preload("res://src/items/equipment_visuals.gd")
func _initialize() -> void: call_deferred("_run")
func _run() -> void:
	var scene = load("res://scenes/biology_showroom.tscn").instantiate()
	root.add_child(scene)
	for frame in range(5): await process_frame
	var ui = scene.find_child("UI", true, false)
	var character = scene.find_child("HumanCharacter", true, false)
	_check(ui.laboratory_workspace != null, "Analysis station exists")
	_check(ui.laboratory_workspace.pages.get_tab_count() == 3, "Analysis and configuration are separated")
	for id in Catalog.all_ids(): _check(Catalog.validate(id).is_empty(), "Catalog references must resolve: " + id)
	character.body.equip_backpack("backpack_school")
	Visuals.sync(character)
	var attachment = character.skeleton.get_node_or_null("Equipment_backpack_school")
	_check(attachment != null and attachment.get_child_count() == 1, "Backpack actually instantiated")
	_check(attachment.get_child(0).find_children("*", "MeshInstance3D", true, false).size() >= 6, "Backpack has geometry")
	var old_id = attachment.get_instance_id()
	var invalid = Visuals.attach(character, "backpack_school", "res://missing-model.glb")
	_check(not invalid.ok, "Missing model must report failure")
	_check(character.skeleton.get_node("Equipment_backpack_school").get_instance_id() == old_id, "Failure preserves existing model")
	Visuals.sync(character)
	_check(character.skeleton.get_node("Equipment_backpack_school").get_instance_id() == old_id, "Sync does not duplicate models")
	character.body.equipped_backpack_id = ""
	Visuals.sync(character)
	_check(not character.skeleton.has_node("Equipment_backpack_school"), "Unequipping removes visual")
	var paper = ui.paperdoll
	_check(paper.get_part_at_canvas_pos(paper._cached_content_rect.position + Vector2(2,2)) == -1, "Background cannot select anatomy")
	paper.hovered_part_type = 7
	paper.selected_part_type = 7
	character.body.body_parts[7].has_fracture = true
	paper._update_scan_shader()
	_check(paper._base_mat.get_shader_parameter("hover_region") >= 0, "Hover reaches shader")
	var levels = paper._base_mat.get_shader_parameter("damage_levels")
	var positive = 0
	for level in levels:
		if level > 0: positive += 1
	_check(positive == 1, "Injury glow is local, not body-wide")
	var key = InputEventKey.new()
	key.keycode = KEY_X
	key.pressed = true
	paper._unhandled_key_input(key)
	_check(paper.scan_hold, "X enables interior")
	key.pressed = false
	paper._unhandled_key_input(key)
	_check(not paper.scan_hold, "Releasing X restores view")
	key.keycode = KEY_3
	key.alt_pressed = true
	key.pressed = true
	paper._unhandled_key_input(key)
	_check(paper.active_layer_mode == 2, "Alt+3 selects skeletal layer")
	ui.laboratory_workspace._update_report()
	_check(ui.laboratory_workspace.diagnosis.text.contains("Fractura"), "Medical report follows actual injury")
	paper.interior_depth = 0.72
	paper._update_scan_shader()
	_check(is_equal_approx(paper._base_mat.get_shader_parameter("interior_depth"), 0.72), "Visible depth reaches shader")
	paper.heartbeat_phase = 0.0
	character.body.cardio.heart_rate_bpm = 120.0
	paper._process(0.125)
	_check(is_equal_approx(paper.heartbeat_phase, 0.25), "Heartbeat cadence uses actual BPM")
	_check(not paper._layer_bar.visible, "Layers integrated without tab strip")
	_check(ui.laboratory_workspace.pages.get_tab_control(0).is_ancestor_of(ui.btn_posture_sit), "Character actions belong to Analysis")
	print("STATION_AND_CATALOG_OK")
	scene.queue_free()
	await process_frame
	quit(0)
func _check(condition: bool, message: String) -> void:
	if not condition:
		push_error(message)
		quit(1)
		assert(condition, message)
