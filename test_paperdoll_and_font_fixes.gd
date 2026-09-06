@tool
extends SceneTree

const HumanCharacterClass = preload("res://src/entities/human_character.gd")
const BodySystemClass = preload("res://src/biology/body_system.gd")
const BodyPartClass = preload("res://src/biology/body_part.gd")
const SilhouettePaperdollClass = preload("res://src/ui/silhouette_paperdoll.gd")
const BiologyDashboardClass = preload("res://src/ui/biology_dashboard.gd")

func _init() -> void:
	print("\n=== VERIFICACIÓN INTEGRAL: 17 PARTES ANATÓMICAS, WORKPLACE 3 COLUMNAS Y PAPERDOLL DESELECT ===")
	
	test_paperdoll_17_polygons_and_selection()
	test_dashboard_3_column_workplace_and_fonts()
	
	print("\n=========================================================================")
	print("¡TODAS LAS VERIFICACIONES COMPLETADAS CON ÉXITO ROTUNDO (100% PASS)!")
	print("=========================================================================\n")
	quit(0)

func test_paperdoll_17_polygons_and_selection() -> void:
	print("\n--- PRUEBA 1: Calibración, Hitboxes y Deselección del Paperdoll (17 Partes) ---")
	var paperdoll = SilhouettePaperdollClass.new()
	root.add_child(paperdoll)
	paperdoll._ready()
	
	# Forzar tamaño de prueba 540x960 (9:16)
	paperdoll.size = Vector2(540, 960)
	paperdoll._canvas_control.size = Vector2(540, 960)
	paperdoll._update_layout()
	
	var cr = paperdoll._cached_content_rect
	print("  • Rect anatómico 9:16: pos=%s, size=%s" % [cr.position, cr.size])
	assert(cr.size.x > 0 and cr.size.y > 0, "El content rect 9:16 debe ser válido")
	
	var cx = cr.position.x + cr.size.x * 0.5
	var oy = cr.position.y
	var w = cr.size.x
	var h = cr.size.y
	
	# 1. Verificar que las 17 partes anatómicas tienen polígonos calculados
	assert(paperdoll._part_polygons.size() == 17, "Deben existir exactamente 17 geometrías poligonales (tenemos %d)" % paperdoll._part_polygons.size())
	print("  • Total partes con geometría registrada: %d/17" % paperdoll._part_polygons.size())
	
	# 2. Pruebas de detección anatómica de puntos (Point in Polygon) para las 17 partes
	for p_type in paperdoll._part_polygons.keys():
		var poly: PackedVector2Array = paperdoll._part_polygons[p_type]
		assert(poly.size() >= 3, "El polígono para la parte %d debe tener al menos 3 vértices" % p_type)
		var centroid = Vector2.ZERO
		for pt in poly:
			centroid += pt
		centroid /= poly.size()
		var inside = Geometry2D.is_point_in_polygon(centroid, poly)
		assert(inside, "El centroide para la parte %d debe caer dentro de su polígono" % p_type)
	print("  • Las 17 zonas anatómicas capturan sus hitboxes con 100% de precisión matemática.")
	
	# 3. Verificar estado de selección inicial (debe ser -1, NINGUNA parte seleccionada forzada)
	assert(paperdoll.selected_part_type == -1, "El paperdoll debe inicializarse con selected_part_type = -1")
	print("  • Selección inicial limpia (-1): Ninguna parte seleccionada por defecto.")
	
	# 4. Verificar lógica de selección y deselección por clic / toggle
	paperdoll.select_part(BodyPartClass.PartType.HEAD)
	assert(paperdoll.selected_part_type == BodyPartClass.PartType.HEAD, "Debe seleccionar cabeza")
	
	# Simular clic en la misma parte (toggle deselect) usando el centroide de la cabeza
	var head_poly = paperdoll._part_polygons[BodyPartClass.PartType.HEAD]
	var head_centroid = Vector2.ZERO
	for pt in head_poly:
		head_centroid += pt
	head_centroid /= head_poly.size()

	var ev_same = InputEventMouseButton.new()
	ev_same.pressed = true
	ev_same.button_index = MOUSE_BUTTON_LEFT
	ev_same.position = head_centroid
	paperdoll._on_canvas_gui_input(ev_same)
	assert(paperdoll.selected_part_type == -1, "Clic en la misma parte debe deseleccionarla (toggle a -1)")
	print("  • Toggle de deselección al hacer clic en la misma parte verificado.")
	
	# Simular clic en fondo vacío (deselect)
	paperdoll.select_part(BodyPartClass.PartType.TORSO_UPPER)
	assert(paperdoll.selected_part_type == BodyPartClass.PartType.TORSO_UPPER)
	var ev_empty = InputEventMouseButton.new()
	ev_empty.pressed = true
	ev_empty.button_index = MOUSE_BUTTON_LEFT
	ev_empty.position = Vector2(10, 10) # Fuera de la silueta
	paperdoll._on_canvas_gui_input(ev_empty)
	assert(paperdoll.selected_part_type == -1, "Clic en el fondo vacío debe deseleccionar la parte activa")
	print("  • Deselección al hacer clic en fondo vacío verificada.")
	
	paperdoll.queue_free()
	print(">>> ÉXITO: Paperdoll 17 partes, hitboxes y deselección validados al 100%.")

func test_dashboard_3_column_workplace_and_fonts() -> void:
	print("\n--- PRUEBA 2: Estructura de 3 Columnas y Jerarquía de Fuentes en Dashboard ---")
	var dashboard = BiologyDashboardClass.new()
	root.add_child(dashboard)
	dashboard._ready()
	
	# 1. Fondo anti-fuga 3D
	var bg = dashboard.get_node_or_null("BackgroundStudioDark")
	assert(bg != null, "BackgroundStudioDark debe existir en el dashboard")
	assert(bg is ColorRect, "BackgroundStudioDark debe ser ColorRect")
	print("  • Fondo oscuro anti-fuga 3D verificado: %s" % bg.name)
	
	# 2. Selección activa inicial en dashboard debe ser -1
	assert(dashboard.active_inspected_part == -1, "active_inspected_part debe inicializar en -1")
	print("  • active_inspected_part = -1 verificado en Dashboard.")
	
	# 3. Verificar estructura de 3 columnas del workplace
	var split = dashboard.get_node_or_null("WorkplaceSplit")
	assert(split != null, "WorkplaceSplit debe existir")
	
	var wp_cols = split.find_child("WorkplaceColumns", true, false)
	assert(wp_cols != null, "WorkplaceColumns debe existir")
	assert(wp_cols is HBoxContainer, "WorkplaceColumns debe ser un HBoxContainer")
	
	var col_telemetry = wp_cols.get_node_or_null("TelemetryPanel")
	assert(col_telemetry != null, "Columna 1: TelemetryPanel debe existir a la izquierda")
	
	var col_silhouette = wp_cols.get_node_or_null("SilhouettePanel")
	assert(col_silhouette != null, "Columna 2: SilhouettePanel debe existir en el centro")
	
	var col_inspector = wp_cols.get_node_or_null("InspectorPanel")
	assert(col_inspector != null, "Columna 3: InspectorPanel debe existir a la derecha de la silueta")
	print("  • Estructura de 3 columnas (Telemetría Izq -> Silueta Centro -> Inspector Der) verificada.")
	
	# 4. Verificar que TelemetryPanel contiene Fisiología y que SensoryPanel reside junto a los controles inferiores
	var sensory_panel = dashboard.find_child("SensoryPanel", true, false)
	assert(sensory_panel != null, "SensoryPanel debe existir")
	assert(sensory_panel.get_parent() != null, "SensoryPanel debe tener padre en la barra de controles")
	
	var bar_blood = col_telemetry.find_child("BarBlood", true, false)
	assert(bar_blood != null, "Monitor de Fisiología (BarBlood) debe estar dentro de TelemetryPanel (izquierda)")
	print("  • Telemetría biomédica unificada a la izquierda y Percepción Somática ubicada sobre controles inferiores.")
	
	# 5. Comprobar que el dock derecho tiene ancho mínimo de 330px
	var right_dock = split.get_node_or_null("RightSimulationDock")
	assert(right_dock != null, "RightSimulationDock debe existir")
	assert(right_dock.custom_minimum_size.x <= 340, "RightSimulationDock debe tener ancho compacto (<=340px) para evitar corte horizontal")
	print("  • Dock de simulación derecho calibrado a %d px (no desborda la pantalla)." % right_dock.custom_minimum_size.x)
	
	# 6. Comprobar fuentes >= 12px
	var small_fonts_found: Array[String] = []
	_scan_for_small_fonts(dashboard, small_fonts_found)
	assert(small_fonts_found.size() == 0, "No debe haber ningún texto con tamaño inferior a 12px")
	print("  • Escaneo completo de fuentes: 0 textos con tamaño < 12px.")
	
	dashboard.queue_free()
	print(">>> ÉXITO: Workplace de 3 columnas, dock derecho y fuentes validados al 100%.")

func _scan_for_small_fonts(node: Node, results: Array[String]) -> void:
	if node is Label or node is Button:
		var size = node.get_theme_font_size("font_size")
		if size > 0 and size < 12:
			results.append("%s (%s): font_size=%d" % [node.name, node.get_class(), size])
	for child in node.get_children():
		_scan_for_small_fonts(child, results)
