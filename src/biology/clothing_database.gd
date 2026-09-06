class_name ClothingDatabase
extends RefCounted

const BodyPartClass = preload("res://src/biology/body_part.gd")

static var _catalog: Dictionary = {}
static var _backpack_catalog: Dictionary = {}
static var _initialized: bool = false

static func _ensure_initialized() -> void:
	if _initialized:
		return
	_initialized = true

	# -------------------------------------------------------------------------
	# CAPA 1: ROPA INTERIOR Y TÉRMICA (LAYER_1_UNDERWEAR)
	# -------------------------------------------------------------------------
	_register_item({
		"id": "boxer_briefs",
		"name": "Bóxer de Algodón",
		"layer": BodyPartClass.ClothingLayer.LAYER_1_UNDERWEAR,
		"covered_parts": [BodyPartClass.PartType.GROIN],
		"insulation_clo": 0.08,
		"dry_weight_kg": 0.12,
		"max_water_absorption_kg": 0.15,
		"wind_resistance": 0.05,
		"waterproof_rating": 0.0,
		"ballistic_protection": 0.0,
		"cut_scratch_protection": 0.05
	})

	_register_item({
		"id": "thermal_top",
		"name": "Camiseta Térmica de Manga Larga",
		"layer": BodyPartClass.ClothingLayer.LAYER_1_UNDERWEAR,
		"covered_parts": [BodyPartClass.PartType.TORSO_UPPER, BodyPartClass.PartType.TORSO_LOWER, BodyPartClass.PartType.LEFT_ARM, BodyPartClass.PartType.RIGHT_ARM, BodyPartClass.PartType.LEFT_FOREARM, BodyPartClass.PartType.RIGHT_FOREARM],
		"insulation_clo": 0.35,
		"dry_weight_kg": 0.30,
		"max_water_absorption_kg": 0.40,
		"wind_resistance": 0.15,
		"waterproof_rating": 0.05,
		"ballistic_protection": 0.0,
		"cut_scratch_protection": 0.10
	})

	_register_item({
		"id": "thermal_bottom",
		"name": "Pantalón Térmico Interior",
		"layer": BodyPartClass.ClothingLayer.LAYER_1_UNDERWEAR,
		"covered_parts": [BodyPartClass.PartType.GROIN, BodyPartClass.PartType.LEFT_LEG, BodyPartClass.PartType.RIGHT_LEG, BodyPartClass.PartType.LEFT_CALF, BodyPartClass.PartType.RIGHT_CALF],
		"insulation_clo": 0.30,
		"dry_weight_kg": 0.28,
		"max_water_absorption_kg": 0.35,
		"wind_resistance": 0.15,
		"waterproof_rating": 0.05,
		"ballistic_protection": 0.0,
		"cut_scratch_protection": 0.10
	})

	_register_item({
		"id": "cotton_socks",
		"name": "Calcetines de Algodón",
		"layer": BodyPartClass.ClothingLayer.LAYER_1_UNDERWEAR,
		"covered_parts": [BodyPartClass.PartType.LEFT_FOOT, BodyPartClass.PartType.RIGHT_FOOT],
		"insulation_clo": 0.12,
		"dry_weight_kg": 0.08,
		"max_water_absorption_kg": 0.12,
		"wind_resistance": 0.05,
		"waterproof_rating": 0.0,
		"ballistic_protection": 0.0,
		"cut_scratch_protection": 0.05
	})

	# -------------------------------------------------------------------------
	# CAPA 2: CAPA BASE (LAYER_2_BASE)
	# -------------------------------------------------------------------------
	_register_item({
		"id": "tshirt_cotton",
		"name": "Remera Clásica de Algodón",
		"layer": BodyPartClass.ClothingLayer.LAYER_2_BASE,
		"covered_parts": [BodyPartClass.PartType.TORSO_UPPER, BodyPartClass.PartType.TORSO_LOWER],
		"insulation_clo": 0.25,
		"dry_weight_kg": 0.22,
		"max_water_absorption_kg": 0.35,
		"wind_resistance": 0.10,
		"waterproof_rating": 0.0,
		"ballistic_protection": 0.0,
		"cut_scratch_protection": 0.08
	})

	_register_item({
		"id": "work_shirt_long",
		"name": "Camisa de Trabajo Reforzada",
		"layer": BodyPartClass.ClothingLayer.LAYER_2_BASE,
		"covered_parts": [BodyPartClass.PartType.TORSO_UPPER, BodyPartClass.PartType.TORSO_LOWER, BodyPartClass.PartType.LEFT_ARM, BodyPartClass.PartType.RIGHT_ARM, BodyPartClass.PartType.LEFT_FOREARM, BodyPartClass.PartType.RIGHT_FOREARM],
		"insulation_clo": 0.35,
		"dry_weight_kg": 0.45,
		"max_water_absorption_kg": 0.55,
		"wind_resistance": 0.25,
		"waterproof_rating": 0.05,
		"ballistic_protection": 0.0,
		"cut_scratch_protection": 0.20
	})

	_register_item({
		"id": "jeans_denim",
		"name": "Pantalón Jeans de Denim",
		"layer": BodyPartClass.ClothingLayer.LAYER_2_BASE,
		"covered_parts": [BodyPartClass.PartType.GROIN, BodyPartClass.PartType.LEFT_LEG, BodyPartClass.PartType.RIGHT_LEG, BodyPartClass.PartType.LEFT_CALF, BodyPartClass.PartType.RIGHT_CALF],
		"insulation_clo": 0.38,
		"dry_weight_kg": 0.75,
		"max_water_absorption_kg": 1.10,
		"wind_resistance": 0.35,
		"waterproof_rating": 0.05,
		"ballistic_protection": 0.0,
		"cut_scratch_protection": 0.35
	})

	_register_item({
		"id": "cargo_pants",
		"name": "Pantalón Cargo Táctico Ripstop",
		"layer": BodyPartClass.ClothingLayer.LAYER_2_BASE,
		"covered_parts": [BodyPartClass.PartType.GROIN, BodyPartClass.PartType.LEFT_LEG, BodyPartClass.PartType.RIGHT_LEG, BodyPartClass.PartType.LEFT_CALF, BodyPartClass.PartType.RIGHT_CALF],
		"insulation_clo": 0.45,
		"dry_weight_kg": 0.85,
		"max_water_absorption_kg": 0.90,
		"wind_resistance": 0.45,
		"waterproof_rating": 0.20,
		"ballistic_protection": 0.0,
		"cut_scratch_protection": 0.45
	})

	_register_item({
		"id": "urban_sneakers",
		"name": "Zapatillas Urbanas Amortiguadas",
		"layer": BodyPartClass.ClothingLayer.LAYER_2_BASE,
		"covered_parts": [BodyPartClass.PartType.LEFT_FOOT, BodyPartClass.PartType.RIGHT_FOOT],
		"insulation_clo": 0.22,
		"dry_weight_kg": 0.85,
		"max_water_absorption_kg": 0.45,
		"wind_resistance": 0.40,
		"waterproof_rating": 0.15,
		"ballistic_protection": 0.0,
		"cut_scratch_protection": 0.40,
		"is_footwear": true
	})

	# -------------------------------------------------------------------------
	# CAPA 3: CAPA EXTERIOR / AISLAMIENTO (LAYER_3_OUTER)
	# -------------------------------------------------------------------------
	_register_item({
		"id": "wool_sweater",
		"name": "Suéter de Lana Gruesa",
		"layer": BodyPartClass.ClothingLayer.LAYER_3_OUTER,
		"covered_parts": [BodyPartClass.PartType.TORSO_UPPER, BodyPartClass.PartType.TORSO_LOWER, BodyPartClass.PartType.LEFT_ARM, BodyPartClass.PartType.RIGHT_ARM, BodyPartClass.PartType.LEFT_FOREARM, BodyPartClass.PartType.RIGHT_FOREARM],
		"insulation_clo": 0.75,
		"dry_weight_kg": 0.95,
		"max_water_absorption_kg": 1.60,
		"wind_resistance": 0.25,
		"waterproof_rating": 0.05,
		"ballistic_protection": 0.0,
		"cut_scratch_protection": 0.25
	})

	_register_item({
		"id": "tactical_jacket",
		"name": "Campera Militar Cortaviento",
		"layer": BodyPartClass.ClothingLayer.LAYER_3_OUTER,
		"covered_parts": [BodyPartClass.PartType.TORSO_UPPER, BodyPartClass.PartType.TORSO_LOWER, BodyPartClass.PartType.LEFT_ARM, BodyPartClass.PartType.RIGHT_ARM, BodyPartClass.PartType.LEFT_FOREARM, BodyPartClass.PartType.RIGHT_FOREARM],
		"insulation_clo": 0.85,
		"dry_weight_kg": 1.40,
		"max_water_absorption_kg": 0.70,
		"wind_resistance": 0.85,
		"waterproof_rating": 0.80,
		"ballistic_protection": 0.10,
		"cut_scratch_protection": 0.65,
		"hides_underlying_skin": true
	})

	_register_item({
		"id": "rain_poncho",
		"name": "Poncho Impermeable de Lluvia",
		"layer": BodyPartClass.ClothingLayer.LAYER_3_OUTER,
		"covered_parts": [BodyPartClass.PartType.TORSO_UPPER, BodyPartClass.PartType.TORSO_LOWER, BodyPartClass.PartType.LEFT_ARM, BodyPartClass.PartType.RIGHT_ARM, BodyPartClass.PartType.LEFT_FOREARM, BodyPartClass.PartType.RIGHT_FOREARM, BodyPartClass.PartType.GROIN],
		"insulation_clo": 0.20,
		"dry_weight_kg": 0.45,
		"max_water_absorption_kg": 0.05,
		"wind_resistance": 0.90,
		"waterproof_rating": 0.95,
		"ballistic_protection": 0.0,
		"cut_scratch_protection": 0.15
	})

	_register_item({
		"id": "winter_parka",
		"name": "Parka Polar Antártica",
		"layer": BodyPartClass.ClothingLayer.LAYER_3_OUTER,
		"covered_parts": [BodyPartClass.PartType.TORSO_UPPER, BodyPartClass.PartType.TORSO_LOWER, BodyPartClass.PartType.LEFT_ARM, BodyPartClass.PartType.RIGHT_ARM, BodyPartClass.PartType.LEFT_FOREARM, BodyPartClass.PartType.RIGHT_FOREARM],
		"insulation_clo": 1.65,
		"dry_weight_kg": 2.20,
		"max_water_absorption_kg": 1.20,
		"wind_resistance": 0.95,
		"waterproof_rating": 0.85,
		"ballistic_protection": 0.05,
		"cut_scratch_protection": 0.50,
		"hides_underlying_skin": true
	})

	_register_item({
		"id": "combat_boots",
		"name": "Botas Militares de Cuero (Caña Alta)",
		"layer": BodyPartClass.ClothingLayer.LAYER_3_OUTER,
		"covered_parts": [BodyPartClass.PartType.LEFT_FOOT, BodyPartClass.PartType.RIGHT_FOOT],
		"insulation_clo": 0.48,
		"dry_weight_kg": 1.65,
		"max_water_absorption_kg": 0.50,
		"wind_resistance": 0.85,
		"waterproof_rating": 0.80,
		"ballistic_protection": 0.20,
		"cut_scratch_protection": 0.85,
		"is_footwear": true
	})

	# -------------------------------------------------------------------------
	# CAPA 4: CAPA TÁCTICA / BLINDAJE (LAYER_4_TACTICAL)
	# -------------------------------------------------------------------------
	_register_item({
		"id": "kevlar_vest",
		"name": "Chaleco Balístico Kevlar Nivel III",
		"layer": BodyPartClass.ClothingLayer.LAYER_4_TACTICAL,
		"covered_parts": [BodyPartClass.PartType.TORSO_UPPER, BodyPartClass.PartType.TORSO_LOWER],
		"insulation_clo": 0.25,
		"dry_weight_kg": 3.60,
		"max_water_absorption_kg": 0.20,
		"wind_resistance": 0.45,
		"waterproof_rating": 0.30,
		"ballistic_protection": 0.82,
		"cut_scratch_protection": 0.92
	})

	_register_item({
		"id": "tactical_helmet",
		"name": "Casco Balístico Militar Kevlar",
		"layer": BodyPartClass.ClothingLayer.LAYER_4_TACTICAL,
		"covered_parts": [BodyPartClass.PartType.HEAD],
		"insulation_clo": 0.15,
		"dry_weight_kg": 1.45,
		"max_water_absorption_kg": 0.05,
		"wind_resistance": 0.55,
		"waterproof_rating": 0.90,
		"ballistic_protection": 0.78,
		"cut_scratch_protection": 0.90
	})

	_register_item({
		"id": "elbow_pads",
		"name": "Coderas Tácticas Rígidas",
		"layer": BodyPartClass.ClothingLayer.LAYER_4_TACTICAL,
		"covered_parts": [BodyPartClass.PartType.LEFT_ARM, BodyPartClass.PartType.RIGHT_ARM, BodyPartClass.PartType.LEFT_FOREARM, BodyPartClass.PartType.RIGHT_FOREARM],
		"insulation_clo": 0.05,
		"dry_weight_kg": 0.40,
		"max_water_absorption_kg": 0.05,
		"wind_resistance": 0.20,
		"waterproof_rating": 0.20,
		"ballistic_protection": 0.25,
		"cut_scratch_protection": 0.75
	})

	_register_item({
		"id": "knee_pads",
		"name": "Rodilleras Tácticas Rígidas",
		"layer": BodyPartClass.ClothingLayer.LAYER_4_TACTICAL,
		"covered_parts": [BodyPartClass.PartType.LEFT_LEG, BodyPartClass.PartType.RIGHT_LEG, BodyPartClass.PartType.LEFT_CALF, BodyPartClass.PartType.RIGHT_CALF],
		"insulation_clo": 0.05,
		"dry_weight_kg": 0.50,
		"max_water_absorption_kg": 0.05,
		"wind_resistance": 0.20,
		"waterproof_rating": 0.20,
		"ballistic_protection": 0.25,
		"cut_scratch_protection": 0.75
	})

	# -------------------------------------------------------------------------
	# CAPA 5: ACCESORIOS (LAYER_5_ACCESSORY)
	# -------------------------------------------------------------------------
	_register_item({
		"id": "wool_beanie",
		"name": "Gorro Térmico de Lana",
		"layer": BodyPartClass.ClothingLayer.LAYER_5_ACCESSORY,
		"covered_parts": [BodyPartClass.PartType.HEAD],
		"insulation_clo": 0.35,
		"dry_weight_kg": 0.12,
		"max_water_absorption_kg": 0.25,
		"wind_resistance": 0.30,
		"waterproof_rating": 0.05,
		"ballistic_protection": 0.0,
		"cut_scratch_protection": 0.10
	})

	_register_item({
		"id": "wool_scarf",
		"name": "Bufanda Gruesa de Cuello",
		"layer": BodyPartClass.ClothingLayer.LAYER_5_ACCESSORY,
		"covered_parts": [BodyPartClass.PartType.NECK],
		"insulation_clo": 0.40,
		"dry_weight_kg": 0.20,
		"max_water_absorption_kg": 0.35,
		"wind_resistance": 0.35,
		"waterproof_rating": 0.05,
		"ballistic_protection": 0.0,
		"cut_scratch_protection": 0.15
	})

	_register_item({
		"id": "leather_gloves",
		"name": "Guantes Tácticos de Cuero",
		"layer": BodyPartClass.ClothingLayer.LAYER_5_ACCESSORY,
		"covered_parts": [BodyPartClass.PartType.LEFT_HAND, BodyPartClass.PartType.RIGHT_HAND],
		"insulation_clo": 0.28,
		"dry_weight_kg": 0.26,
		"max_water_absorption_kg": 0.20,
		"wind_resistance": 0.75,
		"waterproof_rating": 0.60,
		"ballistic_protection": 0.05,
		"cut_scratch_protection": 0.80
	})

	# -------------------------------------------------------------------------
	# MOCHILAS Y CONTENEDORES DE ESPALDA (CONTAINERS)
	# -------------------------------------------------------------------------
	_register_backpack({
		"id": "fanny_pack",
		"name": "Riñonera / Pouch Táctico",
		"dry_weight_kg": 0.35,
		"max_capacity_kg": 4.0,
		"max_water_absorption_kg": 0.15,
		"visual_template": {
			"overlay_type": "fanny_pack",
			"socket_bone": "mixamorig_Hips",
			"default_mesh_path": "",
			"offset_3d": Vector3(0.0, 0.0, 0.12),
			"scale_3d": Vector3(1.0, 1.0, 1.0)
		}
	})

	_register_backpack({
		"id": "sidebag_tactical",
		"name": "Bolso Táctico Cruzado (Sidebag 8L)",
		"dry_weight_kg": 0.55,
		"max_capacity_kg": 8.0,
		"max_water_absorption_kg": 0.30,
		"visual_template": {
			"overlay_type": "sidebag_sling",
			"socket_bone": "mixamorig_Spine",
			"default_mesh_path": "",
			"offset_3d": Vector3(-0.15, -0.05, 0.08),
			"scale_3d": Vector3(1.0, 1.0, 1.0)
		}
	})

	_register_backpack({
		"id": "backpack_school",
		"name": "Mochila Urbana (15L)",
		"dry_weight_kg": 0.75,
		"max_capacity_kg": 12.0,
		"max_water_absorption_kg": 0.60,
		"visual_template": {
			"overlay_type": "backpack_harness",
			"socket_bone": "mixamorig_Spine2",
			"default_mesh_path": "",
			"offset_3d": Vector3(0.0, 0.05, -0.15),
			"scale_3d": Vector3(1.0, 1.0, 1.0)
		}
	})

	_register_backpack({
		"id": "backpack_assault",
		"name": "Mochila de Asalto Táctica (35L)",
		"dry_weight_kg": 1.65,
		"max_capacity_kg": 25.0,
		"max_water_absorption_kg": 0.80,
		"visual_template": {
			"overlay_type": "backpack_harness",
			"socket_bone": "mixamorig_Spine2",
			"default_mesh_path": "",
			"offset_3d": Vector3(0.0, 0.08, -0.18),
			"scale_3d": Vector3(1.0, 1.0, 1.0)
		}
	})

	_register_backpack({
		"id": "backpack_military",
		"name": "Mochila Militar ALICE / Expedición (65L)",
		"dry_weight_kg": 2.85,
		"max_capacity_kg": 45.0,
		"max_water_absorption_kg": 1.10,
		"visual_template": {
			"overlay_type": "backpack_harness",
			"socket_bone": "mixamorig_Spine2",
			"default_mesh_path": "",
			"offset_3d": Vector3(0.0, 0.10, -0.22),
			"scale_3d": Vector3(1.0, 1.0, 1.0)
		}
	})

static func _register_item(dict: Dictionary) -> void:
	if not dict.has("can_stack"):
		var layer = dict.get("layer", -1)
		var is_shoes = dict.get("is_footwear", false)
		if is_shoes or layer == BodyPartClass.ClothingLayer.LAYER_4_TACTICAL:
			dict["can_stack"] = false
			dict["is_unique_slot"] = true
		else:
			dict["can_stack"] = true
			dict["is_unique_slot"] = false
	if not dict.has("max_stack"):
		dict["max_stack"] = 5 if dict.get("can_stack", false) else 1
	_catalog[dict["id"]] = dict

static func _register_backpack(dict: Dictionary) -> void:
	dict["can_stack"] = false
	dict["is_unique_slot"] = true
	dict["max_stack"] = 1
	_backpack_catalog[dict["id"]] = dict

static func get_item(id: String) -> Dictionary:
	_ensure_initialized()
	return _catalog.get(id, {})

static func get_backpack(id: String) -> Dictionary:
	_ensure_initialized()
	return _backpack_catalog.get(id, {})

static func get_all_items() -> Dictionary:
	_ensure_initialized()
	return _catalog

static func get_all_backpacks() -> Dictionary:
	_ensure_initialized()
	return _backpack_catalog

static func get_items_for_part(part_type: int) -> Array:
	_ensure_initialized()
	var res: Array = []
	for item in _catalog.values():
		var parts = item.get("covered_parts", [])
		if part_type in parts:
			res.append(item)
	return res

static func get_items_for_part_and_layer(part_type: int, layer: int) -> Array:
	_ensure_initialized()
	var res: Array = []
	for item in _catalog.values():
		if item.get("layer", -1) == layer:
			var parts = item.get("covered_parts", [])
			if part_type in parts:
				res.append(item)
	return res

static func get_item_visual_template(item_id: String) -> Dictionary:
	_ensure_initialized()
	var item = get_item(item_id)
	if item.is_empty():
		item = get_backpack(item_id)
	if item.has("visual_template"):
		return item["visual_template"]
	
	var layer = item.get("layer", -1)
	var covered = item.get("covered_parts", [])
	var ov_type = "generic"
	var sock = "mixamorig_Spine1"
	if layer == BodyPartClass.ClothingLayer.LAYER_2_BASE:
		ov_type = "tshirt"
		sock = "mixamorig_Spine1"
	elif layer == BodyPartClass.ClothingLayer.LAYER_4_TACTICAL:
		ov_type = "vest_tactical"
		sock = "mixamorig_Spine2"
	elif BodyPartClass.PartType.HEAD in covered:
		ov_type = "headwear"
		sock = "mixamorig_Head"
	elif BodyPartClass.PartType.LEFT_FOOT in covered:
		ov_type = "boots"
		sock = "mixamorig_LeftFoot"
	elif BodyPartClass.PartType.LEFT_LEG in covered:
		ov_type = "pants"
		sock = "mixamorig_Hips"
		
	return {
		"overlay_type": ov_type,
		"socket_bone": sock,
		"default_mesh_path": "",
		"offset_3d": Vector3.ZERO,
		"scale_3d": Vector3.ONE
	}
