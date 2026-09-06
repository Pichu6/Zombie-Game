extends SceneTree

const SHOWROOM_PATH = "res://scenes/biology_showroom.tscn"
const BodySystemClass = preload("res://src/biology/body_system.gd")

func _init() -> void:
	print("\n=========================================================================")
	print("=== TEST: UNIFICACIÓN DE DOCK IZQUIERDO Y SISTEMA DE POSTURA BIOMECÁNICO ===")
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
	
	print(">>> HumanCharacter y BiologyDashboard cargados correctamente.")

	# -------------------------------------------------------------------------
	# TEST 1: Verificar Unificación en el Dock Izquierdo (Todo Junto)
	# -------------------------------------------------------------------------
	print("\n--- TEST 1: Verificación de Tarjetas Unificadas en Dock Izquierdo ---")
	assert(dashboard.lbl_posture_display != null, "lbl_posture_display no existe")
	assert(dashboard.btn_posture_stand != null, "btn_posture_stand no existe")
	assert(dashboard.btn_posture_sit != null, "btn_posture_sit no existe")
	assert(dashboard.btn_posture_lie != null, "btn_posture_lie no existe")
	assert(dashboard.btn_posture_crouch != null, "btn_posture_crouch no existe")
	assert(dashboard.slider_workplace_exertion != null, "slider_workplace_exertion no existe")
	assert(dashboard.bar_blood != null, "bar_blood no existe")
	assert(dashboard.label_bpm_val != null, "label_bpm_val no existe")
	assert(dashboard.bar_glycogen != null, "bar_glycogen no existe")
	assert(dashboard.container_inspector_part != null, "container_inspector_part no existe")
	assert(dashboard.panel_item_card != null, "panel_item_card no existe")
	
	# Verificar que el texto de postura inicial refleje bipedestación y masa
	print("  • Texto Postural Inicial: " + dashboard.lbl_posture_display.text)
	assert("De pie" in dashboard.lbl_posture_display.text or "De Pie" in dashboard.lbl_posture_display.text, "Texto postural no indica De pie")
	assert(str(int(body.get_total_moving_mass_kg())) in dashboard.lbl_posture_display.text, "Texto postural no incluye la masa total")
	print(">>> ÉXITO: Dock Izquierdo contiene Fisiología, Esfuerzo/Postura y Anatomía/Equipo unificados.")

	# -------------------------------------------------------------------------
	# TEST 2: Transiciones de Postura (De Pie, Sentado, Acostado, Agachado)
	# -------------------------------------------------------------------------
	print("\n--- TEST 2: Transiciones de Postura y Clamping de Esfuerzo ---")
	
	# Transición a SENTADO
	char_node.set_character_posture(BodySystemClass.Posture.SITTING)
	dashboard._update_posture_display()
	print("  • Postura SITTING: " + body.get_posture_name() + " | Texto: " + dashboard.lbl_posture_display.text)
	assert(body.current_posture == BodySystemClass.Posture.SITTING, "Fallo al cambiar a SITTING")
	assert(body.exertion_factor == 0.0, "El esfuerzo debe ser 0.0 al estar sentado")
	assert("Sentado" in dashboard.lbl_posture_display.text, "Texto UI no indica Sentado")
	
	# Transición a ACOSTADO
	char_node.set_character_posture(BodySystemClass.Posture.LYING)
	dashboard._update_posture_display()
	print("  • Postura LYING: " + body.get_posture_name() + " | Texto: " + dashboard.lbl_posture_display.text)
	assert(body.current_posture == BodySystemClass.Posture.LYING, "Fallo al cambiar a LYING")
	assert(body.exertion_factor == 0.0, "El esfuerzo debe ser 0.0 al estar acostado")
	assert("Acostado" in dashboard.lbl_posture_display.text, "Texto UI no indica Acostado")
	
	# Transición a AGACHADO
	char_node.set_character_posture(BodySystemClass.Posture.CROUCHING)
	dashboard._update_posture_display()
	print("  • Postura CROUCHING: " + body.get_posture_name() + " | Texto: " + dashboard.lbl_posture_display.text)
	assert(body.current_posture == BodySystemClass.Posture.CROUCHING, "Fallo al cambiar a CROUCHING")
	assert(char_node.is_crouched == true, "is_crouched debe ser true")
	assert("Agachado" in dashboard.lbl_posture_display.text, "Texto UI no indica Agachado")
	
	# Locomoción hace que el personaje se ponga de pie automáticamente
	char_node.set_locomotion_effort(0.35)
	dashboard._update_posture_display()
	print("  • Tras esfuerzo locomotor (0.35): " + body.get_posture_name() + " | is_crouched = " + str(char_node.is_crouched))
	assert(body.current_posture == BodySystemClass.Posture.STANDING, "Locomoción debe levantar al personaje a STANDING")
	assert(char_node.is_crouched == false, "is_crouched debe cancelarse al iniciar marcha")
	print(">>> ÉXITO: Transiciones posturales y reactividad locomotora validadas.")

	# -------------------------------------------------------------------------
	# TEST 3: Simulación de Horas de Pie con Carga Pesada (Sobrecarga Plantar y Lumbar)
	# -------------------------------------------------------------------------
	print("\n--- TEST 3: Biomecánica de Horas Parado con Mochila Pesada ---")
	char_node.reset_character_state()
	body.equip_backpack("backpack_assault", 18.0) # 18 kg de carga interna + 1.65 kg mochila = 19.65 kg
	assert(body.get_backpack_total_weight_kg() >= 19.5, "Mochila no equipada correctamente")
	
	var left_foot = body.body_parts[BodySystemClass.BodyPartClass.PartType.LEFT_FOOT]
	var right_foot = body.body_parts[BodySystemClass.BodyPartClass.PartType.RIGHT_FOOT]
	var torso_lower = body.body_parts[BodySystemClass.BodyPartClass.PartType.TORSO_LOWER]
	
	left_foot.pain_level = 0.0
	right_foot.pain_level = 0.0
	torso_lower.pain_level = 0.0
	body.continuous_standing_seconds = 0.0
	body.exertion_factor = 0.0
	
	# Simular 2 horas de pie estático (7200 segundos)
	var sim_step: float = 1.0
	var total_time: float = 7200.0
	var steps: int = int(total_time / sim_step)
	for i in range(steps):
		body.update(sim_step)
	
	print("  • Tras 2 horas de pie estático:")
	print("    - Tiempo continuo de pie: %.1f horas" % (body.continuous_standing_seconds / 3600.0))
	print("    - Dolor Pie Izquierdo: %.2f / 100" % left_foot.pain_level)
	print("    - Dolor Pie Derecho: %.2f / 100" % right_foot.pain_level)
	print("    - Dolor Lumbar (Torso Inferior): %.2f / 100" % torso_lower.pain_level)
	print("    - Texto Estado Postural: " + body.get_posture_status_text())
	
	assert(left_foot.pain_level > 2.0, "El dolor en pie izquierdo debio acumularse tras 2h de pie")
	assert(right_foot.pain_level > 2.0, "El dolor en pie derecho debio acumularse tras 2h de pie")
	assert(torso_lower.pain_level > 0.5, "La compresion lumbar debio acumularse por mochila pesada > 1.5h")
	assert("Sobrecarga postural" in body.get_posture_status_text(), "Texto no advierte sobrecarga postural")
	print(">>> ÉXITO: Acumulación de dolor plantar y lumbar tras horas de pie validada.")

	# -------------------------------------------------------------------------
	# TEST 4: Alivio al Sentarse y Descompresión al Acostarse
	# -------------------------------------------------------------------------
	print("\n--- TEST 4: Alivio de Sobrecarga al Sentarse y Descompresión al Acostarse ---")
	var pain_feet_before_sit = left_foot.pain_level
	var pain_lumbar_before_sit = torso_lower.pain_level
	
	# Sentarse durante 10 minutos (600 segundos)
	char_node.set_character_posture(BodySystemClass.Posture.SITTING)
	for i in range(600):
		body.update(1.0)
	
	print("  • Tras 10 minutos sentado:")
	print("    - Dolor Pies: %.2f (Antes: %.2f)" % [left_foot.pain_level, pain_feet_before_sit])
	print("    - Dolor Lumbar: %.2f (Antes: %.2f)" % [torso_lower.pain_level, pain_lumbar_before_sit])
	print("    - Texto Postura: " + body.get_posture_status_text())
	
	assert(left_foot.pain_level < pain_feet_before_sit, "El dolor de pies debio aliviarse al estar sentado")
	assert(torso_lower.pain_level < pain_lumbar_before_sit, "El dolor lumbar debio aliviarse al estar sentado")
	
	# Acostarse durante 10 minutos (600 segundos) con estamina gastada
	body.stamina = 20.0
	char_node.set_character_posture(BodySystemClass.Posture.LYING)
	for i in range(600):
		body.update(1.0)
		
	print("  • Tras acostarse:")
	print("    - Dolor Pies: %.2f | Dolor Lumbar: %.2f" % [left_foot.pain_level, torso_lower.pain_level])
	print("    - Estamina recuperada: %.1f / 100" % body.stamina)
	print("    - Texto Postura: " + body.get_posture_status_text())
	assert(body.stamina >= 99.0, "La estamina debio recuperarse rapidamente al estar acostado")
	print(">>> ÉXITO: Mecánicas de alivio postural y recuperación por descompresión validadas.")

	# -------------------------------------------------------------------------
	# TEST 5: Pensamientos Somáticos Sensoriales de Postura
	# -------------------------------------------------------------------------
	print("\n--- TEST 5: Motor de Pensamientos Sensoriales y Somáticos de Postura ---")
	char_node.set_character_posture(BodySystemClass.Posture.LYING)
	body.time_in_posture_seconds = 30.0
	var rep_lying = body.get_sensory_report()
	print("  • Reporte Acostado: Mood = " + rep_lying.mood_tag + " | Pensamiento = «" + rep_lying.primary_thought + "»")
	assert("Descompresión" in rep_lying.mood_tag, "Mood tag de acostado incorrecto")
	
	body.set_posture(BodySystemClass.Posture.STANDING)
	body.continuous_standing_seconds = 8000.0
	var rep_standing = body.get_sensory_report()
	print("  • Reporte Bipedestación Prolongada: Mood = " + rep_standing.mood_tag + " | Pensamiento = «" + rep_standing.primary_thought + "»")
	assert("Bipedestación Prolongada" in rep_standing.mood_tag, "Mood tag de bipedestación incorrecto")
	
	# Somatic tags
	var has_posture_tag = false
	for tag in rep_standing.somatic_tags:
		if "De pie" in tag:
			has_posture_tag = true
			print("    -> Tag somático encontrado: " + tag)
	assert(has_posture_tag, "Somatic tags no contiene información de la postura")
	print(">>> ÉXITO: Percepción sensorial y pensamientos somáticos de postura validados.")

	print("\n=========================================================================")
	print("=== ¡TODAS LAS PRUEBAS DE UNIFICACIÓN Y POSTURA PASARON AL 100%! ===")
	print("=========================================================================\n")
	
	inst.free()
	quit(0)
