class_name EquipmentVisuals
extends RefCounted
const Catalog = preload("res://src/items/item_catalog.gd")
static func attach(character, id: String, path: String = "") -> Dictionary:
	var definition = Catalog.get_definition(id)
	if definition.is_empty(): return {"ok":false, "message":"Ítem desconocido."}
	var visual = definition.visual
	if path == "": path = visual.model_path
	if path == "" or not path.begins_with("res://") or not ResourceLoader.exists(path):
		return {"ok":false, "message":"Falta un archivo de modelo real dentro del proyecto (res://)."}
	var resource = load(path)
	var model: Node3D
	if resource is PackedScene:
		var instance = resource.instantiate()
		if not instance is Node3D:
			instance.free()
			return {"ok":false, "message":"La escena debe tener raíz Node3D."}
		model = instance
	elif resource is Mesh:
		var mesh = MeshInstance3D.new()
		mesh.mesh = resource
		model = mesh
	else: return {"ok":false, "message":"El archivo no contiene una escena 3D ni una malla."}
	var skeleton: Skeleton3D = character.skeleton
	var template: Dictionary = visual.attachment
	var bone: String = template.get("socket_bone", "")
	if not skeleton or skeleton.find_bone(bone) < 0:
		model.free()
		return {"ok":false, "message":"El hueso requerido no existe: " + bone}
	var name = "Equipment_" + id
	var previous = skeleton.get_node_or_null(NodePath(name))
	if previous:
		skeleton.remove_child(previous)
		previous.queue_free()
	var attachment = BoneAttachment3D.new()
	attachment.name = name
	attachment.bone_name = bone
	attachment.set_meta("equipment_id", id)
	attachment.set_meta("source_path", path)
	skeleton.add_child(attachment)
	attachment.add_child(model)
	model.position = template.get("offset_3d", Vector3.ZERO)
	model.scale = template.get("scale_3d", Vector3.ONE)
	return {"ok":true, "message":"Modelo cargado: %s → %s" % [path, bone]}
static func sync(character) -> void:
	if not character.body or not character.skeleton: return
	var ids: Array = character.body.equipped_items.keys()
	if character.body.equipped_backpack_id != "": ids.append(character.body.equipped_backpack_id)
	ids.sort()
	var signature = str(character.skeleton.get_instance_id()) + str(ids)
	if character.get_meta("equipment_signature", "") == signature: return
	character.set_meta("equipment_signature", signature)
	for node in character.skeleton.get_children():
		if node.has_meta("equipment_id") and not ids.has(node.get_meta("equipment_id")):
			character.skeleton.remove_child(node)
			node.queue_free()
	for id in ids:
		if character.skeleton.has_node(NodePath("Equipment_" + id)): continue
		var definition = Catalog.get_definition(id)
		if not definition.is_empty() and definition.visual.model_path != "": attach(character, id)
