class_name ItemCatalog
extends RefCounted
const Clothes = preload("res://src/biology/clothing_database.gd")
static var _source: Dictionary = {}
static func _data() -> Dictionary:
	if _source.is_empty():
		var parsed = JSON.parse_string(FileAccess.get_file_as_string("res://assets/items/catalog.json"))
		if parsed is Dictionary: _source = parsed
	return _source
static func all_ids() -> Array:
	return Clothes.get_all_items().keys() + Clothes.get_all_backpacks().keys()
static func get_definition(id: String) -> Dictionary:
	var simulation = Clothes.get_item(id)
	var category = "clothing"
	if simulation.is_empty():
		simulation = Clothes.get_backpack(id)
		category = "container"
	if simulation.is_empty(): return {}
	var authored: Dictionary = _data().get("items", {}).get(id, {})
	return {
		"id": id, "name": simulation.name, "category": category,
		"simulation": simulation.duplicate(true),
		"visual": {
			"icon_path": authored.get("icon_path", "res://assets/ui/icons/catalog_item.svg"),
			"icon_is_generic": not authored.has("icon_path"),
			"model_path": authored.get("model_path", ""),
			"material_paths": authored.get("material_paths", []),
			"texture_paths": authored.get("texture_paths", []),
			"status": authored.get("asset_status", "missing_model"),
			"attachment": Clothes.get_item_visual_template(id).duplicate(true)
		},
		"components": authored.get("components", []).duplicate(true),
		"quality": authored.get("quality", {"grade":"unspecified", "status":"metadata_only"}).duplicate(true)
	}
static func validate(id: String) -> Array[String]:
	var issues: Array[String] = []
	var definition = get_definition(id)
	if definition.is_empty(): return ["Unknown item: " + id]
	var visual = definition.visual
	if not ResourceLoader.exists(visual.icon_path): issues.append("Missing icon: " + visual.icon_path)
	if visual.model_path != "" and not ResourceLoader.exists(visual.model_path): issues.append("Missing model: " + visual.model_path)
	for path in visual.material_paths + visual.texture_paths:
		if not ResourceLoader.exists(path): issues.append("Missing resource: " + path)
	for component in definition.components:
		if not _data().get("components", {}).has(component.material): issues.append("Unknown component material")
	return issues
