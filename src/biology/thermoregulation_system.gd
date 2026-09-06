class_name ThermoregulationSystem
extends RefCounted

const EnvironmentSystemClass = preload("res://src/biology/environment_system.gd")

var core_temperature: float = 37.0 # °C (Temperatura Central Fisiológica)
var skin_temperature: float = 34.0 # °C (Temperatura de la Piel)
var sweat_rate_ml_per_min: float = 0.0 # mL/minuto de sudoración
var is_shivering: bool = false # Temblores involuntarios por frío
var is_heat_exhausted: bool = false # Agotamiento térmico

# Aislamiento térmico total de la ropa
var clothing_insulation_clo: float = 0.5 # 0.0 = Desnudo | 0.5 = Ropa ligera | 1.0 = Traje de calle | 2.5 = Abrigo polar
var clothing_wetness_pct: float = 0.0 # Humedad de prendas (0% a 100%)

# Capacidad calorífica corporal
var body_thermal_mass: float = 75.0 * 3.47 # kJ/°C

# Referencia ambiental
var environment: EnvironmentSystemClass

# Propiedad retrocompatible para ambient_temperature
var ambient_temperature: float:
	get:
		return environment.ambient_temperature if environment else 22.0
	set(val):
		if environment:
			environment.ambient_temperature = val

func _init(p_env: EnvironmentSystemClass = null) -> void:
	if p_env:
		environment = p_env
	else:
		environment = EnvironmentSystemClass.new()

func update(
	delta: float,
	exertion_factor: float,
	body_fat_pct: float,
	muscle_mass_pct: float,
	total_weight_kg: float,
	height_meters: float,
	current_water_ml: float = 2500.0,
	total_clo: float = -1.0,
	wetness: float = -1.0
) -> void:
	if not environment:
		environment = EnvironmentSystemClass.new()

	if total_clo >= 0.0:
		clothing_insulation_clo = total_clo
	if wetness >= 0.0:
		clothing_wetness_pct = wetness

	# 1. Área de Superficie Corporal (DuBois BSA en m²)
	var bsa: float = 0.007184 * pow(total_weight_kg, 0.425) * pow(height_meters * 100.0, 0.725)
	body_thermal_mass = total_weight_kg * 3.47 # ~3.47 kJ/(kg·°C)

	# 2. Convección Ambiental y Resistencia Térmica
	var runner_speed_mps: float = exertion_factor * 5.0 # 0 m/s reposo a ~5 m/s sprint
	var hc: float = environment.get_convective_heat_transfer_coeff(runner_speed_mps)
	var r_air: float = 1.0 / hc # Resistencia de la capa límite de aire (m²·K/W)

	var r_fat: float = 0.04 + (0.28 * body_fat_pct)

	# Penalización drástica por ropa mojada (agua conduce calor 25x más rápido que el aire)
	var wet_penalty: float = 1.0 - (clamp(clothing_wetness_pct / 100.0, 0.0, 1.0) * 0.75)
	var r_clothing: float = 0.155 * clothing_insulation_clo * wet_penalty
	var r_total: float = max(0.04, r_fat + r_clothing + r_air)

	# 3. Temperatura de la Piel y Gradiente Térmico
	var apparent_temp: float = environment.get_apparent_temperature()
	skin_temperature = lerp(34.0, clamp(apparent_temp, 14.0, 40.0), 0.28 / (1.0 + body_fat_pct))

	# 4. Generación de Calor Metabólico Interno
	var weight_work_mult: float = total_weight_kg / 70.0
	var muscle_mult: float = 0.80 + (muscle_mass_pct * 0.40)
	var metabolic_heat_w: float = 78.0 + (pow(exertion_factor, 1.35) * 850.0 * muscle_mult * weight_work_mult)

	# Termogénesis por temblores (Shivering)
	if core_temperature < 36.5:
		is_shivering = true
		metabolic_heat_w += 150.0
	else:
		is_shivering = false

	# 5. Carga Radiativa Solar Directa
	var solar_exposure_factor: float = max(0.1, 1.0 - min(0.85, clothing_insulation_clo * 0.4))
	var q_solar_w: float = environment.solar_radiation_w_m2 * bsa * 0.20 * solar_exposure_factor

	# 6. Intercambio Térmico con el Entorno (Ley de Fourier / Enfriamiento Convectivo-Radiativo)
	var q_env_w: float = (bsa / r_total) * (apparent_temp - skin_temperature)

	# 7. Termorregulación por Sudoración y Eficiencia Evaporativa
	var evap_efficiency: float = environment.get_evaporation_efficiency()
	
	if current_water_ml > 50.0 and (core_temperature > 37.10 or (apparent_temp > 27.5 and core_temperature > 36.85)):
		var temp_excess: float = max(0.0, core_temperature - 37.10)
		var ambient_excess: float = max(0.0, apparent_temp - 27.5)
		
		var raw_sweat: float = (temp_excess * 24.0) + (ambient_excess * 0.85) + (exertion_factor * 3.5)
		var weight_sweat_factor: float = clamp(total_weight_kg / 70.0, 0.85, 2.0)
		sweat_rate_ml_per_min = clamp(raw_sweat * weight_sweat_factor, 0.0, 42.0)
	else:
		sweat_rate_ml_per_min = 0.0

	# Enfriamiento evaporativo: 1 mL evaporado = ~2.43 kJ = 2430 Watts.
	# Si la humedad ambiente es 100%, evap_efficiency cae hacia 0.05, el sudor no evapora
	# y el calor corporal NO se disipa (provocando hipertermia aguda).
	var evaporative_cooling_w: float = (sweat_rate_ml_per_min / 60.0) * 2430.0 * evap_efficiency

	# Deshidratación severa detiene el sudor
	if current_water_ml < 250.0:
		var water_factor: float = current_water_ml / 250.0
		sweat_rate_ml_per_min *= water_factor
		evaporative_cooling_w *= water_factor

	# 8. Balance Térmico Neto y Variación de Temperatura Central
	var balance_watts: float = metabolic_heat_w + q_solar_w + q_env_w - evaporative_cooling_w
	var heat_rate_deg_per_sec: float = balance_watts / (body_thermal_mass * 1000.0)
	
	core_temperature += heat_rate_deg_per_sec * delta
	core_temperature = clamp(core_temperature, 26.0, 44.0)

	# 9. Estados Clínicos
	is_heat_exhausted = (core_temperature > 38.8)

func get_thermal_status_text() -> String:
	if core_temperature < 30.0:
		return "Hipotermia Fatal / Paro Inminente (<30°C)"
	elif core_temperature < 32.0:
		return "Hipotermia Severa Crítica"
	elif core_temperature < 35.0:
		return "Hipotermia Moderada (Temblores)"
	elif core_temperature < 36.2:
		return "Pérdida de Calor Acelerada"
	elif core_temperature <= 37.6:
		if sweat_rate_ml_per_min > 4.0:
			return "Compensando por Sudoración"
		return "En Equilibrio Térmico"
	elif core_temperature <= 38.5:
		return "Hipertermia Leve por Ejercicio"
	elif core_temperature <= 39.5:
		return "Agotamiento Térmico Severo"
	else:
		return "Golpe de Calor Crítico (>40°C)"
