class_name MetabolismSystem
extends RefCounted

signal body_composition_changed(new_fat_pct: float, new_muscle_pct: float)
signal hunger_level_changed(new_hunger_pct: float)

# Constantes Médicas Fisiológicas
const KCAL_PER_KG_FAT: float = 7700.0       # 1 kg de tejido adiposo humano almacena ~7700 kcal
const KCAL_PER_KG_MUSCLE: float = 4000.0    # 1 kg de tejido muscular funcional almacena ~4000 kcal

# Telemetría Calórica y de Balance
var daily_bmr_kcal: float = 1800.0          # Tasa metabólica basal diaria
var active_burn_rate_kcal_per_s: float = 0.0 # Gasto calórico activo instantáneo
var calories_burned_today: float = 0.0      # Kcal quemadas hoy
var calories_consumed_today: float = 0.0    # Kcal ingeridas hoy
var net_caloric_balance_kcal: float = 0.0   # Consumidas - Quemadas

# Digestión y Reservas en Estómago
var stomach_food_kcal: float = 0.0          # Alimento en proceso digestivo
var stomach_protein_g: float = 0.0          # Proteína disponible para síntesis muscular
var hunger_pct: float = 0.0                 # 0% = Saciado | 100% = Inanición extrema

# Acondicionamiento y Adaptación Física
var cardio_conditioning: float = 40.0       # 0.0 a 100.0 (Capacidad aeróbica / VO2max relativo)
var hypertrophy_stimulus: float = 0.0       # Tensión mecánica y sobrecarga acumulada
var recovery_protein_pool: float = 0.0      # Aminoácidos circulantes para reconstrucción

# Temporizador de Día Circadiano (86400 segundos = 24h)
var day_timer_seconds: float = 0.0

func calculate_bmr(weight_kg: float, height_m: float, muscle_pct: float, fat_pct: float) -> float:
	# Fórmula Katch-McArdle de alta precisión basada en masa magra
	var lean_mass_kg: float = max(25.0, weight_kg * (1.0 - fat_pct))
	daily_bmr_kcal = 370.0 + (21.6 * lean_mass_kg)
	return daily_bmr_kcal

func update(delta: float, exertion_factor: float, total_weight_kg: float, height_m: float, current_fat_pct: float, current_muscle_pct: float, glycogen_pct: float) -> Dictionary:
	calculate_bmr(total_weight_kg, height_m, current_muscle_pct, current_fat_pct)

	# 1. Gasto Calórico Basal Instantáneo (BMR por segundo)
	var bmr_per_second: float = daily_bmr_kcal / 86400.0

	# 2. Gasto Calórico Activo por Movimiento (AEE - Active Energy Expenditure)
	# Mover más masa corporal requiere proporcionalmente más energía mecánica (W = m * v * a)
	var weight_scaling: float = total_weight_kg / 70.0
	var aee_per_second: float = 0.0
	
	if exertion_factor > 0.75:
		# Sprint Anaeróbico: ~18 - 28 kcal/minuto para 70kg
		var sprint_norm: float = clamp((exertion_factor - 0.75) / 0.25, 0.0, 1.0)
		aee_per_second = lerp(18.0, 28.0, sprint_norm) / 60.0 * weight_scaling
		if exertion_factor > 1.0:
			aee_per_second *= exertion_factor
	elif exertion_factor > 0.35:
		# Carrera Aeróbica o esfuerzo intenso en subida/terreno pesado: ~8 - 16 kcal/minuto para 70kg
		var run_norm: float = clamp((exertion_factor - 0.35) / 0.40, 0.0, 1.0)
		aee_per_second = lerp(8.0, 16.0, run_norm) / 60.0 * weight_scaling
	elif exertion_factor > 0.02:
		# Caminata: ~3.5 - 6.0 kcal/minuto para 70kg
		var walk_norm: float = clamp((exertion_factor - 0.02) / 0.33, 0.0, 1.0)
		aee_per_second = lerp(3.5, 6.0, walk_norm) / 60.0 * weight_scaling
	
	active_burn_rate_kcal_per_s = aee_per_second
	var total_burn_rate: float = bmr_per_second + aee_per_second
	var burned_kcal_this_frame: float = total_burn_rate * delta
	calories_burned_today += burned_kcal_this_frame

	# 3. Digestión y Absorción Gástrica
	# Tasa de vaciado gástrico realista: ~15 kcal/minuto (~0.25 kcal/s)
	var absorbed_kcal_this_frame: float = 0.0
	if stomach_food_kcal > 0.0:
		var digestion_rate: float = min(stomach_food_kcal, 0.25 * delta)
		stomach_food_kcal -= digestion_rate
		absorbed_kcal_this_frame = digestion_rate
		calories_consumed_today += digestion_rate
		
		# Absorción de aminoácidos (~0.05 g/s)
		if stomach_protein_g > 0.0:
			var protein_rate: float = min(stomach_protein_g, 0.05 * delta)
			stomach_protein_g -= protein_rate
			recovery_protein_pool = min(100.0, recovery_protein_pool + protein_rate)
			
		hunger_pct = max(0.0, hunger_pct - (digestion_rate * 0.02))
	else:
		# Sin alimento en el estómago, el hambre aumenta progresivamente
		# Fisiología de grelina y demanda energética:
		# En reposo (ayuno basal): ~12% por hora (alcanza apetito moderado ~30% a las 2.5 horas)
		# En actividad física activa (caminata/carrera): el gasto calórico acelera la sensación de hambre
		var exertion_hunger_mult: float = 1.0 + (exertion_factor * 1.5)
		var hunger_growth_per_sec: float = 0.0033 * exertion_hunger_mult
		hunger_pct = min(100.0, hunger_pct + (hunger_growth_per_sec * delta))

	net_caloric_balance_kcal = calories_consumed_today - calories_burned_today

	# 4. Dinámica de Grasa Corporal (Lipólisis vs Lipogénesis)
	var new_fat: float = current_fat_pct
	var new_muscle: float = current_muscle_pct

	var net_instantaneous_balance: float = absorbed_kcal_this_frame - burned_kcal_this_frame
	
	if net_instantaneous_balance < 0.0:
		# Déficit Energético: Se oxida tejido graso para suplir la demanda calórica
		var deficit_kcal: float = -net_instantaneous_balance
		var fat_burned_kg: float = (deficit_kcal * 0.85) / KCAL_PER_KG_FAT
		var delta_fat_pct: float = fat_burned_kg / max(30.0, total_weight_kg)
		new_fat = max(0.04, new_fat - delta_fat_pct)
		
		# Catabolismo muscular si el glucógeno está agotado y el déficit es sostenido
		if glycogen_pct <= 5.0 and exertion_factor > 0.35:
			var muscle_catabolized_kg: float = (deficit_kcal * 0.15) / KCAL_PER_KG_MUSCLE
			var delta_muscle_pct: float = muscle_catabolized_kg / max(30.0, total_weight_kg)
			new_muscle = max(0.15, new_muscle - delta_muscle_pct)
	else:
		# Superávit Energético: El exceso se almacena como grasa (lipogénesis)
		var surplus_kcal: float = net_instantaneous_balance
		var fat_stored_kg: float = (surplus_kcal * 0.90) / KCAL_PER_KG_FAT
		var delta_fat_pct: float = fat_stored_kg / max(30.0, total_weight_kg)
		new_fat = min(0.90, new_fat + delta_fat_pct)

	# 5. Adaptación al Entrenamiento y Síntesis Muscular (Hipertrofia vs Atrofia)
	if exertion_factor > 0.40:
		# El trabajo mecánico estimula las vías de síntesis proteica (mTOR)
		hypertrophy_stimulus = min(100.0, hypertrophy_stimulus + (exertion_factor * 0.04 * delta))
		# Aumenta el acondicionamiento cardiovascular aeróbico
		cardio_conditioning = min(100.0, cardio_conditioning + (0.01 * delta))
	elif exertion_factor <= 0.05:
		# En reposo, si hay estímulo previo y aminoácidos disponibles -> Síntesis Muscular
		if hypertrophy_stimulus > 1.0 and recovery_protein_pool > 0.5:
			var rebuild_amount: float = min(hypertrophy_stimulus, 0.04 * delta)
			hypertrophy_stimulus -= rebuild_amount
			recovery_protein_pool = max(0.0, recovery_protein_pool - (rebuild_amount * 0.25))
			
			var muscle_gain_kg: float = (rebuild_amount * 0.006)
			var delta_muscle: float = muscle_gain_kg / max(30.0, total_weight_kg)
			new_muscle = min(0.85, new_muscle + delta_muscle)
		else:
			# Sedentarismo prolongado sin estímulo -> Atrofia lenta
			hypertrophy_stimulus = max(0.0, hypertrophy_stimulus - (0.001 * delta))
			cardio_conditioning = max(15.0, cardio_conditioning - (0.0005 * delta))

	# Ciclo diario de reseteo de estadísticas de 24h
	day_timer_seconds += delta
	if day_timer_seconds >= 86400.0:
		day_timer_seconds = 0.0
		calories_burned_today = 0.0
		calories_consumed_today = 0.0

	return {
		"fat_pct": new_fat,
		"muscle_pct": new_muscle,
		"absorbed_kcal": absorbed_kcal_this_frame
	}

func ingest_food(kcal: float, protein_g: float, carb_g: float, fat_g: float) -> void:
	stomach_food_kcal += max(0.0, kcal)
	stomach_protein_g += max(0.0, protein_g)
	hunger_pct = max(0.0, hunger_pct - (kcal * 0.06))

func reset_state() -> void:
	calories_burned_today = 0.0
	calories_consumed_today = 0.0
	active_burn_rate_kcal_per_s = 0.0
	stomach_food_kcal = 0.0
	stomach_protein_g = 0.0
	hunger_pct = 0.0
	hypertrophy_stimulus = 0.0
	recovery_protein_pool = 0.0
	cardio_conditioning = 40.0
	day_timer_seconds = 0.0
