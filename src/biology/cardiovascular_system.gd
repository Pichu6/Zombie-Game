class_name CardiovascularSystem
extends RefCounted

enum ShockState {
	NORMAL,             # > 4000 mL
	MILD_HYPOVOLEMIA,   # 3500 - 4000 mL
	MODERATE_SHOCK,     # 2800 - 3500 mL
	SEVERE_SHOCK,       # 2000 - 2800 mL
	CARDIAC_ARREST      # < 2000 mL
}

var total_blood_ml: float = 5000.0
var max_blood_ml: float = 5000.0
var heart_rate_bpm: float = 70.0
var resting_bpm: float = 70.0
var blood_pressure_systolic: float = 120.0
var blood_pressure_diastolic: float = 80.0
var current_shock_state: ShockState = ShockState.NORMAL
var is_heart_beating: bool = true

# Deriva Cardiovascular por esfuerzo acumulado (Cardiovascular Drift)
var sustained_exertion_timer: float = 0.0

func update(delta: float, exertion_factor: float, total_pain: float, total_bleed_rate: float, dehydration_pct: float = 0.0, total_weight_kg: float = 75.0, conditioning: float = 40.0, muscle_pct: float = 0.50, fat_pct: float = 0.20) -> void:
	if not is_heart_beating:
		heart_rate_bpm = 0.0
		return

	# 1. Drenaje de volemia por hemorragias activas
	if total_bleed_rate > 0.0:
		total_blood_ml = max(0.0, total_blood_ml - (total_bleed_rate * delta))

	# 2. Estado de Shock Hipovolémico
	if total_blood_ml > 4000.0:
		current_shock_state = ShockState.NORMAL
	elif total_blood_ml > 3500.0:
		current_shock_state = ShockState.MILD_HYPOVOLEMIA
	elif total_blood_ml > 2800.0:
		current_shock_state = ShockState.MODERATE_SHOCK
	elif total_blood_ml > 2000.0:
		current_shock_state = ShockState.SEVERE_SHOCK
	else:
		current_shock_state = ShockState.CARDIAC_ARREST
		is_heart_beating = false
		heart_rate_bpm = 0.0
		return

	# 3. Deriva Cardiovascular (Sustained Exertion Drift progresiva a lo largo de horas)
	if exertion_factor > 0.35:
		sustained_exertion_timer = min(7200.0, sustained_exertion_timer + (delta * exertion_factor))
	else:
		sustained_exertion_timer = max(0.0, sustained_exertion_timer - (delta * 0.4))

	var drift_bpm: float = min(22.0, (sustained_exertion_timer / 120.0) * 0.75)
	var dehydration_bpm_boost: float = (dehydration_pct / 100.0) * 25.0
	
	# 4. Fisiología de Acondicionamiento y Demanda Relativa (%VO2max)
	# Un individuo con alto porcentaje graso y bajo músculo tiene un VO2max muy bajo (~22-26 mL/kg/min)
	# Correr le exige estar al 90-100% de su capacidad aeróbica -> Taquicardia casi máxima (>175-185 BPM)
	var fitness_ratio: float = clamp((muscle_pct / max(0.12, fat_pct + 0.10)) * (conditioning / 40.0), 0.35, 2.8)
	resting_bpm = lerp(82.0, 56.0, clamp((conditioning - 20.0) / 60.0, 0.0, 1.0))
	if fat_pct > 0.35:
		resting_bpm += (fat_pct - 0.35) * 15.0 # Mayor resistencia vascular periférica por adiposidad

	# Sobrecarga de masa corporal contra la gravedad en locomoción activa
	var mass_load_bpm: float = max(0.0, (total_weight_kg - 70.0) * 0.42 * (1.0 + (exertion_factor * 1.5)))
	
	# Carga de esfuerzo escalada por condición física
	var exertion_cardiac_load: float = pow(exertion_factor, 1.25) * (110.0 / pow(fitness_ratio, 0.65))

	# 5. Cálculo del Ritmo Cardíaco Objetivo (Target BPM)
	var blood_loss_pct: float = 1.0 - (total_blood_ml / max_blood_ml)
	var shock_bpm_boost: float = blood_loss_pct * 55.0
	var pain_bpm_boost: float = (total_pain / 100.0) * 28.0

	var target_bpm: float = resting_bpm + mass_load_bpm + exertion_cardiac_load + drift_bpm + dehydration_bpm_boost + shock_bpm_boost + pain_bpm_boost
	target_bpm = clamp(target_bpm, 45.0, 196.0)

	# 6. Transición exponencial adaptativa
	var lerp_rate: float = 1.2 if target_bpm > heart_rate_bpm else 0.25
	var factor: float = 1.0 - exp(-lerp_rate * delta)
	heart_rate_bpm = lerp(heart_rate_bpm, target_bpm, factor)

	# 6. Presión Arterial
	var pressure_factor: float = total_blood_ml / max_blood_ml
	blood_pressure_systolic = 120.0 * pressure_factor
	blood_pressure_diastolic = 80.0 * pressure_factor
