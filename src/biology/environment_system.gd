class_name EnvironmentSystem
extends RefCounted

# =============================================================================
# MOTOR DE SIMULACIÓN AMBIENTAL, METEOROLÓGICA Y DE TERRENO
# Modela factores termodinámicos, higrométricos, pendientes, sustratos y vegetación
# =============================================================================

enum SubstrateType {
	FIRM_GROUND,  # Asfalto / Piso firme (fricción 1.0, costo metabólico base)
	DIRT_GRASS,   # Tierra compacta / Césped
	LOOSE_SAND,   # Arena suelta / Dunas (costo metabólico x2.1)
	THICK_MUD,    # Barro espeso / Ciénaga (se adhiere al calzado agregando +1.8 kg)
	WATER_WADING, # Vadeo en agua profunda (resistencia hidrodinámica + pérdida térmica x25)
	DEEP_SNOW     # Nieve profunda (marcha de paso alto forzado x2.6)
}

enum VegetationDensity {
	CLEAR,        # Sendero limpio y despejado
	TALL_GRASS,   # Pastizal alto (requiere elevación de rodillas)
	THORNY_BRUSH, # Matorral denso con espinas (rasguños en piel sin protección)
	SCREE_ROCKS   # Pedregal inestable / Rocas sueltas (riesgo de torcedura/esguince)
}

# Parámetros Meteorológicos Fundamentales (Rangos Terrestres Realistas)
var ambient_temperature: float = 22.0 # °C (-40°C Ártico a +50°C Desierto)
var relative_humidity_pct: float = 50.0 # % (5% hiperárido a 100% saturado)
var wind_speed_kmh: float = 5.0 # km/h (0 en calma a 120 km/h huracán)
var rain_intensity: float = 0.0 # 0.0 (despejado) a 1.0 (torrencial)
var solar_radiation_w_m2: float = 250.0 # W/m² (0 noche/interior, 250 nublado, 850 pleno sol)

# Propiedades del Terreno, Inclinación y Obstáculos
var slope_degrees: float = 0.0 # Inclinación: -30.0° (bajada empinada) a +30.0° (subida empinada)
var substrate: SubstrateType = SubstrateType.FIRM_GROUND
var vegetation: VegetationDensity = VegetationDensity.CLEAR

# Rugosidad superficial (retrocompatibilidad)
var surface_roughness: float = 0.0

# -----------------------------------------------------------------------------
# CÁLCULOS FÍSICOS, BIOMECÁNICOS Y TERMODINÁMICOS
# -----------------------------------------------------------------------------

func get_apparent_temperature() -> float:
	# Temperatura aparente combinada (Sensación térmica real)
	if ambient_temperature <= 10.0 and wind_speed_kmh >= 4.8:
		return get_wind_chill_temperature()
	elif ambient_temperature >= 27.0:
		return get_heat_index()
	else:
		var wind_cooling = (wind_speed_kmh / 20.0) * 1.2
		return ambient_temperature - wind_cooling

func get_wind_chill_temperature() -> float:
	# Twc = 13.12 + 0.6215*T - 11.37*(V^0.16) + 0.3965*T*(V^0.16)
	var v_clamped = max(4.8, wind_speed_kmh)
	var v_pow = pow(v_clamped, 0.16)
	var twc = 13.12 + (0.6215 * ambient_temperature) - (11.37 * v_pow) + (0.3965 * ambient_temperature * v_pow)
	return twc

func get_heat_index() -> float:
	var t_f = (ambient_temperature * 1.8) + 32.0
	var r = relative_humidity_pct
	if t_f < 80.0:
		return ambient_temperature
	var hi_f = -42.379 + (2.04901523 * t_f) + (10.14333127 * r) \
		- (0.22475541 * t_f * r) - (0.00683783 * t_f * t_f) \
		- (0.05481717 * r * r) + (0.00122874 * t_f * t_f * r) \
		+ (0.00085282 * t_f * r * r) - (0.00000199 * t_f * t_f * r * r)
	return max(ambient_temperature, (hi_f - 32.0) / 1.8)

func get_evaporation_efficiency() -> float:
	var sat_factor = pow(clamp(relative_humidity_pct / 100.0, 0.0, 1.0), 1.4)
	return clamp(1.0 - sat_factor, 0.05, 1.0)

func get_convective_heat_transfer_coeff(char_speed_mps: float = 0.0) -> float:
	var wind_mps = wind_speed_kmh / 3.6
	var effective_air_velocity = wind_mps + char_speed_mps
	var hc = 10.45 - effective_air_velocity + (10.0 * sqrt(max(0.1, effective_air_velocity)))
	return clamp(hc, 8.5, 45.0)

func get_rain_wetting_rate_ml_per_sec() -> float:
	return rain_intensity * 4.0

func get_surface_impact_multiplier() -> float:
	return 1.0 + (surface_roughness * 3.0)

# -----------------------------------------------------------------------------
# BIOMECÁNICA DE PENDIENTES (SLOPES)
# -----------------------------------------------------------------------------

func get_slope_metabolic_multiplier() -> float:
	# En terreno plano (0°): 1.0
	# En subida (> 0°): Trabajo gravitatorio mecánico contra g (W = m·g·Δh).
	# Para +10° (~17% gradiente): ~1.76x
	# Para +20° (~36% gradiente): ~2.50x
	# Para +30° (~57% gradiente): ~3.20x
	# En bajada (< 0°): Reducción moderada del costo aeróbico (hasta 0.75x), pero con costo articular
	var rad = deg_to_rad(slope_degrees)
	if slope_degrees >= 0.0:
		return 1.0 + (4.4 * sin(rad))
	else:
		return clamp(1.0 + (1.2 * sin(rad)), 0.70, 1.0)

func get_downhill_joint_impact_multiplier() -> float:
	# En bajada pronunciada, la frenada excéntrica incrementa dramáticamente
	# la fuerza de impacto reactivo en rodillas (rótula) y tobillos
	if slope_degrees < 0.0:
		var rad = deg_to_rad(abs(slope_degrees))
		return 1.0 + (3.0 * sin(rad))
	return 1.0

# -----------------------------------------------------------------------------
# BIOMECÁNICA DE SUSTRATOS Y VEGETACIÓN
# -----------------------------------------------------------------------------

func get_substrate_metabolic_multiplier() -> float:
	match substrate:
		SubstrateType.FIRM_GROUND:
			return 1.0
		SubstrateType.DIRT_GRASS:
			return 1.15
		SubstrateType.LOOSE_SAND:
			return 2.1 # Biomecánica humana clásica: x2.1 en arena seca disipativa
		SubstrateType.THICK_MUD:
			return 1.8 # Arrastre de barro viscoso
		SubstrateType.WATER_WADING:
			return 2.4 # Resistencia hidráulica de avance
		SubstrateType.DEEP_SNOW:
			return 2.6 # Paso alto y hundimiento
	return 1.0

func get_mud_adhesion_weight_kg() -> float:
	# El barro espeso se adhiere progresivamente al calzado/pies sumando lastre inercial
	if substrate == SubstrateType.THICK_MUD:
		return 1.8 # 1.8 kg de barro húmedo distribuido en pies
	return 0.0

func get_thermal_conduction_multiplier() -> float:
	# El agua líquida conduce el calor 24-25 veces más rápido que el aire quieto
	if substrate == SubstrateType.WATER_WADING:
		return 25.0
	return 1.0

func get_vegetation_scratch_risk() -> float:
	# Matorral denso con espinas araña y corta extremidades descubiertas
	if vegetation == VegetationDensity.THORNY_BRUSH:
		return 1.0
	return 0.0

func get_scree_sprain_risk() -> float:
	# Pedregal inestable con rocas sueltas desvía tobillos
	if vegetation == VegetationDensity.SCREE_ROCKS:
		return 1.0
	return 0.0

func reset_to_standard() -> void:
	ambient_temperature = 22.0
	relative_humidity_pct = 50.0
	wind_speed_kmh = 5.0
	rain_intensity = 0.0
	solar_radiation_w_m2 = 250.0
	surface_roughness = 0.0
	slope_degrees = 0.0
	substrate = SubstrateType.FIRM_GROUND
	vegetation = VegetationDensity.CLEAR
