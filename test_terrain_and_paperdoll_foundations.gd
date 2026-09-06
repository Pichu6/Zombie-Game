@tool
extends SceneTree

const BodySystemClass = preload("res://src/biology/body_system.gd")
const BodyPartClass = preload("res://src/biology/body_part.gd")
const EnvironmentSystemClass = preload("res://src/biology/environment_system.gd")
const SimulationClockClass = preload("res://src/biology/simulation_clock.gd")
const ClothingDatabaseClass = preload("res://src/biology/clothing_database.gd")

func _init() -> void:
	print("\n=== INICIANDO PRUEBAS DE FASE 2 Y 3: RELOJ, PENDIENTES, SUSTRATOS, ESCALADA Y VEGETACIÓN ===")
	
	test_simulation_clock()
	test_slopes_biomechanics()
	test_substrates_and_mud()
	test_thorny_brush_and_scree()
	test_climbing_and_palm_abrasion()
	
	print("\n=========================================================================")
	print("¡TODAS LAS PRUEBAS DE RELOJ, PENDIENTES, SUSTRATOS Y ESCALADA SUPERADAS!")
	print("=========================================================================\n")
	quit(0)

func test_simulation_clock() -> void:
	print("\n--- PRUEBA 1: Reloj y Calendario de Simulación ---")
	var clock = SimulationClockClass.new()
	
	# Caso 1: Formato base Día 03 — 14h 22m 18s
	# Día 3, 14h, 22m, 18s
	# Día 1 es 0..86400, Día 2 es 86400..172800, Día 3 es 2*86400 + 14*3600 + 22*60 + 18
	clock.set_custom_date(1, 1, 3, 14, 22, 18)
	var str_auto = clock.get_formatted_time(SimulationClockClass.DisplayMode.MODE_AUTO)
	print("  • Tiempo formateado (Día 3): %s" % str_auto)
	assert(str_auto == "Día 03 — 14h 22m 18s", "El formato del Día 3 debe ser exacto sin decimales")
	
	# Caso 2: Transición automática tras mucho tiempo (Año 1, Mes 2, Día 14 — 18h 40m 12s)
	clock.set_custom_date(1, 2, 14, 18, 40, 12)
	var str_extended = clock.get_formatted_time(SimulationClockClass.DisplayMode.MODE_AUTO)
	print("  • Tiempo formateado extendido: %s" % str_extended)
	assert(str_extended == "Año 1, Mes 2, Día 14 — 18h 40m 12s", "El formato extendido debe ser exacto")
	
	# Caso 3: Modo solo días continuos
	var str_days = clock.get_formatted_time(SimulationClockClass.DisplayMode.MODE_DAYS_ONLY)
	print("  • Tiempo solo días: %s" % str_days)
	assert(str_days.begins_with("Día 44 — 18h 40m 12s"), "Modo solo días debe acumular días totales")
	
	print(">>> ÉXITO: Reloj de simulación y calendario astronómico verificados al 100%.")

func test_slopes_biomechanics() -> void:
	print("\n--- PRUEBA 2: Biomecánica de Pendientes (Slopes) ---")
	
	# Caminata en llano (0°) vs caminata en subida empinada (+20°)
	var body_flat = BodySystemClass.new()
	body_flat.is_barefoot = false
	body_flat.environment.slope_degrees = 0.0
	body_flat.exertion_factor = 0.20 # Caminata
	
	for s in range(600): # 10 minutos
		body_flat.update(1.0)
	var burned_flat = body_flat.metabolism.calories_burned_today
	
	var body_uphill = BodySystemClass.new()
	body_uphill.is_barefoot = false
	body_uphill.environment.slope_degrees = 20.0 # Subida +20°
	body_uphill.exertion_factor = 0.20 # Caminata
	
	for s in range(600):
		body_uphill.update(1.0)
	var burned_uphill = body_uphill.metabolism.calories_burned_today
	
	var diff_pct = ((burned_uphill - burned_flat) / burned_flat) * 100.0
	print("  • Calorías en 10 min llano (0°): %.2f kcal" % burned_flat)
	print("  • Calorías en 10 min subida (+20°): %.2f kcal (+%.1f%%)" % [burned_uphill, diff_pct])
	assert(burned_uphill > burned_flat * 1.5, "Subir +20° debe incrementar significativamente el gasto metabólico")
	
	# Impacto excéntrico en rodillas en bajada (-20°)
	var env = EnvironmentSystemClass.new()
	env.slope_degrees = -20.0
	var downhill_impact = env.get_downhill_joint_impact_multiplier()
	print("  • Multiplicador de impacto articular en bajada (-20°): %.2fx" % downhill_impact)
	assert(downhill_impact >= 2.0, "La bajada debe incrementar el impacto articular excéntrico en rodillas")
	
	print(">>> ÉXITO: Física de pendientes validada.")

func test_substrates_and_mud() -> void:
	print("\n--- PRUEBA 3: Sustratos de Terreno y Adhesión de Barro ---")
	var body = BodySystemClass.new()
	var base_mass = body.get_total_moving_mass_kg()
	
	# Barro espeso: añade 1.8 kg de lastre directo a los pies
	body.environment.substrate = EnvironmentSystemClass.SubstrateType.THICK_MUD
	var mud_mass = body.get_total_moving_mass_kg()
	print("  • Masa base: %.2f kg | Masa con barro en calzado: %.2f kg (+%.2f kg)" % [base_mass, mud_mass, mud_mass - base_mass])
	assert(abs((mud_mass - base_mass) - 1.8) < 0.01, "El barro debe adherir 1.8 kg a los pies")
	
	# Arena suelta: costo metabólico x2.1
	body.environment.substrate = EnvironmentSystemClass.SubstrateType.LOOSE_SAND
	var sand_mult = body.environment.get_substrate_metabolic_multiplier()
	print("  • Multiplicador metabólico en arena suelta: %.2fx" % sand_mult)
	assert(sand_mult == 2.1, "La arena suelta debe tener un multiplicador biomecánico de 2.1x")
	
	# Vadeo en agua profunda: pérdida térmica conductiva x25
	body.environment.substrate = EnvironmentSystemClass.SubstrateType.WATER_WADING
	body.environment.ambient_temperature = 10.0 # Agua fría a 10°C
	body.thermo.core_temperature = 37.0
	for s in range(300): # 5 minutos en agua fría
		body.update(1.0)
	print("  • Temp central tras 5 min de vadeo en agua a 10°C: %.2f°C" % body.thermo.core_temperature)
	assert(body.thermo.core_temperature < 36.5, "El vadeo en agua fría debe acelerar la pérdida térmica")
	
	print(">>> ÉXITO: Sustratos, lodo y vadeo acuático validados.")

func test_thorny_brush_and_scree() -> void:
	print("\n--- PRUEBA 4: Matorral Denso con Espinas y Pedregal ---")
	
	# Caso 1: Atravesar arbustos con espinas descalzo y en shorts (sin protección)
	var body_naked = BodySystemClass.new()
	body_naked.is_barefoot = true
	body_naked.environment.vegetation = EnvironmentSystemClass.VegetationDensity.THORNY_BRUSH
	body_naked.exertion_factor = 0.20 # Caminando entre espinas
	
	for s in range(120): # 2 minutos
		body_naked.update(1.0)
		
	var leg_left: BodyPart = body_naked.body_parts[BodyPartClass.PartType.LEFT_LEG]
	print("  • Pierna sin ropa en matorral: Dolor = %.1f%% | Sangrado = %.2f mL/s" % [leg_left.pain_level, leg_left.bleeding_rate_base])
	assert(leg_left.pain_level > 2.0, "Las espinas deben causar dolor en piernas desprotegidas")
	assert(leg_left.bleeding_rate_base > 0.0, "Las espinas deben infligir rasguños con sangrado leve")
	
	# Caso 2: Atravesar con jeans y botas resistentes
	var body_armored = BodySystemClass.new()
	body_armored.equip_item("jeans_denim")
	body_armored.equip_item("combat_boots")
	body_armored.environment.vegetation = EnvironmentSystemClass.VegetationDensity.THORNY_BRUSH
	body_armored.exertion_factor = 0.20
	
	for s in range(120):
		body_armored.update(1.0)
		
	var leg_armored: BodyPart = body_armored.body_parts[BodyPartClass.PartType.LEFT_LEG]
	print("  • Pierna con jeans en matorral: Dolor = %.1f%% | Sangrado = %.2f mL/s" % [leg_armored.pain_level, leg_armored.bleeding_rate_base])
	assert(leg_armored.bleeding_rate_base == 0.0, "Los jeans deben proteger totalmente contra cortes de espinas")
	
	print(">>> ÉXITO: Vegetación hostil y mitigación por prendas validadas.")

func test_climbing_and_palm_abrasion() -> void:
	print("\n--- PRUEBA 5: Acción de Escalar / Trepar y Abrasión de Manos ---")
	
	# Escalar sin guantes con mochila pesada
	var climber = BodySystemClass.new()
	climber.equip_backpack("backpack_assault", 18.0) # Mochila con 18 kg
	climber.is_climbing = true
	climber.exertion_factor = 0.50 # Esfuerzo de escalada
	
	var initial_arm_pain = climber.body_parts[BodyPartClass.PartType.LEFT_ARM].pain_level
	var initial_hand_pain = climber.body_parts[BodyPartClass.PartType.LEFT_HAND].pain_level
	
	for s in range(180): # 3 minutos escalando pared de roca
		climber.update(1.0)
		
	var final_arm_pain = climber.body_parts[BodyPartClass.PartType.LEFT_ARM].pain_level
	var final_hand_pain = climber.body_parts[BodyPartClass.PartType.LEFT_HAND].pain_level
	var report = climber.get_sensory_report()
	
	print("  • Dolor en brazos post-escalada: %.1f%% (Sobrecarga de tren superior)" % final_arm_pain)
	print("  • Dolor en manos sin guantes: %.1f%% (Abrasión de roca)" % final_hand_pain)
	print("  • Pensamiento sensorial: « %s » | Mood: %s" % [report.primary_thought, report.mood_tag])
	
	assert(final_arm_pain > initial_arm_pain + 2.0, "Escalar debe sobrecargar los brazos")
	assert(final_hand_pain > initial_hand_pain + 5.0, "Escalar sin guantes debe causar abrasión palmar")
	assert(report.mood_tag == "🧗 Escalada Vertical", "El estado anímico somático debe ser Escalada Vertical")
	
	print(">>> ÉXITO: Acción de escalada, física de agarre y abrasión palmar validadas.")
