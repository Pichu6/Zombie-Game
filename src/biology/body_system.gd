class_name BodySystem
extends RefCounted

const BodyPartClass = preload("res://src/biology/body_part.gd")
const EnvironmentSystemClass = preload("res://src/biology/environment_system.gd")
const CardiovascularSystemClass = preload("res://src/biology/cardiovascular_system.gd")
const ThermoregulationSystemClass = preload("res://src/biology/thermoregulation_system.gd")
const HydrationSystemClass = preload("res://src/biology/hydration_system.gd")
const MoistureSystemClass = preload("res://src/biology/moisture_system.gd")
const ScentEmitterClass = preload("res://src/biology/scent_emitter.gd")
const MetabolismSystemClass = preload("res://src/biology/metabolism_system.gd")
const ClothingDatabaseClass = preload("res://src/biology/clothing_database.gd")

enum Posture {
	STANDING,
	SITTING,
	LYING,
	CROUCHING
}

signal telemetry_updated
signal player_collapsed(reason: String)
signal posture_changed(new_posture: Posture)

# Postura y Biomecánica Postural
var current_posture: Posture = Posture.STANDING
var time_in_posture_seconds: float = 0.0
var continuous_standing_seconds: float = 0.0

# Biometría
var is_female: bool = false
var is_barefoot: bool = true # Estado de calzado: descalzo = impacto severo sin amortiguar en pies
var height_meters: float = 1.78
var muscle_mass_pct: float = 0.50
var body_fat_pct: float = 0.20
var total_weight_kg: float = 75.0

# Sistema de Equipamiento y Mochila
var equipped_items: Dictionary = {} # item_id -> Dictionary (datos del ítem)
var equipped_backpack_id: String = ""
var backpack_content_weight_kg: float = 0.0

# Subsistemas Orgánicos y Ambientales
var environment: EnvironmentSystemClass
var cardio: CardiovascularSystemClass
var thermo: ThermoregulationSystemClass
var hydration: HydrationSystemClass
var moisture: MoistureSystemClass
var scent: ScentEmitterClass
var metabolism: MetabolismSystemClass

# Partes del Cuerpo (13 Zonas Anatómicas Clínicas)
var body_parts: Dictionary = {}

# Estados Sistémicos y Energéticos
var stamina: float = 100.0
var max_stamina: float = 100.0
var glycogen_pct: float = 100.0 # 0.0% a 100.0% (Reserva de combustible muscular y hepático)
var max_glycogen_kcal: float = 2000.0 # ~2000 kcal (~400-500g glucógeno almacenado)
var total_pain: float = 0.0
var exertion_factor: float = 0.0 # 0.0 a 1.0
var is_climbing: bool = false # Modo de escalada activa (70% carga al tren superior)
var exhaustion_overload_timer: float = 0.0 # Tiempo acumulado en sobreesfuerzo agónico
var is_alive: bool = true

func _init() -> void:
	environment = EnvironmentSystemClass.new()
	cardio = CardiovascularSystemClass.new()
	thermo = ThermoregulationSystemClass.new(environment)
	hydration = HydrationSystemClass.new()
	moisture = MoistureSystemClass.new()
	scent = ScentEmitterClass.new()
	metabolism = MetabolismSystemClass.new()

	# -------------------------------------------------------------------------
	# INICIALIZACIÓN DE LAS 13 ZONAS ANATÓMICAS CLÍNICAS (GDD Sección 4.1)
	# -------------------------------------------------------------------------
	# Cabeza y Cuello
	body_parts[BodyPartClass.PartType.HEAD] = BodyPartClass.new(
		BodyPartClass.PartType.HEAD, "Cráneo y Cara", 1.8, false, "mixamorig_Head", -1, ["hat", "glasses", "mask"]
	)
	body_parts[BodyPartClass.PartType.NECK] = BodyPartClass.new(
		BodyPartClass.PartType.NECK, "Cuello y Tráquea", 2.5, false, "mixamorig_Neck", -1, ["scarf", "necklace"]
	)

	# Tronco y Eje Central
	body_parts[BodyPartClass.PartType.TORSO_UPPER] = BodyPartClass.new(
		BodyPartClass.PartType.TORSO_UPPER, "Tórax Superior", 1.5, false, "mixamorig_Spine2", -1, ["undershirt", "shirt", "jacket", "vest", "backpack"]
	)
	body_parts[BodyPartClass.PartType.TORSO_LOWER] = BodyPartClass.new(
		BodyPartClass.PartType.TORSO_LOWER, "Abdomen", 1.4, false, "mixamorig_Spine", -1, ["belt", "fanny_pack"]
	)
	body_parts[BodyPartClass.PartType.GROIN] = BodyPartClass.new(
		BodyPartClass.PartType.GROIN, "Pelvis e Ingle", 2.0, false, "mixamorig_Hips", -1, ["underwear", "holster"]
	)

	# Extremidades Superiores (Brazo, Antebrazo, Mano)
	body_parts[BodyPartClass.PartType.LEFT_ARM] = BodyPartClass.new(
		BodyPartClass.PartType.LEFT_ARM, "Brazo Izquierdo", 1.0, true, "mixamorig_LeftArm", -1, ["armband_l"]
	)
	body_parts[BodyPartClass.PartType.LEFT_FOREARM] = BodyPartClass.new(
		BodyPartClass.PartType.LEFT_FOREARM, "Antebrazo Izquierdo", 0.9, true, "mixamorig_LeftForeArm", BodyPartClass.PartType.LEFT_ARM, ["elbow_pad_l", "wristband_l"]
	)
	body_parts[BodyPartClass.PartType.LEFT_HAND] = BodyPartClass.new(
		BodyPartClass.PartType.LEFT_HAND, "Mano Izquierda", 0.8, true, "mixamorig_LeftHand", BodyPartClass.PartType.LEFT_FOREARM, ["glove_l", "watch"]
	)

	body_parts[BodyPartClass.PartType.RIGHT_ARM] = BodyPartClass.new(
		BodyPartClass.PartType.RIGHT_ARM, "Brazo Derecho", 1.0, true, "mixamorig_RightArm", -1, ["armband_r"]
	)
	body_parts[BodyPartClass.PartType.RIGHT_FOREARM] = BodyPartClass.new(
		BodyPartClass.PartType.RIGHT_FOREARM, "Antebrazo Derecho", 0.9, true, "mixamorig_RightForeArm", BodyPartClass.PartType.RIGHT_ARM, ["elbow_pad_r", "wristband_r"]
	)
	body_parts[BodyPartClass.PartType.RIGHT_HAND] = BodyPartClass.new(
		BodyPartClass.PartType.RIGHT_HAND, "Mano Derecha", 0.8, true, "mixamorig_RightHand", BodyPartClass.PartType.RIGHT_FOREARM, ["glove_r"]
	)

	# Extremidades Inferiores (Muslo, Pantorrilla, Pie)
	body_parts[BodyPartClass.PartType.LEFT_LEG] = BodyPartClass.new(
		BodyPartClass.PartType.LEFT_LEG, "Muslo Izquierdo", 1.4, true, "mixamorig_LeftUpLeg", -1, ["thigh_holster_l"]
	)
	body_parts[BodyPartClass.PartType.LEFT_CALF] = BodyPartClass.new(
		BodyPartClass.PartType.LEFT_CALF, "Pantorrilla Izquierda", 1.1, true, "mixamorig_LeftLeg", BodyPartClass.PartType.LEFT_LEG, ["pants", "knee_pad_l", "shin_guard_l"]
	)
	body_parts[BodyPartClass.PartType.LEFT_FOOT] = BodyPartClass.new(
		BodyPartClass.PartType.LEFT_FOOT, "Pie Izquierdo", 0.9, true, "mixamorig_LeftFoot", BodyPartClass.PartType.LEFT_CALF, ["socks_l", "boot_l"]
	)

	body_parts[BodyPartClass.PartType.RIGHT_LEG] = BodyPartClass.new(
		BodyPartClass.PartType.RIGHT_LEG, "Muslo Derecho", 1.4, true, "mixamorig_RightUpLeg", -1, ["thigh_holster_r"]
	)
	body_parts[BodyPartClass.PartType.RIGHT_CALF] = BodyPartClass.new(
		BodyPartClass.PartType.RIGHT_CALF, "Pantorrilla Derecha", 1.1, true, "mixamorig_RightLeg", BodyPartClass.PartType.RIGHT_LEG, ["pants", "knee_pad_r", "shin_guard_r"]
	)
	body_parts[BodyPartClass.PartType.RIGHT_FOOT] = BodyPartClass.new(
		BodyPartClass.PartType.RIGHT_FOOT, "Pie Derecho", 0.9, true, "mixamorig_RightFoot", BodyPartClass.PartType.RIGHT_CALF, ["socks_r", "boot_r"]
	)

	# Vincular partes dependientes para cascada de amputación
	body_parts[BodyPartClass.PartType.LEFT_ARM].child_part_types = [BodyPartClass.PartType.LEFT_FOREARM, BodyPartClass.PartType.LEFT_HAND]
	body_parts[BodyPartClass.PartType.LEFT_FOREARM].child_part_types = [BodyPartClass.PartType.LEFT_HAND]
	body_parts[BodyPartClass.PartType.RIGHT_ARM].child_part_types = [BodyPartClass.PartType.RIGHT_FOREARM, BodyPartClass.PartType.RIGHT_HAND]
	body_parts[BodyPartClass.PartType.RIGHT_FOREARM].child_part_types = [BodyPartClass.PartType.RIGHT_HAND]
	body_parts[BodyPartClass.PartType.LEFT_LEG].child_part_types = [BodyPartClass.PartType.LEFT_CALF, BodyPartClass.PartType.LEFT_FOOT]
	body_parts[BodyPartClass.PartType.LEFT_CALF].child_part_types = [BodyPartClass.PartType.LEFT_FOOT]
	body_parts[BodyPartClass.PartType.RIGHT_LEG].child_part_types = [BodyPartClass.PartType.RIGHT_CALF, BodyPartClass.PartType.RIGHT_FOOT]
	body_parts[BodyPartClass.PartType.RIGHT_CALF].child_part_types = [BodyPartClass.PartType.RIGHT_FOOT]

	recalculate_weight()

func recalculate_weight() -> void:
	var visceral_mass: float = 22.0 * pow(height_meters / 1.75, 2.0)
	var muscle_kg: float = (height_meters / 1.75) * (14.0 + (muscle_mass_pct * 36.0))
	var fat_kg: float = (height_meters / 1.75) * (3.0 + (body_fat_pct * 75.0))
	
	total_weight_kg = visceral_mass + muscle_kg + fat_kg
	if thermo:
		thermo.body_thermal_mass = total_weight_kg * 3.47

func amputate_part(part_type: BodyPartClass.PartType, has_tourniquet: bool = false) -> void:
	var part: BodyPart = body_parts.get(part_type)
	if not part or not part.can_be_amputated or part.is_severed:
		return
	
	part.sever(has_tourniquet)
	
	# Cascada inmediata a extremidades hijas
	for child_type in part.child_part_types:
		var child: BodyPart = body_parts.get(child_type)
		if child:
			child.is_severed = true
			child.integrity = 0.0
			child.is_tourniquet_applied = has_tourniquet
			child.bleeding_rate_base = 0.0 # Hemorragia unificada en el muñón proximal

func equip_item(item_id: String) -> bool:
	var item: Dictionary = ClothingDatabaseClass.get_item(item_id)
	if item.is_empty():
		push_warning("BodySystem: Item no encontrado en base de datos: %s" % item_id)
		return false

	var layer: int = item.get("layer", -1)
	var covered_parts: Array = item.get("covered_parts", [])
	var can_stack: bool = item.get("can_stack", false)

	# 1. Si no es apilable (ej. cascos, chalecos rígidos, calzado), desequipar prendas previas en esa capa
	if not can_stack:
		var items_to_remove: Array[String] = []
		for p_type in covered_parts:
			var part: BodyPart = body_parts.get(p_type)
			if part:
				for existing in part.equipped_clothing_list:
					var ex_id = existing.get("id", "")
					if existing.get("layer", -1) == layer and ex_id != "" and not (ex_id in items_to_remove):
						items_to_remove.append(ex_id)
				if part.clothing_layers.has(layer):
					var existing_id = part.clothing_layers[layer].get("id", "")
					if existing_id != "" and not (existing_id in items_to_remove):
						items_to_remove.append(existing_id)

		for old_id in items_to_remove:
			unequip_item(old_id)

	# 2. Equipar en todas las partes anatómicas correspondientes
	for p_type in covered_parts:
		var part: BodyPart = body_parts.get(p_type)
		if part and not part.is_severed:
			part.equip_clothing(layer, item)

	equipped_items[item_id] = item

	# 3. Comprobar si calza los pies
	if item.get("is_footwear", false):
		is_barefoot = false

	# 4. Actualizar termorregulación y clo total
	thermo.clothing_insulation_clo = get_total_clothing_clo()
	return true

func equip_clothing_item(item_or_id) -> bool:
	if item_or_id is String:
		return equip_item(item_or_id)
	elif item_or_id is Dictionary and item_or_id.has("id"):
		return equip_item(item_or_id["id"])
	return false

func unequip_item(item_id: String) -> bool:
	if not equipped_items.has(item_id):
		return false

	var item: Dictionary = equipped_items[item_id]
	var layer: int = item.get("layer", -1)
	var covered_parts: Array = item.get("covered_parts", [])

	for p_type in covered_parts:
		var part: BodyPart = body_parts.get(p_type)
		if part:
			part.unequip_clothing_item_by_id(item_id)

	equipped_items.erase(item_id)

	# Si se desequipó calzado, comprobar si queda otro calzado en los pies
	if item.get("is_footwear", false):
		var has_shoes = false
		for foot_type in [BodyPartClass.PartType.LEFT_FOOT, BodyPartClass.PartType.RIGHT_FOOT]:
			var foot: BodyPart = body_parts.get(foot_type)
			if foot:
				for itm in foot.equipped_clothing_list:
					if itm.get("is_footwear", false):
						has_shoes = true
						break
				for l in foot.clothing_layers.values():
					if l.get("is_footwear", false):
						has_shoes = true
						break
		is_barefoot = not has_shoes

	thermo.clothing_insulation_clo = get_total_clothing_clo()
	return true

func equip_backpack(backpack_id: String, content_weight_kg: float = 0.0) -> bool:
	if backpack_id == "":
		equipped_backpack_id = ""
		backpack_content_weight_kg = 0.0
		return true

	var bp = ClothingDatabaseClass.get_backpack(backpack_id)
	if bp.is_empty():
		push_warning("BodySystem: Mochila no encontrada: %s" % backpack_id)
		return false

	equipped_backpack_id = backpack_id
	var max_cap: float = bp.get("max_capacity_kg", 20.0)
	backpack_content_weight_kg = clamp(content_weight_kg, 0.0, max_cap)
	return true

func set_backpack_content_weight(weight_kg: float) -> void:
	if equipped_backpack_id != "":
		var bp = ClothingDatabaseClass.get_backpack(equipped_backpack_id)
		var max_cap: float = bp.get("max_capacity_kg", 20.0)
		backpack_content_weight_kg = clamp(weight_kg, 0.0, max_cap)
	else:
		backpack_content_weight_kg = 0.0

# -----------------------------------------------------------------------------
# CONTROL Y GESTIÓN DE POSTURA CORPORAL (BIPEDESTACIÓN, SEDESTACIÓN, DECÚBITO)
# -----------------------------------------------------------------------------
func set_posture(new_posture: Posture) -> void:
	if current_posture != new_posture:
		current_posture = new_posture
		time_in_posture_seconds = 0.0
		if new_posture == Posture.SITTING or new_posture == Posture.LYING:
			exertion_factor = 0.0
		posture_changed.emit(new_posture)
		telemetry_updated.emit()

func get_posture_name() -> String:
	match current_posture:
		Posture.STANDING:
			return "De Pie"
		Posture.SITTING:
			return "Sentado"
		Posture.LYING:
			return "Acostado"
		Posture.CROUCHING:
			return "Agachado"
	return "Desconocido"

func get_posture_formatted_time() -> String:
	var total_sec: int = int(time_in_posture_seconds)
	var hours: int = total_sec / 3600
	var minutes: int = (total_sec % 3600) / 60
	var seconds: int = total_sec % 60
	if hours > 0:
		return "%02dh %02dm" % [hours, minutes]
	elif minutes > 0:
		return "%02dm %02ds" % [minutes, seconds]
	else:
		return "%02ds" % seconds

func get_posture_status_text() -> String:
	var time_str = get_posture_formatted_time()
	var mass = get_total_moving_mass_kg()
	match current_posture:
		Posture.STANDING:
			if continuous_standing_seconds < 1800.0:
				return "🧍 De pie (%s) | Carga soportada: %.1f kg" % [time_str, mass]
			elif continuous_standing_seconds < 5400.0:
				return "🧍 De pie (%s) | Fatiga plantar leve (%.1f kg)" % [time_str, mass]
			else:
				return "🧍 De pie (%s) | Sobrecarga postural plantar/lumbar (%.1f kg)" % [time_str, mass]
		Posture.SITTING:
			return "🪑 Sentado (%s) | Aliviando pies y zona lumbar" % time_str
		Posture.LYING:
			return "🛏️ Acostado (%s) | Descompresión gravitatoria total y retorno venoso" % time_str
		Posture.CROUCHING:
			if time_in_posture_seconds > 180.0:
				return "🧎 Agachado (%s) | Tensión severa en rodillas y cuádriceps" % time_str
			else:
				return "🧎 Agachado (%s) | Centro de gravedad bajo" % time_str
	return "De pie"

# -----------------------------------------------------------------------------
# CÁLCULOS FÍSICOS DE MASA DINÁMICA (PESO SECO VS MOJADO + CARGA)
# -----------------------------------------------------------------------------
func get_gear_dry_weight_kg() -> float:
	var total_dry: float = 0.0
	for item in equipped_items.values():
		total_dry += item.get("dry_weight_kg", 0.0)
	return total_dry

func get_gear_absorbed_water_kg() -> float:
	var total_water: float = 0.0
	for item in equipped_items.values():
		var max_water = item.get("max_water_absorption_kg", 0.0)
		if max_water > 0.0:
			var parts: Array = item.get("covered_parts", [])
			var avg_wetness: float = 0.0
			if parts.size() > 0:
				var sum_wet: float = 0.0
				var count: int = 0
				for p_type in parts:
					var part: BodyPart = body_parts.get(p_type)
					if part:
						sum_wet += part.wetness_pct
						count += 1
				avg_wetness = sum_wet / max(1, count)
			else:
				avg_wetness = moisture.wetness_pct
			total_water += max_water * (avg_wetness / 100.0)

	# Agua absorbida por la mochila si está equipada
	if equipped_backpack_id != "":
		var bp = ClothingDatabaseClass.get_backpack(equipped_backpack_id)
		var bp_max_water = bp.get("max_water_absorption_kg", 0.5)
		total_water += bp_max_water * (moisture.wetness_pct / 100.0)

	return total_water

func get_backpack_total_weight_kg() -> float:
	if equipped_backpack_id == "":
		return 0.0
	var bp = ClothingDatabaseClass.get_backpack(equipped_backpack_id)
	return bp.get("dry_weight_kg", 0.0) + backpack_content_weight_kg

func get_total_moving_mass_kg() -> float:
	var mud_kg = environment.get_mud_adhesion_weight_kg() if environment else 0.0
	return total_weight_kg + get_gear_dry_weight_kg() + get_gear_absorbed_water_kg() + get_backpack_total_weight_kg() + mud_kg

func get_total_clothing_clo() -> float:
	var sum_clo: float = 0.0
	var count_equipped: int = 0
	for part in body_parts.values():
		var clo = part.get_effective_insulation_clo()
		if clo > 0.0:
			sum_clo += clo
			count_equipped += 1
	if count_equipped > 0:
		return sum_clo / 13.0
	return thermo.clothing_insulation_clo

func update(delta: float) -> void:
	if not is_alive:
		return

	var total_mass_moving: float = get_total_moving_mass_kg()

	# Factores biomecánicos de pendiente, sustrato y escalada
	var slope_mult: float = environment.get_slope_metabolic_multiplier() if environment else 1.0
	var substrate_mult: float = environment.get_substrate_metabolic_multiplier() if environment else 1.0
	var climb_mult: float = 1.0
	var climb_pull_factor: float = 1.0
	if is_climbing:
		var bp_total: float = get_backpack_total_weight_kg()
		climb_pull_factor = 1.0 + (bp_total / 15.0)
		climb_mult = 1.6 * climb_pull_factor

	var effective_effort: float = exertion_factor * slope_mult * substrate_mult * climb_mult

	# 1. Metabolismo Dinámico y Balance Calórico (BMR, AEE, Lipólisis/Lipogénesis e Hipertrofia)
	var meta_res: Dictionary = metabolism.update(delta, effective_effort, total_mass_moving, height_meters, body_fat_pct, muscle_mass_pct, glycogen_pct)
	body_fat_pct = meta_res.fat_pct
	muscle_mass_pct = meta_res.muscle_pct
	recalculate_weight()

	# Ratio de acondicionamiento físico: músculo útil vs masa grasa y condición cardiovascular
	var fitness_ratio: float = clamp((muscle_mass_pct / max(0.12, body_fat_pct + 0.10)) * (metabolism.cardio_conditioning / 40.0), 0.35, 2.8)

	# 2. Microtrauma por impacto mecánico en pies, piernas y rodillas
	var weight_ratio: float = total_mass_moving / 70.0
	var barefoot_mult: float = 3.5 if is_barefoot else 0.4
	var surface_mult: float = environment.get_surface_impact_multiplier() if environment else 1.0
	var mass_impact_mult: float = pow(weight_ratio, 1.8) * barefoot_mult * surface_mult
	var downhill_mult: float = environment.get_downhill_joint_impact_multiplier() if environment else 1.0

	time_in_posture_seconds += delta

	# Biomecánica según postura corporal
	match current_posture:
		Posture.STANDING:
			if exertion_factor <= 0.02:
				continuous_standing_seconds += delta
				if continuous_standing_seconds > 2700.0:
					# Sobrecarga plantar acumulativa por bipedestación estática sostenida (>45 min)
					var w_ratio = total_mass_moving / 70.0
					var bf_factor = 2.0 if is_barefoot else 1.0
					var foot_strain = 0.00035 * pow(w_ratio, 1.2) * bf_factor * delta
					var left_foot: BodyPart = body_parts.get(BodyPartClass.PartType.LEFT_FOOT)
					var right_foot: BodyPart = body_parts.get(BodyPartClass.PartType.RIGHT_FOOT)
					if left_foot and not left_foot.is_severed:
						left_foot.pain_level = clamp(left_foot.pain_level + foot_strain, 0.0, 45.0)
					if right_foot and not right_foot.is_severed:
						right_foot.pain_level = clamp(right_foot.pain_level + foot_strain, 0.0, 45.0)

				# Compresión de columna lumbar por bipedestación estática con mochila pesada (>10 kg, >1.5h)
				if equipped_backpack_id != "" and get_backpack_total_weight_kg() > 10.0 and continuous_standing_seconds > 5400.0:
					var lumbar_strain = 0.00025 * (get_backpack_total_weight_kg() / 15.0) * delta
					var torso_lower: BodyPart = body_parts.get(BodyPartClass.PartType.TORSO_LOWER)
					if torso_lower and not torso_lower.is_severed:
						torso_lower.pain_level = clamp(torso_lower.pain_level + lumbar_strain, 0.0, 40.0)
			else:
				# Marcha dinámica activa la bomba muscular de la pantorrilla facilitando retorno venoso
				continuous_standing_seconds = max(0.0, continuous_standing_seconds - delta * 0.5)

		Posture.SITTING:
			exertion_factor = 0.0
			continuous_standing_seconds = max(0.0, continuous_standing_seconds - delta * 2.0)
			# Alivio de pies y zona lumbar al descargar el peso
			for foot_type in [BodyPartClass.PartType.LEFT_FOOT, BodyPartClass.PartType.RIGHT_FOOT]:
				var foot: BodyPart = body_parts.get(foot_type)
				if foot and foot.bleeding_rate_base == 0.0 and not foot.is_severed:
					foot.pain_level = max(0.0, foot.pain_level - 0.015 * delta)
			var torso_lower: BodyPart = body_parts.get(BodyPartClass.PartType.TORSO_LOWER)
			if torso_lower and torso_lower.bleeding_rate_base == 0.0 and not torso_lower.is_severed:
				torso_lower.pain_level = max(0.0, torso_lower.pain_level - 0.010 * delta)

		Posture.LYING:
			exertion_factor = 0.0
			continuous_standing_seconds = max(0.0, continuous_standing_seconds - delta * 4.0)
			# Descompresión gravitatoria completa y retorno venoso máximo
			for part_type in [BodyPartClass.PartType.LEFT_FOOT, BodyPartClass.PartType.RIGHT_FOOT, BodyPartClass.PartType.LEFT_LEG, BodyPartClass.PartType.RIGHT_LEG]:
				var p: BodyPart = body_parts.get(part_type)
				if p and p.bleeding_rate_base == 0.0 and not p.is_severed:
					p.pain_level = max(0.0, p.pain_level - 0.040 * delta)
			var torso_lower: BodyPart = body_parts.get(BodyPartClass.PartType.TORSO_LOWER)
			if torso_lower and torso_lower.bleeding_rate_base == 0.0 and not torso_lower.is_severed:
				torso_lower.pain_level = max(0.0, torso_lower.pain_level - 0.030 * delta)

		Posture.CROUCHING:
			# Sobrecarga en tendones rotulianos y cuádriceps tras 3 minutos agachado
			if time_in_posture_seconds > 180.0:
				var crouch_strain = 0.002 * delta
				for leg_type in [BodyPartClass.PartType.LEFT_LEG, BodyPartClass.PartType.RIGHT_LEG]:
					var leg: BodyPart = body_parts.get(leg_type)
					if leg and not leg.is_severed:
						leg.pain_level = clamp(leg.pain_level + crouch_strain, 0.0, 35.0)

	if exertion_factor > 0.35:
		var foot_impact_rate: float = pow(exertion_factor, 1.3) * mass_impact_mult * 0.0025 * delta
		var leg_impact_rate: float = foot_impact_rate * 0.45 * downhill_mult # Transmisión articular hacia rodillas y pantorrillas

		var left_foot: BodyPart = body_parts.get(BodyPartClass.PartType.LEFT_FOOT)
		var right_foot: BodyPart = body_parts.get(BodyPartClass.PartType.RIGHT_FOOT)
		var left_leg: BodyPart = body_parts.get(BodyPartClass.PartType.LEFT_LEG)
		var right_leg: BodyPart = body_parts.get(BodyPartClass.PartType.RIGHT_LEG)

		if left_foot and not left_foot.is_severed:
			left_foot.pain_level = clamp(left_foot.pain_level + foot_impact_rate, 0.0, 50.0)
		if right_foot and not right_foot.is_severed:
			right_foot.pain_level = clamp(right_foot.pain_level + foot_impact_rate, 0.0, 50.0)
		if left_leg and not left_leg.is_severed:
			left_leg.pain_level = clamp(left_leg.pain_level + leg_impact_rate, 0.0, 50.0)
		if right_leg and not right_leg.is_severed:
			right_leg.pain_level = clamp(right_leg.pain_level + leg_impact_rate, 0.0, 50.0)

		# Laceraciones en pies por correr descalzo en superficies filosas (vidrios/escombros)
		if is_barefoot and environment and environment.surface_roughness > 0.6:
			var laceration_rate = (environment.surface_roughness - 0.5) * 0.15 * delta
			if left_foot and left_foot.bleeding_rate_base < 5.0:
				left_foot.bleeding_rate_base += laceration_rate
			if right_foot and right_foot.bleeding_rate_base < 5.0:
				right_foot.bleeding_rate_base += laceration_rate

	elif exertion_factor > 0.02:
		# Caminata continua:
		var walk_impact_rate: float = (exertion_factor * 0.5) * mass_impact_mult * 0.0036 * delta
		var walk_leg_rate: float = walk_impact_rate * 0.25 * downhill_mult

		var left_foot: BodyPart = body_parts.get(BodyPartClass.PartType.LEFT_FOOT)
		var right_foot: BodyPart = body_parts.get(BodyPartClass.PartType.RIGHT_FOOT)
		var left_leg: BodyPart = body_parts.get(BodyPartClass.PartType.LEFT_LEG)
		var right_leg: BodyPart = body_parts.get(BodyPartClass.PartType.RIGHT_LEG)

		if left_foot and not left_foot.is_severed:
			left_foot.pain_level = clamp(left_foot.pain_level + walk_impact_rate, 0.0, 30.0)
		if right_foot and not right_foot.is_severed:
			right_foot.pain_level = clamp(right_foot.pain_level + walk_impact_rate, 0.0, 30.0)
		if left_leg and not left_leg.is_severed:
			left_leg.pain_level = clamp(left_leg.pain_level + walk_leg_rate, 0.0, 30.0)
		if right_leg and not right_leg.is_severed:
			right_leg.pain_level = clamp(right_leg.pain_level + walk_leg_rate, 0.0, 30.0)

	elif exertion_factor <= 0.02:
		if current_posture != Posture.STANDING or continuous_standing_seconds <= 2700.0:
			for part_type in [BodyPartClass.PartType.LEFT_FOOT, BodyPartClass.PartType.RIGHT_FOOT, BodyPartClass.PartType.LEFT_LEG, BodyPartClass.PartType.RIGHT_LEG]:
				var part: BodyPart = body_parts.get(part_type)
				if part and part.bleeding_rate_base == 0.0 and not part.is_severed:
					part.pain_level = max(0.0, part.pain_level - (0.015 * delta))

	# Biomecánica de Escalada / Trepado:
	if is_climbing and exertion_factor > 0.02:
		var left_arm: BodyPart = body_parts.get(BodyPartClass.PartType.LEFT_ARM)
		var right_arm: BodyPart = body_parts.get(BodyPartClass.PartType.RIGHT_ARM)
		var left_hand: BodyPart = body_parts.get(BodyPartClass.PartType.LEFT_HAND)
		var right_hand: BodyPart = body_parts.get(BodyPartClass.PartType.RIGHT_HAND)

		var arm_strain: float = 0.018 * climb_pull_factor * delta
		if left_arm and not left_arm.is_severed:
			left_arm.pain_level = clamp(left_arm.pain_level + arm_strain, 0.0, 45.0)
		if right_arm and not right_arm.is_severed:
			right_arm.pain_level = clamp(right_arm.pain_level + arm_strain, 0.0, 45.0)

		# Abrasión palmar sin guantes tácticos
		var has_tactical_gloves: bool = false
		if left_hand and left_hand.get_cut_scratch_resistance() >= 0.20:
			has_tactical_gloves = true
		if not has_tactical_gloves:
			var palm_abrasion: float = 0.06 * delta
			if left_hand and not left_hand.is_severed:
				left_hand.pain_level = clamp(left_hand.pain_level + palm_abrasion, 0.0, 40.0)
			if right_hand and not right_hand.is_severed:
				right_hand.pain_level = clamp(right_hand.pain_level + palm_abrasion, 0.0, 40.0)

	# Vegetación hostil: matorral denso con espinas
	if environment and environment.vegetation == EnvironmentSystemClass.VegetationDensity.THORNY_BRUSH and exertion_factor > 0.02:
		var left_leg: BodyPart = body_parts.get(BodyPartClass.PartType.LEFT_LEG)
		var right_leg: BodyPart = body_parts.get(BodyPartClass.PartType.RIGHT_LEG)
		var left_arm: BodyPart = body_parts.get(BodyPartClass.PartType.LEFT_ARM)
		var right_arm: BodyPart = body_parts.get(BodyPartClass.PartType.RIGHT_ARM)

		if left_leg and left_leg.get_cut_scratch_resistance() < 0.15 and not left_leg.is_severed:
			left_leg.pain_level = clamp(left_leg.pain_level + (0.025 * delta), 0.0, 30.0)
			if left_leg.bleeding_rate_base < 0.15:
				left_leg.bleeding_rate_base += 0.003 * delta
		if right_leg and right_leg.get_cut_scratch_resistance() < 0.15 and not right_leg.is_severed:
			right_leg.pain_level = clamp(right_leg.pain_level + (0.025 * delta), 0.0, 30.0)
			if right_leg.bleeding_rate_base < 0.15:
				right_leg.bleeding_rate_base += 0.003 * delta
		if left_arm and left_arm.get_cut_scratch_resistance() < 0.15 and not left_arm.is_severed:
			left_arm.pain_level = clamp(left_arm.pain_level + (0.015 * delta), 0.0, 25.0)
		if right_arm and right_arm.get_cut_scratch_resistance() < 0.15 and not right_arm.is_severed:
			right_arm.pain_level = clamp(right_arm.pain_level + (0.015 * delta), 0.0, 25.0)

	# Pedregal inestable con rocas sueltas (riesgo articular si descalzo)
	if environment and environment.vegetation == EnvironmentSystemClass.VegetationDensity.SCREE_ROCKS and exertion_factor > 0.02:
		if is_barefoot:
			var left_foot: BodyPart = body_parts.get(BodyPartClass.PartType.LEFT_FOOT)
			var right_foot: BodyPart = body_parts.get(BodyPartClass.PartType.RIGHT_FOOT)
			var sprain_rate: float = 0.03 * delta
			if left_foot and not left_foot.is_severed:
				left_foot.pain_level = clamp(left_foot.pain_level + sprain_rate, 0.0, 45.0)
			if right_foot and not right_foot.is_severed:
				right_foot.pain_level = clamp(right_foot.pain_level + sprain_rate, 0.0, 45.0)

	# Vadeo en agua profunda (enfriamiento conductivo masivo x25)
	if environment and environment.substrate == EnvironmentSystemClass.SubstrateType.WATER_WADING:
		if thermo.core_temperature > environment.ambient_temperature:
			var water_cooling: float = 0.008 * delta * (thermo.core_temperature - environment.ambient_temperature)
			thermo.core_temperature = max(27.0, thermo.core_temperature - water_cooling)

	# 3. Calcular dolor y tasa total de sangrado
	var total_bleed_rate: float = 0.0
	var pain_sum: float = 0.0
	var total_infection: float = 0.0

	for part in body_parts.values():
		total_bleed_rate += part.get_current_bleed_rate(cardio.heart_rate_bpm)
		pain_sum += part.pain_level
		total_infection += part.infection_level

	total_pain = clamp(pain_sum, 0.0, 100.0)

	# 4. Consumo y Depleción de Glucógeno
	var mass_cost_mult: float = pow(weight_ratio, 1.35) * slope_mult * substrate_mult * climb_mult
	var glycogen_drain: float = 0.0004 # Basal
	if exertion_factor > 0.75:
		glycogen_drain = 0.070 * mass_cost_mult # Sprint
	elif exertion_factor > 0.35:
		glycogen_drain = 0.015 * mass_cost_mult # Carrera
	elif exertion_factor > 0.02:
		glycogen_drain = 0.0035 * mass_cost_mult # Caminata lipolítica
	glycogen_pct = max(0.0, glycogen_pct - (glycogen_drain * delta))

	# Recarga de glucógeno por absorción nutricional
	if meta_res.absorbed_kcal > 0.0:
		glycogen_pct = min(100.0, glycogen_pct + (meta_res.absorbed_kcal * 0.04))

	# "El Muro" fisiológico: cuando el glucógeno cae bajo 25%, el techo de estamina se derrumba
	if glycogen_pct < 25.0:
		max_stamina = lerp(20.0, 100.0, glycogen_pct / 25.0)
	else:
		max_stamina = 100.0
	stamina = min(stamina, max_stamina)

	# 5. Actualizar Subsistemas con telemetría ambiental, masa corporal y condición física
	var rain_wetting: float = environment.get_rain_wetting_rate_ml_per_sec() if environment else 0.0
	var wind_speed: float = environment.wind_speed_kmh if environment else 5.0
	var humidity: float = environment.relative_humidity_pct if environment else 50.0
	var amb_temp: float = environment.ambient_temperature if environment else 22.0

	var total_clo: float = get_total_clothing_clo()
	moisture.update(delta, thermo.sweat_rate_ml_per_min, amb_temp, exertion_factor, rain_wetting, wind_speed, humidity, total_clo)
	for part in body_parts.values():
		part.wetness_pct = moisture.wetness_pct
	thermo.update(delta, exertion_factor, body_fat_pct, muscle_mass_pct, total_mass_moving, height_meters, hydration.current_water_ml, total_clo, moisture.wetness_pct)
	cardio.update(delta, exertion_factor, total_pain, total_bleed_rate, hydration.dehydration_level, total_mass_moving, metabolism.cardio_conditioning, muscle_mass_pct, body_fat_pct)
	hydration.update(delta, thermo.sweat_rate_ml_per_min)
	scent.update(delta, thermo.sweat_rate_ml_per_min, total_bleed_rate, total_infection)

	# 6. Dinámica de Estamina y Agotamiento Realista
	if thermo.is_shivering:
		stamina = max(0.0, stamina - (0.45 * delta))

	# Umbral aeróbico vs anaeróbico: un atleta corre aeróbicamente sin quemar toda su estamina en minutos
	var aerobic_ceiling = 0.35 + (clamp((fitness_ratio - 1.0) * 0.25, 0.0, 0.25)) # Hasta 0.60 para muy entrenados

	if exertion_factor > 0.75:
		# Sprint Anaeróbico: 15 a 45 segundos según masa y fitness
		var sprint_drain: float = 2.2 * mass_cost_mult * (1.5 / fitness_ratio)
		stamina = max(0.0, stamina - (sprint_drain * delta))
		if stamina <= 0.0:
			exhaustion_overload_timer += delta * 2.0
		else:
			exhaustion_overload_timer = max(0.0, exhaustion_overload_timer - (delta * 0.5))
	elif exertion_factor > 0.35:
		# Carrera:
		if exertion_factor > aerobic_ceiling:
			# Carrera anaeróbica: agota estamina
			var heat_penalty: float = 1.0 + (max(0.0, thermo.core_temperature - 38.0) * 0.40)
			var run_drain: float = 0.045 * mass_cost_mult * heat_penalty * (1.6 / fitness_ratio)
			stamina = max(0.0, stamina - (run_drain * delta))
		else:
			# Carrera aeróbica sostenible mientras haya glucógeno
			var aerobic_cost: float = 0.008 * mass_cost_mult * (1.0 / fitness_ratio)
			stamina = max(0.0, stamina - (aerobic_cost * delta))

		if stamina <= 0.0:
			# Claudicación / Agotamiento agudo
			exhaustion_overload_timer += delta * (1.0 / fitness_ratio)
		else:
			exhaustion_overload_timer = max(0.0, exhaustion_overload_timer - (delta * 0.5))
	elif exertion_factor > 0.02:
		# Caminata Aeróbica
		var recovery_mult: float = (cardio.total_blood_ml / cardio.max_blood_ml) * (1.0 - (hydration.dehydration_level / 150.0)) * fitness_ratio
		stamina = min(max_stamina, stamina + (0.8 * recovery_mult * delta))
		exhaustion_overload_timer = max(0.0, exhaustion_overload_timer - delta)
	else:
		# Reposo Completo
		var recovery_mult: float = (cardio.total_blood_ml / cardio.max_blood_ml) * (1.0 - (hydration.dehydration_level / 150.0))
		var posture_recovery_bonus: float = 2.0 if current_posture == Posture.LYING else (1.3 if current_posture == Posture.SITTING else 1.0)
		stamina = min(max_stamina, stamina + (3.0 * recovery_mult * posture_recovery_bonus * delta))
		exhaustion_overload_timer = max(0.0, exhaustion_overload_timer - (delta * 2.0))

	# 7. Comprobación de Signos Vitales y Colapso Clínico Justificado
	if not cardio.is_heart_beating:
		is_alive = false
		player_collapsed.emit("Paro Cardíaco por Pérdida Masiva de Sangre")
	elif thermo.core_temperature < 30.0:
		is_alive = false
		cardio.is_heart_beating = false
		cardio.heart_rate_bpm = 0.0
		player_collapsed.emit("Hipotermia Severa / Congelamiento Central (<30.0°C)")
	elif thermo.core_temperature > 41.5:
		is_alive = false
		cardio.is_heart_beating = false
		cardio.heart_rate_bpm = 0.0
		player_collapsed.emit("Hipertermia Fatal / Golpe de Calor Extremo (41.5°C)")
	elif hydration.current_water_ml <= 150.0:
		is_alive = false
		cardio.is_heart_beating = false
		cardio.heart_rate_bpm = 0.0
		player_collapsed.emit("Deshidratación Fatal / Fallo Multiorgánico")
	elif exhaustion_overload_timer > 120.0:
		# Síncope por sobreesfuerzo: colapso físico
		player_collapsed.emit("Síncope por Sobreesfuerzo Agónico (Colapso Físico)")
		exhaustion_overload_timer = 0.0
	elif total_pain >= 95.0:
		var head_part: BodyPart = body_parts.get(BodyPartClass.PartType.HEAD)
		var torso_part: BodyPart = body_parts.get(BodyPartClass.PartType.TORSO_UPPER)
		var is_vital_trauma = (head_part and head_part.pain_level > 60.0) or (torso_part and torso_part.pain_level > 60.0)
		if is_vital_trauma:
			is_alive = false
			cardio.is_heart_beating = false
			cardio.heart_rate_bpm = 0.0
			player_collapsed.emit("Shock Neurogénico Fatal por Trauma Crítico")
		else:
			player_collapsed.emit("Claudicación e Inmovilidad por Dolor Articular Extremo")

	telemetry_updated.emit()

func get_vital_status_text() -> String:
	if not is_alive:
		return "FALLECIDO (COLAPSO VITAL)"
	if not cardio.is_heart_beating:
		return "PARO CARDÍACO"
	if cardio.current_shock_state == CardiovascularSystemClass.ShockState.SEVERE_SHOCK:
		return "SHOCK HIPOVOLÉMICO SEVERO"
	if thermo.core_temperature > 40.0:
		return "GOLPE DE CALOR CRÍTICO (>40°C)"
	if thermo.is_heat_exhausted:
		return "AGOTAMIENTO TÉRMICO (>38.8°C)"
	if cardio.heart_rate_bpm >= 175.0:
		return "TAQUICARDIA EXTREMA (Sobreesfuerzo)"
	if stamina <= 0.0 and exertion_factor > 0.35:
		return "CLAUDICACIÓN POR AGOTAMIENTO AGUDO"
	if thermo.is_shivering:
		return "HIPOTERMIA (Temblores)"
	if glycogen_pct <= 0.0 and stamina <= 5.0:
		return "DEPLECIÓN DE GLUCÓGENO ('El Muro')"
	if total_pain > 45.0:
		return "DOLOR ARTICULAR SEVERO (Impacto)"
	if glycogen_pct < 20.0 or total_pain > 20.0:
		return "FATIGA MUSCULAR MODERADA"
	if stamina < 40.0 or total_pain > 8.0 or hydration.dehydration_level > 30.0:
		return "FATIGA AERÓBICA"
	return "VITALIDAD ÓPTIMA"

func get_vital_status_color() -> Color:
	if not is_alive:
		return Color(1.0, 0.2, 0.2)
	if cardio.current_shock_state == CardiovascularSystemClass.ShockState.SEVERE_SHOCK:
		return Color(1.0, 0.4, 0.1)
	if thermo.is_heat_exhausted or thermo.core_temperature > 40.0:
		return Color(1.0, 0.5, 0.2)
	if cardio.heart_rate_bpm >= 175.0 or (stamina <= 0.0 and exertion_factor > 0.35):
		return Color(1.0, 0.3, 0.2)
	if thermo.is_shivering:
		return Color(0.3, 0.7, 1.0)
	if glycogen_pct <= 0.0 and stamina <= 5.0:
		return Color(1.0, 0.25, 0.25)
	if glycogen_pct < 20.0:
		return Color(1.0, 0.5, 0.15)
	if total_pain > 45.0 or glycogen_pct < 50.0:
		return Color(1.0, 0.8, 0.2)
	if stamina < 40.0 or total_pain > 8.0:
		return Color(0.85, 0.95, 0.3)
	return Color(0.2, 1.0, 0.4)

func reset_to_base_state() -> void:
	is_alive = true
	exertion_factor = 0.0
	exhaustion_overload_timer = 0.0
	stamina = 100.0
	max_stamina = 100.0
	glycogen_pct = 100.0
	total_pain = 0.0

	if environment:
		environment.reset_to_standard()

	if cardio:
		cardio.is_heart_beating = true
		cardio.total_blood_ml = cardio.max_blood_ml
		cardio.resting_bpm = 70.0
		cardio.heart_rate_bpm = 70.0
		cardio.sustained_exertion_timer = 0.0
		cardio.blood_pressure_systolic = 120.0
		cardio.blood_pressure_diastolic = 80.0
		cardio.current_shock_state = CardiovascularSystemClass.ShockState.NORMAL

	if thermo:
		thermo.core_temperature = 37.0
		thermo.skin_temperature = 34.0
		thermo.ambient_temperature = 22.0
		thermo.sweat_rate_ml_per_min = 0.0
		thermo.is_shivering = false
		thermo.is_heat_exhausted = false
		thermo.clothing_insulation_clo = 0.5
		thermo.clothing_wetness_pct = 0.0

	if hydration:
		hydration.current_water_ml = hydration.max_water_ml
		hydration.dehydration_level = 0.0

	if moisture:
		moisture.accumulated_sweat_ml = 0.0
		moisture.wetness_pct = 0.0

	if scent:
		scent.wash_scent()

	if metabolism:
		metabolism.reset_state()

	# Reiniciar las 13 partes anatómicas
	for part in body_parts.values():
		part.integrity = 100.0
		part.bleeding_rate_base = 0.0
		part.pain_level = 0.0
		part.infection_level = 0.0
		part.is_severed = false
		part.has_fracture = false
		part.is_tourniquet_applied = false
		part.is_bandaged = false
		part.clothing_layers.clear()
		part.recalculate_clothing_stats()

	equipped_items.clear()
	equipped_backpack_id = ""
	backpack_content_weight_kg = 0.0
	is_barefoot = true
	is_climbing = false
	current_posture = Posture.STANDING
	time_in_posture_seconds = 0.0
	continuous_standing_seconds = 0.0

	if environment:
		environment.reset_to_standard()

	recalculate_weight()
	telemetry_updated.emit()

# =============================================================================
# MOTOR DE PERCEPCIÓN SENSORIAL Y PENSAMIENTOS SOMÁTICOS (FEEDBACK REALISTA)
# =============================================================================
func get_sensory_report() -> Dictionary:
	var report = {
		"mood_tag": "🌿 Confortable",
		"primary_thought": "Respiración rítmica y serena, temperatura equilibrada y músculos descansados. El cuerpo se siente ligero.",
		"thought_color": Color(0.35, 0.95, 0.5),
		"somatic_tags": []
	}

	if not is_alive:
		report.mood_tag = "💀 Inerte"
		report.primary_thought = "Sin signos vitales. El organismo ha colapsado por completo."
		report.thought_color = Color(0.65, 0.65, 0.65)
		report.somatic_tags = ["[ Paro Cardíaco ]", "[ Silencio Somático ]"]
		return report

	# 1. Comprobaciones de emergencia letal y shock
	if cardio and cardio.current_shock_state == CardiovascularSystemClass.ShockState.SEVERE_SHOCK:
		report.mood_tag = "🩸 Shock Hipovolémico"
		report.primary_thought = "La vista se me apaga... siento un frío espectral en las extremidades y un zumbido ensordecedor. Me desvanezco..."
		report.thought_color = Color(1.0, 0.25, 0.25)
	
	# 2. Amputaciones y Hemorragias
	elif _has_severed_untourniqueted_part():
		report.mood_tag = "🚨 ¡Hemorragia Arterial Masiva!"
		report.primary_thought = "¡Siento un torrente caliente saliendo a borbotones! ¡Me estoy desangrando por el muñón, necesito un torniquete YA!"
		report.thought_color = Color(1.0, 0.2, 0.2)
	elif _has_severed_part():
		report.mood_tag = "🩹 Muñón Amputado"
		report.primary_thought = "El torniquete me oprime salvajemente el muñón... siento un dolor fantasma lacerante donde solía estar mi extremidad."
		report.thought_color = Color(1.0, 0.45, 0.2)
	elif _get_active_bleeding_part_count() > 0:
		report.mood_tag = "🩸 Sangrado Abierto"
		report.primary_thought = "Siento sangre tibia escurriéndome por la piel; el pulso se me acelera del pánico y el mareo."
		report.thought_color = Color(1.0, 0.35, 0.35)

	# 3. Termorregulación y Clima
	elif thermo and (thermo.core_temperature >= 40.0 or thermo.is_heat_exhausted):
		report.mood_tag = "🔥 Golpe de Calor"
		report.primary_thought = "Siento fuego dentro del cráneo y un latido pulsátil en las sienes; la boca está seca y tengo náuseas por el calor abrasador."
		report.thought_color = Color(1.0, 0.35, 0.15)
	elif thermo and (thermo.core_temperature <= 35.2 or thermo.is_shivering):
		if moisture and moisture.wetness_pct > 40.0:
			report.mood_tag = "🥶 Hipotermia / Ropa Helada"
			report.primary_thought = "La ropa empapada se siente como láminas de hielo pegadas a las costillas; no puedo controlar los temblores y me castañean los dientes."
			report.thought_color = Color(0.4, 0.75, 1.0)
		else:
			report.mood_tag = "❄️ Viento Helado"
			report.primary_thought = "El aire gélido me corta la cara como navajas; tengo los dedos de las manos y los pies totalmente entumecidos y rígidos."
			report.thought_color = Color(0.5, 0.8, 1.0)
	elif environment and environment.ambient_temperature >= 32.0 and environment.relative_humidity_pct >= 80.0:
		report.mood_tag = "🥵 Calor Sofocante"
		report.primary_thought = "El aire es una masa densa y pegajosa; el sudor me chorrea sin evaporar y me cuesta meter aire a los pulmones."
		report.thought_color = Color(1.0, 0.6, 0.25)

	# 4. Pies y Microtrauma Articular
	elif is_barefoot and (_get_foot_pain() > 15.0):
		if environment and environment.surface_roughness >= 0.5:
			report.mood_tag = "🦶 Plantas Laceradas"
			report.primary_thought = "¡Las plantas de los pies me arden como brasas vivas! Cada zancada descalzo sobre las piedras me desgarra la piel."
			report.thought_color = Color(1.0, 0.55, 0.3)
		else:
			report.mood_tag = "🦶 Dolor Plantar"
			report.primary_thought = "Tengo los talones y metatarsos magullados del impacto descalzo; el golpeteo me repercute hasta las rodillas."
			report.thought_color = Color(0.95, 0.75, 0.3)

	# 5. Agotamiento y Glucógeno
	elif glycogen_pct <= 0.0 and stamina <= 10.0:
		report.mood_tag = "🧱 'El Muro' Muscular"
		report.primary_thought = "Mis músculos están completamente vacíos de combustible; las piernas me pesan como bloques de cemento y no responden."
		report.thought_color = Color(0.9, 0.45, 0.45)
	elif cardio and cardio.heart_rate_bpm >= 170.0:
		report.mood_tag = "🫀 Taquicardia Extrema"
		report.primary_thought = "El corazón me golpea desbocado contra la garganta; me quema el esternón y me falta el aire desesperadamente."
		report.thought_color = Color(1.0, 0.3, 0.3)
	elif cardio and cardio.heart_rate_bpm >= 135.0 and exertion_factor > 0.35:
		report.mood_tag = "🏃 Respiración Forzada"
		report.primary_thought = "Jadeando profundamente al ritmo sostenido de la carrera; el pecho trabaja a tope y el sudor me resbala por la cara."
		report.thought_color = Color(0.95, 0.85, 0.4)

	# 6. Deshidratación
	elif hydration and hydration.current_water_ml <= 1200.0:
		report.mood_tag = "🏜️ Sed Abrasadora"
		report.primary_thought = "Tengo la garganta en carne viva y la lengua como lija; siento un dolor sordo en la cabeza y necesito agua urgente."
		report.thought_color = Color(0.9, 0.65, 0.3)
	elif hydration and hydration.current_water_ml <= 1800.0:
		report.mood_tag = "🥤 Boca Seca"
		report.primary_thought = "Garganta reseca y una necesidad persistente de tragar líquido fresco."
		report.thought_color = Color(0.85, 0.85, 0.45)

	# 7. Hambre
	elif metabolism and metabolism.hunger_pct >= 75.0:
		report.mood_tag = "🥣 Inanición Voraz"
		report.primary_thought = "Un vacío doloroso me muerde el estómago; me tiemblan las manos y siento una debilidad difusa por el ayuno prolongado."
		report.thought_color = Color(0.85, 0.7, 0.4)

	# 8. Lluvia y Ropa Mojada
	elif environment and environment.rain_intensity > 0.5 and moisture and moisture.wetness_pct > 50.0:
		report.mood_tag = "🌧️ Empapado Bajo Lluvia"
		report.primary_thought = "El agua de la lluvia me resbala fría por el cuello; la ropa mojada me pesa el doble y se me pega al torso."
		report.thought_color = Color(0.5, 0.75, 0.95)

	# 9. Sobrecarga de Mochila o Ropa Pesada
	elif is_climbing:
		report.mood_tag = "🧗 Escalada Vertical"
		report.primary_thought = "Los antebrazos me arden por la tensión del agarre; cada metro hacia arriba exige toda la fuerza de mis dorsales y la gravedad me tira hacia atrás."
		report.thought_color = Color(1.0, 0.6, 0.25)
	elif current_posture == Posture.STANDING and continuous_standing_seconds > 7200.0:
		report.mood_tag = "🧍 Bipedestación Prolongada"
		report.primary_thought = "Llevo horas de pie sin descanso; los talones me palpitan con dolor sordo y siento la columna baja comprimida bajo el peso."
		report.thought_color = Color(0.95, 0.7, 0.35)
	elif current_posture == Posture.SITTING and time_in_posture_seconds < 120.0 and continuous_standing_seconds > 0.0:
		report.mood_tag = "🪑 Alivio al Sentarse"
		report.primary_thought = "Qué alivio descargar los talones y la cintura; siento cómo se descomprimen las vértebras lumbares."
		report.thought_color = Color(0.4, 0.9, 0.6)
	elif current_posture == Posture.LYING and time_in_posture_seconds < 180.0:
		report.mood_tag = "🛏️ Descompresión Total"
		report.primary_thought = "Tendido horizontalmente, la gravedad deja de machacar la columna; el retorno venoso es pleno y los músculos se relajan."
		report.thought_color = Color(0.35, 0.95, 0.7)
	elif current_posture == Posture.CROUCHING and time_in_posture_seconds > 180.0:
		report.mood_tag = "🧎 Tensión en Rodillas"
		report.primary_thought = "Demasiado tiempo agachado; me queman los cuádriceps y las rodillas se quejan de la flexión forzada."
		report.thought_color = Color(0.95, 0.75, 0.3)
	elif get_backpack_total_weight_kg() >= 20.0 and exertion_factor > 0.15:
		report.mood_tag = "🎒 Sobrecarga en Espalda"
		report.primary_thought = "Las correas de la mochila me clavan los hombros; la carga me parte la zona lumbar y las rodillas crujen a cada paso."
		report.thought_color = Color(0.95, 0.65, 0.35)
	elif get_gear_absorbed_water_kg() >= 1.2 and exertion_factor > 0.15:
		report.mood_tag = "💧 Ropa Pesada Empapada"
		report.primary_thought = "La ropa empapada me pesa una tonelada; siento que arrastro plomo pegado al cuerpo con cada zancada."
		report.thought_color = Color(0.65, 0.75, 0.95)

	# Construcción de somatic_tags contextuales
	var tags: Array[String] = []
	match current_posture:
		Posture.STANDING:
			tags.append("🧍 De pie (%s)" % get_posture_formatted_time())
		Posture.SITTING:
			tags.append("🪑 Sentado (%s)" % get_posture_formatted_time())
		Posture.LYING:
			tags.append("🛏️ Acostado (%s)" % get_posture_formatted_time())
		Posture.CROUCHING:
			tags.append("🧎 Agachado (%s)" % get_posture_formatted_time())

	if is_climbing:
		tags.append("🧗 Escalando roca")

	if environment:
		if environment.substrate == EnvironmentSystemClass.SubstrateType.THICK_MUD:
			tags.append("👞 Barro en calzado (+1.8 kg)")
		elif environment.substrate == EnvironmentSystemClass.SubstrateType.LOOSE_SAND:
			tags.append("🏖️ Arena suelta (x2.1 esfuerzo)")
		elif environment.substrate == EnvironmentSystemClass.SubstrateType.WATER_WADING:
			tags.append("🌊 Vadeo profundo (x25 frío)")

		if environment.slope_degrees > 5.0:
			tags.append("⛰️ Subida: +%d°" % int(environment.slope_degrees))
		elif environment.slope_degrees < -5.0:
			tags.append("📉 Bajada: %d° (Impacto en rodillas)" % int(environment.slope_degrees))

		if environment.vegetation == EnvironmentSystemClass.VegetationDensity.THORNY_BRUSH:
			tags.append("🌿 Matorral con espinas")
		elif environment.vegetation == EnvironmentSystemClass.VegetationDensity.SCREE_ROCKS:
			tags.append("🪨 Pedregal inestable")

	if cardio:
		if cardio.heart_rate_bpm > 150.0:
			tags.append("🫀 Corazón: %d BPM (Desbocado)" % int(cardio.heart_rate_bpm))
		elif cardio.heart_rate_bpm > 110.0:
			tags.append("🫀 Corazón: %d BPM (Acelerado)" % int(cardio.heart_rate_bpm))
		elif cardio.heart_rate_bpm > 82.0:
			tags.append("🫀 Corazón: %d BPM (Activo)" % int(cardio.heart_rate_bpm))
		else:
			tags.append("🫀 Corazón: %d BPM (Tranquilo)" % int(cardio.heart_rate_bpm))

	if thermo:
		if thermo.core_temperature > 38.5:
			tags.append("🌡️ Temp: %.1f°C (Fiebre/Calor)" % thermo.core_temperature)
		elif thermo.core_temperature < 35.5:
			tags.append("🌡️ Temp: %.1f°C (Hipotermia)" % thermo.core_temperature)
		else:
			tags.append("🌡️ Temp: %.1f°C (Equilibrada)" % thermo.core_temperature)

	if is_barefoot:
		tags.append("🦶 Pies: Descalzos (Sanos)" if _get_foot_pain() < 5.0 else "🦶 Pies: Descalzos (Dolor %d%%)" % int(_get_foot_pain()))
	else:
		tags.append("👟 Pies: Calzados y protegidos")

	if moisture and moisture.wetness_pct > 25.0:
		var has_clothes: bool = get_total_clothing_clo() > 0.1
		if has_clothes:
			tags.append("💦 Ropa: %d%% humedad" % int(moisture.wetness_pct))
		else:
			tags.append("💦 Piel: %d%% húmeda" % int(moisture.wetness_pct))

	if get_backpack_total_weight_kg() > 0.0:
		tags.append("🎒 Mochila: %.1f kg" % get_backpack_total_weight_kg())

	report.somatic_tags = tags
	return report

func _has_severed_untourniqueted_part() -> bool:
	for p in body_parts.values():
		if p.is_severed and not p.is_tourniquet_applied and p.bleeding_rate_base > 0.0:
			return true
	return false

func _has_severed_part() -> bool:
	for p in body_parts.values():
		if p.is_severed:
			return true
	return false

func _get_active_bleeding_part_count() -> int:
	var c = 0
	var bpm = cardio.heart_rate_bpm if cardio else 75.0
	for p in body_parts.values():
		if p.get_current_bleed_rate(bpm) > 0.0 and not p.is_severed:
			c += 1
	return c

func _get_foot_pain() -> float:
	var p_lf: BodyPart = body_parts.get(BodyPartClass.PartType.LEFT_FOOT)
	var p_rf: BodyPart = body_parts.get(BodyPartClass.PartType.RIGHT_FOOT)
	var v_lf = p_lf.pain_level if p_lf else 0.0
	var v_rf = p_rf.pain_level if p_rf else 0.0
	return max(v_lf, v_rf)
