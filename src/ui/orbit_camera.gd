class_name OrbitCamera
extends Node3D

@export var target_node: Node3D
@export var distance: float = 2.8
@export var min_distance: float = 1.0
@export var max_distance: float = 6.0
@export var rotation_speed: float = 0.005
@export var zoom_speed: float = 0.3
@export var pitch_limit_up: float = 1.2
@export var pitch_limit_down: float = -0.5

var camera: Camera3D
var yaw: float = 0.0
var pitch: float = 0.15
var is_dragging: bool = false

func _ready() -> void:
	camera = $Camera3D if has_node("Camera3D") else Camera3D.new()
	if not camera.is_inside_tree():
		add_child(camera)
	update_camera_transform()

func _is_mouse_over_ui() -> bool:
	var viewport = get_viewport()
	if not viewport:
		return false
	var hovered: Control = viewport.gui_get_hovered_control()
	if hovered != null and hovered.mouse_filter != Control.MOUSE_FILTER_IGNORE:
		return true
	return false

func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseButton:
		if event.button_index == MOUSE_BUTTON_RIGHT or event.button_index == MOUSE_BUTTON_MIDDLE:
			if event.pressed and _is_mouse_over_ui():
				return
			is_dragging = event.pressed
		elif event.button_index == MOUSE_BUTTON_WHEEL_UP and event.pressed:
			if _is_mouse_over_ui():
				return
			distance = clamp(distance - zoom_speed, min_distance, max_distance)
			update_camera_transform()
		elif event.button_index == MOUSE_BUTTON_WHEEL_DOWN and event.pressed:
			if _is_mouse_over_ui():
				return
			distance = clamp(distance + zoom_speed, min_distance, max_distance)
			update_camera_transform()

	elif event is InputEventMouseMotion and is_dragging:
		yaw -= event.relative.x * rotation_speed
		pitch = clamp(pitch - (event.relative.y * rotation_speed), pitch_limit_down, pitch_limit_up)
		update_camera_transform()

func _process(_delta: float) -> void:
	if target_node:
		global_position = target_node.global_position + Vector3(0, 1.0, 0)
	update_camera_transform()

func update_camera_transform() -> void:
	if not camera:
		return
	var offset: Vector3 = Vector3(
		sin(yaw) * cos(pitch) * distance,
		sin(pitch) * distance,
		cos(yaw) * cos(pitch) * distance
	)
	camera.position = offset
	var dir: Vector3 = global_position - camera.global_position
	if dir.length_squared() > 0.001:
		var norm_dir: Vector3 = dir.normalized()
		var up_vec: Vector3 = Vector3.UP
		if abs(norm_dir.dot(Vector3.UP)) > 0.98:
			up_vec = Vector3.FORWARD
		camera.look_at(global_position, up_vec)
