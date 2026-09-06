extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	root.mode = Window.MODE_WINDOWED
	root.content_scale_size = Vector2i.ZERO
	var scene = load("res://scenes/biology_showroom.tscn").instantiate()
	root.add_child(scene)
	var ui = scene.find_child("UI", true, false)
	for resolution in [Vector2i(1920, 1080), Vector2i(1600, 900), Vector2i(1280, 720)]:
		root.size = resolution
		for frame in range(12): await process_frame
		ui.paperdoll.select_part(7)
		ui.option_env_substrate.select(3)
		ui._on_env_substrate_selected(3)
		for frame in range(4): await process_frame
		var dock = ui.find_child("RightSimulationDock", true, false)
		var workspace = ui.find_child("WorkspaceScroll", true, false)
		for panel in [dock, workspace]:
			_check(panel.get_global_rect().end.x <= ui.size.x + 1, "Panel extends past right edge")
			_check(panel.get_global_rect().end.y <= ui.size.y + 1, "Panel extends past bottom edge")
		var sidebar = dock.get_child(0)
		_check(sidebar.get_child(0).size.x <= sidebar.size.x + 1, "Sidebar content overflows horizontally")
		_check(ui.lbl_env_slope_val.get_line_count() == 1, "Numeric slope should remain on one line")
		# Content wider/taller than a small window must be reachable by scrolling.
		for scroll in [workspace, sidebar]:
			scroll.scroll_horizontal = 100000
			scroll.scroll_vertical = 100000
			await process_frame
			var content = scroll.get_child(0)
			_check(content.get_global_rect().end.x <= scroll.get_global_rect().end.x + 1, "Cannot scroll to right edge of content")
			_check(content.get_global_rect().end.y <= scroll.get_global_rect().end.y + 1, "Cannot scroll to bottom of content")
			scroll.scroll_horizontal = 0
			scroll.scroll_vertical = 0
	print("LABORATORY_LAYOUT_OK: 1920x1080, 1600x900, 1280x720")
	scene.queue_free()
	await process_frame
	quit(0)

func _check(condition: bool, message: String) -> void:
	if not condition:
		push_error(message)
		quit(1)
		assert(condition, message)
