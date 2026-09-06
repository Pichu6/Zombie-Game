class_name BodyPart
extends RefCounted

# Granularidad Anatómica Completa (17 Zonas Clínicas - 3 Segmentos por Miembro)
enum PartType {
	HEAD,
	NECK,
	TORSO_UPPER,
	TORSO_LOWER,
	GROIN,
	# Miembros Superiores (Brazo, Antebrazo, Mano)
	LEFT_ARM,
	RIGHT_ARM,
	LEFT_FOREARM,
	RIGHT_FOREARM,
	LEFT_HAND,
	RIGHT_HAND,
	# Miembros Inferiores (Muslo, Pantorrilla, Pie)
	LEFT_LEG,
	RIGHT_LEG,
	LEFT_CALF,
	RIGHT_CALF,
	LEFT_FOOT,
	RIGHT_FOOT,
	# Alias de retrocompatibilidad
	TORSO = TORSO_UPPER,
	LEFT_UPPER_ARM = LEFT_ARM,
	RIGHT_UPPER_ARM = RIGHT_ARM,
	LEFT_THIGH = LEFT_LEG,
	RIGHT_THIGH = RIGHT_LEG,
	LEFT_SHIN = LEFT_CALF,
	RIGHT_SHIN = RIGHT_CALF
}

# Capas de Ropa (GDD Sección 4.2)
enum ClothingLayer {
	LAYER_0_SKIN = 0,
	LAYER_1_UNDERWEAR = 1,
	LAYER_2_BASE = 2,
	LAYER_3_OUTER = 3,
	LAYER_4_TACTICAL = 4,
	LAYER_5_ACCESSORY = 5
}

# Tipos de Lesiones y Heridas Graduadas
enum WoundType {
	NONE = 0,
	SCRATCH = 1,
	LACERATION = 2,
	DEEP_CUT = 3,
	PUNCTURE_BITE = 4,
	ARTERIAL_RUPTURE = 5
}

# Identificación y Biometría
var type: PartType
var display_name: String
var integrity: float = 100.0 # 0.0 a 100.0
var bleeding_rate_base: float = 0.0 # mL/s de hemorragia base
var pain_level: float = 0.0 # 0.0 a 100.0
var has_fracture: bool = false
var is_tourniquet_applied: bool = false
var is_bandaged: bool = false
var is_splinted: bool = false # Férula o entablillado de inmovilización ósea
var is_disinfected: bool = false # Esterilización antiséptica de la herida
var active_wound_type: WoundType = WoundType.NONE # Clasificación de trauma activo
var infection_level: float = 0.0 # 0.0 a 100.0 (sepsis/infección vírica o bacteriana)
var arterial_weight: float = 1.0 # Multiplicador de riesgo de hemorragia arterial (ej. cuello 2.5x o femoral 2.0x)

# -----------------------------------------------------------------------------
# PUENTE HACIA AMPUTACIONES (Amputation Bridge)
# -----------------------------------------------------------------------------
var can_be_amputated: bool = false
var parent_part_type: int = -1 # Parte anatómica superior (ej. LEFT_HAND tiene como padre LEFT_ARM)
var child_part_types: Array = [] # Partes dependientes en cascada (ej. amputar brazo corta la mano)
var amputation_bleed_rate: float = 120.0 # mL/s de hemorragia letal al amputar sin torniquete
var rig_bone_name: String = "" # Hueso del Skeleton3D de Godot (ej. "mixamorig_LeftHand")
var is_severed: bool = false # Miembro seccionado/amputado

# -----------------------------------------------------------------------------
# PUENTE HACIA ROPA, EQUIPAMIENTO Y ANTI-CLIPPING (Clothing / Equipment Bridge)
# -----------------------------------------------------------------------------
# Capas de ropa activas: { layer_id: int -> Dictionary con metadata del ítem }
var clothing_layers: Dictionary = {}
var equipped_clothing_list: Array[Dictionary] = [] # Lista secuencial de todas las prendas superpuestas
var socket_names: Array[String] = [] # Sockets asociados a esta parte (ej. ["hat", "mask"])
var is_mesh_hidden_by_clothing: bool = false # Anti-clipping: oculta submalla 3D si la cubre ropa cerrada
var base_insulation_clo: float = 0.0 # Aislamiento térmico en clo (1 clo = 0.155 m²·K/W)
var wetness_pct: float = 0.0 # Humedad local por sudor o lluvia (0% a 100%)
var wind_resistance: float = 0.0 # 0.0 (tela porosa) a 1.0 (cortaviento impermeable)
var waterproof_rating: float = 0.0 # 0.0 (algodón absorbente) a 1.0 (gore-tex/plástico)
var ballistic_protection: float = 0.0 # Blindaje contra esquirlas/balas (0.0 a 1.0)
var cut_scratch_protection: float = 0.0 # Protección contra mordeduras y rasguños (0.0 a 1.0)

func _init(
	p_type: PartType,
	p_name: String,
	p_arterial_weight: float = 1.0,
	p_can_amputate: bool = false,
	p_bone_name: String = "",
	p_parent_part: int = -1,
	p_sockets: Array[String] = []
) -> void:
	type = p_type
	display_name = p_name
	arterial_weight = p_arterial_weight
	can_be_amputated = p_can_amputate
	rig_bone_name = p_bone_name
	parent_part_type = p_parent_part
	socket_names = p_sockets

# -----------------------------------------------------------------------------
# MÉTODOS DE FISIOLOGÍA Y HEMOSTASIA
# -----------------------------------------------------------------------------
func get_current_bleed_rate(bpm: float) -> float:
	if is_tourniquet_applied or bleeding_rate_base <= 0.0:
		return 0.0

	if is_bandaged:
		if is_severed:
			# Para un muñón amputado sin torniquete, un vendaje simple ralentiza la pérdida masiva
			var bpm_factor: float = pow(bpm / 75.0, 1.3)
			return max(amputation_bleed_rate, bleeding_rate_base) * bpm_factor * arterial_weight * 0.20
		else:
			# Vendaje compresivo hemostático sobre herida estándar: detiene el sangrado por completo
			return 0.0

	if is_severed:
		var bpm_factor: float = pow(bpm / 75.0, 1.3)
		return max(amputation_bleed_rate, bleeding_rate_base) * bpm_factor * arterial_weight
		
	var bpm_factor: float = pow(bpm / 75.0, 1.5)
	return bleeding_rate_base * bpm_factor * arterial_weight

func apply_wound(w_type: WoundType) -> void:
	active_wound_type = w_type
	is_bandaged = false
	is_disinfected = false
	match w_type:
		WoundType.SCRATCH:
			bleeding_rate_base += 0.1
			pain_level = clamp(pain_level + 5.0, 0.0, 100.0)
			integrity = max(0.0, integrity - 5.0)
		WoundType.LACERATION:
			bleeding_rate_base += 0.8
			pain_level = clamp(pain_level + 15.0, 0.0, 100.0)
			integrity = max(0.0, integrity - 15.0)
		WoundType.DEEP_CUT:
			bleeding_rate_base += 2.2
			pain_level = clamp(pain_level + 30.0, 0.0, 100.0)
			integrity = max(0.0, integrity - 30.0)
		WoundType.PUNCTURE_BITE:
			bleeding_rate_base += 1.2
			pain_level = clamp(pain_level + 25.0, 0.0, 100.0)
			infection_level = clamp(infection_level + 45.0, 0.0, 100.0)
			integrity = max(0.0, integrity - 20.0)
		WoundType.ARTERIAL_RUPTURE:
			bleeding_rate_base += 4.5
			pain_level = clamp(pain_level + 50.0, 0.0, 100.0)
			integrity = max(0.0, integrity - 40.0)
		WoundType.NONE:
			bleeding_rate_base = 0.0
			active_wound_type = WoundType.NONE

func heal_completely() -> void:
	integrity = 100.0
	bleeding_rate_base = 0.0
	pain_level = 0.0
	has_fracture = false
	is_tourniquet_applied = false
	is_bandaged = false
	is_splinted = false
	is_disinfected = true
	infection_level = 0.0
	active_wound_type = WoundType.NONE
	is_severed = false

# -----------------------------------------------------------------------------
# MÉTODOS DEL PUENTE DE AMPUTACIÓN
# -----------------------------------------------------------------------------
func sever(has_tourniquet: bool = false) -> void:
	if not can_be_amputated or is_severed:
		return
	is_severed = true
	integrity = 0.0
	is_tourniquet_applied = has_tourniquet
	
	if has_tourniquet:
		bleeding_rate_base = 0.0
		pain_level = clamp(pain_level + 40.0, 0.0, 100.0)
	else:
		bleeding_rate_base = amputation_bleed_rate
		pain_level = clamp(pain_level + 75.0, 0.0, 100.0)

# -----------------------------------------------------------------------------
# MÉTODOS DEL PUENTE DE ROPA Y PROTECCIÓN
# -----------------------------------------------------------------------------
func equip_clothing(layer: ClothingLayer, item_data: Dictionary) -> void:
	var can_stack: bool = item_data.get("can_stack", false)
	var item_id: String = item_data.get("id", "")
	if not can_stack:
		# Si no es apilable (ej. casco, calzado, chaleco rígido), remover prendas previas en esta capa
		for i in range(equipped_clothing_list.size() - 1, -1, -1):
			if equipped_clothing_list[i].get("layer", -1) == layer:
				equipped_clothing_list.remove_at(i)
	else:
		# Si es apilable pero el mismo ID ya está, removerlo para reinsertarlo al tope
		for i in range(equipped_clothing_list.size() - 1, -1, -1):
			if equipped_clothing_list[i].get("id", "") == item_id:
				equipped_clothing_list.remove_at(i)
				break
	equipped_clothing_list.append(item_data)
	clothing_layers[layer] = item_data
	recalculate_clothing_stats()

func unequip_clothing(layer: ClothingLayer) -> void:
	for i in range(equipped_clothing_list.size() - 1, -1, -1):
		if equipped_clothing_list[i].get("layer", -1) == layer:
			equipped_clothing_list.remove_at(i)
	if clothing_layers.has(layer):
		clothing_layers.erase(layer)
	recalculate_clothing_stats()

func unequip_clothing_item_by_id(item_id: String) -> void:
	for i in range(equipped_clothing_list.size() - 1, -1, -1):
		if equipped_clothing_list[i].get("id", "") == item_id:
			var removed_layer = equipped_clothing_list[i].get("layer", -1)
			equipped_clothing_list.remove_at(i)
			# Buscar reemplazo para clothing_layers en esa misma capa
			var replacement: Dictionary = {}
			for itm in equipped_clothing_list:
				if itm.get("layer", -1) == removed_layer:
					replacement = itm
			if not replacement.is_empty():
				clothing_layers[removed_layer] = replacement
			elif clothing_layers.has(removed_layer):
				clothing_layers.erase(removed_layer)
			break
	recalculate_clothing_stats()

func recalculate_clothing_stats() -> void:
	base_insulation_clo = 0.0
	wind_resistance = 0.0
	waterproof_rating = 0.0
	ballistic_protection = 0.0
	cut_scratch_protection = 0.0
	is_mesh_hidden_by_clothing = false

	var items_to_calc = equipped_clothing_list if equipped_clothing_list.size() > 0 else clothing_layers.values()
	for itm in items_to_calc:
		base_insulation_clo += itm.get("insulation_clo", 0.0)
		wind_resistance = max(wind_resistance, itm.get("wind_resistance", 0.0))
		waterproof_rating = max(waterproof_rating, itm.get("waterproof_rating", 0.0))
		ballistic_protection = max(ballistic_protection, itm.get("ballistic_protection", 0.0))
		cut_scratch_protection = max(cut_scratch_protection, itm.get("cut_scratch_protection", 0.0))
		if itm.get("hides_underlying_skin", false):
			is_mesh_hidden_by_clothing = true

func get_clothing_weight_kg() -> float:
	var total_w = 0.0
	var items_to_calc = equipped_clothing_list if equipped_clothing_list.size() > 0 else clothing_layers.values()
	for itm in items_to_calc:
		total_w += itm.get("dry_weight_kg", 0.0)
	return total_w

func get_anatomical_mass_kg() -> float:
	match type:
		PartType.HEAD: return 5.0
		PartType.NECK: return 1.5
		PartType.TORSO_UPPER: return 18.0
		PartType.TORSO_LOWER: return 14.0
		PartType.GROIN: return 6.0
		PartType.LEFT_ARM, PartType.RIGHT_ARM: return 2.8
		PartType.LEFT_FOREARM, PartType.RIGHT_FOREARM: return 1.6
		PartType.LEFT_HAND, PartType.RIGHT_HAND: return 0.55
		PartType.LEFT_LEG, PartType.RIGHT_LEG: return 7.5
		PartType.LEFT_CALF, PartType.RIGHT_CALF: return 3.6
		PartType.LEFT_FOOT, PartType.RIGHT_FOOT: return 1.1
	return 2.0

func get_effective_insulation_clo() -> float:
	# La ropa mojada colapsa su aislamiento térmico hasta un 75%
	var wetness_penalty: float = 1.0 - (clamp(wetness_pct / 100.0, 0.0, 1.0) * 0.75)
	return base_insulation_clo * wetness_penalty

func get_cut_scratch_resistance() -> float:
	return cut_scratch_protection

func get_ballistic_protection() -> float:
	return ballistic_protection
