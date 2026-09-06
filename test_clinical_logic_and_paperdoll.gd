class_name TestClinicalLogicAndPaperdoll
extends SceneTree

const SHOWROOM_PATH = "res://scenes/biology_showroom.tscn"
const BodyPartClass = preload("res://src/biology/body_part.gd")
const BodySystemClass = preload("res://src/biology/body_system.gd")
const ClothingDatabaseClass = preload("res://src/biology/clothing_database.gd")
const SilhouettePaperdollClass = preload("res://src/ui/silhouette_paperdoll.gd")

func _init() -> void:
	print("\n=========================================================================")
	print("=== TEST: LOGICA CLINICA, PAPERDOLL Y TEMPLATE 3D ===")
	print("=========================================================================\n")
	
	test_wound_types_and_healing()
	test_clothing_templates_and_sidebag()
	test_silhouette_paperdoll_layers_and_overlays()
	
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
	
	test_dashboard_context_menu_and_clinical_logic(dashboard, char_node.body)
	test_makehuman_3d_asset_bridge(dashboard, char_node.body)
	
	print("======================================================")
	print("TODAS LAS PRUEBAS DE LOGICA CLINICA Y PAPERDOLL PASARON EXITOSAMENTE!")
	print("======================================================")
	quit(0)

func test_wound_types_and_healing() -> void:
	print("[1/5] Verificando tipos de heridas graduadas y sanacion...")
	var part = BodyPartClass.new(BodyPartClass.PartType.RIGHT_FOREARM, "Antebrazo Derecho", 1.0, true, "mixamorig_RightForeArm")
	
	# Herida: Rasguño
	part.apply_wound(BodyPartClass.WoundType.SCRATCH)
	assert(part.active_wound_type == BodyPartClass.WoundType.SCRATCH, "Tipo de herida debe ser SCRATCH")
	assert(is_equal_approx(part.bleeding_rate_base, 0.1), "Bleed base de SCRATCH debe ser 0.1")
	assert(part.pain_level >= 5.0, "Dolor debe incrementarse con SCRATCH")
	
	# Herida: Laceración acumulativa
	part.apply_wound(BodyPartClass.WoundType.LACERATION)
	assert(part.active_wound_type == BodyPartClass.WoundType.LACERATION, "Tipo de herida debe ser LACERATION")
	assert(is_equal_approx(part.bleeding_rate_base, 0.9), "Bleed base acumulado debe ser 0.1 + 0.8 = 0.9")
	
	# Herida: Punzante/Mordida (con sepsis)
	part.apply_wound(BodyPartClass.WoundType.PUNCTURE_BITE)
	assert(part.infection_level >= 45.0, "Infeccion de mordida debe ser >= 45%")
	assert(not part.is_disinfected, "Herida de mordida no debe estar desinfectada")
	
	# Herida: Ruptura Arterial Masiva
	part.apply_wound(BodyPartClass.WoundType.ARTERIAL_RUPTURE)
	assert(part.bleeding_rate_base >= 4.5, "Bleed masivo debe ser >= 4.5")
	
	# Sanación completa
	part.has_fracture = true
	part.is_bandaged = true
	part.is_tourniquet_applied = true
	part.is_splinted = true
	part.heal_completely()
	assert(part.active_wound_type == BodyPartClass.WoundType.NONE, "WoundType debe ser NONE tras sanar")
	assert(part.bleeding_rate_base == 0.0, "Bleed base debe ser 0")
	assert(part.pain_level == 0.0, "Dolor debe ser 0")
	assert(part.infection_level == 0.0, "Infeccion debe ser 0")
	assert(not part.has_fracture, "Fractura debe removerse")
	assert(not part.is_bandaged, "Vendaje debe removerse")
	assert(not part.is_tourniquet_applied, "Torniquete debe removerse")
	assert(not part.is_splinted, "Ferula debe removerse")
	assert(part.is_disinfected, "Debe quedar aseptico")
	print("  -> OK: Tipos de heridas y sanacion validados.")

func test_clothing_templates_and_sidebag() -> void:
	print("[2/5] Verificando templates 3D y catalogo de bolsos/mochilas...")
	var sidebag = ClothingDatabaseClass.get_backpack("sidebag_tactical")
	assert(not sidebag.is_empty(), "Bolso tactico cruzado (sidebag_tactical) debe existir en catalogo")
	assert(sidebag.has("visual_template"), "Sidebag debe tener visual_template")
	assert(sidebag["visual_template"]["overlay_type"] == "sidebag_sling", "Overlay type debe ser sidebag_sling")
	assert(sidebag["visual_template"]["socket_bone"] == "mixamorig_Spine", "Socket bone debe ser mixamorig_Spine")
	
	var military_bp = ClothingDatabaseClass.get_backpack("backpack_military")
	assert(military_bp["visual_template"]["overlay_type"] == "backpack_harness", "Overlay type militar debe ser backpack_harness")
	
	var tpl_tshirt = ClothingDatabaseClass.get_item_visual_template("tshirt_cotton")
	assert(tpl_tshirt.has("socket_bone"), "Template de remera debe tener socket_bone")
	assert(tpl_tshirt.has("overlay_type"), "Template de remera debe tener overlay_type")
	print("  -> OK: Templates 3D y catalogo validados.")

func test_silhouette_paperdoll_layers_and_overlays() -> void:
	print("[3/5] Verificando modos de capa, X-ray y centroides del Paperdoll...")
	var paperdoll = SilhouettePaperdollClass.new()
	var root_ctrl = Control.new()
	root_ctrl.size = Vector2(800, 600)
	root_ctrl.add_child(paperdoll)
	paperdoll.size = Vector2(300, 500)
	paperdoll._ready()
	
	# Verificar orden de modos de capa
	assert(SilhouettePaperdollClass.LayerMode.BASE == 0, "LayerMode.BASE debe ser 0")
	assert(SilhouettePaperdollClass.LayerMode.MUSCULAR == 1, "LayerMode.MUSCULAR debe ser 1")
	assert(SilhouettePaperdollClass.LayerMode.SKELETAL == 2, "LayerMode.SKELETAL debe ser 2")
	assert(SilhouettePaperdollClass.LayerMode.CARDIO == 3, "LayerMode.CARDIO debe ser 3")
	assert(SilhouettePaperdollClass.LayerMode.NERVOUS == 4, "LayerMode.NERVOUS debe ser 4")
	assert(SilhouettePaperdollClass.LayerMode.ALL == 5, "LayerMode.ALL debe ser 5")
	
	# Verificar modo BASE sólido por defecto
	paperdoll.set_layer_mode(SilhouettePaperdollClass.LayerMode.BASE)
	assert(is_equal_approx(paperdoll._target_base_alpha, 1.0), "Target base alpha en BASE debe ser 1.0 (solido)")
	assert(is_equal_approx(paperdoll._target_skeletal_alpha, 0.0), "Target skeletal alpha en BASE sano debe ser 0.0")
	assert(is_equal_approx(paperdoll._target_muscular_alpha, 0.0), "Target muscular alpha en BASE sano debe ser 0.0")
	assert(is_equal_approx(paperdoll._target_cardio_alpha, 0.0), "Target cardio alpha en BASE sano debe ser 0.0")
	
	# Verificar modo MUSCULAR
	paperdoll.set_layer_mode(SilhouettePaperdollClass.LayerMode.MUSCULAR)
	assert(is_equal_approx(paperdoll._target_muscular_alpha, 1.0), "Muscular alpha en MUSCULAR debe ser 1.0")
	assert(is_equal_approx(paperdoll._target_base_alpha, 0.20), "Base alpha en MUSCULAR debe atenuarse a 0.20")
	assert(is_equal_approx(paperdoll._target_muscle_weight, 0.0), "Somatotipo muscle weight en MUSCULAR debe ser 0.0 (no distorsionar morfologia)")
	
	# Geometría y cálculo de centroides para las 17 zonas anatómicas
	paperdoll._update_layout()
	assert(paperdoll._part_polygons.size() == 17, "Deben existir 17 zonas anatomicas vectoriales")
	for p_type in paperdoll._part_polygons.keys():
		var poly: PackedVector2Array = paperdoll._part_polygons[p_type]
		var c = paperdoll._get_polygon_centroid(poly)
		assert(c != Vector2.ZERO, "Centroide de zona %d no debe ser cero" % p_type)
	
	root_ctrl.queue_free()
	print("  -> OK: Capas, base solida y muscular anatómico validados.")

func test_dashboard_context_menu_and_clinical_logic(dashboard, b: BodySystemClass) -> void:
	print("[4/5] Verificando menu contextual clinico, logica de decisiones y ausencia de botones en inspector derecho...")
	
	# 1. Verificar que los botones clínicos están fuera del panel visual inspector derecho
	assert(dashboard.hbox_clinical_actions != null, "hbox_clinical_actions existe para testing")
	assert(dashboard.hbox_clinical_actions.get_parent() == null, "hbox_clinical_actions NO debe estar en el arbol visible del inspector")
	assert(dashboard.lbl_inspector_clinical_hint != null, "lbl_inspector_clinical_hint debe existir")
	assert(dashboard.lbl_inspector_clinical_hint.get_parent() == dashboard.container_inspector_part, "El hint debe estar en el inspector")
	
	# 2. Verificar existencia de submenús clínicos y de heridas
	assert(dashboard.submenu_aid != null, "submenu_aid debe existir")
	assert(dashboard.submenu_sim != null, "submenu_sim debe existir")
	assert(dashboard.submenu_wounds != null, "submenu_wounds debe existir")
	assert(dashboard.submenu_wounds.get_item_count() == 5, "submenu_wounds debe tener las 5 heridas graduadas")
	
	# 3. Lógica clínica: Miembro sano (Brazo Derecho)
	var arm_id = BodyPartClass.PartType.RIGHT_FOREARM
	var arm_part: BodyPartClass = b.body_parts[arm_id]
	arm_part.heal_completely()
	
	# Abrir menú en miembro sano
	dashboard._on_paperdoll_context_menu(arm_id, Vector2(100, 100))
	
	# En miembro sano:
	assert(dashboard.submenu_aid.is_item_disabled(0), "Vendaje debe estar deshabilitado si no hay lesion")
	assert(dashboard.submenu_aid.is_item_disabled(1), "Torniquete debe estar deshabilitado si no hay hemorragia activa")
	assert(dashboard.submenu_aid.is_item_disabled(2), "Entablillado debe estar deshabilitado si no hay fractura")
	assert(dashboard.submenu_aid.is_item_disabled(3), "Desinfeccion debe estar deshabilitada en tejido limpio y aseptico")
	
	# 4. Lógica clínica: Provocar Laceración con hemorragia
	arm_part.apply_wound(BodyPartClass.WoundType.LACERATION)
	dashboard._on_paperdoll_context_menu(arm_id, Vector2(100, 100))
	
	# Ahora con hemorragia:
	assert(not dashboard.submenu_aid.is_item_disabled(0), "Vendaje debe estar habilitado con herida abierta")
	assert(not dashboard.submenu_aid.is_item_disabled(1), "Torniquete debe estar habilitado en miembro con sangrado")
	
	# 5. Aplicar Torniquete y verificar cambio de opción a retirar
	dashboard._on_context_menu_action(1)
	assert(arm_part.is_tourniquet_applied, "Torniquete debe estar aplicado")
	dashboard._on_paperdoll_context_menu(arm_id, Vector2(100, 100))
	assert(dashboard.submenu_aid.get_item_text(1).contains("Retirar"), "La opcion debe cambiar a Retirar Torniquete")
	
	# 6. Torniquete en Cabeza / Tórax debe estar deshabilitado siempre
	var head_id = BodyPartClass.PartType.HEAD
	var head_part: BodyPartClass = b.body_parts[head_id]
	head_part.apply_wound(BodyPartClass.WoundType.LACERATION)
	dashboard._on_paperdoll_context_menu(head_id, Vector2(100, 100))
	assert(dashboard.submenu_aid.is_item_disabled(1), "Torniquete en cabeza/tronco debe estar siempre deshabilitado")
	
	# 7. Equipar remera (id 202), chaleco táctico (id 203), pantalón (id 206) y casco (id 207)
	dashboard._on_context_menu_action(202)
	assert(b.equipped_items.has("tshirt_cotton"), "La remera básica debe estar equipada sin crash")
	
	dashboard._on_context_menu_action(203)
	assert(b.equipped_items.has("kevlar_vest"), "El chaleco táctico debe estar equipado sin crash")
	
	dashboard._on_context_menu_action(206)
	assert(b.equipped_items.has("cargo_pants"), "El pantalón táctico debe estar equipado sin crash")
	
	dashboard._on_context_menu_action(207)
	assert(b.equipped_items.has("tactical_helmet"), "El casco debe estar equipado sin crash")
	
	# 8. Verificar lista de equipamiento por capas en el inspector y biometría unificada
	dashboard.active_inspected_part = BodyPartClass.PartType.TORSO_UPPER
	dashboard._refresh_contextual_inspector()
	assert(dashboard.container_equipped_items_list.get_child_count() >= 2, "Deben listarse las prendas de torso en la lista interactiva")
	assert("Masa:" in dashboard.lbl_inspector_part_vitals.text, "Vitals debe contener desglose de masa en zona")
	assert("Salud:" in dashboard.lbl_inspector_part_vitals.text, "Vitals debe contener salud")
	assert("Clo:" in dashboard.lbl_inspector_part_vitals.text or "Aislamiento:" in dashboard.lbl_inspector_part_vitals.text, "Vitals debe contener aislamiento")
	
	# 9. Probar apilamiento (Stacking de suéter sobre remera)
	var torso_p: BodyPartClass = b.body_parts[BodyPartClass.PartType.TORSO_UPPER]
	var clo_before = torso_p.base_insulation_clo
	b.equip_item("wool_sweater")
	assert(torso_p.base_insulation_clo > clo_before, "El suéter apilado debe sumar aislamiento clo térmico")
	assert(torso_p.equipped_clothing_list.size() >= 3, "Torso debe tener remera, chaleco y suéter apilados")
	
	# 10. Probar exclusividad (Solo 1 mochila activa)
	b.equip_backpack("backpack_military")
	assert(b.equipped_backpack_id == "backpack_military", "Mochila militar equipada")
	b.equip_backpack("sidebag_tactical")
	assert(b.equipped_backpack_id == "sidebag_tactical", "Sidebag debe reemplazar a la mochila militar sin duplicar bolsas en espalda")
	
	# 11. Probar desequipar desde la ficha y ocultamiento de ficha técnica en piel desnuda
	dashboard._select_equipped_item("wool_sweater")
	assert(dashboard.panel_item_card.visible, "Ficha tecnica debe ser visible con prenda seleccionada")
	dashboard._on_unequip_item_card_pressed()
	assert(not b.equipped_items.has("wool_sweater"), "El suéter debe haberse desequipado correctamente")
	
	# Seleccionar piel desnuda explícitamente
	dashboard._select_equipped_item("")
	assert(not dashboard.panel_item_card.visible, "Ficha tecnica DEBE estar oculta en piel desnuda")
	
	# Verificar que el botón de equipar sin selección válida está deshabilitado
	dashboard.option_equip_part_item.selected = 0
	dashboard._update_body_part_inspector(b)
	assert(dashboard.btn_equip_selected_part_item.disabled, "Boton equipar debe estar deshabilitado con indice 0")
	
	print("  -> OK: Menu contextual, equipamiento, ficha oculta en piel desnuda y logica estricta validados.")

func test_makehuman_3d_asset_bridge(dashboard, b: BodySystemClass) -> void:
	print("[5/5] Verificando puente MakeHuman, slider de carga en bolso y archivo real...")
	
	# Inspeccionar mochila
	b.equip_backpack("sidebag_tactical")
	dashboard._select_equipped_item("sidebag_tactical")
	dashboard._update_card_for_backpack(b)
	
	assert(dashboard.panel_item_card.visible, "Ficha tecnica debe estar visible con mochila equipada")
	assert(dashboard.lbl_item_socket_bone != null, "lbl_item_socket_bone debe existir en Nivel 4")
	assert(dashboard.lbl_item_socket_bone.text.contains("model normal.fbx"), "Debe indicar el archivo real del rig 'model normal.fbx'")
	assert(dashboard.lbl_item_socket_bone.text.contains("mixamorig_Spine"), "Socket debe ser mixamorig_Spine para sidebag")
	
	# Verificar slider de carga en la ficha del bolso
	assert(dashboard.slider_item_card_cargo != null, "Slider de carga en panel_item_card debe existir")
	dashboard._on_item_card_cargo_slider_changed(3.5)
	assert(is_equal_approx(b.backpack_content_weight_kg, 3.5), "Slider de carga en ficha debe actualizar backpack_content_weight_kg a 3.5")
	assert(dashboard.lbl_item_card_cargo_breakdown.text.contains("Insumos Médicos"), "Debe desglosar insumos medicos")
	assert(dashboard.lbl_item_card_cargo_impact.text.contains("costo metabólico"), "Debe mostrar impacto metabolico")
	
	# Probar botón de adjuntar asset 3D
	assert(dashboard.btn_attach_3d_asset != null, "btn_attach_3d_asset debe existir")
	dashboard._on_attach_3d_asset_pressed()
	assert(dashboard.line_item_mesh_path.text == "", "Un modelo pendiente no debe inventar una ruta")
	assert(dashboard.lbl_item_asset_status.text.contains("Falta"), "Adjuntar sin archivo debe informar el recurso faltante")
	
	print("  -> OK: Puente de templates, slider de carga de bolso y archivo real validados.")
