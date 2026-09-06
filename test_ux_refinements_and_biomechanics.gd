extends SceneTree

const SHOWROOM_PATH = "res://scenes/biology_showroom.tscn"
const BodySystemClass = preload("res://src/biology/body_system.gd")
const BodyPartClass = preload("res://src/biology/body_part.gd")
const SilhouettePaperdollClass = preload("res://src/ui/silhouette_paperdoll.gd")

func _init() -> void:
	print("\n=========================================================================")
	print("=== TEST: REFINAMIENTOS UX, SINCRONIZACIÓN DE ACCIÓN Y MENÚ CONTEXTUAL ===")
	print("=========================================================================\n")

	var res = ResourceLoader.load(SHOWROOM_PATH)
	assert(res != null, "ERROR: No se pudo cargar biology_showroom.tscn")

	var inst = res.instantiate()
	root.add_child(inst)

	await process_frame
	await process_frame

	var char_node = inst.find_child("HumanCharacter", true, false)
	var dashboard = inst.find_child("UI", true, false)
	assert(char_node != null, "HumanCharacter no encontrado")
	assert(dashboard != null, "UI BiologyDashboard no encontrado")

	var body: BodySystemClass = char_node.body
	assert(body != null, "BodySystem no inicializado")

	dashboard._process(0.016)
	dashboard._on_telemetry_updated()
	dashboard._update_posture_display()

	# -------------------------------------------------------------------------
	# PRUEBA 1: Sincronización del Slider de Movimiento con la Etiqueta de Acción
	# -------------------------------------------------------------------------
	print("--- PRUEBA 1: Sincronización de Acción y Slider de Movimiento ---")
	print("  • Acción inicial: %s" % dashboard.current_action_name)
	assert(dashboard.current_action_name == "Reposo", "Debe iniciar en Reposo")

	# Mover slider al 10% (el caso reportado por el usuario)
	dashboard._on_exertion_slider_changed(10.0)
	dashboard._process(0.016)
	print("  • Tras mover slider al 10%%: Acción = %s" % dashboard.current_action_name)
	assert(dashboard.current_action_name == "Caminando (10%)", "Acción debe ser 'Caminando (10%)', pero fue: " + dashboard.current_action_name)
	assert("Caminando (10%)" in dashboard.lbl_action_duration.text, "La barra inferior debe reflejar Caminando (10%)")

	# Mover slider al 50%
	dashboard._on_exertion_slider_changed(50.0)
	dashboard._process(0.016)
	print("  • Tras mover slider al 50%%: Acción = %s" % dashboard.current_action_name)
	assert(dashboard.current_action_name == "Corriendo (50%)", "Acción debe ser 'Corriendo (50%)'")

	# Mover slider al 100%
	dashboard._on_exertion_slider_changed(100.0)
	dashboard._process(0.016)
	print("  • Tras mover slider al 100%%: Acción = %s" % dashboard.current_action_name)
	assert(dashboard.current_action_name == "Sprint (100%)", "Acción debe ser 'Sprint (100%)'")

	# Volver a reposo
	dashboard._on_exertion_slider_changed(0.0)
	dashboard._process(0.016)
	print("  • Tras volver a 0%%: Acción = %s" % dashboard.current_action_name)
	assert(dashboard.current_action_name == "Reposo", "Acción debe regresar a Reposo")
	print(">>> ÉXITO: Sincronización de slider y etiqueta de acción validada al 100%.\n")

	# -------------------------------------------------------------------------
	# PRUEBA 2: Desduplicación de Telemetría y Timers
	# -------------------------------------------------------------------------
	print("--- PRUEBA 2: Desduplicación de Telemetría y Timers ---")
	dashboard.paperdoll.set_layer_mode(SilhouettePaperdollClass.LayerMode.ALL)
	dashboard._on_telemetry_updated()
	dashboard._process(0.016)
	
	print("  • Ficha Anatomía Temp/Aislamiento: %s" % dashboard.lbl_tier1_temp_pain.text)
	assert("Aislamiento" in dashboard.lbl_tier1_temp_pain.text, "Debe contener Aislamiento Clo")
	assert("Agua en Ropa" in dashboard.lbl_tier1_temp_pain.text, "Debe contener Agua en Ropa")
	
	print("  • Ficha Anatomía Traumatología: %s" % dashboard.lbl_tier1_subsystem_telemetry.text)
	assert("Fracturas" in dashboard.lbl_tier1_subsystem_telemetry.text, "Debe contener Fracturas")
	assert("Hemorragias" in dashboard.lbl_tier1_subsystem_telemetry.text, "Debe contener Hemorragias")
	assert(not ("Sangre:" in dashboard.lbl_tier1_subsystem_telemetry.text), "No debe repetir Sangre mL (ya está en la barra)")
	assert(not ("BPM" in dashboard.lbl_tier1_subsystem_telemetry.text), "No debe repetir BPM")
	
	print("  • Barra Inferior de Acción: %s" % dashboard.lbl_action_duration.text)
	assert(not ("⏱️ Sim:" in dashboard.lbl_action_duration.text), "No debe duplicar reloj de simulación en barra inferior")
	assert(dashboard.lbl_action_duration.text.begins_with("🏃 Estado:"), "Barra inferior debe comenzar con Estado")
	print(">>> ÉXITO: Telemetría y temporizadores limpios sin duplicados.\n")

	# -------------------------------------------------------------------------
	# PRUEBA 3: Ubicación de la Barra de Movimiento debajo de la Silueta
	# -------------------------------------------------------------------------
	print("--- PRUEBA 3: Ubicación del Slider de Movimiento bajo la Silueta ---")
	var sil_parent = dashboard.paperdoll.get_parent()
	assert(sil_parent != null, "Paperdoll debe tener contenedor padre")
	assert(sil_parent.is_ancestor_of(dashboard.slider_workplace_exertion), "El slider de movimiento debe residir en el contenedor de la silueta")
	print("  • Jerarquía confirmada: Silueta y Slider de Movimiento comparten el contenedor central.")
	print(">>> ÉXITO: Slider de movimiento reubicado con éxito debajo de la silueta.\n")

	# -------------------------------------------------------------------------
	# PRUEBA 4: Exclusión Mutua entre Posturas y Locomoción
	# -------------------------------------------------------------------------
	print("--- PRUEBA 4: Reglas Biomecánicas de Postura y Locomoción ---")
	
	# SENTADO
	dashboard._set_posture_from_ui(BodySystemClass.Posture.SITTING)
	dashboard._update_posture_display()
	print("  • Postura SENTADO:")
	print("    - Caminar disabled: %s, Carrera disabled: %s, Sprint disabled: %s" % [
		dashboard.btn_anim_walk.disabled, dashboard.btn_anim_run.disabled, dashboard.btn_anim_sprint.disabled
	])
	print("    - Slider editable: %s, Slider value: %s" % [
		dashboard.slider_workplace_exertion.editable, dashboard.slider_workplace_exertion.value
	])
	assert(dashboard.btn_anim_walk.disabled == true, "Caminar debe estar deshabilitado al estar sentado")
	assert(dashboard.btn_anim_run.disabled == true, "Carrera debe estar deshabilitada al estar sentado")
	assert(dashboard.btn_anim_sprint.disabled == true, "Sprint debe estar deshabilitado al estar sentado")
	assert(dashboard.slider_workplace_exertion.editable == false, "Slider debe estar bloqueado al estar sentado")
	assert(dashboard.slider_workplace_exertion.value == 0.0, "Slider debe estar en 0 al estar sentado")
	assert(dashboard.btn_posture_stand.disabled == false, "Ponerse de pie debe estar habilitado")

	# ACOSTADO
	dashboard._set_posture_from_ui(BodySystemClass.Posture.LYING)
	dashboard._update_posture_display()
	print("  • Postura ACOSTADO:")
	assert(dashboard.btn_anim_walk.disabled == true, "Caminar debe estar deshabilitado al estar acostado")
	assert(dashboard.slider_workplace_exertion.editable == false, "Slider bloqueado al estar acostado")

	# AGACHADO
	dashboard._set_posture_from_ui(BodySystemClass.Posture.CROUCHING)
	dashboard._update_posture_display()
	print("  • Postura AGACHADO:")
	print("    - Caminar disabled: %s (Permite marcha agachada)" % dashboard.btn_anim_walk.disabled)
	print("    - Carrera disabled: %s, Sprint disabled: %s" % [dashboard.btn_anim_run.disabled, dashboard.btn_anim_sprint.disabled])
	print("    - Slider max_value: %s" % dashboard.slider_workplace_exertion.max_value)
	assert(dashboard.btn_anim_walk.disabled == false, "Marcha agachada debe estar disponible")
	assert(dashboard.btn_anim_run.disabled == true, "No se puede correr agachado")
	assert(dashboard.btn_anim_sprint.disabled == true, "No se puede sprintar agachado")
	assert(dashboard.slider_workplace_exertion.max_value == 35.0, "Esfuerzo agachado debe tener tope en 35%")

	dashboard._on_exertion_slider_changed(20.0)
	print("  • Tras esfuerzo de marcha (20%%): Postura = %s, Acción = %s" % [body.get_posture_name(), dashboard.current_action_name])
	assert(body.current_posture == BodySystemClass.Posture.STANDING, "Locomoción debe levantar a STANDING")
	assert(dashboard.current_action_name == "Caminando (20%)", "Acción debe ser 'Caminando (20%)'")

	# DE PIE
	dashboard._set_posture_from_ui(BodySystemClass.Posture.STANDING)
	dashboard._update_posture_display()
	print("  • Postura DE PIE:")
	assert(dashboard.btn_anim_walk.disabled == false, "Caminar disponible de pie")
	assert(dashboard.btn_anim_run.disabled == false, "Carrera disponible de pie")
	assert(dashboard.btn_anim_sprint.disabled == false, "Sprint disponible de pie")
	assert(dashboard.slider_workplace_exertion.max_value == 100.0, "Tope de esfuerzo en 100%")

	# Mientras corre a alta velocidad, no puede sentarse ni acostarse de golpe
	dashboard._on_exertion_slider_changed(75.0)
	dashboard._update_posture_display()
	print("  • Durante Sprint (75%%): Sentarse disabled = %s, Acostarse disabled = %s" % [
		dashboard.btn_posture_sit.disabled, dashboard.btn_posture_lie.disabled
	])
	assert(dashboard.btn_posture_sit.disabled == true, "No puede sentarse mientras sprintea")
	assert(dashboard.btn_posture_lie.disabled == true, "No puede acostarse mientras sprintea")
	print(">>> ÉXITO: Reglas de exclusión postural y locomotora validadas al 100%.\n")

	# -------------------------------------------------------------------------
	# PRUEBA 5: Menú Contextual con Submenús y No Modificación Inmediata
	# -------------------------------------------------------------------------
	print("--- PRUEBA 5: Menú Contextual, Submenús y Selección de Detalles ---")
	dashboard._on_exertion_slider_changed(0.0)
	assert(dashboard.active_inspected_part == -1, "Parte activa inicial debe ser -1")
	
	var arm_id = BodyPartClass.PartType.LEFT_ARM
	
	# Simular clic derecho en el brazo izquierdo
	dashboard._on_paperdoll_context_menu(arm_id, Vector2(400, 300))
	print("  • Tras clic derecho en Brazo Izquierdo:")
	print("    - _menu_target_part = %s" % dashboard._menu_target_part)
	print("    - active_inspected_part = %s" % dashboard.active_inspected_part)
	assert(dashboard._menu_target_part == arm_id, "_menu_target_part debe registrar el brazo")
	assert(dashboard.active_inspected_part == -1, "active_inspected_part NO debe cambiar inmediatamente")
	
	# Verificar existencia de submenús
	assert(dashboard.submenu_aid != null, "Submenú de primeros auxilios debe existir")
	assert(dashboard.submenu_sim != null, "Submenú de simulación clínica debe existir")
	assert(dashboard.submenu_equip != null, "Submenú de equipamiento debe existir")
	print("    - Submenús validados: Primeros Auxilios, Simulación Clínica, Equipamiento y Capas.")

	# Ejecutar acción de vendaje desde el menú en la parte seleccionada por click derecho
	var arm_part = body.body_parts[arm_id]
	assert(arm_part.is_bandaged == false, "Brazo no debe estar vendado aún")
	dashboard._on_context_menu_action(0) # 0 = Vendar
	assert(arm_part.is_bandaged == true, "El vendaje debe haberse aplicado al brazo izquierdo por _menu_target_part")
	print("  • Acción clínica aplicada sobre target del menú sin haber cambiado active_inspected_part: is_bandaged = %s" % arm_part.is_bandaged)
	assert(dashboard.active_inspected_part == -1, "active_inspected_part sigue en -1 tras la acción rápida")

	# Seleccionar "Examinar Detalles" (id: 10)
	dashboard._on_context_menu_action(10)
	print("  • Tras presionar 'Examinar Detalles': active_inspected_part = %s" % dashboard.active_inspected_part)
	assert(dashboard.active_inspected_part == arm_id, "active_inspected_part ahora debe ser LEFT_ARM")
	assert(dashboard.paperdoll.selected_part_type == arm_id, "La silueta debe resaltar LEFT_ARM")

	# Clic derecho en espacio vacío cierra el menú
	dashboard._on_paperdoll_context_menu(-1, Vector2.ZERO)
	assert(dashboard._menu_target_part == -1, "_menu_target_part debe resetearse")
	print(">>> ÉXITO: Menú contextual, submenús y flujo de inspección validados al 100%.\n")

	print("=========================================================================")
	print("¡TODAS LAS PRUEBAS DE REFINAMIENTOS UX Y BIOMECÁNICA PASARON AL 100%!")
	print("=========================================================================")
	quit(0)
