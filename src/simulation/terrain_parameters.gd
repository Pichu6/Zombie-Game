class_name TerrainParameters
extends Resource

# =============================================================================
# PARÁMETROS DE TERRENO — Motor de Simulación Traceable
# Cada tipo de terreno define propiedades físicas y enlaces a efectos visuales/audio.
# Patrón: cada parámetro de simulación tiene un ID de efecto visual/audio
# al que se le puede conectar un shader, partícula, animación, sonido, etc.
# =============================================================================

## Nombre para mostrar en UI
@export var display_name: String = "Terreno Base"

## Descripción corta del terreno
@export var description: String = ""

## Ícono emoji para UI
@export var icon: String = "🏗️"

# --- PARÁMETROS FÍSICOS DE LOCOMOCIÓN ---

## Multiplicador de costo energético de locomoción (1.0 = normal)
@export_range(0.5, 5.0, 0.1) var locomotion_cost_multiplier: float = 1.0

## Factor de tracción/agarre (1.0 = firme, 0.1 = muy resbaloso)
@export_range(0.1, 1.5, 0.05) var traction_factor: float = 1.0

## Velocidad máxima permitida como fracción (1.0 = sin límite, 0.5 = mitad)
@export_range(0.1, 1.0, 0.05) var max_speed_factor: float = 1.0

# --- PARÁMETROS DE RIESGO ---

## Riesgo de lesión por paso (probabilidad 0.0 - 1.0 por tick de simulación)
@export_range(0.0, 0.1, 0.001) var injury_risk_per_step: float = 0.0

## Tipo de lesión que puede causar (para descalzo)
@export_enum("Ninguna", "Laceración", "Punzante", "Contusión") var injury_type: String = "Ninguna"

## Profundidad de penetración que afecta calzado (0 = superficie, 1.0 = profundo)
@export_range(0.0, 1.0, 0.05) var surface_penetration: float = 0.0

# --- PARÁMETROS AMBIENTALES ---

## Tasa de acumulación de barro/sustancia en calzado (kg/min)
@export_range(0.0, 2.0, 0.1) var mud_accumulation_rate: float = 0.0

## Nivel de ruido al caminar (0 = silencioso, 1.0 = muy ruidoso) — detección por zombies
@export_range(0.0, 1.0, 0.05) var noise_level: float = 0.3

## Modificador de temperatura del suelo respecto a ambiente (°C)
@export_range(-10.0, 30.0, 0.5) var ground_temp_offset: float = 0.0

## Tasa de absorción de humedad en calzado/ropa
@export_range(0.0, 1.0, 0.05) var moisture_absorption_rate: float = 0.0

# --- ENLACES TRACEABLES A EFECTOS VISUALES/AUDIO ---
# Estos StringName son IDs que el motor visual usará para aplicar
# shaders, partículas, decals, sonidos, etc. al escenario.

## ID del efecto visual de superficie (shader de ground, decals de huellas, etc.)
@export var visual_effect_id: StringName = &""

## ID del efecto de partículas al caminar (polvo, salpicaduras, nieve, etc.)
@export var particle_effect_id: StringName = &""

## ID del sonido ambiente del terreno
@export var ambient_sound_id: StringName = &""

## ID del sonido de pasos sobre este terreno
@export var footstep_sound_id: StringName = &""

## ID del shader de post-proceso ambiental (niebla, calor, etc.)
@export var postprocess_effect_id: StringName = &""

# --- PRESETS ESTÁTICOS ---

static func create_asphalt() -> TerrainParameters:
	var t = TerrainParameters.new()
	t.display_name = "Asfalto / Firme"
	t.description = "Superficie dura y estable. Locomoción óptima."
	t.icon = "🛣️"
	t.locomotion_cost_multiplier = 1.0
	t.traction_factor = 1.0
	t.max_speed_factor = 1.0
	t.injury_risk_per_step = 0.0
	t.noise_level = 0.3
	t.footstep_sound_id = &"footstep_asphalt"
	return t

static func create_dirt_grass() -> TerrainParameters:
	var t = TerrainParameters.new()
	t.display_name = "Tierra / Césped"
	t.description = "Terreno natural suave. Ligeramente más silencioso."
	t.icon = "🌿"
	t.locomotion_cost_multiplier = 1.1
	t.traction_factor = 0.9
	t.max_speed_factor = 0.95
	t.injury_risk_per_step = 0.001
	t.injury_type = "Punzante"
	t.noise_level = 0.2
	t.moisture_absorption_rate = 0.1
	t.footstep_sound_id = &"footstep_grass"
	t.particle_effect_id = &"particle_dust_light"
	return t

static func create_loose_sand() -> TerrainParameters:
	var t = TerrainParameters.new()
	t.display_name = "Arena Suelta"
	t.description = "Superficie inestable. Alto costo energético (x2.1)."
	t.icon = "🏖️"
	t.locomotion_cost_multiplier = 2.1
	t.traction_factor = 0.5
	t.max_speed_factor = 0.6
	t.surface_penetration = 0.4
	t.noise_level = 0.15
	t.footstep_sound_id = &"footstep_sand"
	t.particle_effect_id = &"particle_sand_kick"
	return t

static func create_thick_mud() -> TerrainParameters:
	var t = TerrainParameters.new()
	t.display_name = "Barro Espeso"
	t.description = "Acumula peso en calzado (+1.8 kg). Muy ruidoso."
	t.icon = "💩"
	t.locomotion_cost_multiplier = 1.8
	t.traction_factor = 0.4
	t.max_speed_factor = 0.5
	t.surface_penetration = 0.6
	t.mud_accumulation_rate = 1.8
	t.noise_level = 0.6
	t.moisture_absorption_rate = 0.5
	t.footstep_sound_id = &"footstep_mud"
	t.particle_effect_id = &"particle_mud_splash"
	t.visual_effect_id = &"shader_wet_ground"
	return t

static func create_water_wading() -> TerrainParameters:
	var t = TerrainParameters.new()
	t.display_name = "Vadeo en Agua"
	t.description = "Inmersión parcial. Pérdida térmica x25. Empapa ropa."
	t.icon = "🌊"
	t.locomotion_cost_multiplier = 2.5
	t.traction_factor = 0.6
	t.max_speed_factor = 0.4
	t.surface_penetration = 0.8
	t.noise_level = 0.7
	t.ground_temp_offset = -5.0
	t.moisture_absorption_rate = 1.0
	t.footstep_sound_id = &"footstep_water"
	t.particle_effect_id = &"particle_water_splash"
	t.visual_effect_id = &"shader_water_surface"
	t.ambient_sound_id = &"ambient_flowing_water"
	return t

static func create_deep_snow() -> TerrainParameters:
	var t = TerrainParameters.new()
	t.display_name = "Nieve Profunda"
	t.description = "Alto costo energético. Pérdida de calor moderada."
	t.icon = "❄️"
	t.locomotion_cost_multiplier = 2.3
	t.traction_factor = 0.35
	t.max_speed_factor = 0.45
	t.surface_penetration = 0.7
	t.noise_level = 0.1
	t.ground_temp_offset = -10.0
	t.moisture_absorption_rate = 0.3
	t.footstep_sound_id = &"footstep_snow"
	t.particle_effect_id = &"particle_snow_puff"
	t.visual_effect_id = &"shader_snow_ground"
	t.postprocess_effect_id = &"postprocess_frost"
	return t
