extends MarginContainer
const Catalog = preload("res://src/items/item_catalog.gd")
var ui
var pages: TabContainer
var readings: Label
var session_status: Label
var diagnosis: Label
var zone_title: Label
var findings: VBoxContainer
var catalog_list: ItemList
var catalog_description: Label
var chosen_item: String = "backpack_school"
var _elapsed = 0.0
var _findings_signature = ""

func setup(dashboard) -> void:
	ui = dashboard
	name = "AnalysisStation"
	set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	for edge in ["left", "right", "top", "bottom"]: add_theme_constant_override("margin_" + edge, 16)
	var shell = VBoxContainer.new()
	shell.add_theme_constant_override("separation", 14)
	add_child(shell)
	var header = HBoxContainer.new()
	shell.add_child(header)
	var title = _label("BIO / ESTACIÓN DE ANÁLISIS", 22, Color("b9ece6"))
	title.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	header.add_child(title)
	session_status = _label("", 13, Color("77979e"))
	session_status.autowrap_mode = TextServer.AUTOWRAP_OFF
	header.add_child(session_status)
	readings = _label("", 17, Color("9adcd3"))
	shell.add_child(readings)
	pages = TabContainer.new()
	pages.size_flags_vertical = Control.SIZE_EXPAND_FILL
	pages.add_theme_font_size_override("font_size", 16)
	shell.add_child(pages)
	var analysis = _page("Análisis")
	var equipment = _page("Ítems y assets")
	var simulation = _page("Simulación")
	var character_page = _page("Biometría")
	var legacy = ui.find_child("WorkplaceSplit", true, false)
	var workspace = ui.find_child("WorkspaceScroll", true, false).get_child(0)
	var creation = workspace.get_child(0)
	var bottom = workspace.get_child(workspace.get_child_count() - 1)
	var dock = ui.find_child("RightSimulationDock", true, false)
	var inspector = ui.panel_item_card.get_parent()
	var scanner = ui.paperdoll.get_parent()
	scanner.reparent(analysis)
	scanner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	scanner.size_flags_vertical = Control.SIZE_EXPAND_FILL
	scanner.get_node("SilHeaderStatusCard").hide()
	ui.paperdoll.custom_minimum_size = Vector2(350, 360)
	ui.paperdoll.get_canvas_control().custom_minimum_size = Vector2(260, 280)
	scanner.add_child(_label("HOVER  inspección local     X  interior     ALT + 1…6  capas", 12, Color("72949c")))
	var report_scroll = ScrollContainer.new()
	report_scroll.custom_minimum_size.x = 350
	report_scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	report_scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	analysis.add_child(report_scroll)
	var report = VBoxContainer.new()
	report.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	report.add_theme_constant_override("separation", 16)
	report_scroll.add_child(report)
	report.add_child(_label("01 / DIAGNÓSTICO LOCAL", 12, Color("77979e")))
	zone_title = _label("Seleccioná una zona", 24, Color("d1efe8"))
	report.add_child(zone_title)
	diagnosis = _label("", 16, Color("aec3c7"))
	report.add_child(diagnosis)
	report.add_child(HSeparator.new())
	report.add_child(_label("02 / HALLAZGOS DEL CUERPO", 12, Color("77979e")))
	findings = VBoxContainer.new()
	findings.add_theme_constant_override("separation", 8)
	report.add_child(findings)
	var open_equipment = Button.new()
	open_equipment.text = "Equipamiento y archivos →"
	open_equipment.pressed.connect(func(): pages.current_tab = 1)
	report.add_child(open_equipment)
	ui.panel_sensory.reparent(shell)
	var sim_scroll = ScrollContainer.new()
	sim_scroll.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	sim_scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	simulation.add_child(sim_scroll)
	var sim_body = VBoxContainer.new()
	sim_body.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	sim_scroll.add_child(sim_body)
	dock.reparent(sim_body)
	dock.custom_minimum_size.x = 0
	bottom.reparent(sim_body)
	creation.reparent(character_page)
	creation.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	var catalog_column = VBoxContainer.new()
	catalog_column.custom_minimum_size.x = 280
	equipment.add_child(catalog_column)
	catalog_column.add_child(_label("CATÁLOGO / FUENTES REALES", 13, Color("77979e")))
	var search = LineEdit.new()
	search.placeholder_text = "Buscar ítem…"
	catalog_column.add_child(search)
	catalog_list = ItemList.new()
	catalog_list.size_flags_vertical = Control.SIZE_EXPAND_FILL
	catalog_list.fixed_icon_size = Vector2i(30, 30)
	catalog_column.add_child(catalog_list)
	search.text_changed.connect(_fill_catalog)
	catalog_list.item_selected.connect(func(index):
		chosen_item = catalog_list.get_item_metadata(index)
		_update_catalog_description()
	)
	_fill_catalog("")
	var item_scroll = ScrollContainer.new()
	item_scroll.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	item_scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	item_scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	equipment.add_child(item_scroll)
	var item_details = VBoxContainer.new()
	item_details.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	item_details.add_theme_constant_override("separation", 12)
	item_scroll.add_child(item_details)
	catalog_description = _label("", 15, Color("afccd0"))
	item_details.add_child(catalog_description)
	var equip = Button.new()
	equip.text = "Equipar ítem seleccionado"
	equip.pressed.connect(_equip_catalog_item)
	item_details.add_child(equip)
	inspector.reparent(item_details)
	inspector.custom_minimum_size.x = 0
	ui.lbl_inspector_part_title.hide()
	ui.lbl_inspector_part_vitals.hide()
	ui.lbl_inspector_clinical_hint.hide()
	var preview = ui.mini_3d_viewport_container.get_parent()
	preview.reparent(equipment)
	preview.show()
	preview.custom_minimum_size.x = 300
	preview.size_flags_vertical = Control.SIZE_EXPAND_FILL
	ui.mini_3d_viewport_container.custom_minimum_size.y = 360
	ui.mini_3d_viewport_container.size_flags_vertical = Control.SIZE_EXPAND_FILL
	ui._slider_cam.value = 155.0
	var camera = ui.mini_3d_viewport.get_child(0)
	camera.look_at_from_position(Vector3(0.7, 1.35, -1.45), Vector3(0, 1.2, 0))
	var preview_light = OmniLight3D.new()
	preview_light.position = Vector3(0.4, 2.1, -1.8)
	preview_light.light_energy = 2.0
	preview_light.omni_range = 4.0
	ui.mini_3d_viewport.add_child(preview_light)
	legacy.hide()
	_update_catalog_description()
	_update_report()

func _page(title: String) -> HBoxContainer:
	var page = HBoxContainer.new()
	page.name = title
	page.add_theme_constant_override("separation", 28)
	pages.add_child(page)
	return page

func _label(text: String, font_size: int, color: Color) -> Label:
	var label = Label.new()
	label.text = text
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", color)
	return label

func _process(delta: float) -> void:
	_elapsed += delta
	if _elapsed < 0.2: return
	_elapsed = 0
	_update_report()
	if ui:
		ui.lbl_inspector_part_title.hide()
		ui.lbl_inspector_part_vitals.hide()
		ui.lbl_inspector_clinical_hint.hide()

func _update_report() -> void:
	if not ui or not ui.character or not ui.character.body: return
	var body = ui.character.body
	session_status.text = "SIMULACIÓN · " + ui.current_action_name
	readings.text = "%d BPM     /     %.2f L sangre     /     %.1f °C     /     Agua %.0f%%     /     Dolor %.0f%%" % [body.cardio.heart_rate_bpm, body.cardio.total_blood_ml / 1000.0, body.thermo.core_temperature, body.hydration.current_water_ml / maxf(body.hydration.max_water_ml,1.0) * 100.0, body.total_pain]
	var part_id = ui.paperdoll.hovered_part_type
	if part_id < 0: part_id = ui.active_inspected_part
	var part = body.body_parts.get(part_id)
	if part:
		zone_title.text = part.display_name
		var bleed = part.get_current_bleed_rate(body.cardio.heart_rate_bpm)
		var conditions: Array[String] = []
		if part.has_fracture: conditions.append("Fractura · " + ("inmovilizada" if part.is_splinted else "sin inmovilizar"))
		if bleed > 0.01: conditions.append("Sangrado activo · %.2f mL/s" % bleed)
		if part.infection_level > 0: conditions.append("Infección local · %.0f%%" % part.infection_level)
		if part.is_bandaged: conditions.append("Vendaje aplicado")
		if part.is_tourniquet_applied: conditions.append("Torniquete aplicado")
		if conditions.is_empty(): conditions.append("Sin lesiones activas registradas")
		diagnosis.text = "Integridad  %.0f%%\nDolor  %.0f%%\n\n%s\n\nMasa local  %.2f kg\nAislamiento  %.2f clo" % [part.integrity, part.pain_level, "\n".join(conditions), part.get_anatomical_mass_kg() + part.get_clothing_weight_kg(), part.base_insulation_clo]
	else:
		zone_title.text = "Exploración corporal"
		diagnosis.text = "Pasá el mouse para inspeccionar el interior.\n\nUn clic fija la zona. Los hallazgos llevan al área afectada."
	var records: Array = []
	for id in body.body_parts:
		var p = body.body_parts[id]
		if p.has_fracture or p.get_current_bleed_rate(body.cardio.heart_rate_bpm) > 0.01 or p.pain_level > 20 or p.infection_level > 5:
			records.append([id, p.display_name, "Fractura" if p.has_fracture else ("Sangrado" if p.get_current_bleed_rate(body.cardio.heart_rate_bpm) > 0.01 else ("Infección" if p.infection_level > 5 else "Dolor"))])
	var signature = str(records)
	if signature == _findings_signature: return
	_findings_signature = signature
	for child in findings.get_children():
		findings.remove_child(child)
		child.queue_free()
	if records.is_empty(): findings.add_child(_label("Sin alertas locales.\nLos parámetros globales siguen visibles arriba.", 14, Color("8fbbb0")))
	for record in records:
		var button = Button.new()
		button.text = "%s · %s →" % [record[1], record[2]]
		button.alignment = HORIZONTAL_ALIGNMENT_LEFT
		button.add_theme_color_override("font_color", Color("f2b080"))
		button.pressed.connect(func(): ui.paperdoll.select_part(record[0]))
		findings.add_child(button)

func _fill_catalog(query: String) -> void:
	catalog_list.clear()
	var ids = Catalog.all_ids()
	ids.erase("backpack_school")
	ids.push_front("backpack_school")
	for id in ids:
		var definition = Catalog.get_definition(id)
		if query != "" and not (definition.name + id).to_lower().contains(query.to_lower()): continue
		var suffix = " · 3D" if definition.visual.model_path != "" else ""
		var index = catalog_list.add_item(definition.name + suffix, load(definition.visual.icon_path))
		catalog_list.set_item_metadata(index, id)
		if id == chosen_item: catalog_list.select(index)

func _update_catalog_description() -> void:
	if not catalog_description: return
	var item = Catalog.get_definition(chosen_item)
	var visual = item.visual
	var path = visual.model_path if visual.model_path != "" else "Pendiente de crear"
	var components: Array[String] = []
	var labels = {"shell":"Cuerpo de lona", "straps":"Correas de carga", "buckles":"Hebillas de polímero"}
	for component in item.components: components.append(labels.get(component.id, component.id))
	var physics = item.simulation
	var physical = "Peso seco: %.2f kg" % physics.get("dry_weight_kg", 0.0)
	if physics.has("max_capacity_kg"): physical += "   ·   Capacidad de carga: %.1f kg" % physics.max_capacity_kg
	if physics.has("insulation_clo"): physical += "   ·   Aislamiento: %.2f clo" % physics.insulation_clo
	catalog_description.text = "%s\n%s\n\nComponentes: %s\nCalidad: %s · sin efecto físico implementado\n\nARCHIVOS\nModelo: %s\nIcono: %s\nMaterial: %s\nTexturas: %s" % [item.name, physical, ", ".join(components) if not components.is_empty() else "Pendientes de definir", str(item.quality.grade).replace("standard", "Estándar").replace("unspecified", "Sin definir"), path, visual.icon_path, ", ".join(visual.material_paths) if not visual.material_paths.is_empty() else "Pendiente", ", ".join(visual.texture_paths) if not visual.texture_paths.is_empty() else "Sin archivos externos; color y rugosidad del material"]

func _equip_catalog_item() -> void:
	var item = Catalog.get_definition(chosen_item)
	if item.category == "container":
		ui.character.body.equip_backpack(chosen_item)
		ui.paperdoll.select_part(2)
	else: ui.character.body.equip_item(chosen_item)
	ui._select_equipped_item(chosen_item)
	ui._refresh_contextual_inspector()
	preload("res://src/items/equipment_visuals.gd").sync(ui.character)
