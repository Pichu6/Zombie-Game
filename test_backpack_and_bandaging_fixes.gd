@tool
extends SceneTree

const BodyPartClass = preload("res://src/biology/body_part.gd")
const EnvironmentSystemClass = preload("res://src/biology/environment_system.gd")

func _init() -> void:
	print("\n=== INICIANDO BATERÍA DE PRUEBAS: FIXES DE MOCHILA, VENDAJE Y CLÍNICA ===")
	var scn = load("res://scenes/biology_showroom.tscn")
	var inst = scn.instantiate()
	root.add_child(inst)

	for f in range(3):
		await process_frame

	var char_node = inst.find_child("HumanCharacter", true, false)
	var dashboard = inst.find_child("UI", true, false)
	assert(char_node != null, "HumanCharacter no encontrado")
	assert(dashboard != null, "Nodo UI no encontrado")
	var b = char_node.body
	assert(b != null, "BodySystem no inicializado")
	assert(dashboard.paperdoll != null, "Paperdoll no inicializado")

	# -------------------------------------------------------------------------
	# TEST 1: HERIDA Y COMPORTAMIENTO PRE-VENDAJE
	# -------------------------------------------------------------------------
	print("\n--- TEST 1: Inducción de Herida en Brazo Izquierdo ---")
	dashboard._on_paperdoll_part_selected(BodyPartClass.PartType.LEFT_ARM)
	var left_arm: BodyPartClass = b.body_parts[BodyPartClass.PartType.LEFT_ARM]
	assert(left_arm != null, "Parte brazo izquierdo no encontrada")

	dashboard._on_action_wound_test_pressed()
	assert(left_arm.bleeding_rate_base == 15.0, "Hemorragia base esperada 15.0")
	var initial_bleed = left_arm.get_current_bleed_rate(b.cardio.heart_rate_bpm)
	assert(initial_bleed > 10.0, "Tasa de sangrado actual debe ser > 10 mL/s")
	assert(b._get_active_bleeding_part_count() == 1, "Debe haber 1 parte con hemorragia activa")
	var arm_color_wound = dashboard.paperdoll._get_part_color(BodyPartClass.PartType.LEFT_ARM)
	assert(arm_color_wound == Color(0.85, 0.18, 0.18, 0.92), "La silueta debe pintar la parte herida en rojo vivo")
	print("  • Herida inducida: Hemorragia = %.1f mL/s | Color silueta = Rojo vivo" % initial_bleed)
	print(">>> ÉXITO: Herida registrada correctamente.")

	# -------------------------------------------------------------------------
	# TEST 2: VENDAJE HEMOSTÁTICO (PARAR EL SANGRADO)
	# -------------------------------------------------------------------------
	print("\n--- TEST 2: Aplicación de Vendaje Compresivo ---")
	dashboard._on_action_bandage_pressed()
	assert(left_arm.is_bandaged == true, "La parte debe estar marcada como is_bandaged = true")
	var post_bandage_bleed = left_arm.get_current_bleed_rate(b.cardio.heart_rate_bpm)
	assert(post_bandage_bleed == 0.0, "El sangrado tras el vendaje DEBE SER 0.0 mL/s (Hemostasia)")
	assert(b._get_active_bleeding_part_count() == 0, "No debe haber partes con sangrado activo tras vendar")
	assert(dashboard.btn_action_bandage.text == "🩹 Quitar Vendaje", "El texto del botón debe ser '🩹 Quitar Vendaje'")
	var arm_color_bandaged = dashboard.paperdoll._get_part_color(BodyPartClass.PartType.LEFT_ARM)
	assert(arm_color_bandaged == Color(0.20, 0.55, 0.75, 0.90), "La silueta debe pintar la parte vendada en cian/azul zafiro")
	print("  • Tras vendar: Hemorragia = %.1f mL/s | Botón = '%s' | Color silueta = Azul/Cian vendaje" % [
		post_bandage_bleed, dashboard.btn_action_bandage.text
	])
	print(">>> ÉXITO: El vendaje detiene el sangrado al 100% y actualiza UI y silueta.")

	# -------------------------------------------------------------------------
	# TEST 3: RETIRAR VENDAJE Y REANUDAR SANGRADO
	# -------------------------------------------------------------------------
	print("\n--- TEST 3: Retirar Vendaje y Reanudación de Hemorragia ---")
	dashboard._on_action_bandage_pressed()
	assert(left_arm.is_bandaged == false, "El vendaje debe retirarse")
	var resumed_bleed = left_arm.get_current_bleed_rate(b.cardio.heart_rate_bpm)
	assert(resumed_bleed > 10.0, "El sangrado debe reanudarse al retirar el vendaje si la herida sigue abierta")
	assert(dashboard.btn_action_bandage.text == "🩹 Vendar", "El texto del botón debe volver a '🩹 Vendar'")
	print("  • Vendaje retirado: Hemorragia reanudada = %.1f mL/s" % resumed_bleed)
	print(">>> ÉXITO: Dinámica de desvendado validada.")

	# -------------------------------------------------------------------------
	# TEST 4: ACCIÓN SANAR / SUTURAR
	# -------------------------------------------------------------------------
	print("\n--- TEST 4: Acción Clínica Sanar / Suturar Herida ---")
	dashboard._on_action_heal_pressed()
	assert(left_arm.bleeding_rate_base == 0.0, "Hemorragia base debe ser 0.0 tras sanar")
	assert(left_arm.integrity == 100.0, "Integridad debe ser 100.0")
	assert(left_arm.pain_level == 0.0, "Dolor debe ser 0.0")
	var arm_color_healed = dashboard.paperdoll._get_part_color(BodyPartClass.PartType.LEFT_ARM)
	assert(arm_color_healed == Color(0.18, 0.52, 0.42, 0.85), "La silueta debe volver al verde médico sano")
	print("  • Parte completamente sanada e íntegra.")
	print(">>> ÉXITO: Sanado clínico validado.")

	# -------------------------------------------------------------------------
	# TEST 5: CLIC Y SELECCIÓN DE MOCHILA (APARECE ABAJO SIN REEMPLAZAR MENÚS)
	# -------------------------------------------------------------------------
	print("\n--- TEST 5: Clic en Mochila e Inspector Abajo (Sin reemplazar menús) ---")
	dashboard._on_paperdoll_backpack_selected()
	assert(dashboard.is_inspecting_backpack == true, "Debe estar en modo inspección de mochila")
	assert(dashboard.container_inspector_backpack.visible == true, "Panel inspector de mochila visible abajo")
	assert(dashboard.container_inspector_part.visible == true, "El panel de la parte anatómica NO debe desaparecer (no reemplazar estilo menús)")
	assert(dashboard.option_backpack_model.item_count == 5, "Deben haberse poblado 5 opciones de mochila (ninguna + 4 modelos)")
	print("  • Inspector de mochila abierto ABAJO sin ocultar la parte anatómica. Modelos: %d" % dashboard.option_backpack_model.item_count)

	var assault_idx = -1
	for i in range(dashboard.option_backpack_model.item_count):
		if dashboard.option_backpack_model.get_item_metadata(i) == "backpack_assault":
			assault_idx = i
			break
	assert(assault_idx != -1, "Mochila de asalto debe encontrarse en el dropdown")

	dashboard._on_backpack_model_selected(assault_idx)
	assert(b.equipped_backpack_id == "backpack_assault", "Mochila de asalto debe estar equipada en el cuerpo")
	assert(dashboard.slider_backpack_content_weight.editable == true, "El slider de peso debe ser editable")
	assert(dashboard.slider_backpack_content_weight.max_value == 25.0, "La capacidad máxima debe ser 25.0 kg")
	print("  • Mochila equipada: 'backpack_assault' (Máx 25 kg)")

	dashboard._on_backpack_content_slider_changed(15.5)
	assert(b.backpack_content_weight_kg == 15.5, "El peso interno debe ser 15.5 kg")
	var bp_total_w = b.get_backpack_total_weight_kg()
	assert(is_equal_approx(bp_total_w, 1.65 + 15.5), "Peso total de mochila debe ser ~17.15 kg")
	assert(dashboard.paperdoll.highlighted_part_types.has(BodyPartClass.PartType.TORSO_UPPER), "El torso debe estar destacado")
	print("  • Carga ajustada: 15.5 kg | Peso total mochila: %.2f kg | Silueta: Highlight en Torso" % bp_total_w)

	# Probar volver atrás / cerrar mochila haciendo clic de nuevo (toggle)
	print("  • Probando volver atrás con toggle de mochila...")
	dashboard._on_paperdoll_backpack_selected()
	assert(dashboard.is_inspecting_backpack == false, "Mochila debe cerrarse al hacer clic de nuevo en el icono")
	assert(dashboard.container_inspector_backpack.visible == false, "Panel de mochila debe ocultarse")
	assert(dashboard.container_inspector_part.visible == true, "El panel de la parte anatómica sigue intacto arriba")
	print("  • Mochila cerrada con toggle exitosamente. La parte anatómica sigue visible arriba.")

	# Reabrir mochila para verificar botón de cerrar directo
	dashboard._on_paperdoll_backpack_selected()
	assert(dashboard.container_inspector_backpack.visible == true, "Mochila reabierta")
	dashboard._on_paperdoll_part_selected(BodyPartClass.PartType.HEAD)
	assert(dashboard.active_inspected_part == BodyPartClass.PartType.HEAD, "Parte activa debe ser Cabeza")
	assert(dashboard.container_inspector_part.visible == true, "Parte Cabeza visible arriba")
	assert(dashboard.container_inspector_backpack.visible == true, "Mochila permanece disponible abajo")
	print("  • Cambio de parte a Cabeza mientras la mochila está abajo validado.")

	dashboard._on_backpack_model_selected(0)
	assert(b.equipped_backpack_id == "", "La mochila debe haberse desequipado")
	assert(dashboard.slider_backpack_content_weight.editable == false, "El slider debe ser ineditable")
	print("  • Mochila desequipada: Carga = 0 kg")
	print(">>> ÉXITO: Inspector de mochila apilable abajo y navegación fluida sin bloqueos.")

	# -------------------------------------------------------------------------
	# TEST 6: DROPDOWNS DE SUSTRATO Y VEGETACIÓN
	# -------------------------------------------------------------------------
	print("\n--- TEST 6: Selección de Sustrato y Vegetación con get_item_id ---")
	var mud_idx = -1
	for i in range(dashboard.option_env_substrate.item_count):
		if dashboard.option_env_substrate.get_item_id(i) == EnvironmentSystemClass.SubstrateType.THICK_MUD:
			mud_idx = i
			break
	assert(mud_idx != -1, "Sustrato de barro debe existir")
	dashboard._on_env_substrate_selected(mud_idx)
	assert(b.environment.substrate == EnvironmentSystemClass.SubstrateType.THICK_MUD, "El sustrato del ambiente debe ser THICK_MUD")
	print("  • Sustrato seleccionado: THICK_MUD | Adhesión en calzado = %.1f kg" % b.environment.get_mud_adhesion_weight_kg())

	var veg_idx = -1
	for i in range(dashboard.option_env_vegetation.item_count):
		if dashboard.option_env_vegetation.get_item_id(i) == EnvironmentSystemClass.VegetationDensity.THORNY_BRUSH:
			veg_idx = i
			break
	assert(veg_idx != -1, "Vegetación de matorral debe existir")
	dashboard._on_env_vegetation_selected(veg_idx)
	assert(b.environment.vegetation == EnvironmentSystemClass.VegetationDensity.THORNY_BRUSH, "La vegetación debe ser THORNY_BRUSH")
	print("  • Vegetación seleccionada: THORNY_BRUSH")
	print(">>> ÉXITO: Dropdowns de entorno y sustrato validados.")

	print("\n=========================================================================")
	print("¡TODAS LAS PRUEBAS DE FIXES DE MOCHILA, VENDAJE Y CLÍNICA PASARON AL 100%!")
	print("=========================================================================\n")

	quit(0)
