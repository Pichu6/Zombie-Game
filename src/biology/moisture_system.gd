class_name MoistureSystem
extends RefCounted

var accumulated_sweat_ml: float = 0.0 # mL de agua/sudor acumulados en piel y ropa
var max_sweat_capacity_ml: float = 120.0 # Capacidad de saturación de tela o piel (mL)
var wetness_pct: float = 0.0 # 0.0 a 100.0%
var is_naked: bool = true

func update(
	delta: float,
	sweat_rate_ml_per_min: float,
	ambient_temp: float,
	exertion_factor: float,
	external_wetting_ml_per_sec: float = 0.0,
	wind_speed_kmh: float = 5.0,
	humidity_pct: float = 50.0,
	clothing_clo: float = 0.0
) -> void:
	is_naked = (clothing_clo < 0.1)

	# 1. Capacidad de retención hídrica:
	# Piel desnuda retiene una película superficial delgada (~28 mL para ~1.8 m2 BSA) antes de escurrir/gotear.
	# La ropa textil absorbe agua en sus fibras (80 a 220 mL según capas).
	if is_naked:
		max_sweat_capacity_ml = 28.0
	else:
		max_sweat_capacity_ml = lerp(80.0, 220.0, clamp(clothing_clo / 1.5, 0.0, 1.0))

	# 2. Producción de sudor (mL/min -> mL/s)
	# Parte del sudor se pierde por goteo si se sobrepasa la película superficial
	var sweat_produced = (sweat_rate_ml_per_min / 60.0) * delta
	accumulated_sweat_ml += sweat_produced

	# 3. Humectación externa por lluvia
	if external_wetting_ml_per_sec > 0.0:
		accumulated_sweat_ml += external_wetting_ml_per_sec * delta

	# 4. Tasa Máxima de Evaporación Convectiva y Psicrométrica (E_max en mL/s)
	var base_evap: float = 0.32 if is_naked else 0.22 # mL/s en aire quieto
	var ventilation_factor: float = 1.0 if is_naked else (1.0 / (1.0 + clothing_clo * 0.6))
	var airflow = (1.0 + (wind_speed_kmh / 12.0) + (exertion_factor * 1.4)) * ventilation_factor
	var temp_factor = clamp(ambient_temp / 20.0, 0.25, 2.2)
	var dryness_efficiency = clamp(1.0 - (humidity_pct / 100.0), 0.05, 1.0)

	var max_evaporation_rate = base_evap * airflow * temp_factor * dryness_efficiency # mL/s

	# 5. Dinámica de Evaporación Física (Ecuación de transferencia de masa de Dalton / Gagge):
	# La evaporación real es proporcional a la fracción de humectación superficial (w = M / M_max).
	# Si la superficie está seca (M=0), la evaporación es 0. Al humedecerse, la tasa se aproxima a E_max.
	if accumulated_sweat_ml > 0.0:
		var current_wettedness = accumulated_sweat_ml / max_sweat_capacity_ml
		var actual_evap_rate = max_evaporation_rate * clamp(current_wettedness, 0.10, 1.0)
		accumulated_sweat_ml = max(0.0, accumulated_sweat_ml - (actual_evap_rate * delta))

	# El exceso de sudor más allá de la capacidad de retención escurre (gotas al suelo)
	accumulated_sweat_ml = clamp(accumulated_sweat_ml, 0.0, max_sweat_capacity_ml)
	wetness_pct = (accumulated_sweat_ml / max_sweat_capacity_ml) * 100.0

func set_wetness_pct(pct: float) -> void:
	wetness_pct = clamp(pct, 0.0, 100.0)
	accumulated_sweat_ml = max_sweat_capacity_ml * (wetness_pct / 100.0)

func get_wetness_status() -> String:
	if is_naked:
		if wetness_pct < 6.0:
			return "Piel Seca (0%)"
		elif wetness_pct < 35.0:
			return "Piel Húmeda (%d%%)" % int(wetness_pct)
		elif wetness_pct < 70.0:
			return "Piel Muy Sudorosa (%d%%)" % int(wetness_pct)
		else:
			return "Piel Empapada (%d%%)" % int(wetness_pct)
	else:
		if wetness_pct < 6.0:
			return "Ropa Seca (0%)"
		elif wetness_pct < 35.0:
			return "Ropa Ligeramente Húmeda (%d%%)" % int(wetness_pct)
		elif wetness_pct < 70.0:
			return "Ropa Húmeda (%d%%)" % int(wetness_pct)
		else:
			return "Ropa Empapada (%d%%)" % int(wetness_pct)
