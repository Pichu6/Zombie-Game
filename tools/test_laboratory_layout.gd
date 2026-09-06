extends SceneTree
func _initialize() -> void: call_deferred("_run")
func _run() -> void:
	root.mode = Window.MODE_WINDOWED
	root.content_scale_size = Vector2i.ZERO
	var scene = load("res://scenes/biology_showroom.tscn").instantiate()
	root.add_child(scene)
	var ui = scene.find_child("UI", true, false)
	var station = ui.laboratory_workspace
	for resolution in [Vector2i(1920,1080), Vector2i(1600,900), Vector2i(1280,720)]:
		root.size = resolution
		for tab in range(station.pages.get_tab_count()):
			station.pages.current_tab = tab
			for frame in range(8): await process_frame
			_check(station.get_global_rect().end.x <= ui.size.x + 1, "Station exceeds window width")
			_check(station.get_global_rect().end.y <= ui.size.y + 1, "Station exceeds window height")
			var page = station.pages.get_current_tab_control()
			for child in page.get_children():
				if child is Control and child.visible:
					_check(child.get_global_rect().end.x <= ui.size.x + 1, "Page column is clipped")
					_check(child.get_global_rect().end.y <= ui.size.y + 1, "Page column exceeds window height")
	print("LABORATORY_LAYOUT_OK: all three workspaces at 1920, 1600 and 1280")
	scene.queue_free()
	await process_frame
	quit(0)
func _check(condition: bool, message: String) -> void:
	if not condition:
		push_error(message)
		quit(1)
		assert(condition, message)
