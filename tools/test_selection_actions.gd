extends SceneTree

const Part = preload("res://src/biology/body_part.gd")

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	var scene = load("res://scenes/biology_showroom.tscn").instantiate()
	root.add_child(scene)
	await process_frame
	await process_frame
	var ui = scene.find_child("UI", true, false)
	var body = scene.find_child("HumanCharacter", true, false).body
	var arm = Part.PartType.LEFT_FOREARM
	var leg = Part.PartType.RIGHT_CALF
	_check(ui.btn_selected_part_actions.disabled, "No actions without selection")
	ui.paperdoll.select_part(arm)
	_check(not ui.btn_selected_part_actions.disabled, "Selection enables actions")
	ui.btn_selected_part_actions.pressed.emit()
	_check(ui._get_action_target_part_id() == arm, "Button targets selected arm")
	_check(ui.lbl_part_selection.text.contains(body.body_parts[arm].display_name), "Target is visible")
	ui.paperdoll.select_part(leg)
	_check(ui._get_action_target_part_id() == leg, "Selection must replace stale menu target")
	ui._on_context_menu_action(6)
	_check(body.body_parts[leg].has_fracture, "Action reaches newly selected leg")
	_check(not body.body_parts[arm].has_fracture, "Previous target is unchanged")
	ui._on_paperdoll_context_menu(arm, Vector2(100, 100))
	_check(ui.active_inspected_part == leg and ui.paperdoll.selected_part_type == leg, "Quick actions preserve inspector selection")
	_check(ui.context_menu.get_item_text(0) == body.body_parts[arm].display_name, "Quick action target is named in the menu")
	ui.paperdoll.select_part(-1)
	_check(ui.btn_selected_part_actions.disabled, "Clearing selection disables actions")
	_check(ui._get_action_target_part_id() == -1, "Clearing selection clears target")
	_check(not ui.context_menu.visible, "Clearing selection closes old menu")
	ui._on_context_menu_action(6)
	_check(not body.body_parts[arm].has_fracture, "No selection cannot affect previous target")
	print("SELECTION_ACTIONS_OK")
	scene.queue_free()
	await process_frame
	quit(0)

func _check(condition: bool, message: String) -> void:
	if not condition:
		push_error(message)
		quit(1)
		assert(condition, message)
