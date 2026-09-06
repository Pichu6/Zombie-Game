class_name SilhouettePaperdoll
extends VBoxContainer

# =============================================================================
# SILUETA ANATÓMICA MÉDICA HOLOGRÁFICA 3D Y PAPERDOLL MULTICAPA
# Sistema híbrido de última generación:
#   • Switcher de capas prominente con retroalimentación inmediata
#   • SubViewport 3D aislado con MakeHuman y cámara 9:16 calibrada al milímetro
#   • Malla corporal limpia (oculta globos oculares flotantes en el holograma)
#   • Morphing continuo de somatotipos (Skinny, Muscle, Fat) en tiempo acelerado
#   • Capas dinámicas fisiológicas (Vascular limpio, Nervioso, Esquelético)
#   • Polígonos de selección clínica 100% alineados con el cuerpo y las capas
# =============================================================================

const BodyPartClass = preload("res://src/biology/body_part.gd")
const BodySystemClass = preload("res://src/biology/body_system.gd")
const ClothingDatabaseClass = preload("res://src/biology/clothing_database.gd")
const MasterLoader = preload("res://src/entities/character_master_loader.gd")

signal part_selected(part_type: int)
signal part_hovered(part_type: int)
signal backpack_selected()
signal layer_mode_changed(new_mode: int)
signal part_context_menu(part_type: int, global_position: Vector2)

enum LayerMode {
	BASE = 0,
	MUSCULAR = 1,
	SKELETAL = 2,
	CARDIO = 3,
	NERVOUS = 4,
	ALL = 5
}

# Estado de Selección e Interacción Clínica
var scan_hold: bool = false
var interior_depth: float = 0.35
var heartbeat_phase: float = 0.0
var _hover_canvas_position: Vector2 = Vector2.ZERO
var _picking_images: Array[Image] = []
var selected_part_type: int = -1
var hovered_part_type: int = -1
var highlighted_part_types: Array[int] = []

var is_backpack_hovered: bool = false
var is_backpack_selected: bool = false

# Referencias del Motor Fisiológico
var body_ref: BodySystemClass = null
var character_ref = null

# Estado de Capas (Base sólida por defecto con translucidez reactiva)
var active_layer_mode: LayerMode = LayerMode.BASE
var _target_base_alpha: float = 1.0
var _target_muscular_alpha: float = 0.0
var _target_cardio_alpha: float = 0.0
var _target_nervous_alpha: float = 0.0
var _target_skeletal_alpha: float = 0.0
var _target_muscle_weight: float = 0.0

var _current_base_alpha: float = 1.0
var _current_muscular_alpha: float = 0.0
var _current_cardio_alpha: float = 0.0
var _current_nervous_alpha: float = 0.0
var _current_skeletal_alpha: float = 0.0
var _current_muscle_weight: float = 0.0

# Feedback visual — pulso para condiciones activas
var _pulse_time: float = 0.0
var _standing_alert_active: bool = false

# Biometría y Morphing Continuo
var _display_fat: float = 0.18
var _display_muscle: float = 0.45

# Caché de geometrías de las 13 partes anatómicas
var _part_polygons: Dictionary = {}
var _backpack_rect: Rect2 = Rect2()
var _cached_content_rect: Rect2 = Rect2()

# Barra Superior de Selector de Capas
var _layer_bar: HBoxContainer
var _layer_buttons: Dictionary = {}

# Contenedor del Lienzo Anatómico
var _canvas_control: Control
var _viewport_container: SubViewportContainer
var _sub_viewport: SubViewport
var _camera_3d: Camera3D
var _hologram_model: Node3D
var _holo_material: ShaderMaterial

# Nodos de Capas 2D Texturizadas
var _layers_container: Control
var _base_rect: TextureRect
var _base_mat: ShaderMaterial
var _muscular_rect: TextureRect
var _muscular_mat: ShaderMaterial
var _circulatory_rect: TextureRect
var _nervous_rect: TextureRect
var _skeletal_rect: TextureRect

func _init() -> void:
	custom_minimum_size = Vector2(260, 360)
	size_flags_horizontal = Control.SIZE_EXPAND_FILL
	size_flags_vertical = Control.SIZE_EXPAND_FILL
	add_theme_constant_override("separation", 6)

func _ready() -> void:
	_setup_layer_filter_bar()
	_setup_canvas_hierarchy()
	_setup_3d_hologram_viewport()
	_setup_2d_internal_layers()
	_update_layout()
	for file in ["silhouette_normal", "silhouette_fat", "silhouette_skinny", "silhouette_muscle"]:
		var texture: Texture2D = load("res://assets/ui/paperdoll/" + file + ".jpg")
		var img = texture.get_image()
		if img.is_compressed(): img.decompress()
		_picking_images.append(img)
	_base_mat.set_shader_parameter("scan_bones", load("res://assets/ui/paperdoll/body_skeletal.jpg"))
	_base_mat.set_shader_parameter("scan_muscles", load("res://assets/ui/paperdoll/body_muscular.jpg"))
	_base_mat.set_shader_parameter("scan_vessels", load("res://assets/ui/paperdoll/body_circulatory.jpg"))
	_base_mat.set_shader_parameter("scan_nerves", load("res://assets/ui/paperdoll/body_nervous.jpg"))

# -----------------------------------------------------------------------------
# 1. SWITCHER DE CAPAS PROMINENTE (BARRA SUPERIOR AMPLIA Y ESTILIZADA)
# -----------------------------------------------------------------------------
func _setup_layer_filter_bar() -> void:
	if _layer_bar:
		return

	_layer_bar = HBoxContainer.new()
	_layer_bar.custom_minimum_size = Vector2(0, 32)
	_layer_bar.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_layer_bar.add_theme_constant_override("separation", 4)
	add_child(_layer_bar)

	_layer_buttons[LayerMode.BASE] = _create_layer_btn("🧬 Base", LayerMode.BASE, "Silueta Biometría, Equipamiento y Visión Translúcida X-Ray")
	_layer_buttons[LayerMode.MUSCULAR] = _create_layer_btn("💪 Muscular", LayerMode.MUSCULAR, "Sistema Muscular, Fibras y Fatiga")
	_layer_buttons[LayerMode.SKELETAL] = _create_layer_btn("🦴 Huesos", LayerMode.SKELETAL, "Sistema Esquelético y Fracturas")
	_layer_buttons[LayerMode.CARDIO] = _create_layer_btn("🫀 Vascular", LayerMode.CARDIO, "Sistema Cardiovascular y Hemorragias")
	_layer_buttons[LayerMode.NERVOUS] = _create_layer_btn("⚡ Nervios", LayerMode.NERVOUS, "Sistema Nervioso y Dolor Somático")
	_layer_buttons[LayerMode.ALL] = _create_layer_btn("🌐 Todo", LayerMode.ALL, "Vista Compuesta Integral")

	_update_layer_buttons_visual()

func _create_layer_btn(text: String, mode: LayerMode, tooltip: String) -> Button:
	var btn = Button.new()
	btn.text = text
	btn.tooltip_text = tooltip
	btn.focus_mode = Control.FOCUS_NONE
	btn.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	btn.add_theme_font_size_override("font_size", 12)
	btn.pressed.connect(func(): set_layer_mode(mode))
	_layer_bar.add_child(btn)
	return btn

func set_layer_mode(mode: LayerMode) -> void:
	active_layer_mode = mode
	match mode:
		LayerMode.BASE:
			_target_base_alpha = 1.0
			_target_muscular_alpha = 0.0
			_target_skeletal_alpha = 0.0
			_target_cardio_alpha = 0.0
			_target_nervous_alpha = 0.0
			_target_muscle_weight = 0.0
		LayerMode.MUSCULAR:
			_target_base_alpha = 0.20
			_target_muscular_alpha = 1.0
			_target_skeletal_alpha = 0.15
			_target_cardio_alpha = 0.0
			_target_nervous_alpha = 0.0
			_target_muscle_weight = 0.0
		LayerMode.SKELETAL:
			_target_base_alpha = 0.20
			_target_muscular_alpha = 0.0
			_target_skeletal_alpha = 1.0
			_target_cardio_alpha = 0.0
			_target_nervous_alpha = 0.0
			_target_muscle_weight = 0.0
		LayerMode.CARDIO:
			_target_base_alpha = 0.20
			_target_muscular_alpha = 0.0
			_target_skeletal_alpha = 0.0
			_target_cardio_alpha = 1.0
			_target_nervous_alpha = 0.0
			_target_muscle_weight = 0.0
		LayerMode.NERVOUS:
			_target_base_alpha = 0.20
			_target_muscular_alpha = 0.0
			_target_skeletal_alpha = 0.0
			_target_cardio_alpha = 0.0
			_target_nervous_alpha = 1.0
			_target_muscle_weight = 0.0
		LayerMode.ALL:
			_target_base_alpha = 0.50
			_target_muscular_alpha = 0.60
			_target_skeletal_alpha = 0.60
			_target_cardio_alpha = 0.70
			_target_nervous_alpha = 0.50
			_target_muscle_weight = 0.0

	_update_layer_buttons_visual()
	layer_mode_changed.emit(mode)
	if _canvas_control:
		_canvas_control.queue_redraw()

func _update_layer_buttons_visual() -> void:
	for mode in _layer_buttons.keys():
		var btn: Button = _layer_buttons[mode]
		if mode == active_layer_mode:
			btn.modulate = Color(0.2, 1.0, 0.85, 1.0)
		else:
			btn.modulate = Color(0.7, 0.75, 0.8, 0.65)

# -----------------------------------------------------------------------------
# 2. JERARQUÍA DEL LIENZO Y ALINEACIÓN EXACTA 9:16
# -----------------------------------------------------------------------------
func _setup_canvas_hierarchy() -> void:
	_canvas_control = Control.new()
	_canvas_control.custom_minimum_size = Vector2(240, 320)
	_canvas_control.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_canvas_control.size_flags_vertical = Control.SIZE_EXPAND_FILL
	_canvas_control.mouse_filter = Control.MOUSE_FILTER_STOP
	_canvas_control.draw.connect(_on_canvas_draw)
	_canvas_control.gui_input.connect(_on_canvas_gui_input)
	_canvas_control.resized.connect(_update_layout)
	_canvas_control.mouse_exited.connect(func():
		hovered_part_type = -1
		part_hovered.emit(-1)
	)
	add_child(_canvas_control)

func _get_content_rect() -> Rect2:
	if not _canvas_control:
		return Rect2()
	var c_size = _canvas_control.size
	if c_size.x <= 10 or c_size.y <= 10:
		return Rect2(Vector2.ZERO, Vector2(240, 426))

	var target_aspect = 9.0 / 16.0 # Relación de aspecto anatómica exacta
	var cur_aspect = c_size.x / c_size.y

	if cur_aspect > target_aspect:
		# Más ancho: centrar horizontalmente
		var w = c_size.y * target_aspect
		var x = (c_size.x - w) * 0.5
		return Rect2(x, 0.0, w, c_size.y)
	else:
		# Más alto: centrar verticalmente
		var h = c_size.x / target_aspect
		var y = (c_size.y - h) * 0.5
		return Rect2(0.0, y, c_size.x, h)

func _update_layout() -> void:
	if not _canvas_control:
		return
	_cached_content_rect = _get_content_rect()

	if _viewport_container:
		_viewport_container.position = _cached_content_rect.position
		_viewport_container.size = _cached_content_rect.size

	if _layers_container:
		_layers_container.position = _cached_content_rect.position
		_layers_container.size = _cached_content_rect.size

	_build_base_geometry()
	if _canvas_control:
		_canvas_control.queue_redraw()

# -----------------------------------------------------------------------------
# 3. CONFIGURACIÓN DEL SUBVIEWPORT 3D HOLOGRÁFICO (CALIBRADO Y LIMPIO)
# -----------------------------------------------------------------------------
func _setup_3d_hologram_viewport() -> void:
	if _viewport_container:
		return

	_viewport_container = SubViewportContainer.new()
	_viewport_container.stretch = true
	_viewport_container.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_viewport_container.visible = false # Desactivado en paperdoll 2D para eliminar el glitch de 4 brazos y desalineación
	_canvas_control.add_child(_viewport_container)

	_sub_viewport = SubViewport.new()
	_sub_viewport.own_world_3d = true
	_sub_viewport.transparent_bg = true
	_sub_viewport.handle_input_locally = false
	_sub_viewport.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	_sub_viewport.size = Vector2i(540, 960) # Exacto 9:16
	_viewport_container.add_child(_sub_viewport)

	var env = Environment.new()
	env.background_mode = Environment.BG_COLOR
	env.background_color = Color(0, 0, 0, 0)
	env.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
	env.ambient_light_color = Color(0.10, 0.16, 0.22, 1.0)
	var we = WorldEnvironment.new()
	we.environment = env
	_sub_viewport.add_child(we)

	# Cámara ortográfica frontal perfectamente centrada en el cuerpo (1.756m)
	_camera_3d = Camera3D.new()
	_camera_3d.projection = Camera3D.PROJECTION_ORTHOGONAL
	_camera_3d.size = 2.02
	_camera_3d.look_at_from_position(Vector3(0, 0.88, 2.2), Vector3(0, 0.88, 0), Vector3.UP)
	_sub_viewport.add_child(_camera_3d)

	var light = DirectionalLight3D.new()
	light.position = Vector3(0, 2.5, 2.5)
	light.rotation_degrees = Vector3(-35, 0, 0)
	light.light_color = Color(0.6, 0.9, 1.0)
	light.light_energy = 1.6
	_sub_viewport.add_child(light)

	_hologram_model = MasterLoader.load_character_model()
	if _hologram_model:
		_hologram_model.name = "HologramCharacter"
		_sub_viewport.add_child(_hologram_model)
		_apply_hologram_materials(_hologram_model)

func _apply_hologram_materials(model: Node3D) -> void:
	var holo_shader = load("res://src/ui/shaders/medical_hologram.gdshader")
	if not holo_shader:
		return

	_holo_material = ShaderMaterial.new()
	_holo_material.shader = holo_shader
	_holo_material.set_shader_parameter("base_color", Color(0.02, 0.20, 0.30, 0.40))
	_holo_material.set_shader_parameter("rim_color", Color(0.20, 0.80, 1.0, 0.95))
	_holo_material.set_shader_parameter("rim_power", 2.2)
	_holo_material.set_shader_parameter("scanline_frequency", 90.0)
	_holo_material.set_shader_parameter("scanline_speed", 1.2)
	_holo_material.set_shader_parameter("glow_intensity", 1.5)

	for mi in model.find_children("*", "MeshInstance3D", true, false):
		if mi is MeshInstance3D and mi.mesh:
			var mname = mi.name.to_lower()
			# Ocultar globos oculares, pestañas y dientes para un holograma médico limpio
			if "eye" in mname or "high-poly" in mname or "cornea" in mname or "teeth" in mname or "tongue" in mname or "lash" in mname or "tear" in mname or "occlusion" in mname:
				mi.visible = false
			else:
				mi.visible = true
				for s in range(mi.mesh.get_surface_count()):
					mi.set_surface_override_material(s, _holo_material)

# -----------------------------------------------------------------------------
# 4. CONFIGURACIÓN DE LAS CAPAS FISIOLÓGICAS 2D Y MULTI-SOMATOTIPO
# -----------------------------------------------------------------------------
func _setup_2d_internal_layers() -> void:
	if _layers_container:
		return

	_layers_container = Control.new()
	_layers_container.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_canvas_control.add_child(_layers_container)

	# 0. Capa Base Silueta Anatómica Multi-Somatotipo (Normal, Flaco, Músculo, Grasa)
	var norm_tex = load("res://assets/ui/paperdoll/silhouette_normal.jpg")
	var blend_shader = load("res://src/ui/shaders/silhouette_blend.gdshader")
	if norm_tex and blend_shader:
		_base_rect = TextureRect.new()
		_base_rect.name = "SilhouetteBase"
		_base_rect.texture = norm_tex
		_base_rect.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
		_base_rect.stretch_mode = TextureRect.STRETCH_SCALE
		_base_rect.set_anchors_preset(PRESET_FULL_RECT)
		_base_rect.mouse_filter = Control.MOUSE_FILTER_IGNORE

		_base_mat = ShaderMaterial.new()
		_base_mat.shader = blend_shader
		_base_mat.set_shader_parameter("tex_skinny", load("res://assets/ui/paperdoll/silhouette_skinny.jpg"))
		_base_mat.set_shader_parameter("tex_muscle", load("res://assets/ui/paperdoll/silhouette_muscle.jpg"))
		_base_mat.set_shader_parameter("tex_fat", load("res://assets/ui/paperdoll/silhouette_fat.jpg"))
		_base_mat.set_shader_parameter("weight_skinny", 0.0)
		_base_mat.set_shader_parameter("weight_muscle", 0.0)
		_base_mat.set_shader_parameter("weight_fat", 0.0)
		_base_mat.set_shader_parameter("overall_alpha", _target_base_alpha)
		_base_rect.material = _base_mat
		_layers_container.add_child(_base_rect)

	# 1. Capa Muscular Anatómica Interna (Músculos y Fibras)
	var musc_tex = load("res://assets/ui/paperdoll/body_muscular.jpg")
	var musc_shader = load("res://src/ui/shaders/muscular_tension.gdshader")
	if musc_tex and musc_shader:
		_muscular_rect = TextureRect.new()
		_muscular_rect.name = "SilhouetteMuscular"
		_muscular_rect.texture = musc_tex
		_muscular_rect.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
		_muscular_rect.stretch_mode = TextureRect.STRETCH_SCALE
		_muscular_rect.set_anchors_preset(PRESET_FULL_RECT)
		_muscular_rect.mouse_filter = Control.MOUSE_FILTER_IGNORE

		_muscular_mat = ShaderMaterial.new()
		_muscular_mat.shader = musc_shader
		_muscular_mat.set_shader_parameter("overall_alpha", _target_muscular_alpha)
		_muscular_rect.material = _muscular_mat
		_layers_container.add_child(_muscular_rect)

	# 2. Capa Esquelética
	var skel_tex = load("res://assets/ui/paperdoll/body_skeletal.jpg")
	var skel_shader = load("res://src/ui/shaders/skeletal_trauma.gdshader")
	if skel_tex and skel_shader:
		_skeletal_rect = TextureRect.new()
		_skeletal_rect.texture = skel_tex
		_skeletal_rect.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
		_skeletal_rect.stretch_mode = TextureRect.STRETCH_SCALE
		_skeletal_rect.set_anchors_preset(PRESET_FULL_RECT)
		_skeletal_rect.mouse_filter = Control.MOUSE_FILTER_IGNORE
		
		var skel_mat = ShaderMaterial.new()
		skel_mat.shader = skel_shader
		skel_mat.set_shader_parameter("overall_alpha", _target_skeletal_alpha)
		_skeletal_rect.material = skel_mat
		_layers_container.add_child(_skeletal_rect)

	# 2. Capa Sistema Nervioso
	var nerv_tex = load("res://assets/ui/paperdoll/body_nervous.jpg")
	var nerv_shader = load("res://src/ui/shaders/nervous_pain.gdshader")
	if nerv_tex and nerv_shader:
		_nervous_rect = TextureRect.new()
		_nervous_rect.texture = nerv_tex
		_nervous_rect.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
		_nervous_rect.stretch_mode = TextureRect.STRETCH_SCALE
		_nervous_rect.set_anchors_preset(PRESET_FULL_RECT)
		_nervous_rect.mouse_filter = Control.MOUSE_FILTER_IGNORE
		
		var nerv_mat = ShaderMaterial.new()
		nerv_mat.shader = nerv_shader
		nerv_mat.set_shader_parameter("overall_alpha", _target_nervous_alpha)
		_nervous_rect.material = nerv_mat
		_layers_container.add_child(_nervous_rect)

	# 3. Capa Circulatoria / Vascular (Limpia, sin UI)
	var cardio_tex = load("res://assets/ui/paperdoll/body_circulatory.jpg")
	var cardio_shader = load("res://src/ui/shaders/cardio_pulse.gdshader")
	if cardio_tex and cardio_shader:
		_circulatory_rect = TextureRect.new()
		_circulatory_rect.texture = cardio_tex
		_circulatory_rect.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
		_circulatory_rect.stretch_mode = TextureRect.STRETCH_SCALE
		_circulatory_rect.set_anchors_preset(PRESET_FULL_RECT)
		_circulatory_rect.mouse_filter = Control.MOUSE_FILTER_IGNORE
		
		var cardio_mat = ShaderMaterial.new()
		cardio_mat.shader = cardio_shader
		cardio_mat.set_shader_parameter("overall_alpha", _target_cardio_alpha)
		_circulatory_rect.material = cardio_mat
		_layers_container.add_child(_circulatory_rect)

# -----------------------------------------------------------------------------
# 5. ACTUALIZACIÓN CONTINUA: MORPHING Y TELEMETRÍA DE SHADERS
# -----------------------------------------------------------------------------
func _process(delta: float) -> void:
	_pulse_time += delta
	if body_ref and body_ref.cardio:
		heartbeat_phase = fposmod(heartbeat_phase + delta * body_ref.cardio.heart_rate_bpm / 60.0, 1.0)
	_update_scan_shader()
	
	# Detectar alerta de pie prolongado
	if body_ref:
		if "continuous_standing_seconds" in body_ref:
			_standing_alert_active = body_ref.continuous_standing_seconds > 2700.0 # 45 min
		
	# 1. Transición suave de capas
	_current_base_alpha = lerp(_current_base_alpha, _target_base_alpha, delta * 7.0)
	_current_muscular_alpha = lerp(_current_muscular_alpha, _target_muscular_alpha, delta * 7.0)
	_current_cardio_alpha = lerp(_current_cardio_alpha, _target_cardio_alpha, delta * 7.0)
	_current_nervous_alpha = lerp(_current_nervous_alpha, _target_nervous_alpha, delta * 7.0)
	_current_skeletal_alpha = lerp(_current_skeletal_alpha, _target_skeletal_alpha, delta * 7.0)
	_current_muscle_weight = lerp(_current_muscle_weight, _target_muscle_weight, delta * 7.0)

	# 2. Somatotipo y morphing continuo (2D + 3D)
	if body_ref:
		var target_fat: float = body_ref.body_fat_pct
		var target_muscle: float = body_ref.muscle_mass_pct
		_display_fat = lerp(_display_fat, target_fat, delta * 8.0)
		_display_muscle = lerp(_display_muscle, target_muscle, delta * 8.0)

	# Pesos de mezcla somatotípica suave sin saltos
	var w_fat = clampf((_display_fat - 0.18) / 0.22, 0.0, 1.0)
	var w_skinny = clampf((0.18 - _display_fat) / 0.12, 0.0, 1.0)
	var w_muscle = clampf((_display_muscle - 0.45) / 0.25, 0.0, 1.0)
	if _current_muscle_weight > 0.01:
		w_muscle = max(w_muscle, _current_muscle_weight)

	if _base_mat:
		_base_mat.set_shader_parameter("weight_fat", w_fat)
		_base_mat.set_shader_parameter("weight_skinny", w_skinny)
		_base_mat.set_shader_parameter("weight_muscle", w_muscle)
		_base_mat.set_shader_parameter("overall_alpha", _current_base_alpha)

	if _muscular_rect and _muscular_rect.material:
		_muscular_rect.material.set_shader_parameter("overall_alpha", _current_muscular_alpha)
	if _circulatory_rect and _circulatory_rect.material:
		_circulatory_rect.material.set_shader_parameter("overall_alpha", _current_cardio_alpha)
	if _nervous_rect and _nervous_rect.material:
		_nervous_rect.material.set_shader_parameter("overall_alpha", _current_nervous_alpha)
	if _skeletal_rect and _skeletal_rect.material:
		_skeletal_rect.material.set_shader_parameter("overall_alpha", _current_skeletal_alpha)

	# Morphing biométrico del modelo 3D (cuando esté activo)
	if _hologram_model:
		MasterLoader.apply_morph_weights(_hologram_model, _display_fat, _display_muscle)

	# 3. Telemetría fisiológica y translucidez reactiva
	if body_ref:
		var bpm: float = body_ref.cardio.heart_rate_bpm if body_ref.cardio else 75.0
		var blood_ml: float = body_ref.cardio.total_blood_ml if body_ref.cardio else 5000.0
		var max_blood: float = body_ref.cardio.max_blood_ml if body_ref.cardio else 5000.0
		var blood_pct: float = clamp(blood_ml / max(1.0, max_blood), 0.0, 1.0)
		var pain: float = body_ref.total_pain

		var has_active_bleed: bool = false
		var has_fracture: bool = false
		var has_tourniquet: bool = false
		for part in body_ref.body_parts.values():
			if part.get_current_bleed_rate(bpm) > 0.0:
				has_active_bleed = true
			if part.has_fracture:
				has_fracture = true
			if part.is_tourniquet_applied:
				has_tourniquet = true

		if active_layer_mode == LayerMode.BASE:
			_target_base_alpha = 1.0
			_target_skeletal_alpha = 0.0
			_target_cardio_alpha = 0.0
			_target_nervous_alpha = 0.0
			_target_muscular_alpha = 0.0
		if _muscular_rect and _muscular_rect.material:
			_muscular_rect.material.set_shader_parameter("muscle_pain", clamp(pain, 0.0, 100.0))
			var exertion: float = 0.0
			if character_ref and "movement_exertion" in character_ref:
				exertion = character_ref.movement_exertion
			_muscular_rect.material.set_shader_parameter("muscle_fatigue", clamp(exertion, 0.0, 1.0))

		if _circulatory_rect and _circulatory_rect.material:
			_circulatory_rect.material.set_shader_parameter("heart_rate_bpm", bpm)
			_circulatory_rect.material.set_shader_parameter("blood_volume_pct", blood_pct)
			_circulatory_rect.material.set_shader_parameter("bleed_intensity", 1.0 if has_active_bleed else 0.0)

		if _nervous_rect and _nervous_rect.material:
			_nervous_rect.material.set_shader_parameter("pain_level", clamp(pain, 0.0, 100.0))

		if _skeletal_rect and _skeletal_rect.material:
			_skeletal_rect.material.set_shader_parameter("fracture_intensity", 1.0 if has_fracture else 0.0)

# -----------------------------------------------------------------------------
# 6. DEFINICIÓN VECTORIAL CALIBRADA DE LAS 17 ZONAS CLÍNICAS (3 SEGMENTOS/MIEMBRO)
# -----------------------------------------------------------------------------
func _build_base_geometry() -> void:
	var cr = _cached_content_rect
	if cr.size.x <= 10 or cr.size.y <= 10:
		return

	var ox = cr.position.x
	var oy = cr.position.y
	var w = cr.size.x
	var h = cr.size.y
	var cx = ox + w * 0.5

	_part_polygons.clear()

	# 1. Cabeza (HEAD)
	_part_polygons[BodyPartClass.PartType.HEAD] = PackedVector2Array([
		Vector2(cx + w * +0.0560, oy + h * 0.1642),
		Vector2(cx + w * -0.0586, oy + h * 0.1635),
		Vector2(cx + w * -0.0794, oy + h * 0.1148),
		Vector2(cx + w * -0.0677, oy + h * 0.0778),
		Vector2(cx + w * -0.0326, oy + h * 0.0574),
		Vector2(cx + w * +0.0208, oy + h * 0.0552),
		Vector2(cx + w * +0.0651, oy + h * 0.0756),
		Vector2(cx + w * +0.0781, oy + h * 0.1192),
	])

	# 2. Cuello (NECK)
	_part_polygons[BodyPartClass.PartType.NECK] = PackedVector2Array([
		Vector2(cx + w * +0.0781, oy + h * 0.2086),
		Vector2(cx + w * -0.0807, oy + h * 0.2086),
		Vector2(cx + w * -0.0807, oy + h * 0.1868),
		Vector2(cx + w * -0.0573, oy + h * 0.1650),
		Vector2(cx + w * +0.0560, oy + h * 0.1650),
		Vector2(cx + w * +0.0781, oy + h * 0.1860),
	])

	# 3. Torso Superior / Tórax (TORSO_UPPER)
	_part_polygons[BodyPartClass.PartType.TORSO_UPPER] = PackedVector2Array([
		Vector2(cx + w * +0.1302, oy + h * 0.3539),
		Vector2(cx + w * -0.1302, oy + h * 0.3539),
		Vector2(cx + w * -0.1602, oy + h * 0.3198),
		Vector2(cx + w * -0.1602, oy + h * 0.2093),
		Vector2(cx + w * +0.1576, oy + h * 0.2093),
		Vector2(cx + w * +0.1576, oy + h * 0.3161),
	])

	# 4. Torso Inferior / Abdomen (TORSO_LOWER)
	_part_polygons[BodyPartClass.PartType.TORSO_LOWER] = PackedVector2Array([
		Vector2(cx + w * +0.1471, oy + h * 0.4491),
		Vector2(cx + w * -0.1484, oy + h * 0.4491),
		Vector2(cx + w * -0.1302, oy + h * 0.3547),
		Vector2(cx + w * +0.1302, oy + h * 0.3547),
	])

	# 5. Pelvis / Ingle (GROIN)
	_part_polygons[BodyPartClass.PartType.GROIN] = PackedVector2Array([
		Vector2(cx + w * +0.1484, oy + h * 0.5291),
		Vector2(cx + w * -0.1510, oy + h * 0.5291),
		Vector2(cx + w * -0.1484, oy + h * 0.4513),
		Vector2(cx + w * +0.1471, oy + h * 0.4499),
	])

	# 6. Brazo Superior Derecho (RIGHT_ARM: Hombro a Codo)
	_part_polygons[BodyPartClass.PartType.RIGHT_ARM] = PackedVector2Array([
		Vector2(cx + w * -0.2982, oy + h * 0.3641),
		Vector2(cx + w * -0.2070, oy + h * 0.2275),
		Vector2(cx + w * -0.1615, oy + h * 0.2057),
		Vector2(cx + w * -0.1615, oy + h * 0.3212),
		Vector2(cx + w * -0.2031, oy + h * 0.3641),
	])

	# 7. Brazo Superior Izquierdo (LEFT_ARM: Hombro a Codo)
	_part_polygons[BodyPartClass.PartType.LEFT_ARM] = PackedVector2Array([
		Vector2(cx + w * +0.2969, oy + h * 0.3641),
		Vector2(cx + w * +0.1992, oy + h * 0.3641),
		Vector2(cx + w * +0.1667, oy + h * 0.3270),
		Vector2(cx + w * +0.1589, oy + h * 0.2049),
		Vector2(cx + w * +0.2109, oy + h * 0.2340),
	])

	# 8. Antebrazo Derecho (RIGHT_FOREARM: Codo a Muñeca)
	_part_polygons[BodyPartClass.PartType.RIGHT_FOREARM] = PackedVector2Array([
		Vector2(cx + w * -0.2031, oy + h * 0.3648),
		Vector2(cx + w * -0.3190, oy + h * 0.4840),
		Vector2(cx + w * -0.4219, oy + h * 0.4840),
		Vector2(cx + w * -0.3008, oy + h * 0.3648),
	])

	# 9. Antebrazo Izquierdo (LEFT_FOREARM: Codo a Muñeca)
	_part_polygons[BodyPartClass.PartType.LEFT_FOREARM] = PackedVector2Array([
		Vector2(cx + w * +0.4206, oy + h * 0.4840),
		Vector2(cx + w * +0.3177, oy + h * 0.4840),
		Vector2(cx + w * +0.1992, oy + h * 0.3648),
		Vector2(cx + w * +0.2982, oy + h * 0.3648),
	])

	# 10. Mano Derecha (RIGHT_HAND: Muñeca a Dedos)
	_part_polygons[BodyPartClass.PartType.RIGHT_HAND] = PackedVector2Array([
		Vector2(cx + w * -0.3190, oy + h * 0.4847),
		Vector2(cx + w * -0.3503, oy + h * 0.5356),
		Vector2(cx + w * -0.3841, oy + h * 0.5451),
		Vector2(cx + w * -0.4154, oy + h * 0.5443),
		Vector2(cx + w * -0.4349, oy + h * 0.5305),
		Vector2(cx + w * -0.4310, oy + h * 0.4906),
		Vector2(cx + w * -0.4232, oy + h * 0.4855),
	])

	# 11. Mano Izquierda (LEFT_HAND: Muñeca a Dedos)
	_part_polygons[BodyPartClass.PartType.LEFT_HAND] = PackedVector2Array([
		Vector2(cx + w * +0.4141, oy + h * 0.5443),
		Vector2(cx + w * +0.3828, oy + h * 0.5451),
		Vector2(cx + w * +0.3490, oy + h * 0.5356),
		Vector2(cx + w * +0.3190, oy + h * 0.4847),
		Vector2(cx + w * +0.4219, oy + h * 0.4847),
		Vector2(cx + w * +0.4310, oy + h * 0.4913),
		Vector2(cx + w * +0.4349, oy + h * 0.5291),
	])

	# 12. Muslo Derecho (RIGHT_LEG: Cadera a Rodilla)
	_part_polygons[BodyPartClass.PartType.RIGHT_LEG] = PackedVector2Array([
		Vector2(cx + w * -0.0013, oy + h * 0.5247),
		Vector2(cx + w * -0.0612, oy + h * 0.6991),
		Vector2(cx + w * -0.1562, oy + h * 0.6991),
		Vector2(cx + w * -0.1680, oy + h * 0.5262),
	])

	# 13. Muslo Izquierdo (LEFT_LEG: Cadera a Rodilla)
	_part_polygons[BodyPartClass.PartType.LEFT_LEG] = PackedVector2Array([
		Vector2(cx + w * +0.1654, oy + h * 0.5247),
		Vector2(cx + w * +0.1549, oy + h * 0.6991),
		Vector2(cx + w * +0.0612, oy + h * 0.6991),
		Vector2(cx + w * +0.0000, oy + h * 0.5247),
	])

	# 14. Pantorrilla Derecha (RIGHT_CALF: Rodilla a Tobillo)
	_part_polygons[BodyPartClass.PartType.RIGHT_CALF] = PackedVector2Array([
		Vector2(cx + w * -0.0625, oy + h * 0.6999),
		Vector2(cx + w * -0.1016, oy + h * 0.8765),
		Vector2(cx + w * -0.1641, oy + h * 0.8786),
		Vector2(cx + w * -0.1562, oy + h * 0.6999),
	])

	# 15. Pantorrilla Izquierda (LEFT_CALF: Rodilla a Tobillo)
	_part_polygons[BodyPartClass.PartType.LEFT_CALF] = PackedVector2Array([
		Vector2(cx + w * +0.1549, oy + h * 0.6999),
		Vector2(cx + w * +0.1732, oy + h * 0.7609),
		Vector2(cx + w * +0.1641, oy + h * 0.8757),
		Vector2(cx + w * +0.1003, oy + h * 0.8786),
		Vector2(cx + w * +0.0612, oy + h * 0.6999),
	])

	# 16. Pie Derecho (RIGHT_FOOT: Tobillo a Dedos)
	_part_polygons[BodyPartClass.PartType.RIGHT_FOOT] = PackedVector2Array([
		Vector2(cx + w * -0.1029, oy + h * 0.8794),
		Vector2(cx + w * -0.1029, oy + h * 0.9142),
		Vector2(cx + w * -0.1185, oy + h * 0.9317),
		Vector2(cx + w * -0.1471, oy + h * 0.9433),
		Vector2(cx + w * -0.2005, oy + h * 0.9368),
		Vector2(cx + w * -0.2005, oy + h * 0.9244),
		Vector2(cx + w * -0.1641, oy + h * 0.8794),
	])

	# 17. Pie Izquierdo (LEFT_FOOT: Tobillo a Dedos)
	_part_polygons[BodyPartClass.PartType.LEFT_FOOT] = PackedVector2Array([
		Vector2(cx + w * +0.1966, oy + h * 0.9375),
		Vector2(cx + w * +0.1510, oy + h * 0.9433),
		Vector2(cx + w * +0.1302, oy + h * 0.9397),
		Vector2(cx + w * +0.1016, oy + h * 0.9135),
		Vector2(cx + w * +0.1003, oy + h * 0.8794),
		Vector2(cx + w * +0.1628, oy + h * 0.8794),
		Vector2(cx + w * +0.1979, oy + h * 0.9230),
	])

	# Insignia de Mochila en la Esquina Superior Derecha
	var bp_w = 84.0
	var bp_h = 38.0
	_backpack_rect = Rect2(_canvas_control.size.x - bp_w - 8.0, 8.0, bp_w, bp_h)

func _create_ellipse_poly(center: Vector2, rx: float, ry: float, segments: int) -> PackedVector2Array:
	var poly = PackedVector2Array()
	for i in range(segments):
		var angle = (float(i) / float(segments)) * TAU
		poly.append(center + Vector2(cos(angle) * rx, sin(angle) * ry))
	return poly

# -----------------------------------------------------------------------------
# 7. DIBUJADO DE RESALTADOS CLÍNICOS E INTERACCIÓN
# -----------------------------------------------------------------------------
func _on_canvas_draw() -> void:
	if not _canvas_control:
		return

	# Selection and trauma are rendered inside the silhouette shader, not as boxes.
	_draw_pinpoint_clinical_markers()
	_draw_equipment_overlays()
	_draw_backpack_badge()
	_draw_scan_callout()

func _get_polygon_centroid(poly: PackedVector2Array) -> Vector2:
	if poly.is_empty():
		return Vector2.ZERO
	var sum = Vector2.ZERO
	for pt in poly:
		sum += pt
	return sum / float(poly.size())

func _draw_pinpoint_clinical_markers() -> void:
	if not body_ref or not _canvas_control:
		return

	var bpm = body_ref.cardio.heart_rate_bpm if body_ref.cardio else 75.0

	for part_type in _part_polygons.keys():
		var poly: PackedVector2Array = _part_polygons[part_type]
		var centroid = _get_polygon_centroid(poly)
		var part: BodyPartClass = body_ref.body_parts.get(part_type)
		if not part or part.is_severed:
			continue

		var bleed = part.get_current_bleed_rate(bpm)

		# 1. HEMORRAGIA ACTIVA: Ondas concéntricas de pulso vascular y foco brillante
		if bleed > 0.0:
			var speed = 24.0 if bleed > 2.0 else 16.0
			var max_r = 20.0 if bleed > 2.0 else 14.0
			var ring_r = fposmod(_pulse_time * speed, max_r) + 3.0
			var ring_alpha = clampf(1.0 - (ring_r / max_r), 0.15, 0.9)
			_canvas_control.draw_arc(centroid, ring_r, 0.0, TAU, 24, Color(1.0, 0.15, 0.15, ring_alpha * 0.9), 2.0)

			var ring_r2 = fposmod(_pulse_time * speed + (max_r * 0.5), max_r) + 3.0
			var ring_alpha2 = clampf(1.0 - (ring_r2 / max_r), 0.15, 0.9)
			_canvas_control.draw_arc(centroid, ring_r2, 0.0, TAU, 24, Color(1.0, 0.35, 0.15, ring_alpha2 * 0.7), 1.5)

			_canvas_control.draw_circle(centroid, 5.0, Color(0.95, 0.05, 0.05, 0.95))
			_canvas_control.draw_circle(centroid, 2.5, Color(1.0, 0.85, 0.85, 1.0))

		# 2. INFECCIÓN / SEPSIS: Aura tóxica verdosa
		if part.infection_level > 5.0:
			var inf_pulse = (sin(_pulse_time * 4.0) + 1.0) * 0.5
			var inf_r = 10.0 + inf_pulse * 4.0
			_canvas_control.draw_circle(centroid, inf_r, Color(0.65, 0.95, 0.15, 0.28 * (part.infection_level / 100.0)))
			_canvas_control.draw_arc(centroid, inf_r, 0.0, TAU, 16, Color(0.75, 1.0, 0.2, 0.65), 1.2)

		# 3. FRACTURA ÓSEA: Grieta angular en zigzag
		if part.has_fracture and not part.is_splinted:
			var frac_col = Color(1.0, 0.92, 0.15, 0.95)
			var crack = PackedVector2Array([
				centroid + Vector2(-6, -7),
				centroid + Vector2(-1, -1),
				centroid + Vector2(4, 2),
				centroid + Vector2(-2, 8)
			])
			_canvas_control.draw_polyline(crack, frac_col, 2.5)
			_canvas_control.draw_circle(centroid, 2.0, Color(1.0, 1.0, 0.8))

		# 4. OVERLAY CLÍNICO: VENDAJE MÉDICO COMPRESIVO (Parche textil con textura estéril y cruz roja)
		if part.is_bandaged:
			var bw = 24.0
			var bh = 14.0
			var b_rect = Rect2(centroid.x - bw * 0.5, centroid.y - bh * 0.5, bw, bh)
			_canvas_control.draw_rect(b_rect, Color(0.94, 0.96, 0.98, 0.95), true)
			_canvas_control.draw_rect(b_rect, Color(0.50, 0.70, 0.90, 0.95), false, 1.2)
			_canvas_control.draw_line(b_rect.position + Vector2(4, 0), b_rect.position + Vector2(0, 4), Color(0.7, 0.8, 0.9, 0.7), 1.0)
			_canvas_control.draw_line(b_rect.position + Vector2(12, 0), b_rect.position + Vector2(0, 12), Color(0.7, 0.8, 0.9, 0.7), 1.0)
			_canvas_control.draw_line(b_rect.position + Vector2(20, 0), b_rect.position + Vector2(6, 14), Color(0.7, 0.8, 0.9, 0.7), 1.0)
			var cross_col = Color(0.85, 0.15, 0.15, 0.95)
			_canvas_control.draw_line(centroid + Vector2(-4, 0), centroid + Vector2(4, 0), cross_col, 2.0)
			_canvas_control.draw_line(centroid + Vector2(0, -4), centroid + Vector2(0, 4), cross_col, 2.0)

		# 5. OVERLAY CLÍNICO: TORNIQUETE TÁCTICO DE COMPRESIÓN
		if part.is_tourniquet_applied:
			var t_pos = centroid + Vector2(0, -8)
			var tw = 24.0
			var th = 7.0
			var t_rect = Rect2(t_pos.x - tw * 0.5, t_pos.y - th * 0.5, tw, th)
			_canvas_control.draw_rect(t_rect, Color(0.10, 0.12, 0.16, 0.95), true)
			_canvas_control.draw_rect(t_rect, Color(0.35, 0.60, 0.95, 0.9), false, 1.2)
			_canvas_control.draw_rect(Rect2(t_pos.x - 3, t_pos.y - 3, 6, 6), Color(0.85, 0.90, 0.95, 0.95), true)
			_canvas_control.draw_line(t_pos + Vector2(-6, -4), t_pos + Vector2(6, 4), Color(1.0, 0.55, 0.0, 1.0), 2.4)

		# 6. OVERLAY CLÍNICO: FÉRULA RÍGIDA / ENTABLILLADO
		if part.is_splinted:
			var sw = 18.0
			var sh = 28.0
			var s_rect = Rect2(centroid.x - sw * 0.5, centroid.y - sh * 0.5, sw, sh)
			_canvas_control.draw_line(Vector2(s_rect.position.x + 2, s_rect.position.y), Vector2(s_rect.position.x + 2, s_rect.end.y), Color(0.75, 0.58, 0.40, 0.95), 3.0)
			_canvas_control.draw_line(Vector2(s_rect.end.x - 2, s_rect.position.y), Vector2(s_rect.end.x - 2, s_rect.end.y), Color(0.75, 0.58, 0.40, 0.95), 3.0)
			_canvas_control.draw_line(Vector2(s_rect.position.x, centroid.y - 8), Vector2(s_rect.end.x, centroid.y - 8), Color(0.9, 0.9, 0.95, 0.9), 2.0)
			_canvas_control.draw_line(Vector2(s_rect.position.x, centroid.y), Vector2(s_rect.end.x, centroid.y), Color(0.9, 0.9, 0.95, 0.9), 2.0)
			_canvas_control.draw_line(Vector2(s_rect.position.x, centroid.y + 8), Vector2(s_rect.end.x, centroid.y + 8), Color(0.9, 0.9, 0.95, 0.9), 2.0)

func _draw_equipment_overlays() -> void:
	if not body_ref or not _canvas_control:
		return

	var cr = _cached_content_rect
	if cr.size.x <= 10 or cr.size.y <= 10:
		return

	var ox = cr.position.x
	var oy = cr.position.y
	var w = cr.size.x
	var h = cr.size.y
	var cx = ox + w * 0.5

	# 1. MOCHILA / ARNÉS TÁCTICO DE HOMBROS O BOLSO CRUZADO
	var bp_id = body_ref.equipped_backpack_id
	if bp_id != "":
		var tpl = ClothingDatabaseClass.get_item_visual_template(bp_id)
		var ov_type = tpl.get("overlay_type", "backpack_harness")

		if ov_type == "backpack_harness":
			var strap_col = Color(0.16, 0.22, 0.20, 0.92)
			var strap_border = Color(0.35, 0.55, 0.42, 0.85)
			var buckle_col = Color(0.85, 0.88, 0.92, 0.95)

			var l_top = Vector2(cx - w * 0.10, oy + h * 0.210)
			var l_bot = Vector2(cx - w * 0.08, oy + h * 0.355)
			_canvas_control.draw_line(l_top, l_bot, strap_col, 5.0)
			_canvas_control.draw_line(l_top, l_bot, strap_border, 1.2)

			var r_top = Vector2(cx + w * 0.10, oy + h * 0.210)
			var r_bot = Vector2(cx + w * 0.08, oy + h * 0.355)
			_canvas_control.draw_line(r_top, r_bot, strap_col, 5.0)
			_canvas_control.draw_line(r_top, r_bot, strap_border, 1.2)

			var sternum_y = oy + h * 0.275
			var stern_left = Vector2(cx - w * 0.09, sternum_y)
			var stern_right = Vector2(cx + w * 0.09, sternum_y)
			_canvas_control.draw_line(stern_left, stern_right, strap_col, 3.5)
			_canvas_control.draw_rect(Rect2(cx - 3.5, sternum_y - 3.0, 7.0, 6.0), buckle_col, true)

		elif ov_type == "sidebag_sling":
			var strap_col = Color(0.24, 0.20, 0.16, 0.92)
			var s_start = Vector2(cx + w * 0.12, oy + h * 0.210)
			var s_end = Vector2(cx - w * 0.16, oy + h * 0.440)
			_canvas_control.draw_line(s_start, s_end, strap_col, 6.0)
			_canvas_control.draw_line(s_start, s_end, Color(0.60, 0.50, 0.40, 0.85), 1.2)

			var pouch_rect = Rect2(cx - w * 0.24, oy + h * 0.410, w * 0.12, h * 0.08)
			_canvas_control.draw_rect(pouch_rect, Color(0.18, 0.15, 0.12, 0.95), true)
			_canvas_control.draw_rect(pouch_rect, Color(0.65, 0.55, 0.45, 0.90), false, 1.4)
			_canvas_control.draw_line(pouch_rect.position + Vector2(2, 4), pouch_rect.position + Vector2(pouch_rect.size.x - 2, 4), Color(0.85, 0.8, 0.7, 0.8), 1.5)
			_canvas_control.draw_rect(Rect2(pouch_rect.position.x + pouch_rect.size.x * 0.5 - 2.5, pouch_rect.position.y + 6, 5.0, 5.0), Color(0.9, 0.8, 0.5), true)

		elif ov_type == "fanny_pack":
			var belt_y = oy + h * 0.450
			var b_rect = Rect2(cx - w * 0.12, belt_y, w * 0.24, 4.0)
			_canvas_control.draw_rect(b_rect, Color(0.15, 0.15, 0.15, 0.9), true)
			var pouch_rect = Rect2(cx - w * 0.08, belt_y - 4.0, w * 0.16, 14.0)
			_canvas_control.draw_rect(pouch_rect, Color(0.20, 0.22, 0.25, 0.95), true)
			_canvas_control.draw_rect(pouch_rect, Color(0.45, 0.60, 0.75, 0.9), false, 1.2)

func _get_part_clinical_color(p_type: int) -> Color:
	if not body_ref or not body_ref.body_parts.has(p_type):
		return Color.TRANSPARENT

	var part: BodyPartClass = body_ref.body_parts[p_type]
	var bpm = body_ref.cardio.heart_rate_bpm if body_ref.cardio else 75.0

	if part.is_severed:
		return Color(0.15, 0.05, 0.05, 0.50)
	elif part.get_current_bleed_rate(bpm) > 0.0:
		return Color(0.85, 0.15, 0.15, 0.38)
	elif part.is_tourniquet_applied:
		return Color(0.45, 0.25, 0.70, 0.35)
	elif part.is_bandaged:
		return Color(0.18, 0.50, 0.75, 0.25)
	elif part.has_fracture:
		return Color(0.90, 0.40, 0.15, 0.32)
	elif part.pain_level > 25.0:
		return Color(0.85, 0.55, 0.10, 0.24)

	return Color.TRANSPARENT

func _draw_backpack_badge() -> void:
	var bp_bg = Color(0.06, 0.10, 0.14, 0.85)
	var bp_border = Color(0.25, 0.40, 0.55, 0.70)
	var bp_weight_text = "Vacía"

	if body_ref:
		var bp_weight = body_ref.get_backpack_total_weight_kg()
		if bp_weight > 0.0:
			bp_weight_text = "%.1f kg" % bp_weight
			bp_bg = Color(0.10, 0.16, 0.22, 0.95)
			bp_border = Color(0.35, 0.75, 1.0, 0.90)

	if is_backpack_selected:
		bp_border = Color(1.0, 1.0, 1.0, 1.0)
		bp_bg = bp_bg.lerp(Color(0.2, 0.5, 0.8, 1.0), 0.3)
	elif is_backpack_hovered:
		bp_border = Color(0.4, 0.85, 1.0, 1.0)

	_canvas_control.draw_rect(_backpack_rect, bp_bg, true)
	_canvas_control.draw_rect(_backpack_rect, bp_border, false, 1.5)

	var font = ThemeDB.fallback_font
	var icon_pos = _backpack_rect.position + Vector2(6, 17)
	_canvas_control.draw_string(font, icon_pos, "🎒", HORIZONTAL_ALIGNMENT_LEFT, -1, 12)

	var text_pos = _backpack_rect.position + Vector2(25, 16)
	_canvas_control.draw_string(font, text_pos, "Mochila", HORIZONTAL_ALIGNMENT_LEFT, -1, 12, Color(0.75, 0.85, 0.95))

	var sub_pos = _backpack_rect.position + Vector2(25, 30)
	_canvas_control.draw_string(font, sub_pos, bp_weight_text, HORIZONTAL_ALIGNMENT_LEFT, -1, 12, Color(0.4, 1.0, 0.6) if bp_weight_text != "Vacía" else Color(0.55, 0.6, 0.65))

# -----------------------------------------------------------------------------
# 8. ENTRADA Y SELECCIÓN DE USUARIO
# -----------------------------------------------------------------------------
func _on_canvas_gui_input(event: InputEvent) -> void:
	if event is InputEventMouseMotion:
		_hover_canvas_position = event.position
		_canvas_control.queue_redraw()
		var pos = event.position
		var found_type: int = -1

		var over_bp = _backpack_rect.has_point(pos)
		if over_bp != is_backpack_hovered:
			is_backpack_hovered = over_bp
			if _canvas_control: _canvas_control.queue_redraw()

		if not over_bp: found_type = get_part_at_canvas_pos(pos)

		if found_type != hovered_part_type:
			hovered_part_type = found_type
			part_hovered.emit(hovered_part_type)
			if _canvas_control: _canvas_control.queue_redraw()

	elif event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		var pos = event.position
		if _backpack_rect.has_point(pos):
			select_backpack()
			accept_event()
			return

		var picked = get_part_at_canvas_pos(pos)
		if picked >= 0:
			select_part(-1 if picked == selected_part_type else picked)
			accept_event()
			return
		# Clic en el fondo vacío del lienzo deselecciona la parte activa
		select_part(-1)
		accept_event()

	elif event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_RIGHT:
		var pos = event.position
		var clicked_part: int = get_part_at_canvas_pos(pos)
		part_context_menu.emit(clicked_part, _canvas_control.get_global_mouse_position())
		accept_event()

# -----------------------------------------------------------------------------
# 9. API PÚBLICA DE RETROCOMPATIBILIDAD
# -----------------------------------------------------------------------------
func get_canvas_control() -> Control:
	return _canvas_control

func get_part_at_canvas_pos(pos: Vector2) -> int:
	if not _cached_content_rect.has_point(pos): return -1
	var uv = (pos - _cached_content_rect.position) / _cached_content_rect.size
	if not _picking_images.is_empty():
		var colors: Array[Color] = []
		for img in _picking_images:
			colors.append(img.get_pixel(clampi(int(uv.x * img.get_width()), 0, img.get_width() - 1), clampi(int(uv.y * img.get_height()), 0, img.get_height() - 1)))
		var color = colors[0].lerp(colors[1], clampf((_display_fat - 0.18) / 0.22, 0, 1))
		color = color.lerp(colors[2], clampf((0.18 - _display_fat) / 0.12, 0, 1))
		color = color.lerp(colors[3], clampf((_display_muscle - 0.45) / 0.25, 0, 1))
		if maxf(color.r, maxf(color.g, color.b)) < 0.10: return -1
	for p_type in _part_polygons:
		if Geometry2D.is_point_in_polygon(pos, _part_polygons[p_type]): return p_type
	# Morphs can extend beyond the reference polygons; choose the closest region
	# only after confirming that the pointer is on the actual body texture.
	var closest = -1
	var distance = INF
	for p_type in _part_polygons:
		var d = pos.distance_squared_to(_get_polygon_centroid(_part_polygons[p_type]))
		if d < distance:
			distance = d
			closest = p_type
	return closest

func _update_scan_shader() -> void:
	if not _base_mat or _part_polygons.size() != 17: return
	var regions = PackedVector4Array()
	var damage = PackedFloat32Array()
	var hover_index = -1
	var selected_index = -1
	for part_id in _part_polygons:
		var points: PackedVector2Array = _part_polygons[part_id]
		var bounds = Rect2(points[0], Vector2.ZERO)
		for point in points: bounds = bounds.expand(point)
		var center = (bounds.get_center() - _cached_content_rect.position) / _cached_content_rect.size
		var radius = bounds.size / _cached_content_rect.size * 0.65
		if part_id == hovered_part_type: hover_index = regions.size()
		if part_id == selected_part_type: selected_index = regions.size()
		regions.append(Vector4(center.x, center.y, radius.x, radius.y))
		var intensity = 0.0
		if body_ref:
			var part = body_ref.body_parts.get(part_id)
			if part:
				intensity = maxf((100.0 - part.integrity) / 100.0, part.pain_level / 100.0)
				if part.has_fracture or part.get_current_bleed_rate(75.0) > 0.01: intensity = maxf(intensity, 0.8)
				intensity = maxf(intensity, part.infection_level / 100.0)
		damage.append(clampf(intensity, 0, 1))
	_base_mat.set_shader_parameter("scan_regions", regions)
	_base_mat.set_shader_parameter("damage_levels", damage)
	_base_mat.set_shader_parameter("hover_region", hover_index)
	_base_mat.set_shader_parameter("selected_region", selected_index)
	_base_mat.set_shader_parameter("scan_mode", int(active_layer_mode))
	_base_mat.set_shader_parameter("scan_hold", scan_hold)
	_base_mat.set_shader_parameter("interior_depth", interior_depth)
	_base_mat.set_shader_parameter("cursor_uv", (_hover_canvas_position - _cached_content_rect.position) / _cached_content_rect.size)
	_base_mat.set_shader_parameter("heartbeat_phase", heartbeat_phase)
	if body_ref:
		_base_mat.set_shader_parameter("muscle_effort", clampf(body_ref.exertion_factor, 0.0, 1.0))

func _unhandled_key_input(event: InputEvent) -> void:
	if not event is InputEventKey: return
	if event.keycode == KEY_X and not event.pressed:
		scan_hold = false
		return
	var focus = get_viewport().gui_get_focus_owner()
	if focus is LineEdit or focus is TextEdit: return
	if event.keycode == KEY_X:
		scan_hold = event.pressed
		get_viewport().set_input_as_handled()
	elif event.pressed and not event.echo and event.alt_pressed and event.keycode >= KEY_1 and event.keycode <= KEY_6:
		set_layer_mode(event.keycode - KEY_1)
		get_viewport().set_input_as_handled()
func set_body_system(b: BodySystemClass) -> void:
	body_ref = b
	if body_ref:
		_display_fat = body_ref.body_fat_pct
		_display_muscle = body_ref.muscle_mass_pct
	if _canvas_control: _canvas_control.queue_redraw()

func set_character(c) -> void:
	character_ref = c
	if c and c.body:
		set_body_system(c.body)

func select_part(part_type: int) -> void:
	selected_part_type = part_type
	part_selected.emit(part_type)
	if _canvas_control: _canvas_control.queue_redraw()

func select_backpack() -> void:
	is_backpack_selected = !is_backpack_selected
	backpack_selected.emit()
	if _canvas_control: _canvas_control.queue_redraw()

func set_highlighted_parts(parts: Array) -> void:
	highlighted_part_types.clear()
	for p in parts:
		highlighted_part_types.append(int(p))
	if _canvas_control: _canvas_control.queue_redraw()

func clear_highlighted_parts() -> void:
	highlighted_part_types.clear()
	if _canvas_control: _canvas_control.queue_redraw()

func _notification(what: int) -> void:
	if what == NOTIFICATION_WM_WINDOW_FOCUS_OUT:
		scan_hold = false
		hovered_part_type = -1

func _draw_scan_callout() -> void:
	var id = hovered_part_type if hovered_part_type >= 0 else selected_part_type
	if not body_ref or not _part_polygons.has(id): return
	var part = body_ref.body_parts.get(id)
	if not part: return
	var point = _hover_canvas_position if hovered_part_type >= 0 and _cached_content_rect.has_point(_hover_canvas_position) else _get_polygon_centroid(_part_polygons[id])
	var side = 1.0 if point.x >= _canvas_control.size.x * 0.5 else -1.0
	var elbow = point + Vector2(side * 55, -25)
	var end = elbow + Vector2(side * 65, 0)
	var color = Color(0.4, 0.78, 0.8, 0.75)
	if part.has_fracture or part.pain_level > 20: color = Color(1.0, 0.57, 0.32, 0.85)
	_canvas_control.draw_arc(point, 5, 0, TAU, 20, color, 1, true)
	_canvas_control.draw_polyline(PackedVector2Array([point, elbow, end]), color, 1, true)
	var label_position = end + Vector2(6 if side > 0 else -155, -5)
	label_position.x = clampf(label_position.x, 4, maxf(4, _canvas_control.size.x - 164))
	_canvas_control.draw_string(ThemeDB.fallback_font, label_position, part.display_name, HORIZONTAL_ALIGNMENT_LEFT, 160, 12, color)