class_name BiologyDashboard
extends Control

# =============================================================================
# DASHBOARD BIOMÉDICO Y WORKPLACE REDIMENSIONABLE
# Gestiona la silueta 2D interactiva, el inspector en cascada de 4 niveles,
# el reloj/calendario astronómico, la biomecánica de terrenos y el rig 3D
# =============================================================================

const HumanCharacterClass = preload("res://src/entities/human_character.gd")
const BodySystemClass = preload("res://src/biology/body_system.gd")
const BodyPartClass = preload("res://src/biology/body_part.gd")
const EnvironmentSystemClass = preload("res://src/biology/environment_system.gd")
const SimulationClockClass = preload("res://src/biology/simulation_clock.gd")
const ClothingDatabaseClass = preload("res://src/biology/clothing_database.gd")
const SilhouettePaperdollClass = preload("res://src/ui/silhouette_paperdoll.gd")
const MasterLoader = preload("res://src/entities/character_master_loader.gd")

@export var character: HumanCharacterClass

# Reloj y Calendario
var clock: SimulationClockClass

# Widget de la Silueta 2D
var paperdoll: SilhouettePaperdollClass

# Nodos del Workplace Redimensionable
var workplace_split: HSplitContainer
var left_split: VSplitContainer
var center_right_split: HSplitContainer
var center_area: Control
var left_panel_container: PanelContainer
var right_panel_container: PanelContainer

# Nuevos nodos del layout rediseñado
var lbl_action_duration: Label
var lbl_substrate_specs: Label
var mini_3d_container: VBoxContainer
var mini_3d_viewport_container: SubViewportContainer
var mini_3d_viewport: SubViewport
var _chk_turntable: CheckBox
var _slider_cam: HSlider
var lbl_part_selection: Label
var btn_selected_part_actions: Button
var context_menu: PopupMenu
var submenu_aid: PopupMenu
var submenu_sim: PopupMenu
var submenu_wounds: PopupMenu
var submenu_equip: PopupMenu
var _menu_target_part: int = -1

# Tracking de acciones para testing
var current_action_name: String = "Reposo"
var current_action_time: float = 0.0
var last_event_text: String = "Ninguno"
var last_event_time: float = 0.0

# Nivel 1: Resumen Superior
var lbl_tier1_vital_status: Label
var lbl_tier1_mass_breakdown: Label
var lbl_tier1_temp_pain: Label
var lbl_tier1_subsystem_telemetry: Label
var lbl_tier1_part_specs: Label

# Nivel 3: Inspector Contextual (Parte Anatómica o Mochila)
var container_inspector_part: VBoxContainer
var lbl_inspector_part_title: Label
var lbl_inspector_part_vitals: Label
var lbl_inspector_clinical_hint: Label
var hbox_clinical_actions: GridContainer
var btn_action_bandage: Button
var btn_action_tourniquet: Button
var btn_action_splint: Button
var btn_action_disinfect: Button
var btn_action_heal: Button
var btn_action_wound_test: Button
var container_layer_dropdowns: VBoxContainer
var layer_dropdown_nodes: Dictionary = {}
var lbl_layers_title: Label

# Nivel 4: Template y Asset 3D Bridge
var lbl_item_socket_bone: Label
var line_item_mesh_path: LineEdit
var btn_attach_3d_asset: Button
var lbl_item_asset_status: Label

var container_inspector_backpack: VBoxContainer
var option_backpack_model: OptionButton
var slider_backpack_content_weight: HSlider
var lbl_backpack_weight_info: Label
var lbl_backpack_mass_breakdown: Label

# Nivel 4: Ficha Técnica del Ítem
var panel_item_card: PanelContainer
var lbl_item_card_title: Label
var lbl_item_card_specs: Label
var bar_item_water_absorbed: ProgressBar
var lbl_item_water_val: Label
var lbl_item_covered_badges: Label
var btn_unequip_item_card: Button
var selected_equipped_item_id: String = ""
var container_equipped_items_list: VBoxContainer
var option_equip_part_item: OptionButton
var btn_equip_selected_part_item: Button

# Almacenamiento y Carga en Ficha Técnica
var container_item_card_cargo: VBoxContainer
var lbl_item_card_cargo_title: Label
var slider_item_card_cargo: HSlider
var lbl_item_card_cargo_info: Label
var lbl_item_card_cargo_breakdown: Label
var lbl_item_card_cargo_impact: Label

# Panel Derecho: Reloj, Entorno, Marcha, Escalada y Testbench
var lbl_clock_display: Label
var option_clock_mode: OptionButton
var btn_clock_pause: Button
var slider_env_slope: HSlider
var lbl_env_slope_val: Label
var option_env_substrate: OptionButton
var option_env_vegetation: OptionButton
var slider_env_humidity: HSlider
var lbl_env_humidity_val: Label
var slider_env_wind: HSlider
var lbl_env_wind_val: Label
var btn_toggle_climb: Button
var chk_decoupled_anim: CheckBox

# Postura y Biomecánica Postural
var lbl_posture_display: Label
var btn_posture_stand: Button
var btn_posture_sit: Button
var btn_posture_lie: Button
var btn_posture_crouch: Button
var slider_workplace_exertion: HSlider

# Nodos de la Escena (Compatibilidad con Tests y UI)
var btn_arch_skinny: Button
var btn_arch_normal: Button
var btn_arch_muscle: Button
var btn_arch_fat: Button

var slider_height: HSlider
var label_height_val: Label
var slider_muscle: HSlider
var label_muscle_val: Label
var slider_fat: HSlider
var label_fat_val: Label
var label_weight_val: Label

var bar_blood: ProgressBar
var label_blood_val: Label
var label_bpm_val: Label
var label_temp_val: Label
var bar_water: ProgressBar
var label_water_val: Label
var bar_glycogen: ProgressBar
var label_glycogen_val: Label
var bar_moisture: ProgressBar
var label_moisture_val: Label
var label_pain_val: Label
var label_status_val: Label

var btn_anim_idle: Button
var btn_anim_walk: Button
var btn_anim_run: Button
var btn_anim_sprint: Button
var btn_anim_crouch: Button
var btn_death_fall_flat: Button
var btn_death_bwd: Button
var btn_reset_vitality: Button
var slider_exertion: HSlider
var label_exertion_val: Label

var slider_time_scale: HSlider
var label_time_scale_val: Label
var label_movement_timer_val: Label
var slider_ambient_temp: HSlider
var label_ambient_temp_val: Label
var label_thermal_status_val: Label
var label_sweat_rate_val: Label

var label_caloric_balance_val: Label
var label_hunger_val: Label
var btn_eat_healthy: Button
var btn_eat_junk: Button
var btn_drink_water: Button
var btn_fast: Button

var container_parts: VBoxContainer

# Percepción Sensorial
var panel_sensory: PanelContainer
var label_sensory_mood: Label
var label_sensory_thought: Label
var label_sensory_somatic: Label

# Estado Interno
var _is_updating_ui: bool = false
var _is_crouched: bool = false
var movement_time_seconds: float = 0.0

var active_inspected_part: int = -1
var is_inspecting_backpack: bool = false
var active_inspected_item_id: String = ""
var is_anim_decoupled: bool = false

# -----------------------------------------------------------------------------
# CICLO DE VIDA
# -----------------------------------------------------------------------------
func _ready() -> void:
	clock = SimulationClockClass.new()
	bind_nodes()
	setup_signals()
	_setup_sensory_ui()
	_setup_workplace_and_cascade_inspector()

	# Desactivar la cámara 3D principal del showroom para que no se filtre en el fondo
	var orbit_cam = get_parent().get_node_or_null("OrbitCamera/Camera3D") if get_parent() else null
	if orbit_cam:
		orbit_cam.current = false

	if character:
		character.animation_state_changed.connect(_on_character_animation_state_changed)
		character.archetype_changed.connect(_on_character_archetype_changed)
		if character.has_method("set_sensory_bubble_visible"):
			character.set_sensory_bubble_visible(false)
		if character.body:
			character.body.telemetry_updated.connect(_on_telemetry_updated)
			character.body.player_collapsed.connect(_on_player_collapsed)
			if paperdoll:
				paperdoll.set_character(character)
				paperdoll.set_body_system(character.body)
			_sync_biometrics_from_character()

	_highlight_anim_button("Idle")
	_highlight_archetype_button("normal")
	_update_ambient_temp_label(22.0)
	_update_posture_display()
	_on_telemetry_updated()
	_refresh_contextual_inspector()

func _process(delta: float) -> void:
	var sim_speed: float = character.simulation_speed if character else 1.0
	clock.simulation_speed = sim_speed
	clock.advance(delta)
	_update_clock_display()
	_update_posture_display()
	
	current_action_time += delta * sim_speed
	last_event_time += delta * sim_speed
	
	if lbl_action_duration and character and character.body:
		var a_sec = int(current_action_time)
		var a_min = a_sec / 60
		a_sec = a_sec % 60
		var event_str = ""
		if last_event_text != "Ninguno":
			event_str = " | ⚡ Último: %s (%02ds)" % [last_event_text, int(last_event_time)]
		lbl_action_duration.text = "🏃 Estado: %s (%02dm %02ds)%s" % [
			current_action_name, a_min, a_sec, event_str
		]

	if _chk_turntable and _chk_turntable.button_pressed and _slider_cam:
		_slider_cam.value = wrapf(_slider_cam.value + delta * 35.0, -180.0, 180.0)

	if character and not character.is_collapsed:
		if character.target_effort > 0.02 or character.current_effort > 0.02 or character.current_anim_state != "Idle":
			movement_time_seconds += delta * sim_speed
			_update_movement_timer_display()


func _update_clock_display() -> void:
	if lbl_clock_display:
		lbl_clock_display.text = "⏱️ Simulación: %s  |  📅 Mundo: %s" % [
			clock.get_elapsed_formatted() if clock.has_method("get_elapsed_formatted") else "00h 00m 00s",
			clock.get_formatted_time()
		]

func _update_movement_timer_display() -> void:
	if not label_movement_timer_val:
		return
	var total_sec: int = int(movement_time_seconds)
	var hours: int = total_sec / 3600
	var minutes: int = (total_sec % 3600) / 60
	var seconds: int = total_sec % 60
	if hours > 0:
		label_movement_timer_val.text = "%02dh %02dm %02ds" % [hours, minutes, seconds]
	else:
		label_movement_timer_val.text = "%02dm %02ds" % [minutes, seconds]

func _input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_RIGHT:
		if context_menu and context_menu.visible and paperdoll:
			var canvas_ctrl = paperdoll.get_canvas_control() if paperdoll.has_method("get_canvas_control") else null
			if canvas_ctrl and canvas_ctrl.is_inside_tree():
				var local_pos = canvas_ctrl.get_local_mouse_position()
				if Rect2(Vector2.ZERO, canvas_ctrl.size).has_point(local_pos):
					var p_type = paperdoll.get_part_at_canvas_pos(local_pos)
					if p_type >= 0:
						context_menu.hide()
						_on_paperdoll_context_menu(p_type, canvas_ctrl.get_global_mouse_position())
						get_viewport().set_input_as_handled()

func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventKey and event.pressed and not event.echo:
		if event.keycode == KEY_F11:
			var mode = DisplayServer.window_get_mode()
			if mode == DisplayServer.WINDOW_MODE_FULLSCREEN or mode == DisplayServer.WINDOW_MODE_EXCLUSIVE_FULLSCREEN:
				DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_MAXIMIZED)
			else:
				DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_FULLSCREEN)
		elif event.keycode == KEY_B:
			if character and character.has_method("toggle_sensory_bubble"):
				character.toggle_sensory_bubble()

# -----------------------------------------------------------------------------
# ENLACE DE NODOS EXISTENTES
# -----------------------------------------------------------------------------
func bind_nodes() -> void:
	btn_arch_skinny = get_node_or_null("%BtnArchSkinny")
	btn_arch_normal = get_node_or_null("%BtnArchNormal")
	btn_arch_muscle = get_node_or_null("%BtnArchMuscle")
	btn_arch_fat = get_node_or_null("%BtnArchFat")

	slider_height = get_node_or_null("%SliderHeight")
	label_height_val = get_node_or_null("%LabelHeightVal")
	slider_muscle = get_node_or_null("%SliderMuscle")
	label_muscle_val = get_node_or_null("%LabelMuscleVal")
	slider_fat = get_node_or_null("%SliderFat")
	label_fat_val = get_node_or_null("%LabelFatVal")
	label_weight_val = get_node_or_null("%LabelWeightVal")

	bar_blood = get_node_or_null("%BarBlood")
	label_blood_val = get_node_or_null("%LabelBloodVal")
	label_bpm_val = get_node_or_null("%LabelBpmVal")
	label_temp_val = get_node_or_null("%LabelTempVal")
	bar_water = get_node_or_null("%BarWater")
	label_water_val = get_node_or_null("%LabelWaterVal")
	bar_glycogen = get_node_or_null("%BarGlycogen")
	label_glycogen_val = get_node_or_null("%LabelGlycogenVal")
	bar_moisture = get_node_or_null("%BarMoisture")
	label_moisture_val = get_node_or_null("%LabelMoistureVal")
	label_pain_val = get_node_or_null("%LabelPainVal")
	label_status_val = get_node_or_null("%LabelStatusVal")

	btn_anim_idle = get_node_or_null("%BtnAnimIdle")
	btn_anim_walk = get_node_or_null("%BtnAnimWalk")
	btn_anim_run = get_node_or_null("%BtnAnimRun")
	btn_anim_sprint = get_node_or_null("%BtnAnimSprint")
	btn_anim_crouch = get_node_or_null("%BtnAnimCrouch")
	btn_death_fall_flat = get_node_or_null("%BtnDeathFallFlat")
	btn_death_bwd = get_node_or_null("%BtnDeathBwd")
	btn_reset_vitality = get_node_or_null("%BtnResetVitality")
	slider_exertion = get_node_or_null("%SliderExertion")
	label_exertion_val = get_node_or_null("%LabelExertionVal")

	slider_time_scale = get_node_or_null("%SliderTimeScale")
	label_time_scale_val = get_node_or_null("%LabelTimeScaleVal")
	label_movement_timer_val = get_node_or_null("%LabelMovementTimerVal")
	slider_ambient_temp = get_node_or_null("%SliderAmbientTemp")
	label_ambient_temp_val = get_node_or_null("%LabelAmbientTempVal")
	label_thermal_status_val = get_node_or_null("%LabelThermalStatusVal")
	label_sweat_rate_val = get_node_or_null("%LabelSweatRateVal")

	label_caloric_balance_val = get_node_or_null("%LabelCaloricBalanceVal")
	label_hunger_val = get_node_or_null("%LabelHungerVal")
	btn_eat_healthy = get_node_or_null("%BtnEatHealthy")
	btn_eat_junk = get_node_or_null("%BtnEatJunk")
	btn_drink_water = get_node_or_null("%BtnDrinkWater")
	btn_fast = get_node_or_null("%BtnFast")

	container_parts = get_node_or_null("%ContainerParts")
	left_panel_container = get_node_or_null("LeftPanel")
	right_panel_container = get_node_or_null("RightPanel")

# -----------------------------------------------------------------------------
# CONEXIONES DE SEÑALES (ESENA Y COMPONENTES)
# -----------------------------------------------------------------------------
func setup_signals() -> void:
	if btn_reset_vitality:
		btn_reset_vitality.pressed.connect(_on_reset_pressed)

	if btn_arch_skinny:
		btn_arch_skinny.pressed.connect(func(): if character: character.set_character_archetype("skinny"))
	if btn_arch_normal:
		btn_arch_normal.pressed.connect(func(): if character: character.set_character_archetype("normal"))
	if btn_arch_muscle:
		btn_arch_muscle.pressed.connect(func(): if character: character.set_character_archetype("muscle"))
	if btn_arch_fat:
		btn_arch_fat.pressed.connect(func(): if character: character.set_character_archetype("fat"))

	if slider_height: slider_height.value_changed.connect(_on_biometrics_slider_changed)
	if slider_muscle: slider_muscle.value_changed.connect(_on_biometrics_slider_changed)
	if slider_fat: slider_fat.value_changed.connect(_on_biometrics_slider_changed)

	if btn_anim_idle: btn_anim_idle.pressed.connect(func(): _apply_effort(0.0, "Idle"))
	if btn_anim_walk: btn_anim_walk.pressed.connect(func(): _apply_effort(20.0, "Walking"))
	if btn_anim_run: btn_anim_run.pressed.connect(func(): _apply_effort(55.0, "Running"))
	if btn_anim_sprint: btn_anim_sprint.pressed.connect(func(): _apply_effort(100.0, "Sprint"))
	if btn_anim_crouch: btn_anim_crouch.pressed.connect(_toggle_crouch)
	if btn_death_fall_flat: btn_death_fall_flat.pressed.connect(func(): if character: character.play_fall_flat())
	if btn_death_bwd: btn_death_bwd.pressed.connect(func(): if character: character.play_death_bwd())

	if slider_exertion: slider_exertion.value_changed.connect(_on_exertion_slider_changed)
	if slider_time_scale: slider_time_scale.value_changed.connect(_on_time_scale_changed)
	if slider_ambient_temp:
		slider_ambient_temp.value_changed.connect(func(val: float):
			if character and character.body:
				character.body.environment.ambient_temperature = val
				character.body.thermo.ambient_temperature = val
			_update_ambient_temp_label(val)
			_on_telemetry_updated()
		)

	if btn_eat_healthy:
		btn_eat_healthy.pressed.connect(func():
			if character and character.body and character.body.metabolism:
				character.body.metabolism.ingest_food(650.0, 35.0, 75.0, 15.0)
				_on_telemetry_updated()
		)
	if btn_eat_junk:
		btn_eat_junk.pressed.connect(func():
			if character and character.body and character.body.metabolism:
				character.body.metabolism.ingest_food(1400.0, 15.0, 180.0, 65.0)
				_on_telemetry_updated()
		)
	if btn_drink_water:
		btn_drink_water.pressed.connect(func():
			if character and character.body and character.body.hydration:
				character.body.hydration.drink_water(500.0)
				_on_telemetry_updated()
		)
	if btn_fast:
		btn_fast.pressed.connect(func():
			if character and character.body and character.body.metabolism:
				character.body.metabolism.stomach_food_kcal = 0.0
				character.body.metabolism.stomach_protein_g = 0.0
				character.body.metabolism.hunger_pct = 75.0
				_on_telemetry_updated()
		)

# -----------------------------------------------------------------------------
# CONSTRUCCIÓN DEL WORKPLACE REDIMENSIONABLE Y EL INSPECTOR EN CASCADA
# -----------------------------------------------------------------------------
func _build_collapsible_card(title: String, parent: Control, initially_open: bool = true) -> VBoxContainer:
	var container = VBoxContainer.new()
	parent.add_child(container)
	
	var btn_header = Button.new()
	btn_header.text = ("▼ " if initially_open else "▶ ") + title
	btn_header.alignment = HORIZONTAL_ALIGNMENT_LEFT
	btn_header.flat = true
	btn_header.add_theme_font_size_override("font_size", 13)
	btn_header.add_theme_color_override("font_color", Color(1.0, 0.85, 0.4))
	container.add_child(btn_header)
	
	var content = VBoxContainer.new()
	content.add_theme_constant_override("separation", 6)
	content.visible = initially_open
	container.add_child(content)
	
	btn_header.pressed.connect(func():
		content.visible = !content.visible
		btn_header.text = ("▼ " if content.visible else "▶ ") + title
	)
	
	return content

func _setup_workplace_and_cascade_inspector() -> void:
	# 1. Fondo sólido oscuro para que el mundo 3D NO se filtre por los huecos
	var bg_dark = ColorRect.new()
	bg_dark.name = "BackgroundStudioDark"
	bg_dark.color = Color(0.03, 0.04, 0.06, 1.0)
	bg_dark.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(bg_dark)
	move_child(bg_dark, 0)
	bg_dark.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)

	# 2. HSplit principal: Área de Trabajo (Izquierda) + Opciones de Simulación Sandbox (Derecha)
	var main_hbox = HSplitContainer.new()
	main_hbox.name = "WorkplaceSplit"
	main_hbox.anchors_preset = Control.PRESET_FULL_RECT
	main_hbox.anchor_right = 1.0
	main_hbox.anchor_bottom = 1.0
	main_hbox.offset_left = 12.0
	main_hbox.offset_top = 10.0
	main_hbox.offset_right = -12.0
	main_hbox.offset_bottom = -10.0
	main_hbox.split_offset = 580
	main_hbox.mouse_filter = Control.MOUSE_FILTER_PASS
	add_child(main_hbox)

	# Ajuste dinámico del divisor para garantizar 350px al dock derecho sin desbordamiento
	main_hbox.resized.connect(func():
		var total_w = main_hbox.size.x
		if total_w > 800:
			var half = total_w * 0.5
			main_hbox.split_offset = int(half - 350.0)
	)

	var left_main_vbox = VBoxContainer.new()
	left_main_vbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	left_main_vbox.size_flags_vertical = Control.SIZE_EXPAND_FILL
	left_main_vbox.add_theme_constant_override("separation", 8)
	main_hbox.add_child(left_main_vbox)

	var panel_style = StyleBoxFlat.new()
	panel_style.bg_color = Color(0.07, 0.09, 0.13, 0.96)
	panel_style.border_color = Color(0.22, 0.30, 0.42, 0.85)
	panel_style.set_border_width_all(1)
	panel_style.set_corner_radius_all(8)
	panel_style.content_margin_left = 10
	panel_style.content_margin_right = 10
	panel_style.content_margin_top = 8
	panel_style.content_margin_bottom = 8

	# --- A. BARRA SUPERIOR DE CREACIÓN ---
	var top_creation_bar = PanelContainer.new()
	top_creation_bar.add_theme_stylebox_override("panel", panel_style)
	left_main_vbox.add_child(top_creation_bar)

	var top_hbox = HBoxContainer.new()
	top_hbox.alignment = BoxContainer.ALIGNMENT_CENTER
	top_hbox.add_theme_constant_override("separation", 12)
	top_creation_bar.add_child(top_hbox)

	btn_arch_skinny = Button.new()
	btn_arch_skinny.text = "🧑 Flaco"
	top_hbox.add_child(btn_arch_skinny)
	btn_arch_skinny.pressed.connect(func(): if character: character.set_character_archetype("skinny"))

	btn_arch_normal = Button.new()
	btn_arch_normal.text = "🧑 Normal"
	top_hbox.add_child(btn_arch_normal)
	btn_arch_normal.pressed.connect(func(): if character: character.set_character_archetype("normal"))

	btn_arch_muscle = Button.new()
	btn_arch_muscle.text = "💪 Músculo"
	top_hbox.add_child(btn_arch_muscle)
	btn_arch_muscle.pressed.connect(func(): if character: character.set_character_archetype("muscle"))

	btn_arch_fat = Button.new()
	btn_arch_fat.text = "🧑 Gordo"
	top_hbox.add_child(btn_arch_fat)
	btn_arch_fat.pressed.connect(func(): if character: character.set_character_archetype("fat"))

	top_hbox.add_child(VSeparator.new())

	var lbl_h = Label.new()
	lbl_h.text = "Altura:"
	top_hbox.add_child(lbl_h)
	slider_height = HSlider.new()
	slider_height.min_value = 1.50
	slider_height.max_value = 2.05
	slider_height.step = 0.01
	slider_height.custom_minimum_size = Vector2(100, 0)
	slider_height.size_flags_vertical = Control.SIZE_SHRINK_CENTER
	top_hbox.add_child(slider_height)
	label_height_val = Label.new()
	label_height_val.text = "1.78 m"
	top_hbox.add_child(label_height_val)
	slider_height.value_changed.connect(_on_biometrics_slider_changed)

	var lbl_m = Label.new()
	lbl_m.text = "Músculo:"
	top_hbox.add_child(lbl_m)
	slider_muscle = HSlider.new()
	slider_muscle.min_value = 0
	slider_muscle.max_value = 100
	slider_muscle.step = 1
	slider_muscle.custom_minimum_size = Vector2(100, 0)
	slider_muscle.size_flags_vertical = Control.SIZE_SHRINK_CENTER
	top_hbox.add_child(slider_muscle)
	label_muscle_val = Label.new()
	label_muscle_val.text = "45%"
	top_hbox.add_child(label_muscle_val)
	slider_muscle.value_changed.connect(_on_biometrics_slider_changed)

	var lbl_f = Label.new()
	lbl_f.text = "Grasa:"
	top_hbox.add_child(lbl_f)
	slider_fat = HSlider.new()
	slider_fat.min_value = 0
	slider_fat.max_value = 100
	slider_fat.step = 1
	slider_fat.custom_minimum_size = Vector2(100, 0)
	slider_fat.size_flags_vertical = Control.SIZE_SHRINK_CENTER
	top_hbox.add_child(slider_fat)
	label_fat_val = Label.new()
	label_fat_val.text = "18%"
	top_hbox.add_child(label_fat_val)
	slider_fat.value_changed.connect(_on_biometrics_slider_changed)

	top_hbox.add_child(VSeparator.new())
	label_weight_val = Label.new()
	label_weight_val.text = "70.3 kg"
	label_weight_val.add_theme_color_override("font_color", Color(0.4, 1.0, 0.7))
	top_hbox.add_child(label_weight_val)

	# --- B. ÁREA DE TRABAJO EN 3 COLUMNAS:
	#   1. Izquierda: Telemetría unificada (Anatomía, Percepción Somática y Fisiología)
	#   2. Centro: Silueta Paperdoll 2D interactiva (Ocupa el 100% de la altura)
	#   3. Derecha: Inspector Clínico Contextual y Ficha Técnica del Ítem
	var workplace_columns = HBoxContainer.new()
	workplace_columns.name = "WorkplaceColumns"
	workplace_columns.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	workplace_columns.size_flags_vertical = Control.SIZE_EXPAND_FILL
	workplace_columns.add_theme_constant_override("separation", 8)
	left_main_vbox.add_child(workplace_columns)

	# 1. Columna Izquierda: Telemetría Unificada (Anatomía, Somática y Fisiología)
	var col_telemetry_panel = PanelContainer.new()
	col_telemetry_panel.name = "TelemetryPanel"
	col_telemetry_panel.size_flags_horizontal = Control.SIZE_FILL
	col_telemetry_panel.size_flags_vertical = Control.SIZE_EXPAND_FILL
	col_telemetry_panel.custom_minimum_size = Vector2(310, 0)
	col_telemetry_panel.add_theme_stylebox_override("panel", panel_style)
	workplace_columns.add_child(col_telemetry_panel)

	var col_telemetry_scroll = ScrollContainer.new()
	col_telemetry_scroll.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	col_telemetry_scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	col_telemetry_scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	col_telemetry_panel.add_child(col_telemetry_scroll)

	var col_telemetry_vbox = VBoxContainer.new()
	col_telemetry_vbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	col_telemetry_vbox.add_theme_constant_override("separation", 10)
	col_telemetry_scroll.add_child(col_telemetry_vbox)

	_build_unified_biomedical_card(col_telemetry_vbox)

	# 2. Columna Central: Silueta Paperdoll 2D Interactiva con Banner Superior de Estado
	var col_silhouette_panel = PanelContainer.new()
	col_silhouette_panel.name = "SilhouettePanel"
	col_silhouette_panel.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	col_silhouette_panel.size_flags_vertical = Control.SIZE_EXPAND_FILL
	col_silhouette_panel.size_flags_stretch_ratio = 1.3
	col_silhouette_panel.custom_minimum_size = Vector2(260, 440)
	col_silhouette_panel.add_theme_stylebox_override("panel", panel_style)
	workplace_columns.add_child(col_silhouette_panel)

	var sil_vbox = VBoxContainer.new()
	sil_vbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	sil_vbox.size_flags_vertical = Control.SIZE_EXPAND_FILL
	sil_vbox.add_theme_constant_override("separation", 6)
	col_silhouette_panel.add_child(sil_vbox)

	# Banner Superior de Estado y Postura sobre la silueta
	var sil_header_status_card = PanelContainer.new()
	sil_header_status_card.name = "SilHeaderStatusCard"
	var sil_status_style = StyleBoxFlat.new()
	sil_status_style.bg_color = Color(0.08, 0.10, 0.14, 0.95)
	sil_status_style.border_color = Color(0.25, 0.4, 0.55, 0.8)
	sil_status_style.set_border_width_all(1)
	sil_status_style.set_corner_radius_all(6)
	sil_status_style.content_margin_left = 10
	sil_status_style.content_margin_right = 10
	sil_status_style.content_margin_top = 5
	sil_status_style.content_margin_bottom = 5
	sil_header_status_card.add_theme_stylebox_override("panel", sil_status_style)
	sil_vbox.add_child(sil_header_status_card)

	var hbox_status = HBoxContainer.new()
	hbox_status.add_theme_constant_override("separation", 8)
	sil_header_status_card.add_child(hbox_status)

	lbl_action_duration = Label.new()
	lbl_action_duration.text = "🏃 Estado: Reposo (00m 00s)"
	lbl_action_duration.add_theme_font_size_override("font_size", 12)
	lbl_action_duration.add_theme_color_override("font_color", Color(0.4, 1.0, 0.7))
	hbox_status.add_child(lbl_action_duration)

	lbl_posture_display = Label.new()
	lbl_posture_display.text = "🧍 De pie (00s) | Carga soportada: 75.0 kg"
	lbl_posture_display.add_theme_font_size_override("font_size", 12)
	lbl_posture_display.add_theme_color_override("font_color", Color(0.4, 1.0, 0.65))
	lbl_posture_display.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	lbl_posture_display.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	hbox_status.add_child(lbl_posture_display)

	paperdoll = SilhouettePaperdollClass.new()
	paperdoll.custom_minimum_size = Vector2(260, 400)
	paperdoll.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	paperdoll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	paperdoll.part_selected.connect(_on_paperdoll_part_selected)
	paperdoll.part_hovered.connect(_on_paperdoll_part_hovered)
	paperdoll.backpack_selected.connect(_on_paperdoll_backpack_selected)
	paperdoll.layer_mode_changed.connect(_on_paperdoll_layer_mode_changed)
	if paperdoll.has_signal("part_context_menu"):
		paperdoll.part_context_menu.connect(_on_paperdoll_context_menu)
	sil_vbox.add_child(paperdoll)

	lbl_part_selection = Label.new()
	lbl_part_selection.name = "PartSelectionHint"
	lbl_part_selection.text = "Seleccioná una zona del cuerpo para inspeccionarla."
	lbl_part_selection.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	lbl_part_selection.add_theme_font_size_override("font_size", 12)
	sil_vbox.add_child(lbl_part_selection)

	btn_selected_part_actions = Button.new()
	btn_selected_part_actions.name = "SelectedPartActions"
	btn_selected_part_actions.text = "Acciones de esta zona ▾"
	btn_selected_part_actions.disabled = true
	btn_selected_part_actions.tooltip_text = "Seleccioná una zona del cuerpo. También podés usar clic derecho sobre ella."
	btn_selected_part_actions.pressed.connect(_on_selected_part_actions_pressed)
	sil_vbox.add_child(btn_selected_part_actions)

	# Barra de Velocidad de Movimiento debajo de la silueta
	var move_card = PanelContainer.new()
	var move_style = StyleBoxFlat.new()
	move_style.bg_color = Color(0.08, 0.10, 0.14, 0.95)
	move_style.border_color = Color(0.25, 0.4, 0.55, 0.8)
	move_style.set_border_width_all(1)
	move_style.set_corner_radius_all(6)
	move_style.content_margin_left = 10
	move_style.content_margin_right = 10
	move_style.content_margin_top = 6
	move_style.content_margin_bottom = 6
	move_card.add_theme_stylebox_override("panel", move_style)
	sil_vbox.add_child(move_card)

	var move_vbox = VBoxContainer.new()
	move_vbox.add_theme_constant_override("separation", 4)
	move_card.add_child(move_vbox)

	var hbox_move_header = HBoxContainer.new()
	move_vbox.add_child(hbox_move_header)

	var lbl_move_title = Label.new()
	lbl_move_title.text = "🏃 Velocidad de Movimiento"
	lbl_move_title.add_theme_font_size_override("font_size", 12)
	lbl_move_title.add_theme_color_override("font_color", Color(0.9, 0.95, 1.0))
	lbl_move_title.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_move_header.add_child(lbl_move_title)

	label_exertion_val = Label.new()
	label_exertion_val.name = "LabelExertionVal"
	label_exertion_val.text = "0%"
	label_exertion_val.add_theme_font_size_override("font_size", 12)
	label_exertion_val.add_theme_color_override("font_color", Color(0.4, 1.0, 0.7))
	hbox_move_header.add_child(label_exertion_val)

	slider_workplace_exertion = HSlider.new()
	slider_workplace_exertion.name = "SliderExertionWorkplace"
	slider_workplace_exertion.min_value = 0.0
	slider_workplace_exertion.max_value = 100.0
	slider_workplace_exertion.step = 1.0
	slider_workplace_exertion.value = 0.0
	slider_workplace_exertion.value_changed.connect(_on_exertion_slider_changed)
	move_vbox.add_child(slider_workplace_exertion)

	# 3. Columna Derecha: Inspector Clínico Contextual y Ficha Técnica
	var col_inspector_panel = PanelContainer.new()
	col_inspector_panel.name = "InspectorPanel"
	col_inspector_panel.size_flags_horizontal = Control.SIZE_FILL
	col_inspector_panel.size_flags_vertical = Control.SIZE_EXPAND_FILL
	col_inspector_panel.custom_minimum_size = Vector2(310, 0)
	col_inspector_panel.add_theme_stylebox_override("panel", panel_style)
	workplace_columns.add_child(col_inspector_panel)

	var col_inspector_scroll = ScrollContainer.new()
	col_inspector_scroll.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	col_inspector_scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	col_inspector_scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	col_inspector_panel.add_child(col_inspector_scroll)

	var col_inspector_vbox = VBoxContainer.new()
	col_inspector_vbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	col_inspector_vbox.add_theme_constant_override("separation", 10)
	col_inspector_scroll.add_child(col_inspector_vbox)

	_build_tier3_inspector_ui(col_inspector_vbox)
	_build_tier4_item_card(col_inspector_vbox)

	# --- C. BARRA INFERIOR DE TESTING (COMPACTA, DEMARCADA) ---
	var bottom_testing_bar = PanelContainer.new()
	bottom_testing_bar.add_theme_stylebox_override("panel", panel_style)
	left_main_vbox.add_child(bottom_testing_bar)

	var bottom_vbox = VBoxContainer.new()
	bottom_vbox.add_theme_constant_override("separation", 6)
	bottom_testing_bar.add_child(bottom_vbox)

	# Fila 1: Percepción Somática Narrativa (Mood y Flujo de Pensamientos)
	if panel_sensory:
		bottom_vbox.add_child(panel_sensory)

	# Fila 2: Controles agrupados con demarcación clara
	var hbox_ctrls = HBoxContainer.new()
	hbox_ctrls.add_theme_constant_override("separation", 8)
	bottom_vbox.add_child(hbox_ctrls)

	# Grupo Posturas
	var lbl_p_tag = Label.new()
	lbl_p_tag.text = "Postura:"
	lbl_p_tag.add_theme_color_override("font_color", Color(1.0, 0.85, 0.4))
	lbl_p_tag.add_theme_font_size_override("font_size", 12)
	hbox_ctrls.add_child(lbl_p_tag)

	btn_posture_stand = Button.new()
	btn_posture_stand.text = "🧍 De Pie"
	hbox_ctrls.add_child(btn_posture_stand)
	btn_posture_sit = Button.new()
	btn_posture_sit.text = "🪑 Sentarse"
	hbox_ctrls.add_child(btn_posture_sit)
	btn_posture_lie = Button.new()
	btn_posture_lie.text = "🛏️ Acostarse"
	hbox_ctrls.add_child(btn_posture_lie)
	btn_posture_crouch = Button.new()
	btn_posture_crouch.text = "🧎 Agacharse"
	hbox_ctrls.add_child(btn_posture_crouch)

	btn_posture_stand.pressed.connect(func(): _set_posture_from_ui(BodySystemClass.Posture.STANDING))
	btn_posture_sit.pressed.connect(func(): _set_posture_from_ui(BodySystemClass.Posture.SITTING))
	btn_posture_lie.pressed.connect(func(): _set_posture_from_ui(BodySystemClass.Posture.LYING))
	btn_posture_crouch.pressed.connect(func(): _set_posture_from_ui(BodySystemClass.Posture.CROUCHING))

	hbox_ctrls.add_child(VSeparator.new())

	# Grupo Marcha
	var lbl_m_tag = Label.new()
	lbl_m_tag.text = "Marcha:"
	lbl_m_tag.add_theme_color_override("font_color", Color(1.0, 0.85, 0.4))
	lbl_m_tag.add_theme_font_size_override("font_size", 12)
	hbox_ctrls.add_child(lbl_m_tag)

	btn_anim_idle = Button.new()
	btn_anim_idle.text = "🧘 Reposo"
	hbox_ctrls.add_child(btn_anim_idle)
	btn_anim_walk = Button.new()
	btn_anim_walk.text = "🚶 Caminar"
	hbox_ctrls.add_child(btn_anim_walk)
	btn_anim_run = Button.new()
	btn_anim_run.text = "🏃 Carrera"
	hbox_ctrls.add_child(btn_anim_run)
	btn_anim_sprint = Button.new()
	btn_anim_sprint.text = "⚡ Sprint"
	hbox_ctrls.add_child(btn_anim_sprint)

	btn_anim_idle.pressed.connect(func(): _apply_effort(0.0, "Idle"))
	btn_anim_walk.pressed.connect(func(): _apply_effort(20.0, "Walking"))
	btn_anim_run.pressed.connect(func(): _apply_effort(55.0, "Running"))
	btn_anim_sprint.pressed.connect(func(): _apply_effort(100.0, "Sprint"))


	# Fila 3: Acciones Fisiológicas
	var hbox_actions = HBoxContainer.new()
	hbox_actions.add_theme_constant_override("separation", 8)
	bottom_vbox.add_child(hbox_actions)

	var lbl_a_tag = Label.new()
	lbl_a_tag.text = "Acciones Fisiológicas:"
	lbl_a_tag.add_theme_color_override("font_color", Color(1.0, 0.85, 0.4))
	lbl_a_tag.add_theme_font_size_override("font_size", 12)
	hbox_actions.add_child(lbl_a_tag)

	btn_eat_healthy = Button.new()
	btn_eat_healthy.text = "🥗 Comer Sano"
	hbox_actions.add_child(btn_eat_healthy)
	btn_eat_junk = Button.new()
	btn_eat_junk.text = "🍔 Comida Chatarra"
	hbox_actions.add_child(btn_eat_junk)
	btn_drink_water = Button.new()
	btn_drink_water.text = "💧 Beber Agua"
	hbox_actions.add_child(btn_drink_water)
	btn_fast = Button.new()
	btn_fast.text = "⏳ Ayunar (Vaciar)"
	hbox_actions.add_child(btn_fast)
	var btn_bowel = Button.new()
	btn_bowel.text = "🚽 Evacuar Vejiga"
	hbox_actions.add_child(btn_bowel)
	btn_toggle_climb = Button.new()
	btn_toggle_climb.text = "🧗 Trepar / Escalar"
	hbox_actions.add_child(btn_toggle_climb)

	btn_eat_healthy.pressed.connect(func():
		if character and character.body and character.body.metabolism:
			character.body.metabolism.ingest_food(650.0, 35.0, 75.0, 15.0)
			last_event_text = "Comer Sano (+650 kcal)"
			last_event_time = 0.0
			_on_telemetry_updated()
	)
	btn_eat_junk.pressed.connect(func():
		if character and character.body and character.body.metabolism:
			character.body.metabolism.ingest_food(1400.0, 15.0, 180.0, 65.0)
			last_event_text = "Chatarra (+1400 kcal)"
			last_event_time = 0.0
			_on_telemetry_updated()
	)
	btn_drink_water.pressed.connect(func():
		if character and character.body and character.body.hydration:
			character.body.hydration.drink_water(500.0)
			last_event_text = "Beber Agua (+500 mL)"
			last_event_time = 0.0
			_on_telemetry_updated()
	)
	btn_fast.pressed.connect(func():
		if character and character.body and character.body.metabolism:
			character.body.metabolism.stomach_food_kcal = 0.0
			character.body.metabolism.stomach_protein_g = 0.0
			character.body.metabolism.hunger_pct = 75.0
			last_event_text = "Ayunar (Vaciado)"
			last_event_time = 0.0
			_on_telemetry_updated()
	)
	btn_bowel.pressed.connect(func():
		if character and character.body:
			if character.body.metabolism:
				character.body.metabolism.stomach_food_kcal = max(0.0, character.body.metabolism.stomach_food_kcal - 200.0)
			if character.body.hydration:
				character.body.hydration.current_water_ml = max(500.0, character.body.hydration.current_water_ml - 200.0)
			last_event_text = "Evacuación (-200 mL)"
			last_event_time = 0.0
			_on_telemetry_updated()
	)
	btn_toggle_climb.pressed.connect(func():
		if character and character.body:
			_on_toggle_climb_pressed()
			last_event_text = "Escalada " + ("ACTIVA" if character.body.is_climbing else "Inactiva")
			last_event_time = 0.0
	)

	# --- D. PANEL DERECHO (OPCIONES DE SIMULACIÓN SANDBOX Y VISOR 3D) ---
	var right_panel = PanelContainer.new()
	right_panel.name = "RightSimulationDock"
	right_panel.custom_minimum_size = Vector2(330, 0)
	right_panel.size_flags_horizontal = Control.SIZE_FILL
	right_panel.size_flags_vertical = Control.SIZE_EXPAND_FILL
	right_panel.add_theme_stylebox_override("panel", panel_style)
	main_hbox.add_child(right_panel)

	var right_sidebar_scroll = ScrollContainer.new()
	right_sidebar_scroll.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	right_sidebar_scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	right_sidebar_scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	right_panel.add_child(right_sidebar_scroll)

	var sidebar_vbox = VBoxContainer.new()
	sidebar_vbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	sidebar_vbox.add_theme_constant_override("separation", 10)
	right_sidebar_scroll.add_child(sidebar_vbox)

	# Tarjeta 1: Reloj y Simulación
	var clock_content = _build_collapsible_card("⏱️ Reloj y Simulación", sidebar_vbox, true)
	_build_card_clock(clock_content)
	var btn_reset_sim = Button.new()
	btn_reset_sim.text = "🔄 Resetear Simulación Completa"
	btn_reset_sim.pressed.connect(_on_reset_pressed)
	clock_content.add_child(btn_reset_sim)

	# Tarjeta 2: Entorno y Terreno
	var env_content = _build_collapsible_card("🌤️ Entorno y Terreno", sidebar_vbox, true)
	_build_card_environment_terrains(env_content)

	# Tarjeta 3: Banco de Animación
	var anim_content = _build_collapsible_card("🏃 Banco Animación", sidebar_vbox, false)
	_build_card_anim_testbench(anim_content)
	
	# Tarjeta 4: VISUALIZADOR 3D (RIG Y ANIMACIONES) - DOCK DEDICADO
	var rig_content = _build_collapsible_card("🎥 Visualizador 3D (Rig)", sidebar_vbox, true)
	mini_3d_viewport_container = SubViewportContainer.new()
	mini_3d_viewport_container.stretch = true
	mini_3d_viewport_container.custom_minimum_size = Vector2(290, 220)
	rig_content.add_child(mini_3d_viewport_container)

	mini_3d_viewport = SubViewport.new()
	mini_3d_viewport.size = Vector2i(290, 220)
	mini_3d_viewport.own_world_3d = false
	var vp = get_viewport()
	if vp:
		mini_3d_viewport.world_3d = vp.find_world_3d()
	var cam = Camera3D.new()
	cam.current = true
	cam.look_at_from_position(Vector3(0, 1.1, 2.3), Vector3(0, 0.9, 0), Vector3.UP)
	mini_3d_viewport.add_child(cam)
	mini_3d_viewport_container.add_child(mini_3d_viewport)

	var hbox_cam = HBoxContainer.new()
	rig_content.add_child(hbox_cam)
	_chk_turntable = CheckBox.new()
	_chk_turntable.text = "🔄 Rotar 3D"
	hbox_cam.add_child(_chk_turntable)
	_slider_cam = HSlider.new()
	_slider_cam.min_value = -180.0
	_slider_cam.max_value = 180.0
	_slider_cam.value = 0.0
	_slider_cam.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_slider_cam.size_flags_vertical = Control.SIZE_SHRINK_CENTER
	hbox_cam.add_child(_slider_cam)
	
	_slider_cam.value_changed.connect(func(v: float):
		var rad = deg_to_rad(v)
		cam.position = Vector3(sin(rad) * 2.3, 1.1, cos(rad) * 2.3)
		cam.look_at(Vector3(0, 0.9, 0))
	)

	# Menú contextual para click derecho en el paperdoll
	context_menu = PopupMenu.new()
	context_menu.name = "PaperdollContextMenu"
	context_menu.exclusive = false
	context_menu.add_separator("Zona del cuerpo")

	submenu_aid = PopupMenu.new()
	submenu_aid.name = "SubmenuAid"
	submenu_aid.exclusive = false
	submenu_aid.id_pressed.connect(_on_context_menu_action)
	context_menu.add_child(submenu_aid)
	context_menu.add_submenu_node_item("🩹 Primeros Auxilios", submenu_aid)

	# Submenú de Tipos de Heridas Graduadas
	submenu_wounds = PopupMenu.new()
	submenu_wounds.name = "SubmenuWounds"
	submenu_wounds.exclusive = false
	submenu_wounds.add_item("🩹 Rasguño Superficial (0.1 mL/s | Dolor +5)", 20)
	submenu_wounds.add_item("🩸 Laceración Dérmica (0.8 mL/s | Dolor +15)", 21)
	submenu_wounds.add_item("🩸🔪 Herida Incisa Profunda (2.2 mL/s | Dolor +30)", 22)
	submenu_wounds.add_item("💥 Herida Punzante / Mordida (1.2 mL/s | Sepsis +45%)", 23)
	submenu_wounds.add_item("🚨 Hemorragia Arterial Masiva (4.5 mL/s | Crítica)", 24)
	submenu_wounds.id_pressed.connect(_on_context_menu_action)

	submenu_sim = PopupMenu.new()
	submenu_sim.name = "SubmenuSim"
	submenu_sim.exclusive = false
	submenu_sim.add_child(submenu_wounds)
	submenu_sim.add_submenu_node_item("⚠️ Provocar Herida y Sangrado", submenu_wounds)
	submenu_sim.add_item("🦴 Provocar Fractura Ósea", 6)
	submenu_sim.add_item("🧼 Infectar Herida (Sepsis +35%)", 7)
	submenu_sim.add_separator()
	submenu_sim.add_item("✨ Sanar Completamente", 4)
	submenu_sim.id_pressed.connect(_on_context_menu_action)
	context_menu.add_child(submenu_sim)
	context_menu.add_submenu_node_item("🧪 Laboratorio (depuración)", submenu_sim)

	submenu_equip = PopupMenu.new()
	submenu_equip.name = "SubmenuEquip"
	submenu_equip.exclusive = false
	submenu_equip.id_pressed.connect(_on_context_menu_action)
	context_menu.add_child(submenu_equip)
	context_menu.add_submenu_node_item("🧥 Equipamiento y Capas", submenu_equip)

	context_menu.add_separator()
	context_menu.add_item("🔍 Examinar Detalles", 10)
	add_child(context_menu)
	context_menu.id_pressed.connect(_on_context_menu_action)

	if left_panel_container: left_panel_container.visible = false
	if right_panel_container: right_panel_container.visible = false


# -----------------------------------------------------------------------------
# NIVEL 1: RESUMEN SUPERIOR
# -----------------------------------------------------------------------------
func _build_unified_biomedical_card(parent: Control) -> void:
	var panel = _create_card_container("🧬 FISIOLOGÍA, ANATOMÍA Y EQUIPAMIENTO", parent)
	var vbox = panel.get_child(0)

	# Fila 1: Subtítulo / Vitalidad
	var hbox_status = HBoxContainer.new()
	vbox.add_child(hbox_status)

	var lbl_sub = Label.new()
	lbl_sub.text = "ESTADO VITAL:"
	lbl_sub.add_theme_font_size_override("font_size", 12)
	lbl_sub.add_theme_color_override("font_color", Color(0.65, 0.75, 0.85))
	lbl_sub.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_status.add_child(lbl_sub)

	lbl_tier1_vital_status = Label.new()
	lbl_tier1_vital_status.text = "VITALIDAD ÓPTIMA"
	lbl_tier1_vital_status.add_theme_font_size_override("font_size", 12)
	lbl_tier1_vital_status.modulate = Color(0.3, 1.0, 0.5)
	hbox_status.add_child(lbl_tier1_vital_status)

	# Fila 2: Masa Desglosada
	lbl_tier1_mass_breakdown = Label.new()
	lbl_tier1_mass_breakdown.text = "Masa Total: 75.0 kg (Cuerpo: 75.0 | Ropa: 0.0 | Agua: 0.0 | Mochila: 0.0)"
	lbl_tier1_mass_breakdown.add_theme_font_size_override("font_size", 12)
	lbl_tier1_mass_breakdown.add_theme_color_override("font_color", Color(0.65, 0.85, 1.0))
	lbl_tier1_mass_breakdown.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	lbl_tier1_mass_breakdown.custom_minimum_size = Vector2(0, 0)
	vbox.add_child(lbl_tier1_mass_breakdown)

	# Fila 3: Aislamiento y Agua en Ropa
	lbl_tier1_temp_pain = Label.new()
	lbl_tier1_temp_pain.text = "🧥 Aislamiento: 0.00 Clo  |  💧 Agua en Ropa: 0.00 kg"
	lbl_tier1_temp_pain.add_theme_font_size_override("font_size", 12)
	lbl_tier1_temp_pain.add_theme_color_override("font_color", Color(0.85, 0.85, 0.85))
	lbl_tier1_temp_pain.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	lbl_tier1_temp_pain.custom_minimum_size = Vector2(0, 0)
	vbox.add_child(lbl_tier1_temp_pain)

	# Fila 4: Traumatología (Fracturas, Hemorragias, Dolor)
	lbl_tier1_subsystem_telemetry = Label.new()
	lbl_tier1_subsystem_telemetry.text = "🦴 Fracturas: 0  |  🩸 Hemorragias: 0  |  ⚡ Dolor Global: 0/100"
	lbl_tier1_subsystem_telemetry.add_theme_font_size_override("font_size", 12)
	lbl_tier1_subsystem_telemetry.add_theme_color_override("font_color", Color(0.4, 0.95, 0.8))
	lbl_tier1_subsystem_telemetry.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	lbl_tier1_subsystem_telemetry.custom_minimum_size = Vector2(0, 0)
	vbox.add_child(lbl_tier1_subsystem_telemetry)

	# Fila 5: Datos Somáticos de Órganos y Extremidades (De pie, corazón, temp, pies)
	label_sensory_somatic = Label.new()
	label_sensory_somatic.text = "🧍 De pie (00s)  •  🫀 Corazón: 70 BPM  •  🌡️ Temp: 37.0°C  •  🦶 Pies: Calzados"
	label_sensory_somatic.add_theme_font_size_override("font_size", 12)
	label_sensory_somatic.add_theme_color_override("font_color", Color(0.55, 0.85, 1.0))
	label_sensory_somatic.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	label_sensory_somatic.custom_minimum_size = Vector2(0, 0)
	vbox.add_child(label_sensory_somatic)

	var sep1 = HSeparator.new()
	sep1.modulate = Color(1.0, 1.0, 1.0, 0.25)
	vbox.add_child(sep1)

	# Barras Fisiológicas y Nutrición en Vivo
	var hbox_b = HBoxContainer.new()
	vbox.add_child(hbox_b)
	var lbl_b = Label.new()
	lbl_b.text = "🩸 Volemia (Sangre):"
	lbl_b.add_theme_font_size_override("font_size", 12)
	lbl_b.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_b.add_child(lbl_b)
	label_blood_val = Label.new()
	label_blood_val.name = "LabelBloodVal"
	label_blood_val.text = "5000 / 5000 mL"
	label_blood_val.add_theme_font_size_override("font_size", 12)
	hbox_b.add_child(label_blood_val)

	bar_blood = ProgressBar.new()
	bar_blood.name = "BarBlood"
	bar_blood.max_value = 5000.0
	bar_blood.value = 5000.0
	bar_blood.custom_minimum_size = Vector2(0, 12)
	_style_progress_bar(bar_blood, Color(0.85, 0.15, 0.2, 0.95))
	vbox.add_child(bar_blood)

	var hbox_v = HBoxContainer.new()
	vbox.add_child(hbox_v)
	var lbl_hr = Label.new()
	lbl_hr.text = "💓 Frecuencia Cardíaca:"
	lbl_hr.add_theme_font_size_override("font_size", 12)
	lbl_hr.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_v.add_child(lbl_hr)
	label_bpm_val = Label.new()
	label_bpm_val.name = "LabelBpmVal"
	label_bpm_val.text = "70 BPM"
	label_bpm_val.add_theme_font_size_override("font_size", 12)
	hbox_v.add_child(label_bpm_val)

	var hbox_g = HBoxContainer.new()
	vbox.add_child(hbox_g)
	var lbl_g = Label.new()
	lbl_g.text = "🥩 Glucógeno Muscular:"
	lbl_g.add_theme_font_size_override("font_size", 12)
	lbl_g.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_g.add_child(lbl_g)
	label_glycogen_val = Label.new()
	label_glycogen_val.name = "LabelGlycogenVal"
	label_glycogen_val.text = "100%"
	label_glycogen_val.add_theme_font_size_override("font_size", 12)
	hbox_g.add_child(label_glycogen_val)

	bar_glycogen = ProgressBar.new()
	bar_glycogen.name = "BarGlycogen"
	bar_glycogen.max_value = 100.0
	bar_glycogen.value = 100.0
	bar_glycogen.custom_minimum_size = Vector2(0, 12)
	_style_progress_bar(bar_glycogen, Color(0.95, 0.65, 0.15, 0.95))
	vbox.add_child(bar_glycogen)

	var hbox_w = HBoxContainer.new()
	vbox.add_child(hbox_w)
	var lbl_w = Label.new()
	lbl_w.text = "💧 Hidratación Corporal:"
	lbl_w.add_theme_font_size_override("font_size", 12)
	lbl_w.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_w.add_child(lbl_w)
	label_water_val = Label.new()
	label_water_val.name = "LabelWaterVal"
	label_water_val.text = "2500 / 2500 mL"
	label_water_val.add_theme_font_size_override("font_size", 12)
	hbox_w.add_child(label_water_val)

	bar_water = ProgressBar.new()
	bar_water.name = "BarWater"
	bar_water.max_value = 2500.0
	bar_water.value = 2500.0
	bar_water.custom_minimum_size = Vector2(0, 12)
	_style_progress_bar(bar_water, Color(0.2, 0.65, 0.95, 0.95))
	vbox.add_child(bar_water)

	var sep2 = HSeparator.new()
	sep2.modulate = Color(1.0, 1.0, 1.0, 0.25)
	vbox.add_child(sep2)

	# Resumen de Parte Seleccionada
	lbl_tier1_part_specs = Label.new()
	lbl_tier1_part_specs.text = "🦴 Silueta: Haz clic en una parte para examinarla"
	lbl_tier1_part_specs.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	lbl_tier1_part_specs.custom_minimum_size = Vector2(0, 0)
	lbl_tier1_part_specs.add_theme_font_size_override("font_size", 12)
	lbl_tier1_part_specs.add_theme_color_override("font_color", Color(0.75, 0.85, 0.95))
	vbox.add_child(lbl_tier1_part_specs)

func _build_tier1_summary_header(parent: Control) -> void:
	_build_unified_biomedical_card(parent)


# -----------------------------------------------------------------------------
# NIVEL 3: INSPECTOR CONTEXTUAL (PARTE ANATÓMICA O MOCHILA)
# -----------------------------------------------------------------------------
func _build_tier3_inspector_ui(parent: Control) -> void:
	var margin = MarginContainer.new()
	margin.add_theme_constant_override("margin_left", 8)
	margin.add_theme_constant_override("margin_right", 8)
	margin.add_theme_constant_override("margin_top", 4)
	margin.add_theme_constant_override("margin_bottom", 4)
	parent.add_child(margin)

	var root_vbox = VBoxContainer.new()
	root_vbox.add_theme_constant_override("separation", 8)
	margin.add_child(root_vbox)

	container_inspector_part = VBoxContainer.new()
	container_inspector_part.add_theme_constant_override("separation", 6)
	root_vbox.add_child(container_inspector_part)

	lbl_inspector_part_title = Label.new()
	lbl_inspector_part_title.text = "🩺 Examen Clínico / Inspección"
	lbl_inspector_part_title.add_theme_font_size_override("font_size", 13)
	lbl_inspector_part_title.add_theme_color_override("font_color", Color(1.0, 0.85, 0.4))
	container_inspector_part.add_child(lbl_inspector_part_title)

	lbl_inspector_part_vitals = Label.new()
	lbl_inspector_part_vitals.text = "Selecciona una parte anatómica en la silueta para examinar lesiones y prendas."
	lbl_inspector_part_vitals.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	lbl_inspector_part_vitals.add_theme_font_size_override("font_size", 12)
	lbl_inspector_part_vitals.add_theme_color_override("font_color", Color(0.8, 0.9, 0.95))
	container_inspector_part.add_child(lbl_inspector_part_vitals)

	lbl_inspector_clinical_hint = Label.new()
	lbl_inspector_clinical_hint.text = "💡 Usá «Acciones de esta zona» bajo la silueta o hacé clic derecho sobre el cuerpo."
	lbl_inspector_clinical_hint.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	lbl_inspector_clinical_hint.add_theme_font_size_override("font_size", 12)
	lbl_inspector_clinical_hint.add_theme_color_override("font_color", Color(0.4, 0.8, 1.0, 0.85))
	lbl_inspector_clinical_hint.visible = false
	container_inspector_part.add_child(lbl_inspector_clinical_hint)

	# Instanciación interna para compatibilidad con tests (no se añade al árbol visible)
	hbox_clinical_actions = GridContainer.new()
	hbox_clinical_actions.visible = false

	btn_action_bandage = Button.new()
	btn_action_bandage.text = "🩹 Vendar"
	btn_action_bandage.pressed.connect(_on_action_bandage_pressed)
	hbox_clinical_actions.add_child(btn_action_bandage)

	btn_action_tourniquet = Button.new()
	btn_action_tourniquet.text = "🩸 Torniquete"
	btn_action_tourniquet.pressed.connect(_on_action_tourniquet_pressed)
	hbox_clinical_actions.add_child(btn_action_tourniquet)

	btn_action_splint = Button.new()
	btn_action_splint.text = "🪵 Entablillar"
	btn_action_splint.pressed.connect(_on_action_splint_pressed)
	hbox_clinical_actions.add_child(btn_action_splint)

	btn_action_disinfect = Button.new()
	btn_action_disinfect.text = "🧼 Desinfectar"
	btn_action_disinfect.pressed.connect(_on_action_disinfect_pressed)
	hbox_clinical_actions.add_child(btn_action_disinfect)

	btn_action_heal = Button.new()
	btn_action_heal.text = "✨ Sanar"
	btn_action_heal.pressed.connect(_on_action_heal_pressed)
	hbox_clinical_actions.add_child(btn_action_heal)

	btn_action_wound_test = Button.new()
	btn_action_wound_test.text = "⚠️ Herir"
	btn_action_wound_test.pressed.connect(_on_action_wound_test_pressed)
	hbox_clinical_actions.add_child(btn_action_wound_test)

	lbl_layers_title = Label.new()
	lbl_layers_title.text = "👗 Equipamiento en esta Zona:"
	lbl_layers_title.add_theme_font_size_override("font_size", 13)
	lbl_layers_title.add_theme_color_override("font_color", Color(0.7, 0.85, 1.0))
	lbl_layers_title.visible = false
	container_inspector_part.add_child(lbl_layers_title)

	container_equipped_items_list = VBoxContainer.new()
	container_equipped_items_list.add_theme_constant_override("separation", 4)
	container_inspector_part.add_child(container_equipped_items_list)

	# Selector interactivo para equipar prendas en esta zona
	var hbox_new_equip = HBoxContainer.new()
	hbox_new_equip.add_theme_constant_override("separation", 6)
	container_inspector_part.add_child(hbox_new_equip)

	option_equip_part_item = OptionButton.new()
	option_equip_part_item.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	option_equip_part_item.add_theme_font_size_override("font_size", 12)
	option_equip_part_item.item_selected.connect(func(idx: int):
		btn_equip_selected_part_item.disabled = (idx <= 0)
	)
	hbox_new_equip.add_child(option_equip_part_item)

	btn_equip_selected_part_item = Button.new()
	btn_equip_selected_part_item.text = "➕ Equipar"
	btn_equip_selected_part_item.add_theme_font_size_override("font_size", 12)
	btn_equip_selected_part_item.disabled = true
	btn_equip_selected_part_item.pressed.connect(_on_equip_selected_part_item_pressed)
	hbox_new_equip.add_child(btn_equip_selected_part_item)

	container_layer_dropdowns = VBoxContainer.new()
	container_layer_dropdowns.visible = false
	container_inspector_part.add_child(container_layer_dropdowns)

	container_inspector_backpack = VBoxContainer.new()
	container_inspector_backpack.add_theme_constant_override("separation", 6)
	container_inspector_backpack.visible = false
	root_vbox.add_child(container_inspector_backpack)

	var sep_bp = HSeparator.new()
	container_inspector_backpack.add_child(sep_bp)

	var hbox_bp_header = HBoxContainer.new()
	container_inspector_backpack.add_child(hbox_bp_header)

	var lbl_bp_title = Label.new()
	lbl_bp_title.text = "🎒 MOCHILA Y ALMACENAMIENTO (Espalda)"
	lbl_bp_title.add_theme_font_size_override("font_size", 13)
	lbl_bp_title.add_theme_color_override("font_color", Color(1.0, 0.85, 0.4))
	lbl_bp_title.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_bp_header.add_child(lbl_bp_title)

	var btn_close_bp = Button.new()
	btn_close_bp.text = "✖ Ocultar Mochila"
	btn_close_bp.add_theme_font_size_override("font_size", 12)
	btn_close_bp.pressed.connect(func():
		is_inspecting_backpack = false
		if paperdoll:
			paperdoll.is_backpack_selected = false
			paperdoll.queue_redraw()
		_refresh_contextual_inspector()
	)
	hbox_bp_header.add_child(btn_close_bp)

	var hbox_bp_select = HBoxContainer.new()
	container_inspector_backpack.add_child(hbox_bp_select)

	var lbl_bp_model = Label.new()
	lbl_bp_model.text = "Modelo:"
	lbl_bp_model.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_bp_select.add_child(lbl_bp_model)

	option_backpack_model = OptionButton.new()
	option_backpack_model.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	option_backpack_model.item_selected.connect(_on_backpack_model_selected)
	hbox_bp_select.add_child(option_backpack_model)

	lbl_backpack_weight_info = Label.new()
	lbl_backpack_weight_info.text = "Carga interna: 0.0 kg / 0.0 kg máx"
	lbl_backpack_weight_info.add_theme_font_size_override("font_size", 12)
	container_inspector_backpack.add_child(lbl_backpack_weight_info)

	slider_backpack_content_weight = HSlider.new()
	slider_backpack_content_weight.min_value = 0.0
	slider_backpack_content_weight.max_value = 25.0
	slider_backpack_content_weight.step = 0.5
	slider_backpack_content_weight.value_changed.connect(_on_backpack_content_slider_changed)
	container_inspector_backpack.add_child(slider_backpack_content_weight)

	lbl_backpack_mass_breakdown = Label.new()
	lbl_backpack_mass_breakdown.text = "Vacía: 0.0 kg + Carga: 0.0 kg = Total: 0.0 kg"
	lbl_backpack_mass_breakdown.add_theme_font_size_override("font_size", 12)
	lbl_backpack_mass_breakdown.add_theme_color_override("font_color", Color(0.4, 1.0, 0.6))
	container_inspector_backpack.add_child(lbl_backpack_mass_breakdown)

# -----------------------------------------------------------------------------
# NIVEL 4: FICHA TÉCNICA DEL ÍTEM
# -----------------------------------------------------------------------------
func _build_tier4_item_card(parent: Control) -> void:
	panel_item_card = PanelContainer.new()
	panel_item_card.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	panel_item_card.visible = false
	var card_style = StyleBoxFlat.new()
	card_style.bg_color = Color(0.08, 0.11, 0.15, 0.95)
	card_style.border_color = Color(0.3, 0.45, 0.6, 0.8)
	card_style.set_border_width_all(1)
	card_style.set_corner_radius_all(6)
	card_style.content_margin_left = 10
	card_style.content_margin_right = 10
	card_style.content_margin_top = 8
	card_style.content_margin_bottom = 8
	panel_item_card.add_theme_stylebox_override("panel", card_style)
	parent.add_child(panel_item_card)

	var vbox = VBoxContainer.new()
	vbox.add_theme_constant_override("separation", 4)
	panel_item_card.add_child(vbox)

	lbl_item_card_title = Label.new()
	lbl_item_card_title.text = "📋 Ficha Técnica: Ningún ítem seleccionado"
	lbl_item_card_title.add_theme_font_size_override("font_size", 13)
	lbl_item_card_title.add_theme_color_override("font_color", Color(0.75, 0.9, 1.0))
	vbox.add_child(lbl_item_card_title)

	lbl_item_card_specs = Label.new()
	lbl_item_card_specs.text = "Aislamiento: 0.0 clo | Peso Seco: 0.0 kg | Viento: 0%"
	lbl_item_card_specs.add_theme_font_size_override("font_size", 12)
	lbl_item_card_specs.add_theme_color_override("font_color", Color(0.85, 0.85, 0.85))
	vbox.add_child(lbl_item_card_specs)

	var hbox_water = HBoxContainer.new()
	vbox.add_child(hbox_water)

	var lbl_water_title = Label.new()
	lbl_water_title.text = "💧 Agua Absorbida:"
	lbl_water_title.add_theme_font_size_override("font_size", 12)
	hbox_water.add_child(lbl_water_title)

	bar_item_water_absorbed = ProgressBar.new()
	bar_item_water_absorbed.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	bar_item_water_absorbed.max_value = 1.0
	bar_item_water_absorbed.value = 0.0
	bar_item_water_absorbed.show_percentage = false
	hbox_water.add_child(bar_item_water_absorbed)

	lbl_item_water_val = Label.new()
	lbl_item_water_val.text = "0.00 / 0.00 kg (0%)"
	lbl_item_water_val.add_theme_font_size_override("font_size", 12)
	lbl_item_water_val.add_theme_color_override("font_color", Color(0.5, 0.85, 1.0))
	hbox_water.add_child(lbl_item_water_val)

	lbl_item_covered_badges = Label.new()
	lbl_item_covered_badges.text = "Zonas Cubiertas: Ninguna"
	lbl_item_covered_badges.add_theme_font_size_override("font_size", 12)
	lbl_item_covered_badges.add_theme_color_override("font_color", Color(0.4, 0.9, 0.7))
	vbox.add_child(lbl_item_covered_badges)

	# Botón de desequipar la prenda
	btn_unequip_item_card = Button.new()
	btn_unequip_item_card.text = "❌ Desequipar esta Prenda"
	btn_unequip_item_card.add_theme_font_size_override("font_size", 12)
	btn_unequip_item_card.visible = false
	btn_unequip_item_card.pressed.connect(_on_unequip_item_card_pressed)
	vbox.add_child(btn_unequip_item_card)

	# Sección de Almacenamiento y Carga (sólo visible para bolsos/mochilas)
	container_item_card_cargo = VBoxContainer.new()
	container_item_card_cargo.add_theme_constant_override("separation", 4)
	container_item_card_cargo.visible = false
	vbox.add_child(container_item_card_cargo)

	var sep_cargo = HSeparator.new()
	container_item_card_cargo.add_child(sep_cargo)

	lbl_item_card_cargo_title = Label.new()
	lbl_item_card_cargo_title.text = "🎒 Almacenamiento y Carga Interna:"
	lbl_item_card_cargo_title.add_theme_font_size_override("font_size", 12)
	lbl_item_card_cargo_title.add_theme_color_override("font_color", Color(1.0, 0.85, 0.4))
	container_item_card_cargo.add_child(lbl_item_card_cargo_title)

	var hbox_cargo_slider = HBoxContainer.new()
	container_item_card_cargo.add_child(hbox_cargo_slider)

	lbl_item_card_cargo_info = Label.new()
	lbl_item_card_cargo_info.text = "Carga: 0.0 kg / 0.0 kg máx"
	lbl_item_card_cargo_info.add_theme_font_size_override("font_size", 12)
	lbl_item_card_cargo_info.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_cargo_slider.add_child(lbl_item_card_cargo_info)

	slider_item_card_cargo = HSlider.new()
	slider_item_card_cargo.min_value = 0.0
	slider_item_card_cargo.max_value = 25.0
	slider_item_card_cargo.step = 0.5
	slider_item_card_cargo.value = 0.0
	slider_item_card_cargo.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	slider_item_card_cargo.value_changed.connect(_on_item_card_cargo_slider_changed)
	hbox_cargo_slider.add_child(slider_item_card_cargo)

	lbl_item_card_cargo_breakdown = Label.new()
	lbl_item_card_cargo_breakdown.text = "📦 Contenido: Insumos Médicos (40%) • Cantimplora/Raciones (35%) • Equipo (25%)"
	lbl_item_card_cargo_breakdown.add_theme_font_size_override("font_size", 12)
	lbl_item_card_cargo_breakdown.add_theme_color_override("font_color", Color(0.7, 0.85, 0.95))
	lbl_item_card_cargo_breakdown.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	container_item_card_cargo.add_child(lbl_item_card_cargo_breakdown)

	lbl_item_card_cargo_impact = Label.new()
	lbl_item_card_cargo_impact.text = "⚖️ Impacto: +0.0% costo metabólico | Carga lumbar: 0.0 kg"
	lbl_item_card_cargo_impact.add_theme_font_size_override("font_size", 12)
	lbl_item_card_cargo_impact.add_theme_color_override("font_color", Color(0.4, 1.0, 0.6))
	container_item_card_cargo.add_child(lbl_item_card_cargo_impact)

	# Sección 3D Asset Template Bridge (MakeHuman Rigging)
	var sep = HSeparator.new()
	vbox.add_child(sep)

	var lbl_tpl_header = Label.new()
	lbl_tpl_header.text = "🧩 Template y Asset 3D (MakeHuman Rig Bridge):"
	lbl_tpl_header.add_theme_font_size_override("font_size", 12)
	lbl_tpl_header.add_theme_color_override("font_color", Color(0.95, 0.80, 0.45))
	vbox.add_child(lbl_tpl_header)

	lbl_item_socket_bone = Label.new()
	lbl_item_socket_bone.text = "Hueso Socket: Ninguno | Overlay 2D: Ninguno"
	lbl_item_socket_bone.add_theme_font_size_override("font_size", 12)
	lbl_item_socket_bone.add_theme_color_override("font_color", Color(0.7, 0.85, 1.0))
	vbox.add_child(lbl_item_socket_bone)

	var hbox_asset = HBoxContainer.new()
	hbox_asset.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	vbox.add_child(hbox_asset)

	line_item_mesh_path = LineEdit.new()
	line_item_mesh_path.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	line_item_mesh_path.placeholder_text = "Ruta Mesh 3D (.glb, .obj)..."
	line_item_mesh_path.add_theme_font_size_override("font_size", 12)
	hbox_asset.add_child(line_item_mesh_path)

	btn_attach_3d_asset = Button.new()
	btn_attach_3d_asset.text = "📁 Adjuntar a Skeleton"
	btn_attach_3d_asset.add_theme_font_size_override("font_size", 12)
	btn_attach_3d_asset.pressed.connect(_on_attach_3d_asset_pressed)
	hbox_asset.add_child(btn_attach_3d_asset)

	lbl_item_asset_status = Label.new()
	lbl_item_asset_status.text = "Estado: Selecciona un ítem o prenda para configurar su template 3D."
	lbl_item_asset_status.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	lbl_item_asset_status.add_theme_font_size_override("font_size", 12)
	lbl_item_asset_status.add_theme_color_override("font_color", Color(0.5, 0.9, 0.6))
	vbox.add_child(lbl_item_asset_status)

# -----------------------------------------------------------------------------
# CONSTRUCCIÓN DE LAS TARJETAS DEL PANEL DERECHO
# -----------------------------------------------------------------------------
func _build_right_dock_cards(parent: Control) -> void:
	_build_card_clock(parent)
	_build_card_environment_terrains(parent)
	_build_card_anim_testbench(parent)

func _build_card_clock(parent: Control) -> void:
	var panel = _create_card_container("⏱️ RELOJ Y CALENDARIO DE SIMULACIÓN", parent)
	var vbox = panel.get_child(0)

	lbl_clock_display = Label.new()
	lbl_clock_display.text = "Día 01 — 08h 00m 00s"
	lbl_clock_display.add_theme_font_size_override("font_size", 15)
	lbl_clock_display.add_theme_color_override("font_color", Color(0.4, 1.0, 0.6))
	vbox.add_child(lbl_clock_display)

	var hbox_mode = HBoxContainer.new()
	vbox.add_child(hbox_mode)

	var lbl_mode = Label.new()
	lbl_mode.text = "Formato:"
	lbl_mode.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_mode.add_child(lbl_mode)

	option_clock_mode = OptionButton.new()
	option_clock_mode.add_item("Automático (Día / Año-Mes)", SimulationClockClass.DisplayMode.MODE_AUTO)
	option_clock_mode.add_item("Solo Días Continuos", SimulationClockClass.DisplayMode.MODE_DAYS_ONLY)
	option_clock_mode.add_item("Calendario Extendido", SimulationClockClass.DisplayMode.MODE_FULL_CALENDAR)
	option_clock_mode.add_item("Tiempo Total H:M:S", SimulationClockClass.DisplayMode.MODE_TOTAL_HMS)
	option_clock_mode.item_selected.connect(func(idx: int):
		var mode_id = option_clock_mode.get_item_id(idx)
		clock.current_mode = mode_id as SimulationClockClass.DisplayMode
		_update_clock_display()
	)
	hbox_mode.add_child(option_clock_mode)

	var hbox_speed = HBoxContainer.new()
	vbox.add_child(hbox_speed)

	var lbl_speed = Label.new()
	lbl_speed.text = "Velocidad:"
	lbl_speed.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_speed.add_child(lbl_speed)

	var lbl_speed_val = Label.new()
	lbl_speed_val.text = "1.0x"
	hbox_speed.add_child(lbl_speed_val)

	var s_speed = HSlider.new()
	s_speed.min_value = 1.0
	s_speed.max_value = 200.0
	s_speed.step = 1.0
	s_speed.value = 1.0
	s_speed.value_changed.connect(func(val: float):
		if character: character.simulation_speed = val
		lbl_speed_val.text = "%.1fx" % val
		if label_time_scale_val: label_time_scale_val.text = "%.1fx" % val
	)
	vbox.add_child(s_speed)

	btn_clock_pause = Button.new()
	btn_clock_pause.text = "⏸️ Pausar Simulación"
	btn_clock_pause.pressed.connect(func():
		clock.is_paused = !clock.is_paused
		btn_clock_pause.text = "▶️ Reanudar Simulación" if clock.is_paused else "⏸️ Pausar Simulación"
	)
	vbox.add_child(btn_clock_pause)

func _build_card_environment_terrains(parent: Control) -> void:
	var panel = _create_card_container("🌤️ ENTORNO, PENDIENTE Y TERRENO", parent)
	var vbox = panel.get_child(0)

	var hbox_t = HBoxContainer.new()
	vbox.add_child(hbox_t)
	var lbl_t = Label.new()
	lbl_t.text = "Temperatura Ambiente:"
	lbl_t.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_t.add_child(lbl_t)
	var lbl_t_val = Label.new()
	lbl_t_val.text = "22°C"
	hbox_t.add_child(lbl_t_val)

	var s_temp = HSlider.new()
	s_temp.min_value = -40.0
	s_temp.max_value = 50.0
	s_temp.step = 1.0
	s_temp.value = 22.0
	s_temp.value_changed.connect(func(val: float):
		if character and character.body:
			character.body.environment.ambient_temperature = val
			character.body.thermo.ambient_temperature = val
		_update_ambient_temp_label(val)
		lbl_t_val.text = "%d°C" % int(val)
		_on_telemetry_updated()
	)
	vbox.add_child(s_temp)

	var hbox_slope = HBoxContainer.new()
	vbox.add_child(hbox_slope)
	var lbl_slope = Label.new()
	lbl_slope.text = "Inclinación del Terreno:"
	lbl_slope.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_slope.add_child(lbl_slope)
	lbl_env_slope_val = Label.new()
	lbl_env_slope_val.text = "0° (Plano)"
	hbox_slope.add_child(lbl_env_slope_val)

	slider_env_slope = HSlider.new()
	slider_env_slope.min_value = -30.0
	slider_env_slope.max_value = 30.0
	slider_env_slope.step = 1.0
	slider_env_slope.value = 0.0
	slider_env_slope.value_changed.connect(_on_env_slope_changed)
	vbox.add_child(slider_env_slope)

	var hbox_sub = HBoxContainer.new()
	vbox.add_child(hbox_sub)
	var lbl_sub = Label.new()
	lbl_sub.text = "Sustrato / Suelo:"
	lbl_sub.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_sub.add_child(lbl_sub)

	option_env_substrate = OptionButton.new()
	option_env_substrate.add_item("Asfalto / Firme (Base)", EnvironmentSystemClass.SubstrateType.FIRM_GROUND)
	option_env_substrate.add_item("Tierra / Césped", EnvironmentSystemClass.SubstrateType.DIRT_GRASS)
	option_env_substrate.add_item("Arena Suelta (x2.1 costo)", EnvironmentSystemClass.SubstrateType.LOOSE_SAND)
	option_env_substrate.add_item("Barro Espeso (+1.8 kg pies)", EnvironmentSystemClass.SubstrateType.THICK_MUD)
	option_env_substrate.add_item("Vadeo en Agua (x25 frío)", EnvironmentSystemClass.SubstrateType.WATER_WADING)
	option_env_substrate.add_item("Nieve Profunda", EnvironmentSystemClass.SubstrateType.DEEP_SNOW)
	option_env_substrate.item_selected.connect(_on_env_substrate_selected)
	hbox_sub.add_child(option_env_substrate)

	lbl_substrate_specs = Label.new()
	lbl_substrate_specs.text = "📊 Costo: x1.0 | Tracción: 100% | Ruido: 30% | Riesgo: 0.0%"
	lbl_substrate_specs.add_theme_font_size_override("font_size", 12)
	lbl_substrate_specs.add_theme_color_override("font_color", Color(0.4, 0.9, 0.8))
	vbox.add_child(lbl_substrate_specs)

	var hbox_veg = HBoxContainer.new()
	vbox.add_child(hbox_veg)
	var lbl_veg = Label.new()
	lbl_veg.text = "Vegetación / Obstáculos:"
	lbl_veg.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_veg.add_child(lbl_veg)

	option_env_vegetation = OptionButton.new()
	option_env_vegetation.add_item("Sendero Despejado", EnvironmentSystemClass.VegetationDensity.CLEAR)
	option_env_vegetation.add_item("Pastizal Alto", EnvironmentSystemClass.VegetationDensity.TALL_GRASS)
	option_env_vegetation.add_item("Matorral con Espinas", EnvironmentSystemClass.VegetationDensity.THORNY_BRUSH)
	option_env_vegetation.add_item("Pedregal Inestable", EnvironmentSystemClass.VegetationDensity.SCREE_ROCKS)
	option_env_vegetation.item_selected.connect(_on_env_vegetation_selected)
	hbox_veg.add_child(option_env_vegetation)

func _build_card_posture_and_effort(parent: Control) -> void:
	var panel = _create_card_container("🧘 POSTURA Y ESFUERZO FÍSICO", parent)
	var vbox = panel.get_child(0)

	# 1. Indicador en vivo de postura y consecuencias gravitatorias/masa
	var panel_posture_display = PanelContainer.new()
	var disp_style = StyleBoxFlat.new()
	disp_style.bg_color = Color(0.04, 0.06, 0.09, 0.95)
	disp_style.border_color = Color(0.3, 0.45, 0.65, 0.8)
	disp_style.set_border_width_all(1)
	disp_style.set_corner_radius_all(4)
	disp_style.content_margin_left = 8
	disp_style.content_margin_right = 8
	disp_style.content_margin_top = 6
	disp_style.content_margin_bottom = 6
	panel_posture_display.add_theme_stylebox_override("panel", disp_style)
	vbox.add_child(panel_posture_display)

	lbl_posture_display = Label.new()
	lbl_posture_display.text = "🧍 De pie (00s) | Carga soportada: 75.0 kg"
	lbl_posture_display.add_theme_font_size_override("font_size", 12)
	lbl_posture_display.add_theme_color_override("font_color", Color(0.4, 1.0, 0.65))
	lbl_posture_display.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	panel_posture_display.add_child(lbl_posture_display)

	# 2. Selector de postura rápida (Botones de Estado)
	var grid_postures = GridContainer.new()
	grid_postures.columns = 4
	grid_postures.add_theme_constant_override("h_separation", 4)
	grid_postures.add_theme_constant_override("v_separation", 4)
	vbox.add_child(grid_postures)

	btn_posture_stand = Button.new()
	btn_posture_stand.text = "🧍 De Pie"
	btn_posture_stand.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	btn_posture_stand.pressed.connect(func(): _set_posture_from_ui(BodySystemClass.Posture.STANDING))
	grid_postures.add_child(btn_posture_stand)

	btn_posture_sit = Button.new()
	btn_posture_sit.text = "🪑 Sentarse"
	btn_posture_sit.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	btn_posture_sit.pressed.connect(func(): _set_posture_from_ui(BodySystemClass.Posture.SITTING))
	grid_postures.add_child(btn_posture_sit)

	btn_posture_lie = Button.new()
	btn_posture_lie.text = "🛏️ Acostarse"
	btn_posture_lie.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	btn_posture_lie.pressed.connect(func(): _set_posture_from_ui(BodySystemClass.Posture.LYING))
	grid_postures.add_child(btn_posture_lie)

	btn_posture_crouch = Button.new()
	btn_posture_crouch.text = "🧎 Agacharse"
	btn_posture_crouch.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	btn_posture_crouch.pressed.connect(func(): _set_posture_from_ui(BodySystemClass.Posture.CROUCHING))
	grid_postures.add_child(btn_posture_crouch)

	# 3. Slider de Esfuerzo Locomotor
	var hbox_e = HBoxContainer.new()
	vbox.add_child(hbox_e)
	var lbl_e = Label.new()
	lbl_e.text = "Esfuerzo de Marcha:"
	lbl_e.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_e.add_child(lbl_e)
	label_exertion_val = Label.new()
	label_exertion_val.name = "LabelExertionVal"
	label_exertion_val.text = "0%"
	hbox_e.add_child(label_exertion_val)

	var s_unused = HSlider.new()
	s_unused.min_value = 0.0
	s_unused.max_value = 100.0
	vbox.add_child(s_unused)

	# 4. Toggle de Escalada
	btn_toggle_climb = Button.new()
	btn_toggle_climb.text = "🧗 Acción: Iniciar Escalada"
	btn_toggle_climb.pressed.connect(_on_toggle_climb_pressed)
	vbox.add_child(btn_toggle_climb)

func _set_posture_from_ui(posture_idx: int) -> void:
	var p_name = "De Pie"
	match posture_idx:
		BodySystemClass.Posture.SITTING: p_name = "Sentado"
		BodySystemClass.Posture.LYING: p_name = "Acostado"
		BodySystemClass.Posture.CROUCHING: p_name = "Agachado"
	last_event_text = "Cambio a %s" % p_name
	last_event_time = 0.0
	if character:
		character.set_character_posture(posture_idx)
	elif character and character.body:
		character.body.set_posture(posture_idx as BodySystemClass.Posture)

	if posture_idx == BodySystemClass.Posture.SITTING or posture_idx == BodySystemClass.Posture.LYING:
		current_action_name = "Reposo (" + p_name + ")"
		current_action_time = 0.0
		_is_updating_ui = true
		if slider_exertion: slider_exertion.value = 0.0
		if slider_workplace_exertion: slider_workplace_exertion.value = 0.0
		_is_updating_ui = false
		_update_exertion_label(0.0)
		_highlight_anim_button("Idle")
	elif posture_idx == BodySystemClass.Posture.CROUCHING:
		if slider_workplace_exertion and slider_workplace_exertion.value > 35.0:
			_on_exertion_slider_changed(35.0)
		else:
			current_action_name = "Reposo (Agachado)"
			current_action_time = 0.0
	_update_posture_display()
	_on_telemetry_updated()

func _update_posture_display() -> void:
	if not lbl_posture_display or not character or not character.body:
		return
	var b = character.body
	lbl_posture_display.text = b.get_posture_status_text()
	match b.current_posture:
		BodySystemClass.Posture.STANDING:
			if b.continuous_standing_seconds > 5400.0:
				lbl_posture_display.add_theme_color_override("font_color", Color(1.0, 0.7, 0.3))
			elif b.continuous_standing_seconds > 2700.0:
				lbl_posture_display.add_theme_color_override("font_color", Color(0.95, 0.85, 0.4))
			else:
				lbl_posture_display.add_theme_color_override("font_color", Color(0.4, 1.0, 0.65))
		BodySystemClass.Posture.SITTING:
			lbl_posture_display.add_theme_color_override("font_color", Color(0.35, 0.85, 1.0))
		BodySystemClass.Posture.LYING:
			lbl_posture_display.add_theme_color_override("font_color", Color(0.4, 0.7, 1.0))
		BodySystemClass.Posture.CROUCHING:
			lbl_posture_display.add_theme_color_override("font_color", Color(1.0, 0.85, 0.45))

	_update_posture_button_highlights(b.current_posture)

func _update_posture_button_highlights(active_posture: int) -> void:
	var cur_effort = slider_workplace_exertion.value if slider_workplace_exertion else 0.0
	var is_seated_or_lying = (active_posture == BodySystemClass.Posture.SITTING or active_posture == BodySystemClass.Posture.LYING)
	var is_crouching = (active_posture == BodySystemClass.Posture.CROUCHING)

	if btn_posture_stand:
		btn_posture_stand.disabled = (active_posture == BodySystemClass.Posture.STANDING)
		btn_posture_stand.modulate = Color(1.3, 1.3, 1.3) if active_posture == BodySystemClass.Posture.STANDING else Color(0.7, 0.7, 0.7)
	if btn_posture_sit:
		btn_posture_sit.disabled = (active_posture == BodySystemClass.Posture.SITTING or cur_effort > 35.0)
		btn_posture_sit.modulate = Color(1.3, 1.3, 1.3) if active_posture == BodySystemClass.Posture.SITTING else Color(0.7, 0.7, 0.7)
	if btn_posture_lie:
		btn_posture_lie.disabled = (active_posture == BodySystemClass.Posture.LYING or cur_effort > 35.0)
		btn_posture_lie.modulate = Color(1.3, 1.3, 1.3) if active_posture == BodySystemClass.Posture.LYING else Color(0.7, 0.7, 0.7)
	if btn_posture_crouch:
		btn_posture_crouch.disabled = (is_crouching or is_seated_or_lying or cur_effort > 35.0)
		btn_posture_crouch.modulate = Color(1.3, 1.3, 1.3) if is_crouching else Color(0.7, 0.7, 0.7)

	# Restricciones de locomoción según postura
	if btn_anim_walk:
		btn_anim_walk.disabled = is_seated_or_lying
	if btn_anim_run:
		btn_anim_run.disabled = is_seated_or_lying or is_crouching
	if btn_anim_sprint:
		btn_anim_sprint.disabled = is_seated_or_lying or is_crouching
	if btn_anim_idle:
		btn_anim_idle.disabled = false

	if slider_workplace_exertion:
		if is_seated_or_lying:
			slider_workplace_exertion.editable = false
			if slider_workplace_exertion.value != 0.0:
				slider_workplace_exertion.value = 0.0
		elif is_crouching:
			slider_workplace_exertion.editable = true
			slider_workplace_exertion.max_value = 35.0
			if slider_workplace_exertion.value > 35.0:
				slider_workplace_exertion.value = 35.0
		else: # STANDING
			slider_workplace_exertion.editable = true
			slider_workplace_exertion.max_value = 100.0

func _build_card_anim_testbench(parent: Control) -> void:
	var panel = _create_card_container("🎬 BANCO DE ANIMACIÓN 3D (RIG)", parent)
	var vbox = panel.get_child(0)

	chk_decoupled_anim = CheckBox.new()
	chk_decoupled_anim.text = "Desacoplar animaciones de simulación"
	chk_decoupled_anim.toggled.connect(func(btn_pressed: bool):
		is_anim_decoupled = btn_pressed
	)
	vbox.add_child(chk_decoupled_anim)

	var grid = GridContainer.new()
	grid.columns = 3
	grid.add_theme_constant_override("h_separation", 4)
	grid.add_theme_constant_override("v_separation", 4)
	vbox.add_child(grid)

	var b_idle = Button.new()
	b_idle.text = "🧘 Reposo"
	b_idle.pressed.connect(func():
		if character: character.set_character_posture(BodySystemClass.Posture.STANDING)
		_apply_effort(0.0, "Idle")
	)
	grid.add_child(b_idle)

	var b_walk = Button.new()
	b_walk.text = "🚶 Caminar"
	b_walk.pressed.connect(func(): _apply_effort(20.0, "Walking"))
	grid.add_child(b_walk)

	var b_run = Button.new()
	b_run.text = "🏃 Carrera"
	b_run.pressed.connect(func(): _apply_effort(55.0, "Running"))
	grid.add_child(b_run)

	var b_sprint = Button.new()
	b_sprint.text = "⚡ Sprint"
	b_sprint.pressed.connect(func(): _apply_effort(100.0, "Sprint"))
	grid.add_child(b_sprint)

	var b_crouch = Button.new()
	b_crouch.text = "🧎 Agacharse"
	b_crouch.pressed.connect(_toggle_crouch)
	grid.add_child(b_crouch)

	var b_reset = Button.new()
	b_reset.text = "🔄 Postura Base"
	b_reset.pressed.connect(func():
		if character: character.play_idle()
		_apply_effort(0.0, "Idle")
	)
	grid.add_child(b_reset)

	var b_flat = Button.new()
	b_flat.text = "💀 Caer Seco"
	b_flat.pressed.connect(func():
		if character:
			character.set_character_posture(BodySystemClass.Posture.LYING)
	)
	grid.add_child(b_flat)

	var b_bwd = Button.new()
	b_bwd.text = "💀 Caer Atrás"
	b_bwd.pressed.connect(func(): if character: character.play_death_bwd())
	grid.add_child(b_bwd)

func _style_progress_bar(bar: ProgressBar, fill_color: Color, bg_color: Color = Color(0.10, 0.14, 0.18, 0.95)) -> void:
	bar.custom_minimum_size = Vector2(0, 10)
	bar.show_percentage = false
	var bg_style = StyleBoxFlat.new()
	bg_style.bg_color = bg_color
	bg_style.set_corner_radius_all(3)
	bar.add_theme_stylebox_override("background", bg_style)
	var fill_style = StyleBoxFlat.new()
	fill_style.bg_color = fill_color
	fill_style.set_corner_radius_all(3)
	bar.add_theme_stylebox_override("fill", fill_style)

func _build_card_physiology_telemetry(_parent: Control) -> void:
	pass

func _create_card_container(title_text: String, parent: Control) -> PanelContainer:
	var panel = PanelContainer.new()
	var style = StyleBoxFlat.new()
	style.bg_color = Color(0.08, 0.10, 0.14, 0.90)
	style.border_color = Color(0.25, 0.35, 0.45, 0.7)
	style.set_border_width_all(1)
	style.set_corner_radius_all(6)
	style.content_margin_left = 10
	style.content_margin_right = 10
	style.content_margin_top = 8
	style.content_margin_bottom = 8
	panel.add_theme_stylebox_override("panel", style)
	parent.add_child(panel)

	var vbox = VBoxContainer.new()
	vbox.add_theme_constant_override("separation", 6)
	panel.add_child(vbox)

	var lbl_title = Label.new()
	lbl_title.text = title_text
	lbl_title.add_theme_font_size_override("font_size", 13)
	lbl_title.add_theme_color_override("font_color", Color(1.0, 0.85, 0.4))
	vbox.add_child(lbl_title)

	return panel

# -----------------------------------------------------------------------------
# GESTIÓN DEL INSPECTOR CONTEXTUAL (NIVEL 3 Y NIVEL 4)
# -----------------------------------------------------------------------------
func _on_paperdoll_part_selected(part_type: int) -> void:
	_menu_target_part = -1
	if context_menu:
		context_menu.hide()
	active_inspected_part = part_type
	active_inspected_item_id = ""
	_refresh_contextual_inspector()
	_on_telemetry_updated()

func _on_selected_part_actions_pressed() -> void:
	if not character or not character.body or not character.body.body_parts.has(active_inspected_part):
		return
	var button_rect = btn_selected_part_actions.get_global_rect()
	_on_paperdoll_context_menu(active_inspected_part, button_rect.position + Vector2(0, button_rect.size.y))

func _update_part_selection_controls() -> void:
	if not lbl_part_selection or not btn_selected_part_actions:
		return
	var part: BodyPartClass = character.body.body_parts.get(active_inspected_part) if character and character.body else null
	btn_selected_part_actions.disabled = part == null
	if part:
		lbl_part_selection.text = "Zona seleccionada: %s" % part.display_name
		btn_selected_part_actions.tooltip_text = "Abrir acciones sobre %s. También disponible con clic derecho." % part.display_name
	else:
		lbl_part_selection.text = "Seleccioná una zona del cuerpo para inspeccionarla."
		btn_selected_part_actions.tooltip_text = "Primero seleccioná una zona del cuerpo."

func _on_paperdoll_part_hovered(_part_type: int) -> void:
	pass

func _on_paperdoll_backpack_selected() -> void:
	is_inspecting_backpack = !is_inspecting_backpack
	if paperdoll:
		paperdoll.is_backpack_selected = is_inspecting_backpack
		if paperdoll.has_method("queue_redraw"): paperdoll.queue_redraw()
	_refresh_contextual_inspector()
	_on_telemetry_updated()

func _on_paperdoll_context_menu(part_type: int, global_pos: Vector2) -> void:
	_menu_target_part = -1
	if part_type < 0:
		if context_menu and context_menu.visible:
			context_menu.hide()
		return

	if not character or not character.body:
		return

	var b: BodySystemClass = character.body
	var part: BodyPartClass = b.body_parts.get(part_type)
	if not part:
		return

	_menu_target_part = part_type
	context_menu.set_item_text(0, part.display_name)
	var bpm = b.cardio.heart_rate_bpm if b.cardio else 75.0
	var bleed = part.get_current_bleed_rate(bpm)
	var has_bleed = bleed > 0.01
	var is_limb = part.can_be_amputated

	# 1. ACTUALIZAR SUBMENU PRIMEROS AUXILIOS CON LÓGICA CLÍNICA ESTRICTA
	if submenu_aid:
		submenu_aid.clear()

		# --- VENDAJE ---
		if part.is_bandaged:
			submenu_aid.add_item("🩹 Retirar Vendaje Compresivo", 0)
		elif not has_bleed and part.integrity >= 100.0:
			submenu_aid.add_item("🩹 Vendar (🚫 Tejido Sano sin Lesión)", 0)
			submenu_aid.set_item_disabled(submenu_aid.get_item_count() - 1, true)
		elif not part.is_disinfected and (part.infection_level > 10.0 or part.active_wound_type == BodyPartClass.WoundType.PUNCTURE_BITE):
			submenu_aid.add_item("🩹 Vendar (⚠️ Herida Séptica - Desinfectar Antes)", 0)
		else:
			submenu_aid.add_item("🩹 Aplicar Vendaje Compresivo Estéril", 0)

		# --- TORNIQUETE ---
		if part.is_tourniquet_applied:
			submenu_aid.add_item("🩸 Retirar Torniquete Táctico", 1)
		elif not is_limb:
			submenu_aid.add_item("🩸 Torniquete (🚫 Inaplicable en Tronco / Cabeza)", 1)
			submenu_aid.set_item_disabled(submenu_aid.get_item_count() - 1, true)
		elif not has_bleed:
			submenu_aid.add_item("🩸 Torniquete (🚫 Requiere Hemorragia Activa)", 1)
			submenu_aid.set_item_disabled(submenu_aid.get_item_count() - 1, true)
		else:
			submenu_aid.add_item("🩸 Aplicar Torniquete Táctico de Oclusión", 1)

		# --- ENTABLILLADO ---
		if part.is_splinted:
			submenu_aid.add_item("🪵 Retirar Férula / Entablillado", 2)
		elif not part.has_fracture:
			submenu_aid.add_item("🪵 Entablillar (🚫 Estructura Ósea Intacta)", 2)
			submenu_aid.set_item_disabled(submenu_aid.get_item_count() - 1, true)
		else:
			submenu_aid.add_item("🪵 Inmovilizar con Férula Rígida", 2)

		# --- DESINFECCIÓN ---
		if part.is_bandaged:
			submenu_aid.add_item("🧼 Desinfectar (🚫 Retirar Vendaje Primero)", 3)
			submenu_aid.set_item_disabled(submenu_aid.get_item_count() - 1, true)
		elif not has_bleed and part.infection_level <= 0.0 and part.is_disinfected:
			submenu_aid.add_item("🧼 Desinfectar (✅ Zona Aséptica y Limpia)", 3)
			submenu_aid.set_item_disabled(submenu_aid.get_item_count() - 1, true)
		else:
			submenu_aid.add_item("🧼 Desinfectar con Alcohol / Antiséptico", 3)

	# 2. ACTUALIZAR SUBMENU EQUIPAMIENTO Y CAPAS
	if submenu_equip:
		submenu_equip.clear()
		submenu_equip.add_item("🎯 Gestionar Ropa y Equipo en esta Zona...", 210)
		submenu_equip.add_separator()

		var current_items = part.equipped_clothing_list if part.equipped_clothing_list.size() > 0 else part.clothing_layers.values()
		if current_items.size() > 0:
			for idx in range(current_items.size()):
				var item = current_items[idx]
				var layer_idx = item.get("layer", -1)
				var layer_name = BodyPartClass.ClothingLayer.keys()[layer_idx].capitalize() if layer_idx >= 0 and layer_idx < BodyPartClass.ClothingLayer.size() else "Prenda"
				var item_name = item.get("name", "Prenda")
				submenu_equip.add_item("🧥 Ver [%s] %s" % [layer_name, item_name], 1000 + idx)
			submenu_equip.add_separator()

		if part_type == BodyPartClass.PartType.TORSO_UPPER:
			submenu_equip.add_item("🎒 Equipar Mochila Militar 45L (Arnés)", 200)
			submenu_equip.add_item("👝 Equipar Bolso Táctico Cruzado 8L", 201)
			submenu_equip.add_item("👕 Equipar Remera Básica", 202)
			submenu_equip.add_item("🦺 Equipar Chaleco Balístico Kevlar", 203)
			if b.equipped_backpack_id != "":
				submenu_equip.add_item("🚫 Desequipar Mochila / Bolso", 204)
		elif part_type == BodyPartClass.PartType.TORSO_LOWER or part_type == BodyPartClass.PartType.GROIN:
			submenu_equip.add_item("👝 Equipar Riñonera Táctica 3L", 205)
			submenu_equip.add_item("👖 Equipar Pantalón Cargo", 206)
		elif part_type == BodyPartClass.PartType.HEAD:
			submenu_equip.add_item("🪖 Equipar Casco Táctico", 207)
			submenu_equip.add_item("🧢 Equipar Gorro Térmico de Lana", 208)
		elif part_type == BodyPartClass.PartType.LEFT_FOOT or part_type == BodyPartClass.PartType.RIGHT_FOOT:
			submenu_equip.add_item("🥾 Equipar Botas Militares de Cuero", 209)
		elif part_type == BodyPartClass.PartType.LEFT_HAND or part_type == BodyPartClass.PartType.RIGHT_HAND:
			submenu_equip.add_item("🧤 Equipar Guantes Tácticos de Cuero", 211)
		elif part_type == BodyPartClass.PartType.NECK:
			submenu_equip.add_item("🧣 Equipar Bufanda Gruesa de Cuello", 212)

		submenu_equip.add_separator()
		submenu_equip.add_item("🧩 Configurar Template y Asset 3D...", 210)

	if context_menu:
		if context_menu.visible:
			context_menu.hide()
		context_menu.reset_size()
		context_menu.popup(Rect2i(Vector2i(global_pos), Vector2i.ZERO))

func _on_context_menu_action(id: int) -> void:
	match id:
		0: _on_action_bandage_pressed()
		1: _on_action_tourniquet_pressed()
		2: _on_action_splint_pressed()
		3: _on_action_disinfect_pressed()
		4: _on_action_heal_pressed()
		5: _on_action_wound_test_pressed()
		6:
			# Provocar Fractura Ósea
			if character and character.body:
				var target_id = _get_action_target_part_id()
				var part: BodyPartClass = character.body.body_parts.get(target_id)
				if part:
					part.has_fracture = true
					part.is_splinted = false
					part.pain_level = clamp(part.pain_level + 40.0, 0.0, 100.0)
					_on_telemetry_updated()
					_refresh_contextual_inspector()
		7:
			# Provocar Infección / Sepsis
			if character and character.body:
				var target_id = _get_action_target_part_id()
				var part: BodyPartClass = character.body.body_parts.get(target_id)
				if part:
					part.infection_level = clamp(part.infection_level + 45.0, 0.0, 100.0)
					part.is_disinfected = false
					_on_telemetry_updated()
					_refresh_contextual_inspector()
		10:
			# Examinar Detalles
			if _menu_target_part >= 0:
				active_inspected_part = _menu_target_part
				if paperdoll:
					paperdoll.select_part(active_inspected_part)
				_refresh_contextual_inspector()
				_on_telemetry_updated()
		20: _apply_wound_to_target(BodyPartClass.WoundType.SCRATCH)
		21: _apply_wound_to_target(BodyPartClass.WoundType.LACERATION)
		22: _apply_wound_to_target(BodyPartClass.WoundType.DEEP_CUT)
		23: _apply_wound_to_target(BodyPartClass.WoundType.PUNCTURE_BITE)
		24: _apply_wound_to_target(BodyPartClass.WoundType.ARTERIAL_RUPTURE)
		200:
			if character and character.body:
				if _menu_target_part >= 0:
					active_inspected_part = _menu_target_part
					if paperdoll: paperdoll.select_part(active_inspected_part)
				character.body.equip_backpack("backpack_military")
				selected_equipped_item_id = "backpack_military"
				_on_telemetry_updated()
				_refresh_contextual_inspector()
		201:
			if character and character.body:
				if _menu_target_part >= 0:
					active_inspected_part = _menu_target_part
					if paperdoll: paperdoll.select_part(active_inspected_part)
				character.body.equip_backpack("sidebag_tactical")
				selected_equipped_item_id = "sidebag_tactical"
				_on_telemetry_updated()
				_refresh_contextual_inspector()
		202:
			if character and character.body:
				if _menu_target_part >= 0:
					active_inspected_part = _menu_target_part
					if paperdoll: paperdoll.select_part(active_inspected_part)
				character.body.equip_item("tshirt_cotton")
				selected_equipped_item_id = "tshirt_cotton"
				_on_telemetry_updated()
				_refresh_contextual_inspector()
		203:
			if character and character.body:
				if _menu_target_part >= 0:
					active_inspected_part = _menu_target_part
					if paperdoll: paperdoll.select_part(active_inspected_part)
				character.body.equip_item("kevlar_vest")
				selected_equipped_item_id = "kevlar_vest"
				_on_telemetry_updated()
				_refresh_contextual_inspector()
		204:
			if character and character.body:
				if _menu_target_part >= 0:
					active_inspected_part = _menu_target_part
					if paperdoll: paperdoll.select_part(active_inspected_part)
				character.body.equipped_backpack_id = ""
				character.body.backpack_content_weight_kg = 0.0
				selected_equipped_item_id = ""
				_on_telemetry_updated()
				_refresh_contextual_inspector()
		205:
			if character and character.body:
				if _menu_target_part >= 0:
					active_inspected_part = _menu_target_part
					if paperdoll: paperdoll.select_part(active_inspected_part)
				character.body.equip_backpack("fanny_pack")
				selected_equipped_item_id = "fanny_pack"
				_on_telemetry_updated()
				_refresh_contextual_inspector()
		206:
			if character and character.body:
				if _menu_target_part >= 0:
					active_inspected_part = _menu_target_part
					if paperdoll: paperdoll.select_part(active_inspected_part)
				character.body.equip_item("cargo_pants")
				selected_equipped_item_id = "cargo_pants"
				_on_telemetry_updated()
				_refresh_contextual_inspector()
		207:
			if character and character.body:
				if _menu_target_part >= 0:
					active_inspected_part = _menu_target_part
					if paperdoll: paperdoll.select_part(active_inspected_part)
				character.body.equip_item("tactical_helmet")
				selected_equipped_item_id = "tactical_helmet"
				_on_telemetry_updated()
				_refresh_contextual_inspector()
		208:
			if character and character.body:
				if _menu_target_part >= 0:
					active_inspected_part = _menu_target_part
					if paperdoll: paperdoll.select_part(active_inspected_part)
				character.body.equip_item("wool_beanie")
				selected_equipped_item_id = "wool_beanie"
				_on_telemetry_updated()
				_refresh_contextual_inspector()
		209:
			if character and character.body:
				if _menu_target_part >= 0:
					active_inspected_part = _menu_target_part
					if paperdoll: paperdoll.select_part(active_inspected_part)
				character.body.equip_item("combat_boots")
				selected_equipped_item_id = "combat_boots"
				_on_telemetry_updated()
				_refresh_contextual_inspector()
		210:
			if _menu_target_part >= 0:
				active_inspected_part = _menu_target_part
				if paperdoll:
					paperdoll.select_part(active_inspected_part)
				_refresh_contextual_inspector()
				_on_telemetry_updated()
		211:
			if character and character.body:
				if _menu_target_part >= 0:
					active_inspected_part = _menu_target_part
					if paperdoll: paperdoll.select_part(active_inspected_part)
				character.body.equip_item("leather_gloves")
				selected_equipped_item_id = "leather_gloves"
				_on_telemetry_updated()
				_refresh_contextual_inspector()
		212:
			if character and character.body:
				if _menu_target_part >= 0:
					active_inspected_part = _menu_target_part
					if paperdoll: paperdoll.select_part(active_inspected_part)
				character.body.equip_item("wool_scarf")
				selected_equipped_item_id = "wool_scarf"
				_on_telemetry_updated()
				_refresh_contextual_inspector()
		_:
			if id >= 1000 and id < 1050:
				var target_idx = id - 1000
				if _menu_target_part >= 0:
					active_inspected_part = _menu_target_part
					if paperdoll:
						paperdoll.select_part(active_inspected_part)
					var part: BodyPartClass = character.body.body_parts.get(active_inspected_part)
					if part:
						var items = part.equipped_clothing_list if part.equipped_clothing_list.size() > 0 else part.clothing_layers.values()
						if target_idx < items.size():
							selected_equipped_item_id = items[target_idx].get("id", "")
					_refresh_contextual_inspector()
					_on_telemetry_updated()

func _apply_wound_to_target(w_type: int) -> void:
	if not character or not character.body:
		return
	var target_id = _get_action_target_part_id()
	var part: BodyPartClass = character.body.body_parts.get(target_id)
	if part:
		part.apply_wound(w_type)
		_on_telemetry_updated()
		_refresh_contextual_inspector()


func _on_paperdoll_layer_mode_changed(_new_mode: int) -> void:
	_on_telemetry_updated()

func _refresh_contextual_inspector() -> void:
	_update_part_selection_controls()
	if not character or not character.body:
		return
	var b: BodySystemClass = character.body

	# 1. El inspector de la parte anatómica SIEMPRE se muestra arriba (no se reemplaza estilo menú)
	container_inspector_part.visible = true
	_update_body_part_inspector(b)

	# 2. El inspector de la mochila aparece ABAJO si está abierta
	if is_inspecting_backpack:
		container_inspector_backpack.visible = true
		_update_backpack_inspector(b)
	else:
		container_inspector_backpack.visible = false
		if paperdoll:
			paperdoll.is_backpack_selected = false
		var part: BodyPartClass = b.body_parts.get(active_inspected_part)
		if part:
			_update_card_for_part(part)

func _update_body_part_inspector(b: BodySystemClass) -> void:
	if active_inspected_part == -1:
		lbl_inspector_part_title.text = "🩺 Examen Clínico / Inspección"
		lbl_inspector_part_vitals.text = "Selecciona una parte anatómica en la silueta central para examinar su estado fisiológico y equipamiento."
		if lbl_inspector_clinical_hint:
			lbl_inspector_clinical_hint.visible = false
		hbox_clinical_actions.visible = false
		if lbl_layers_title:
			lbl_layers_title.visible = false
		if container_equipped_items_list:
			for child in container_equipped_items_list.get_children():
				container_equipped_items_list.remove_child(child)
				child.queue_free()
		if option_equip_part_item:
			option_equip_part_item.clear()
			option_equip_part_item.add_item("[ Selecciona una zona ]", 0)
			option_equip_part_item.set_item_metadata(0, "")
		for child in container_layer_dropdowns.get_children():
			container_layer_dropdowns.remove_child(child)
			child.queue_free()
		layer_dropdown_nodes.clear()
		_update_card_for_part(null)
		return

	var part: BodyPartClass = b.body_parts.get(active_inspected_part)
	if not part:
		return

	if lbl_inspector_clinical_hint:
		lbl_inspector_clinical_hint.visible = true
	hbox_clinical_actions.visible = false
	if lbl_layers_title:
		lbl_layers_title.visible = true

	lbl_inspector_part_title.text = "🦴 %s" % part.display_name

	# 1. INFORMACIÓN REAL Y BIOMETRÍA DE LA ZONA (Siempre arriba)
	var current_bleed = part.get_current_bleed_rate(b.cardio.heart_rate_bpm)
	var bio_mass = part.get_anatomical_mass_kg()
	var cloth_mass = part.get_clothing_weight_kg()
	var total_mass = bio_mass + cloth_mass

	var wound_name = "Intacto / Sano"
	match part.active_wound_type:
		BodyPartClass.WoundType.SCRATCH: wound_name = "Rasguño Superficial"
		BodyPartClass.WoundType.LACERATION: wound_name = "Laceración Abierta"
		BodyPartClass.WoundType.DEEP_CUT: wound_name = "Herida Incisa Profunda"
		BodyPartClass.WoundType.PUNCTURE_BITE: wound_name = "Punción / Mordida"
		BodyPartClass.WoundType.ARTERIAL_RUPTURE: wound_name = "Ruptura Arterial Masiva"

	var clin_tags: Array[String] = []
	if part.is_tourniquet_applied: clin_tags.append("🩸 TORNIQUETE")
	if part.is_bandaged: clin_tags.append("🩹 VENDADO")
	if part.is_splinted: clin_tags.append("🪵 ENTABLILLADO")
	if part.has_fracture: clin_tags.append("🦴 FRACTURA")
	if part.infection_level > 0.0: clin_tags.append("⚠️ INFECCIÓN %d%%" % int(part.infection_level))
	elif part.is_disinfected: clin_tags.append("🧼 ASÉPTICO")
	if clin_tags.is_empty(): clin_tags.append("✅ Sin apósitos")

	var skin_water_kg = (part.wetness_pct / 100.0) * 0.15
	lbl_inspector_part_vitals.text = (
		"❤️ Salud: %d%%  |  ⚡ Dolor: %d%%  |  🩸 Hemorragia: %.1f mL/s (%s)\n" +
		"⚖️ Masa: %.1f kg biológica + %.2f kg prendas = %.2f kg total en zona\n" +
		"🛡️ Aislamiento: %.2f clo  |  💧 Agua/Humedad Piel: %.2f kg (%d%%)  |  💨 Viento: %d%%\n" +
		"🛡️ Balística: %d%%, Corte: %d%%  |  🏥 Estado Clínico: %s"
	) % [
		int(part.integrity),
		int(part.pain_level),
		current_bleed,
		wound_name,
		bio_mass,
		cloth_mass,
		total_mass,
		part.base_insulation_clo,
		skin_water_kg,
		int(part.wetness_pct),
		int(part.wind_resistance * 100.0),
		int(part.ballistic_protection * 100.0),
		int(part.cut_scratch_protection * 100.0),
		" • ".join(clin_tags)
	]

	btn_action_tourniquet.disabled = not part.can_be_amputated
	btn_action_tourniquet.text = "🩸 Quitar Torn." if part.is_tourniquet_applied else "🩸 Torniquete"
	btn_action_bandage.text = "🩹 Desvendar" if part.is_bandaged else "🩹 Vendar"

	# 2. LISTA INTERACTIVA DE PRENDAS Y EQUIPAMIENTO DE ESTA ZONA
	if container_equipped_items_list:
		for child in container_equipped_items_list.get_children():
			container_equipped_items_list.remove_child(child)
			child.queue_free()

		var all_items_on_part: Array[Dictionary] = []
		if part.equipped_clothing_list.size() > 0:
			for itm in part.equipped_clothing_list:
				all_items_on_part.append(itm)
		elif part.clothing_layers.size() > 0:
			for itm in part.clothing_layers.values():
				all_items_on_part.append(itm)

		if (part.type == BodyPartClass.PartType.TORSO_UPPER or part.type == BodyPartClass.PartType.TORSO_LOWER) and b.equipped_backpack_id != "":
			var bp = ClothingDatabaseClass.get_backpack(b.equipped_backpack_id)
			if not bp.is_empty():
				all_items_on_part.append({
					"id": b.equipped_backpack_id,
					"name": bp.get("name", "Mochila"),
					"layer": -2,
					"dry_weight_kg": b.get_backpack_total_weight_kg(),
					"is_backpack": true
				})

		if all_items_on_part.is_empty():
			var lbl_empty = Label.new()
			lbl_empty.text = "Ninguna prenda equipada en esta zona (Piel desnuda)."
			lbl_empty.add_theme_font_size_override("font_size", 12)
			lbl_empty.add_theme_color_override("font_color", Color(0.6, 0.7, 0.8, 0.75))
			container_equipped_items_list.add_child(lbl_empty)
			selected_equipped_item_id = ""
			if panel_item_card:
				panel_item_card.visible = false
		else:
			# Opción para seleccionar explícitamente piel desnuda y ocultar ficha técnica
			var btn_naked = Button.new()
			btn_naked.text = "👤 [Piel Desnuda] (Ocultar ficha técnica de prenda)"
			btn_naked.alignment = HORIZONTAL_ALIGNMENT_LEFT
			btn_naked.add_theme_font_size_override("font_size", 12)
			if selected_equipped_item_id == "":
				btn_naked.add_theme_color_override("font_color", Color(0.4, 0.9, 1.0))
			btn_naked.pressed.connect(func():
				_select_equipped_item("")
			)
			container_equipped_items_list.add_child(btn_naked)

			for itm in all_items_on_part:
				var itm_id: String = itm.get("id", "")
				var itm_name: String = itm.get("name", "Prenda")
				var itm_weight: float = itm.get("dry_weight_kg", 0.0)
				var layer_idx: int = itm.get("layer", -1)
				var prefix = "🧥"
				var layer_str = "Capa %d" % layer_idx
				if itm.get("is_backpack", false):
					prefix = "🎒"
					layer_str = "Espalda / Carga"
				elif layer_idx == BodyPartClass.ClothingLayer.LAYER_1_UNDERWEAR:
					prefix = "🩲"
					layer_str = "Interior"
				elif layer_idx == BodyPartClass.ClothingLayer.LAYER_2_BASE:
					prefix = "👕"
					layer_str = "Base"
				elif layer_idx == BodyPartClass.ClothingLayer.LAYER_3_OUTER:
					prefix = "🧥"
					layer_str = "Exterior"
				elif layer_idx == BodyPartClass.ClothingLayer.LAYER_4_TACTICAL:
					prefix = "🦺"
					layer_str = "Táctico"
				elif layer_idx == BodyPartClass.ClothingLayer.LAYER_5_ACCESSORY:
					prefix = "🧣"
					layer_str = "Accesorio"

				var btn_item = Button.new()
				btn_item.text = "%s [%s] %s  (%.2f kg)" % [prefix, layer_str, itm_name, itm_weight]
				btn_item.alignment = HORIZONTAL_ALIGNMENT_LEFT
				btn_item.add_theme_font_size_override("font_size", 12)
				if itm_id == selected_equipped_item_id:
					btn_item.add_theme_color_override("font_color", Color(1.0, 0.9, 0.4))
				btn_item.pressed.connect(func():
					_select_equipped_item(itm_id)
				)
				container_equipped_items_list.add_child(btn_item)

	# 3. SELECTOR PARA EQUIPAR PRENDAS COMPATIBLES
	if option_equip_part_item:
		option_equip_part_item.clear()
		option_equip_part_item.add_item("[ ➕ Seleccionar Prenda Compatible ]", 0)
		option_equip_part_item.set_item_metadata(0, "")

		var compat_items = ClothingDatabaseClass.get_items_for_part(active_inspected_part)
		var opt_idx = 1
		for itm in compat_items:
			var can_stk = itm.get("can_stack", false)
			var stack_label = "(Apilable)" if can_stk else "(Exclusivo)"
			var l_idx = itm.get("layer", -1)
			var l_name = BodyPartClass.ClothingLayer.keys()[l_idx].capitalize() if l_idx >= 0 and l_idx < BodyPartClass.ClothingLayer.size() else "Capa"
			option_equip_part_item.add_item("%s — %s %s" % [itm.name, l_name, stack_label], opt_idx)
			option_equip_part_item.set_item_metadata(opt_idx, itm.id)
			opt_idx += 1

		if active_inspected_part == BodyPartClass.PartType.TORSO_UPPER or active_inspected_part == BodyPartClass.PartType.TORSO_LOWER or active_inspected_part == BodyPartClass.PartType.GROIN:
			var backpacks = ClothingDatabaseClass.get_all_backpacks()
			for bp in backpacks.values():
				option_equip_part_item.add_item("🎒 %s (Mochila / Carga)" % bp.name, opt_idx)
				option_equip_part_item.set_item_metadata(opt_idx, bp.id)
				opt_idx += 1

		if btn_equip_selected_part_item:
			btn_equip_selected_part_item.disabled = (option_equip_part_item.selected <= 0)

	_update_card_for_part(part)

func _on_equip_selected_part_item_pressed() -> void:
	if not character or not character.body or not option_equip_part_item:
		return
	var idx = option_equip_part_item.selected
	if idx <= 0:
		return
	var item_id = option_equip_part_item.get_item_metadata(idx)
	if item_id == null or str(item_id) == "":
		return
	var target_id = str(item_id)
	var b = character.body
	if ClothingDatabaseClass.get_backpack(target_id).size() > 0:
		b.equip_backpack(target_id)
	else:
		b.equip_item(target_id)
	selected_equipped_item_id = target_id
	_on_telemetry_updated()
	_refresh_contextual_inspector()

func _on_unequip_item_card_pressed() -> void:
	if not character or not character.body or selected_equipped_item_id == "":
		return
	var b = character.body
	if selected_equipped_item_id == b.equipped_backpack_id:
		b.equipped_backpack_id = ""
		b.backpack_content_weight_kg = 0.0
	else:
		b.unequip_item(selected_equipped_item_id)
	selected_equipped_item_id = ""
	_on_telemetry_updated()
	_refresh_contextual_inspector()

func _select_equipped_item(item_id: String) -> void:
	selected_equipped_item_id = item_id
	if not character or not character.body:
		return
	var b = character.body
	if item_id == "":
		if panel_item_card:
			panel_item_card.visible = false
		if paperdoll:
			paperdoll.clear_highlighted_parts()
	elif item_id == b.equipped_backpack_id:
		if panel_item_card:
			panel_item_card.visible = true
		_update_card_for_backpack(b)
	else:
		var itm = ClothingDatabaseClass.get_item(item_id)
		if not itm.is_empty():
			if panel_item_card:
				panel_item_card.visible = true
			_render_item_card_data(itm)
		else:
			if panel_item_card:
				panel_item_card.visible = false
	_update_equipped_items_list_visuals()

func _update_equipped_items_list_visuals() -> void:
	if not container_equipped_items_list:
		return
	for child in container_equipped_items_list.get_children():
		if child is Button:
			if selected_equipped_item_id != "" and child.text.contains(selected_equipped_item_id):
				child.add_theme_color_override("font_color", Color(1.0, 0.9, 0.4))
			else:
				child.remove_theme_color_override("font_color")

func _on_layer_item_selected(item_id: String, layer: int) -> void:
	if not character or not character.body:
		return
	var b: BodySystemClass = character.body

	if item_id == "":
		var part: BodyPartClass = b.body_parts.get(active_inspected_part)
		if part and part.clothing_layers.has(layer):
			var cur_id = part.clothing_layers[layer].get("id", "")
			b.unequip_item(cur_id)
		paperdoll.clear_highlighted_parts()
	else:
		b.equip_item(item_id)
		selected_equipped_item_id = item_id
		var itm = ClothingDatabaseClass.get_item(item_id)
		if itm:
			var typed_cov: Array[int] = []
			for p in itm.get("covered_parts", []):
				typed_cov.append(int(p))
			paperdoll.set_highlighted_parts(typed_cov)

	_on_telemetry_updated()
	_refresh_contextual_inspector()

func _update_backpack_inspector(b: BodySystemClass) -> void:
	option_backpack_model.clear()
	option_backpack_model.add_item("[ Ninguna Mochila ]", 0)
	option_backpack_model.set_item_metadata(0, "")

	var backpacks = ClothingDatabaseClass.get_all_backpacks()
	var selected_idx = 0
	var idx = 1
	for bp in backpacks.values():
		var bp_name: String = bp.get("name", "Mochila")
		var max_cap: float = bp.get("max_capacity_kg", 0.0)
		var bp_id: String = bp.get("id", "")
		option_backpack_model.add_item("%s (Máx %.0f kg)" % [bp_name, max_cap], idx)
		option_backpack_model.set_item_metadata(idx, bp_id)
		if bp_id == b.equipped_backpack_id:
			selected_idx = idx
		idx += 1

	option_backpack_model.selected = selected_idx

	if b.equipped_backpack_id != "":
		var bp = ClothingDatabaseClass.get_backpack(b.equipped_backpack_id)
		slider_backpack_content_weight.editable = true
		slider_backpack_content_weight.max_value = bp.get("max_capacity_kg", 20.0)
		_is_updating_ui = true
		slider_backpack_content_weight.value = b.backpack_content_weight_kg
		_is_updating_ui = false
		lbl_backpack_weight_info.text = "Carga interna: %.1f kg / %.1f kg máx" % [b.backpack_content_weight_kg, slider_backpack_content_weight.max_value]
		lbl_backpack_mass_breakdown.text = "Vacía: %.1f kg + Carga: %.1f kg = Total Mochila: %.1f kg" % [
			bp.get("dry_weight_kg", 1.0),
			b.backpack_content_weight_kg,
			b.get_backpack_total_weight_kg()
		]
	else:
		slider_backpack_content_weight.editable = false
		_is_updating_ui = true
		slider_backpack_content_weight.value = 0.0
		_is_updating_ui = false
		lbl_backpack_weight_info.text = "Sin mochila equipada"
		lbl_backpack_mass_breakdown.text = "Mochila Inactiva"

	_update_card_for_backpack(b)

func _on_backpack_model_selected(index: int) -> void:
	if not character or not character.body:
		return
	var b: BodySystemClass = character.body
	var bp_id = option_backpack_model.get_item_metadata(index)
	b.equip_backpack(bp_id, slider_backpack_content_weight.value)
	_on_telemetry_updated()
	_refresh_contextual_inspector()

func _on_backpack_content_slider_changed(val: float) -> void:
	if _is_updating_ui or not character or not character.body:
		return
	var b: BodySystemClass = character.body
	b.set_backpack_content_weight(val)
	if slider_item_card_cargo and not is_equal_approx(slider_item_card_cargo.value, val):
		_is_updating_ui = true
		slider_item_card_cargo.value = val
		_is_updating_ui = false
	_on_telemetry_updated()
	lbl_backpack_weight_info.text = "Carga interna: %.1f kg / %.1f kg máx" % [val, slider_backpack_content_weight.max_value]
	var bp = ClothingDatabaseClass.get_backpack(b.equipped_backpack_id)
	var dry_w = bp.get("dry_weight_kg", 1.0) if not bp.is_empty() else 0.0
	lbl_backpack_mass_breakdown.text = "Vacía: %.1f kg + Carga: %.1f kg = Total Mochila: %.1f kg" % [dry_w, val, b.get_backpack_total_weight_kg()]
	_update_card_for_backpack(b)

func _on_item_card_cargo_slider_changed(val: float) -> void:
	if _is_updating_ui or not character or not character.body:
		return
	var b: BodySystemClass = character.body
	b.set_backpack_content_weight(val)
	if slider_backpack_content_weight and not is_equal_approx(slider_backpack_content_weight.value, val):
		_is_updating_ui = true
		slider_backpack_content_weight.value = val
		_is_updating_ui = false
	_on_telemetry_updated()
	_update_card_for_backpack(b)

# -----------------------------------------------------------------------------
# GESTIÓN DE LA FICHA TÉCNICA DEL ÍTEM (NIVEL 4)
# -----------------------------------------------------------------------------
func _update_card_for_part(part: BodyPartClass) -> void:
	if not part or not character or not character.body:
		if panel_item_card:
			panel_item_card.visible = false
		if paperdoll:
			paperdoll.clear_highlighted_parts()
		return

	var b = character.body
	var is_selected_valid = false
	if selected_equipped_item_id != "":
		if selected_equipped_item_id == b.equipped_backpack_id and (part.type == BodyPartClass.PartType.TORSO_UPPER or part.type == BodyPartClass.PartType.TORSO_LOWER or part.type == BodyPartClass.PartType.GROIN):
			is_selected_valid = true
		else:
			for itm in part.equipped_clothing_list:
				if itm.get("id", "") == selected_equipped_item_id:
					is_selected_valid = true
					break
			if not is_selected_valid and part.clothing_layers.size() > 0:
				for l_itm in part.clothing_layers.values():
					if l_itm.get("id", "") == selected_equipped_item_id:
						is_selected_valid = true
						break

	if not is_selected_valid:
		if part.equipped_clothing_list.size() > 0:
			selected_equipped_item_id = part.equipped_clothing_list[part.equipped_clothing_list.size() - 1].get("id", "")
		elif part.clothing_layers.size() > 0:
			for l in range(5, -1, -1):
				if part.clothing_layers.has(l):
					selected_equipped_item_id = part.clothing_layers[l].get("id", "")
					break
		elif b.equipped_backpack_id != "" and (part.type == BodyPartClass.PartType.TORSO_UPPER or part.type == BodyPartClass.PartType.TORSO_LOWER):
			selected_equipped_item_id = b.equipped_backpack_id
		else:
			selected_equipped_item_id = ""

	if selected_equipped_item_id != "":
		if selected_equipped_item_id == b.equipped_backpack_id:
			_update_card_for_backpack(b)
		else:
			var itm = ClothingDatabaseClass.get_item(selected_equipped_item_id)
			if not itm.is_empty():
				_render_item_card_data(itm)
			else:
				_update_card_for_part_naked(part)
	else:
		_update_card_for_part_naked(part)

func _update_card_for_part_naked(_part: BodyPartClass) -> void:
	if panel_item_card:
		panel_item_card.visible = false
	if paperdoll:
		paperdoll.clear_highlighted_parts()

func _update_card_for_backpack(b: BodySystemClass) -> void:
	if b.equipped_backpack_id == "":
		if panel_item_card:
			panel_item_card.visible = false
		if paperdoll:
			paperdoll.clear_highlighted_parts()
	else:
		if panel_item_card:
			panel_item_card.visible = true
		if container_item_card_cargo:
			container_item_card_cargo.visible = true

		var bp = ClothingDatabaseClass.get_backpack(b.equipped_backpack_id)
		var bp_name: String = bp.get("name", "Mochila")
		var max_cap: float = bp.get("max_capacity_kg", 0.0)
		var dry_w: float = bp.get("dry_weight_kg", 0.0)
		var cur_cargo: float = b.backpack_content_weight_kg
		var total_bp_w: float = b.get_backpack_total_weight_kg()

		lbl_item_card_title.text = "📋 Mochila / Carga: %s" % bp_name
		lbl_item_card_specs.text = "Capacidad: %.1f kg | Peso Vacía: %.2f kg | Carga Actual: %.1f kg" % [
			max_cap,
			dry_w,
			cur_cargo
		]

		if slider_item_card_cargo:
			slider_item_card_cargo.max_value = max(0.1, max_cap)
			_is_updating_ui = true
			slider_item_card_cargo.value = cur_cargo
			_is_updating_ui = false

		if lbl_item_card_cargo_info:
			var cargo_pct = int((cur_cargo / max(0.1, max_cap)) * 100.0)
			lbl_item_card_cargo_info.text = "Carga: %.1f / %.1f kg (%d%%)" % [cur_cargo, max_cap, cargo_pct]

		if lbl_item_card_cargo_breakdown:
			var med_kg = cur_cargo * 0.40
			var ration_kg = cur_cargo * 0.35
			var tool_kg = cur_cargo * 0.25
			lbl_item_card_cargo_breakdown.text = "📦 Contenido: Insumos Médicos (%.1f kg) • Cantimplora/Raciones (%.1f kg) • Equipo (%.1f kg)" % [
				med_kg, ration_kg, tool_kg
			]

		if lbl_item_card_cargo_impact:
			var metab_pen = (cur_cargo / 20.0) * 15.0
			lbl_item_card_cargo_impact.text = "⚖️ Sobrecarga: +%.1f%% costo metabólico | Carga lumbar: %.1f kg sobre vértebras L1-L5 y pies" % [
				metab_pen, total_bp_w
			]

		var max_w = bp.get("max_water_absorption_kg", 0.5)
		var cur_w = max_w * (b.moisture.wetness_pct / 100.0)
		bar_item_water_absorbed.max_value = max(0.01, max_w)
		bar_item_water_absorbed.value = cur_w
		lbl_item_water_val.text = "%.2f / %.2f kg (%d%%)" % [cur_w, max_w, int(b.moisture.wetness_pct)]
		lbl_item_covered_badges.text = "Zonas: Espalda y Hombros (Carga directa sobre columna lumbar)"
		if btn_unequip_item_card:
			btn_unequip_item_card.visible = true
			btn_unequip_item_card.text = "🚫 Desequipar %s" % bp_name

		var base_rig_file = MasterLoader.BASE_MODEL_PATH.get_file()
		var tpl = ClothingDatabaseClass.get_item_visual_template(b.equipped_backpack_id)
		var sock = tpl.get("socket_bone", "mixamorig_Spine2")
		var ov = tpl.get("overlay_type", "backpack_harness")
		var mesh_file = "%s.glb" % b.equipped_backpack_id
		if lbl_item_socket_bone: lbl_item_socket_bone.text = "Rig Base: %s | Socket: %s | Overlay: %s" % [base_rig_file, sock, ov]
		if line_item_mesh_path: line_item_mesh_path.text = mesh_file
		if lbl_item_asset_status: lbl_item_asset_status.text = "Estado: Template 3D sincronizado con socket '%s' del rig '%s'" % [sock, base_rig_file]

		if paperdoll:
			var typed_bp: Array[int] = [int(BodyPartClass.PartType.TORSO_UPPER), int(BodyPartClass.PartType.TORSO_LOWER)]
			paperdoll.set_highlighted_parts(typed_bp)

func _render_item_card_data(item: Dictionary) -> void:
	if panel_item_card:
		panel_item_card.visible = true
	if container_item_card_cargo:
		container_item_card_cargo.visible = false

	var item_name = item.get("name", "Prenda")
	var layer_idx = item.get("layer", -1)
	var layer_name = BodyPartClass.ClothingLayer.keys()[layer_idx].capitalize() if layer_idx >= 0 and layer_idx < BodyPartClass.ClothingLayer.size() else "Accesorio"
	lbl_item_card_title.text = "📋 Prenda Seleccionada: %s (%s)" % [item_name, layer_name]
	lbl_item_card_specs.text = "Clo: %.2f | Seco: %.2f kg | Balística: %d%% | Corte: %d%%" % [
		item.get("insulation_clo", 0.0),
		item.get("dry_weight_kg", 0.0),
		int(item.get("ballistic_protection", 0.0) * 100.0),
		int(item.get("cut_scratch_protection", 0.0) * 100.0)
	]

	var max_water = item.get("max_water_absorption_kg", 0.0)
	var wet_pct = character.body.moisture.wetness_pct if character and character.body else 0.0
	var cur_water = max_water * (wet_pct / 100.0)
	bar_item_water_absorbed.max_value = max(0.01, max_water)
	bar_item_water_absorbed.value = cur_water
	lbl_item_water_val.text = "%.2f / %.2f kg (%d%%)" % [cur_water, max_water, int(wet_pct)]

	var covered: Array = item.get("covered_parts", [])
	var names: Array[String] = []
	for p_id in covered:
		if character and character.body and character.body.body_parts.has(p_id):
			names.append(character.body.body_parts[p_id].display_name)
	lbl_item_covered_badges.text = "Cubre: " + " • ".join(names)

	if btn_unequip_item_card:
		btn_unequip_item_card.visible = true
		btn_unequip_item_card.text = "❌ Desequipar %s" % item_name

	var base_rig_file = MasterLoader.BASE_MODEL_PATH.get_file()
	var item_id = item.get("id", "")
	var tpl = ClothingDatabaseClass.get_item_visual_template(item_id)
	var sock = tpl.get("socket_bone", "mixamorig_Spine1")
	var ov = tpl.get("overlay_type", "generic")
	var mesh_file = "%s.glb" % item_id
	if lbl_item_socket_bone: lbl_item_socket_bone.text = "Rig Base: %s | Socket: %s | Overlay: %s" % [base_rig_file, sock, ov]
	if line_item_mesh_path: line_item_mesh_path.text = mesh_file
	if lbl_item_asset_status: lbl_item_asset_status.text = "Estado: Template 3D sincronizado con socket '%s' del rig '%s'" % [sock, base_rig_file]

	var typed_parts: Array[int] = []
	for p in covered:
		typed_parts.append(int(p))
	if paperdoll:
		paperdoll.set_highlighted_parts(typed_parts)

func _on_attach_3d_asset_pressed() -> void:
	if not line_item_mesh_path or not lbl_item_asset_status:
		return
	var sock = "mixamorig_Spine2"
	if lbl_item_socket_bone and "Socket:" in lbl_item_socket_bone.text:
		var parts = lbl_item_socket_bone.text.split("|")
		for p in parts:
			if "Socket:" in p:
				sock = p.replace("Socket:", "").strip_edges()

	var base_rig_file = MasterLoader.BASE_MODEL_PATH.get_file()
	var mesh_file = line_item_mesh_path.text.get_file()
	if mesh_file == "":
		mesh_file = "asset.glb"

	var attached = false
	if character:
		var skels = character.find_children("*", "Skeleton3D", true, false)
		if skels.size() > 0:
			var skel: Skeleton3D = skels[0]
			var bone_idx = skel.find_bone(sock)
			if bone_idx >= 0:
				var att_name = "Attach_" + sock
				var att: BoneAttachment3D = skel.get_node_or_null(att_name)
				if not att:
					att = BoneAttachment3D.new()
					att.name = att_name
					att.bone_name = sock
					skel.add_child(att)
				attached = true

	if attached:
		lbl_item_asset_status.text = "✅ Archivo '%s' acoplado al socket '%s' del rig '%s' (Skeleton3D MakeHuman)" % [mesh_file, sock, base_rig_file]
		lbl_item_asset_status.add_theme_color_override("font_color", Color(0.3, 1.0, 0.5))
	else:
		lbl_item_asset_status.text = "ℹ️ Template registrado: '%s' -> Socket '%s' en '%s' (Listo para acoplar)" % [mesh_file, sock, base_rig_file]
		lbl_item_asset_status.add_theme_color_override("font_color", Color(0.4, 0.8, 1.0))

# -----------------------------------------------------------------------------
# BOTONES CLÍNICOS DE PRIMEROS AUXILIOS Y TEST
# -----------------------------------------------------------------------------
func _get_action_target_part_id() -> int:
	if _menu_target_part >= 0:
		return _menu_target_part
	return active_inspected_part

func _on_action_bandage_pressed() -> void:
	if character and character.body:
		var target_id = _get_action_target_part_id()
		var part: BodyPartClass = character.body.body_parts.get(target_id)
		if part:
			part.is_bandaged = !part.is_bandaged
			if part.is_bandaged:
				part.pain_level = max(0.0, part.pain_level - 15.0)
				btn_action_bandage.text = "🩹 Quitar Vendaje"
			else:
				btn_action_bandage.text = "🩹 Vendar"
			_on_telemetry_updated()
			_refresh_contextual_inspector()

func _on_action_tourniquet_pressed() -> void:
	if character and character.body:
		var target_id = _get_action_target_part_id()
		var part: BodyPartClass = character.body.body_parts.get(target_id)
		if part and part.can_be_amputated:
			part.is_tourniquet_applied = !part.is_tourniquet_applied
			if part.is_tourniquet_applied:
				part.pain_level = clamp(part.pain_level + 10.0, 0.0, 100.0)
				btn_action_tourniquet.text = "🩸 Quitar Torn."
			else:
				btn_action_tourniquet.text = "🩸 Torniquete"
			_on_telemetry_updated()
			_refresh_contextual_inspector()

func _on_action_splint_pressed() -> void:
	if character and character.body:
		var target_id = _get_action_target_part_id()
		var part: BodyPartClass = character.body.body_parts.get(target_id)
		if part:
			part.is_splinted = !part.is_splinted
			if part.is_splinted:
				part.pain_level = max(0.0, part.pain_level - 30.0)
			_on_telemetry_updated()
			_refresh_contextual_inspector()

func _on_action_disinfect_pressed() -> void:
	if character and character.body:
		var target_id = _get_action_target_part_id()
		var part: BodyPartClass = character.body.body_parts.get(target_id)
		if part:
			part.infection_level = 0.0
			part.is_disinfected = true
			_on_telemetry_updated()
			_refresh_contextual_inspector()

func _on_action_heal_pressed() -> void:
	if character and character.body:
		var target_id = _get_action_target_part_id()
		var part: BodyPartClass = character.body.body_parts.get(target_id)
		if part:
			part.heal_completely()
			btn_action_bandage.text = "🩹 Vendar"
			btn_action_tourniquet.text = "🩸 Torniquete"
			_on_telemetry_updated()
			_refresh_contextual_inspector()

func _on_action_wound_test_pressed() -> void:
	if character and character.body:
		var target_id = _get_action_target_part_id()
		var part: BodyPartClass = character.body.body_parts.get(target_id)
		if part:
			part.apply_wound(BodyPartClass.WoundType.LACERATION)
			_on_telemetry_updated()
			_refresh_contextual_inspector()

# -----------------------------------------------------------------------------
# ACCIONES DE TERRENO Y ESCALADA
# -----------------------------------------------------------------------------
func _on_env_slope_changed(val: float) -> void:
	if character and character.body:
		character.body.environment.slope_degrees = val
		if val > 0.0:
			lbl_env_slope_val.text = "+%d° (Subida +%.0f%%)" % [int(val), sin(deg_to_rad(val)) * 100.0]
		elif val < 0.0:
			lbl_env_slope_val.text = "%d° (Bajada / Rodillas)" % int(val)
		else:
			lbl_env_slope_val.text = "0° (Plano)"
		_on_telemetry_updated()

func _on_env_substrate_selected(index: int) -> void:
	if character and character.body:
		var sub_type = option_env_substrate.get_item_id(index)
		character.body.environment.substrate = sub_type as EnvironmentSystemClass.SubstrateType
		var t_param: TerrainParameters = null
		match sub_type:
			EnvironmentSystemClass.SubstrateType.FIRM_GROUND: t_param = TerrainParameters.create_asphalt()
			EnvironmentSystemClass.SubstrateType.DIRT_GRASS: t_param = TerrainParameters.create_dirt_grass()
			EnvironmentSystemClass.SubstrateType.LOOSE_SAND: t_param = TerrainParameters.create_loose_sand()
			EnvironmentSystemClass.SubstrateType.THICK_MUD: t_param = TerrainParameters.create_thick_mud()
			EnvironmentSystemClass.SubstrateType.WATER_WADING: t_param = TerrainParameters.create_water_wading()
			EnvironmentSystemClass.SubstrateType.DEEP_SNOW: t_param = TerrainParameters.create_deep_snow()
		if t_param and lbl_substrate_specs:
			lbl_substrate_specs.text = "📊 Costo: x%.1f | Tracción: %d%% | Ruido: %d%% | Riesgo: %.1f%%" % [
				t_param.locomotion_cost_multiplier,
				int(t_param.traction_factor * 100.0),
				int(t_param.noise_level * 100.0),
				t_param.injury_risk_per_step * 100.0
			]
		_on_telemetry_updated()

func _on_env_vegetation_selected(index: int) -> void:
	if character and character.body:
		var veg_type = option_env_vegetation.get_item_id(index)
		character.body.environment.vegetation = veg_type as EnvironmentSystemClass.VegetationDensity
		_on_telemetry_updated()

func _on_toggle_climb_pressed() -> void:
	if character and character.body:
		character.body.is_climbing = !character.body.is_climbing
		if character.body.is_climbing:
			btn_toggle_climb.text = "🧗 Escalando: ACTIVO (Tren Sup. 70%)"
			btn_toggle_climb.modulate = Color(1.0, 0.6, 0.2)
		else:
			btn_toggle_climb.text = "🧗 Acción: Iniciar Escalada"
			btn_toggle_climb.modulate = Color(1.0, 1.0, 1.0)
		_on_telemetry_updated()

# -----------------------------------------------------------------------------
# ACTUALIZACIÓN DE TELEMETRÍA FISIOLÓGICA Y SINCRONIZACIÓN
# -----------------------------------------------------------------------------
func _on_telemetry_updated() -> void:
	if not character or not character.body:
		return
	var b: BodySystemClass = character.body

	if paperdoll:
		paperdoll.queue_redraw()

	if lbl_tier1_vital_status:
		lbl_tier1_vital_status.text = b.get_vital_status_text()
		lbl_tier1_vital_status.modulate = b.get_vital_status_color()

	if lbl_tier1_mass_breakdown:
		var total_m = b.get_total_moving_mass_kg()
		var dry_gear = b.get_gear_dry_weight_kg()
		var wet_gear = b.get_gear_absorbed_water_kg()
		var bp_tot = b.get_backpack_total_weight_kg()
		var mud_w = b.environment.get_mud_adhesion_weight_kg() if b.environment else 0.0
		var mud_str = (" | Barro: +%.1f" % mud_w) if mud_w > 0.0 else ""
		lbl_tier1_mass_breakdown.text = "Masa Total: %.1f kg (Cuerpo: %.1f | Ropa Seca: %.1f | Agua: %.1f | Mochila: %.1f%s)" % [
			total_m, b.total_weight_kg, dry_gear, wet_gear, bp_tot, mud_str
		]

	if lbl_tier1_temp_pain:
		var clo = b.get_total_clothing_clo()
		var wet_gear = b.get_gear_absorbed_water_kg()
		lbl_tier1_temp_pain.text = "🧥 Aislamiento: %.2f Clo  |  💧 Agua en Ropa: %.2f kg" % [clo, wet_gear]

	var mode: int = paperdoll.active_layer_mode if paperdoll else 0
	var part: BodyPartClass = b.body_parts.get(active_inspected_part)
	var part_name = part.display_name if part else "General"
	var bpm = b.cardio.heart_rate_bpm

	if lbl_tier1_subsystem_telemetry:
		match mode:
			0: # BASE / BIOMETRÍA & X-RAY
				var arch_str = character.active_archetype.capitalize() if character else "Normal"
				lbl_tier1_subsystem_telemetry.text = "🧬 Somatotipo: %s  |  Estatura: %.2f m  |  Grasa: %d%%  |  Músculo: %d%%  |  X-Ray: 72%%" % [
					arch_str, b.height_meters, int(b.body_fat_pct * 100.0), int(b.muscle_mass_pct * 100.0)
				]
				lbl_tier1_subsystem_telemetry.add_theme_color_override("font_color", Color(0.3, 1.0, 0.85))

			1: # MUSCULAR / FIBRAS & METABOLISMO
				var glyc_pct = clamp(b.muscular_glycogen_pct if "muscular_glycogen_pct" in b else 1.0, 0.0, 1.0) * 100.0
				var exertion_str = "Reposo / Óptimo"
				if b.stamina < 30.0: exertion_str = "Fatiga Severa"
				elif b.stamina < 70.0: exertion_str = "Esfuerzo Moderado"
				lbl_tier1_subsystem_telemetry.text = "💪 Masa Muscular: %d%%  |  Glucógeno: %d%%  |  Estado: %s" % [
					int(b.muscle_mass_pct * 100.0), int(glyc_pct), exertion_str
				]
				lbl_tier1_subsystem_telemetry.add_theme_color_override("font_color", Color(1.0, 0.65, 0.35))

			2: # SKELETAL / HUESOS
				var fractures_count = 0
				var splints_count = 0
				for p in b.body_parts.values():
					if p.has_fracture: fractures_count += 1
					if p.is_splinted: splints_count += 1
				var bone_status = "Estructura Íntegra" if fractures_count == 0 else ("%d Fractura(s) Activa(s)" % fractures_count)
				lbl_tier1_subsystem_telemetry.text = "🦴 Esqueleto: %s  |  Entablillados: %d  |  Marcha: %s" % [
					bone_status, splints_count, ("Severa" if (fractures_count > 0 and splints_count < fractures_count) else ("Inmovilizada" if fractures_count > 0 else "Normal"))
				]
				lbl_tier1_subsystem_telemetry.add_theme_color_override("font_color", Color(0.5, 0.85, 1.0))

			3: # CARDIO / VASCULAR
				var shock_str = "Normal (Euvolémico)"
				if b.cardio.current_shock_state == CardiovascularSystem.ShockState.MILD_HYPOVOLEMIA: shock_str = "Hipovolemia Leve"
				elif b.cardio.current_shock_state == CardiovascularSystem.ShockState.MODERATE_SHOCK: shock_str = "Shock Moderado"
				elif b.cardio.current_shock_state == CardiovascularSystem.ShockState.SEVERE_SHOCK: shock_str = "Shock Severo"
				elif b.cardio.current_shock_state == CardiovascularSystem.ShockState.CARDIAC_ARREST: shock_str = "Paro Cardíaco"
				
				lbl_tier1_subsystem_telemetry.text = "🫀 Volemia: %d / %d mL  |  💓 %d BPM (%s)  |  Presión: %d/%d mmHg" % [
					int(b.cardio.total_blood_ml), int(b.cardio.max_blood_ml), int(bpm), shock_str,
					int(b.cardio.blood_pressure_systolic), int(b.cardio.blood_pressure_diastolic)
				]
				lbl_tier1_subsystem_telemetry.add_theme_color_override("font_color", Color(1.0, 0.45, 0.45))

			4: # NERVOUS / DOLOR
				var shock_pct = clamp(b.total_pain / 100.0, 0.0, 1.0) * 100.0
				var conscious_str = "Lúcido (100%)" if b.is_alive else "Inconsciente / Colapso"
				lbl_tier1_subsystem_telemetry.text = "⚡ Dolor Somático: %d/100  |  Shock Neurogénico: %d%%  |  Consciencia: %s" % [
					int(b.total_pain), int(shock_pct), conscious_str
				]
				lbl_tier1_subsystem_telemetry.add_theme_color_override("font_color", Color(1.0, 0.9, 0.3))

			_: # ALL / GENERAL
				var fractures_cnt = 0
				var active_bleeds = 0
				var total_bleed_rate = 0.0
				for p in b.body_parts.values():
					if p.has_fracture: fractures_cnt += 1
					var br = p.get_current_bleed_rate(bpm)
					if br > 0.01:
						active_bleeds += 1
						total_bleed_rate += br
				var bleed_str = ("%d (%.1f mL/m)" % [active_bleeds, total_bleed_rate]) if active_bleeds > 0 else "0"
				lbl_tier1_subsystem_telemetry.text = "🦴 Fracturas: %d  |  🩸 Hemorragias: %s  |  ⚡ Dolor Global: %d/100" % [
					fractures_cnt, bleed_str, int(b.total_pain)
				]
				lbl_tier1_subsystem_telemetry.add_theme_color_override("font_color", Color(1.0, 0.5, 0.4) if (fractures_cnt > 0 or active_bleeds > 0) else Color(0.4, 0.95, 0.8))

	if lbl_tier1_part_specs:
		if is_inspecting_backpack:
			var bp_w = b.get_backpack_total_weight_kg()
			lbl_tier1_part_specs.text = "🎒 Mochila: Carga Total %.1f kg  |  Estado: %s" % [
				bp_w, ("Ocupada" if bp_w > 0.0 else "Vacía")
			]
			lbl_tier1_part_specs.add_theme_color_override("font_color", Color(0.4, 1.0, 0.6) if bp_w > 0.0 else Color(0.7, 0.75, 0.8))
		elif part:
			match mode:
				0: # BASE / EQUIPAMIENTO PARTE
					lbl_tier1_part_specs.text = "🧬 %s: Capas Ropa: %d/5  |  Aislamiento: %.2f clo\n   Blindaje: %d%%  |  Corte: %d%%" % [
						part_name, part.clothing_layers.size(), part.base_insulation_clo,
						int(part.ballistic_protection * 100.0), int(part.cut_scratch_protection * 100.0)
					]
					lbl_tier1_part_specs.add_theme_color_override("font_color", Color(0.4, 0.9, 1.0))
				1: # MUSCULAR PARTE
					var strain_str = "Óptimo" if part.pain_level < 15 else ("Sobrecarga" if part.pain_level < 40 else "Traumatismo Fibras")
					lbl_tier1_part_specs.text = "💪 %s: Fibras Musculares: %s\n   Masa Relativa: Normal  |  Fatiga Motora: %d%%" % [
						part_name, strain_str, int(part.pain_level * 0.7)
					]
					lbl_tier1_part_specs.add_theme_color_override("font_color", Color(1.0, 0.75, 0.4))
				2: # ESQUELÉTICO PARTE
					var frac_str = "FRACTURA ACTIVA" if part.has_fracture else "Íntegro (Sin fractura)"
					var spl_str = " (Entablillado)" if part.is_splinted else ""
					var sev_str = "AMPUTADO" if part.is_severed else "Presente"
					lbl_tier1_part_specs.text = "🦴 %s: Hueso: %s%s\n   Miembro: %s  |  Integridad: %d%%" % [
						part_name, frac_str, spl_str, sev_str, int(part.integrity)
					]
					lbl_tier1_part_specs.add_theme_color_override("font_color", Color(1.0, 0.4, 0.3) if part.has_fracture else Color(0.7, 0.9, 1.0))
				3: # VASCULAR PARTE
					var bleed = part.get_current_bleed_rate(bpm)
					var tq_str = "Aplicado" if part.is_tourniquet_applied else "Ninguno"
					var bnd_str = "Aplicado" if part.is_bandaged else "No"
					lbl_tier1_part_specs.text = "🫀 %s: Hemorragia %.1f mL/s  |  Arteria: x%.1f\n   Torniquete: %s  |  Vendaje: %s" % [
						part_name, bleed, part.arterial_weight, tq_str, bnd_str
					]
					lbl_tier1_part_specs.add_theme_color_override("font_color", Color(1.0, 0.5, 0.5) if bleed > 0 else Color(0.8, 0.9, 1.0))
				4: # NERVIOSO PARTE
					lbl_tier1_part_specs.text = "⚡ %s: Dolor Local %d%%\n   Sensibilidad: %s  |  Hueso Rig: %s" % [
						part_name, int(part.pain_level), ("Sensibilizado" if part.pain_level > 20 else "Normal"), part.rig_bone_name
					]
					lbl_tier1_part_specs.add_theme_color_override("font_color", Color(1.0, 0.85, 0.3) if part.pain_level > 20 else Color(0.8, 0.9, 1.0))
				_: # GENERAL PARTE
					var bleed = part.get_current_bleed_rate(bpm)
					lbl_tier1_part_specs.text = "🦴 %s: Salud %d%%  |  Sangrado: %.1f mL/s\n   Dolor: %d%%  |  Aislamiento: %.2f clo  |  Blindaje: %d%%" % [
						part_name, int(part.integrity), bleed, int(part.pain_level), part.base_insulation_clo, int(part.ballistic_protection * 100.0)
					]
					lbl_tier1_part_specs.add_theme_color_override("font_color", Color(1.0, 0.85, 0.4))
		else:
			lbl_tier1_part_specs.text = "🦴 Silueta: Haz clic en una parte para examinarla"
			lbl_tier1_part_specs.add_theme_color_override("font_color", Color(0.65, 0.75, 0.85))

	if bar_blood: bar_blood.value = b.cardio.total_blood_ml
	if label_blood_val: label_blood_val.text = "%d / %d mL" % [int(b.cardio.total_blood_ml), int(b.cardio.max_blood_ml)]

	if label_bpm_val:
		label_bpm_val.text = "%d BPM" % int(b.cardio.heart_rate_bpm)
		label_bpm_val.modulate = Color(1.0, 0.3, 0.3) if b.cardio.heart_rate_bpm > 140 else Color(1.0, 1.0, 1.0)

	if bar_glycogen: bar_glycogen.value = b.glycogen_pct
	if label_glycogen_val: label_glycogen_val.text = "%d%%" % int(b.glycogen_pct)

	if bar_water and b.hydration:
		bar_water.value = b.hydration.current_water_ml
	if label_water_val and b.hydration:
		label_water_val.text = "%d / %d mL" % [int(b.hydration.current_water_ml), int(b.hydration.max_water_ml)]

	if label_pain_val:
		label_pain_val.text = "%d / 100" % int(b.total_pain)
	if label_status_val:
		label_status_val.text = b.get_vital_status_text()
	_update_posture_display()

	if label_sensory_thought and b:
		var rep = b.get_sensory_report()
		if label_sensory_mood:
			label_sensory_mood.text = rep.mood_tag
			label_sensory_mood.modulate = rep.thought_color
		label_sensory_thought.text = "« %s »" % rep.primary_thought
		label_sensory_thought.modulate = rep.thought_color
		if label_sensory_somatic:
			label_sensory_somatic.text = "  •  ".join(rep.somatic_tags)

func _on_reset_pressed() -> void:
	movement_time_seconds = 0.0
	current_action_name = "Reposo"
	current_action_time = 0.0
	last_event_text = "Simulación Reiniciada"
	last_event_time = 0.0
	_update_movement_timer_display()
	_is_crouched = false
	if btn_anim_crouch:
		btn_anim_crouch.text = "🧎 Agacharse"
	if character:
		character.reset_character_state()
	_is_updating_ui = true
	if slider_exertion: slider_exertion.value = 0.0
	if slider_workplace_exertion: slider_workplace_exertion.value = 0.0
	if slider_ambient_temp: slider_ambient_temp.value = 22.0
	if slider_env_slope: slider_env_slope.value = 0.0
	if btn_toggle_climb:
		btn_toggle_climb.text = "🧗 Acción: Iniciar Escalada"
		btn_toggle_climb.modulate = Color(1.0, 1.0, 1.0)
	_is_updating_ui = false
	clock.reset_to_initial()
	_update_exertion_label(0.0)
	_update_ambient_temp_label(22.0)
	_highlight_anim_button("Idle")
	_update_posture_display()
	_on_telemetry_updated()
	_refresh_contextual_inspector()

func _apply_effort(effort_val: float, active_btn_name: String) -> void:
	_is_crouched = false
	var act_label = "Reposo"
	if active_btn_name == "Walking": act_label = "Caminando (%d%%)" % int(effort_val)
	elif active_btn_name == "Running": act_label = "Corriendo (%d%%)" % int(effort_val)
	elif active_btn_name == "Sprint": act_label = "Sprint (%d%%)" % int(effort_val)
	elif active_btn_name == "Idle":
		if character and character.body:
			match character.body.current_posture:
				BodySystemClass.Posture.SITTING: act_label = "Reposo (Sentado)"
				BodySystemClass.Posture.LYING: act_label = "Reposo (Acostado)"
				BodySystemClass.Posture.CROUCHING: act_label = "Reposo (Agachado)"
				_: act_label = "Reposo"
		else:
			act_label = "Reposo"
	elif active_btn_name == "Crouch_Down": act_label = "Reposo (Agachado)"

	if current_action_name != act_label:
		current_action_name = act_label
		current_action_time = 0.0

	if effort_val > 1.0 and character and character.body and character.body.current_posture != BodySystemClass.Posture.STANDING:
		character.set_character_posture(BodySystemClass.Posture.STANDING)

	if character:
		character.is_collapsed = false
		if not is_anim_decoupled:
			if effort_val <= 1.0:
				character.play_idle()
			else:
				character.set_locomotion_effort(effort_val / 100.0)
		else:
			if active_btn_name == "Walking": character.playback.travel(&"Walking")
			elif active_btn_name == "Running": character.playback.travel(&"Running")
			elif active_btn_name == "Sprint": character.playback.travel(&"Sprint")
			elif active_btn_name == "Idle": character.playback.travel(&"Idle")

	_is_updating_ui = true
	if slider_exertion:
		slider_exertion.value = effort_val
	if slider_workplace_exertion:
		slider_workplace_exertion.value = effort_val
	_is_updating_ui = false
	_update_exertion_label(effort_val)
	_highlight_anim_button(active_btn_name)
	_update_posture_display()

func _on_exertion_slider_changed(val: float) -> void:
	if _is_updating_ui:
		return
	if val > 1.0 and character and character.body and character.body.current_posture != BodySystemClass.Posture.STANDING:
		character.set_character_posture(BodySystemClass.Posture.STANDING)

	var act_label = "Reposo"
	if character and character.body:
		match character.body.current_posture:
			BodySystemClass.Posture.SITTING:
				act_label = "Reposo (Sentado)"
			BodySystemClass.Posture.LYING:
				act_label = "Reposo (Acostado)"
			BodySystemClass.Posture.CROUCHING:
				if val > 1.0:
					act_label = "Marcha Agachada (%d%%)" % int(val)
				else:
					act_label = "Reposo (Agachado)"
			_:
				if val >= 70.0:
					act_label = "Sprint (%d%%)" % int(val)
				elif val >= 35.0:
					act_label = "Corriendo (%d%%)" % int(val)
				elif val > 1.0:
					act_label = "Caminando (%d%%)" % int(val)
				else:
					act_label = "Reposo"
	else:
		if val >= 70.0: act_label = "Sprint (%d%%)" % int(val)
		elif val >= 35.0: act_label = "Corriendo (%d%%)" % int(val)
		elif val > 1.0: act_label = "Caminando (%d%%)" % int(val)
		else: act_label = "Reposo"

	if current_action_name != act_label:
		current_action_name = act_label
		current_action_time = 0.0

	if character and not character.is_collapsed and not is_anim_decoupled:
		if val <= 1.0:
			character.play_idle()
			_highlight_anim_button("Idle")
		else:
			character.set_locomotion_effort(val / 100.0)
			if val >= 70.0: _highlight_anim_button("Sprint")
			elif val >= 35.0: _highlight_anim_button("Running")
			elif val >= 2.0: _highlight_anim_button("Walking")
			else: _highlight_anim_button("Idle")

	_is_updating_ui = true
	if slider_exertion and slider_exertion.value != val:
		slider_exertion.value = val
	if slider_workplace_exertion and slider_workplace_exertion.value != val:
		slider_workplace_exertion.value = val
	_is_updating_ui = false

	_update_exertion_label(val)
	_update_posture_display()

func _on_time_scale_changed(val: float) -> void:
	if character: character.simulation_speed = val
	if label_time_scale_val: label_time_scale_val.text = "%.1fx" % val

func _toggle_crouch() -> void:
	_is_crouched = !_is_crouched
	if character:
		if _is_crouched:
			character.set_character_posture(BodySystemClass.Posture.CROUCHING)
			if btn_anim_crouch: btn_anim_crouch.text = "🚶 De Pie"
			_highlight_anim_button("Crouch_Down")
		else:
			character.set_character_posture(BodySystemClass.Posture.STANDING)
			if btn_anim_crouch: btn_anim_crouch.text = "🧎 Agacharse"
			_highlight_anim_button("Idle")
	_update_posture_display()

func _highlight_anim_button(anim_name: String) -> void:
	var btns = {
		"Idle": btn_anim_idle,
		"Walking": btn_anim_walk,
		"Running": btn_anim_run,
		"Sprint": btn_anim_sprint,
		"Crouch_Down": btn_anim_crouch,
		"Fall_Flat": btn_death_fall_flat,
		"Death_Backward": btn_death_bwd
	}
	for k in btns.keys():
		var b: Button = btns[k]
		if b:
			b.modulate = Color(1.3, 1.3, 1.3) if k == anim_name else Color(0.7, 0.7, 0.7)

func _highlight_archetype_button(arch_name: String) -> void:
	var n = arch_name.to_lower()
	if btn_arch_skinny: btn_arch_skinny.modulate = Color(1.3, 1.3, 1.3) if n == "skinny" else Color(0.65, 0.65, 0.65)
	if btn_arch_normal: btn_arch_normal.modulate = Color(1.3, 1.3, 1.3) if n == "normal" else Color(0.65, 0.65, 0.65)
	if btn_arch_muscle: btn_arch_muscle.modulate = Color(1.3, 1.3, 1.3) if n == "muscle" else Color(0.65, 0.65, 0.65)
	if btn_arch_fat: btn_arch_fat.modulate = Color(1.3, 1.3, 1.3) if n == "fat" else Color(0.65, 0.65, 0.65)

func _on_biometrics_slider_changed(_val: float) -> void:
	if _is_updating_ui or not character:
		return
	var h: float = slider_height.value if slider_height else 1.78
	var m: float = (slider_muscle.value / 100.0) if slider_muscle else 0.50
	var f: float = (slider_fat.value / 100.0) if slider_fat else 0.20

	var target_arch: String = "normal"
	if f >= 0.45: target_arch = "fat"
	elif f <= 0.08: target_arch = "skinny"
	elif m >= 0.65: target_arch = "muscle"
	else: target_arch = "normal"

	if character.active_archetype != target_arch:
		character.active_archetype = target_arch
		character.archetype_changed.emit(target_arch)

	character.apply_biometrics(h, m, f)
	_highlight_archetype_button(target_arch)
	if character.body:
		_update_biometrics_labels(h, m, f, character.body.total_weight_kg)

func _update_biometrics_labels(h: float, m: float, f: float, w: float) -> void:
	if label_height_val: label_height_val.text = "%.2f m" % h
	if label_muscle_val: label_muscle_val.text = "%d%%" % int(m * 100.0)
	if label_fat_val: label_fat_val.text = "%d%%" % int(f * 100.0)
	if label_weight_val: label_weight_val.text = "%.1f kg" % w

func _update_exertion_label(val: float) -> void:
	if label_exertion_val: label_exertion_val.text = "%d%%" % int(val)

func _update_ambient_temp_label(val: float) -> void:
	if not label_ambient_temp_val: return
	var badge: String = ""
	var col: Color = Color(1.0, 1.0, 1.0)
	if val <= -5.0: badge = "🥶 %.0f°C (Congelante)" % val; col = Color(0.4, 0.7, 1.0)
	elif val <= 5.0: badge = "❄️ %.0f°C (Helada)" % val; col = Color(0.5, 0.8, 1.0)
	elif val <= 14.0: badge = "🧥 %.0f°C (Frío)" % val; col = Color(0.7, 0.9, 1.0)
	elif val <= 24.0: badge = "⛅ %.0f°C (Templado)" % val; col = Color(0.3, 1.0, 0.5)
	elif val <= 34.0: badge = "☀️ %.0f°C (Caluroso)" % val; col = Color(1.0, 0.85, 0.3)
	elif val <= 44.0: badge = "🔥 %.0f°C (Calor Extremo)" % val; col = Color(1.0, 0.5, 0.2)
	else: badge = "☠️ %.0f°C (Calor Letal)" % val; col = Color(1.0, 0.2, 0.2)
	label_ambient_temp_val.text = badge
	label_ambient_temp_val.modulate = col

func _sync_biometrics_from_character() -> void:
	if not character or not character.body: return
	_highlight_archetype_button(character.active_archetype)
	_is_updating_ui = true
	if slider_height: slider_height.value = character.body.height_meters
	if slider_muscle: slider_muscle.value = character.body.muscle_mass_pct * 100.0
	if slider_fat: slider_fat.value = character.body.body_fat_pct * 100.0
	_is_updating_ui = false
	_update_biometrics_labels(character.body.height_meters, character.body.muscle_mass_pct, character.body.body_fat_pct, character.body.total_weight_kg)

func _on_character_archetype_changed(new_archetype: String) -> void:
	_highlight_archetype_button(new_archetype)

func _on_character_animation_state_changed(anim_name: String) -> void:
	_highlight_anim_button(anim_name)

func _on_player_collapsed(reason: String) -> void:
	if lbl_tier1_vital_status:
		lbl_tier1_vital_status.text = "COLAPSO: %s" % reason
		lbl_tier1_vital_status.modulate = Color(1.0, 0.1, 0.1)

func _setup_sensory_ui() -> void:
	panel_sensory = PanelContainer.new()
	panel_sensory.name = "SensoryPanel"
	panel_sensory.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	panel_sensory.mouse_filter = Control.MOUSE_FILTER_IGNORE

	var style = StyleBoxFlat.new()
	style.bg_color = Color(0.06, 0.08, 0.11, 0.90)
	style.border_color = Color(0.25, 0.45, 0.65, 0.8)
	style.set_border_width_all(1)
	style.set_corner_radius_all(6)
	style.content_margin_left = 10
	style.content_margin_right = 10
	style.content_margin_top = 6
	style.content_margin_bottom = 6
	panel_sensory.add_theme_stylebox_override("panel", style)

	var vbox = VBoxContainer.new()
	vbox.add_theme_constant_override("separation", 2)
	panel_sensory.add_child(vbox)

	var hbox_header = HBoxContainer.new()
	vbox.add_child(hbox_header)

	var lbl_title = Label.new()
	lbl_title.text = "💭 PERCEPCIÓN SOMÁTICA"
	lbl_title.add_theme_font_size_override("font_size", 12)
	lbl_title.add_theme_color_override("font_color", Color(0.65, 0.75, 0.85))
	lbl_title.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	hbox_header.add_child(lbl_title)

	label_sensory_mood = Label.new()
	label_sensory_mood.text = "🌿 Confortable"
	label_sensory_mood.add_theme_font_size_override("font_size", 12)
	label_sensory_mood.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	hbox_header.add_child(label_sensory_mood)

	label_sensory_thought = Label.new()
	label_sensory_thought.text = "«...»"
	label_sensory_thought.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	label_sensory_thought.add_theme_font_size_override("font_size", 13)
	label_sensory_thought.add_theme_color_override("font_color", Color(0.95, 0.95, 0.95))
	vbox.add_child(label_sensory_thought)
