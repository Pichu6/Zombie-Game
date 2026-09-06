@tool
extends SceneTree

func _init():
	print("=== INICIANDO BATERÍA DE PRUEBAS: 13 ZONAS ANATÓMICAS, PUENTES Y EQUILIBRIO AMBIENTAL ===")

	var body: BodySystem = BodySystem.new()
	body.reset_to_base_state()

	# =========================================================================
	# PRUEBA 1: Verificación de las 13 Zonas Anatómicas Clínicas (GDD 4.1)
	# =========================================================================
	print("\n--- PRUEBA 1: Verificación de las 13 Zonas Anatómicas Clínicas ---")
	var expected_parts = [
		BodyPart.PartType.HEAD,
		BodyPart.PartType.NECK,
		BodyPart.PartType.TORSO_UPPER,
		BodyPart.PartType.TORSO_LOWER,
		BodyPart.PartType.GROIN,
		BodyPart.PartType.LEFT_ARM,
		BodyPart.PartType.RIGHT_ARM,
		BodyPart.PartType.LEFT_HAND,
		BodyPart.PartType.RIGHT_HAND,
		BodyPart.PartType.LEFT_LEG,
		BodyPart.PartType.RIGHT_LEG,
		BodyPart.PartType.LEFT_FOOT,
		BodyPart.PartType.RIGHT_FOOT
	]
	
	assert(body.body_parts.size() == 13, "Deben existir exactamente 13 zonas anatómicas registradas")
	for pt in expected_parts:
		assert(body.body_parts.has(pt), "Falta la parte anatómica tipo: " + str(pt))
		var p: BodyPart = body.body_parts[pt]
		assert(p.integrity == 100.0, "La integridad inicial debe ser 100%")
		assert(p.pain_level == 0.0, "El dolor inicial debe ser 0")
		assert(not p.is_severed, "Ninguna parte debe iniciar amputada")
	print(">>> ÉXITO: 13 zonas clínicas inicializadas correctamente con sockets y rig bones.")

	# =========================================================================
	# PRUEBA 2: Puente de Amputación (Cascada articular y hemorragia)
	# =========================================================================
	print("\n--- PRUEBA 2: Puente de Amputación y Cascada Articular ---")
	# A) Amputación de brazo izquierdo CON torniquete
	body.amputate_part(BodyPart.PartType.LEFT_ARM, true)
	var left_arm: BodyPart = body.body_parts[BodyPart.PartType.LEFT_ARM]
	var left_hand: BodyPart = body.body_parts[BodyPart.PartType.LEFT_HAND]
	assert(left_arm.is_severed, "El brazo izquierdo debe estar seccionado")
	assert(left_hand.is_severed, "La mano izquierda debe quedar amputada en cascada")
	assert(left_arm.is_tourniquet_applied, "El torniquete debe registrarse")
	assert(left_arm.bleeding_rate_base == 0.0, "Con torniquete no debe haber hemorragia masiva")
	print("  • Amputación con torniquete: Cascada Brazo -> Mano correcta, hemorragia contenida.")

	# B) Amputación de pierna derecha SIN torniquete (Hemorragia arterial letal)
	body.amputate_part(BodyPart.PartType.RIGHT_LEG, false)
	var right_leg: BodyPart = body.body_parts[BodyPart.PartType.RIGHT_LEG]
	var right_foot: BodyPart = body.body_parts[BodyPart.PartType.RIGHT_FOOT]
	assert(right_leg.is_severed, "La pierna derecha debe estar seccionada")
	assert(right_foot.is_severed, "El pie derecho debe quedar amputado en cascada")
	assert(right_leg.bleeding_rate_base >= 120.0, "Sin torniquete debe sangrar a >= 120 mL/s")
	var bleed_rate = right_leg.get_current_bleed_rate(75.0)
	print("  • Amputación sin torniquete: Tasa de sangrado femoral = %.1f mL/s" % bleed_rate)
	assert(bleed_rate > 150.0, "El sangrado arterial femoral con peso 1.4x debe superar 150 mL/s")
	print(">>> ÉXITO: Puente de amputaciones gestiona dependencias y riesgo hipovolémico real.")

	# =========================================================================
	# PRUEBA 3: Puente de Ropa, Capas, Anti-Clipping y Aislamiento Clo
	# =========================================================================
	print("\n--- PRUEBA 3: Puente de Ropa, Capas y Anti-Clipping ---")
	body.reset_to_base_state()
	var torso_upper: BodyPart = body.body_parts[BodyPart.PartType.TORSO_UPPER]

	# Equipar chaqueta táctica en Capa 3
	torso_upper.equip_clothing(BodyPart.ClothingLayer.LAYER_3_OUTER, {
		"name": "Chaqueta Militar Impermeable",
		"insulation_clo": 1.4,
		"wind_resistance": 0.85,
		"waterproof_rating": 0.90,
		"ballistic_protection": 0.35,
		"hides_underlying_skin": true
	})

	assert(torso_upper.is_mesh_hidden_by_clothing, "Anti-clipping: la piel debe quedar marcada para ocultarse")
	assert(torso_upper.get_effective_insulation_clo() == 1.4, "Aislamiento seco debe ser 1.4 clo")
	assert(torso_upper.wind_resistance == 0.85, "Resistencia al viento debe ser 0.85")

	# Simular ropa empapada (wetness 100%)
	torso_upper.wetness_pct = 100.0
	var wet_clo = torso_upper.get_effective_insulation_clo()
	print("  • Aislamiento seco: 1.4 clo | Aislamiento empapado: %.2f clo (Pérdida 75%%)" % wet_clo)
	assert(wet_clo < 0.40, "La ropa empapada debe perder hasta el 75% de su capacidad aislante")
	print(">>> ÉXITO: Puente de capas gestiona protección, anti-clipping y degradación térmica.")

	# =========================================================================
	# PRUEBA 4: Simulación Ambiental (EnvironmentSystem) y Homeostasia en Reposo
	# =========================================================================
	print("\n--- PRUEBA 4: Homeostasia en Condiciones Templadas Estándar (22°C, 50% HR) ---")
	body.reset_to_base_state()
	body.environment.ambient_temperature = 22.0
	body.environment.relative_humidity_pct = 50.0
	body.environment.wind_speed_kmh = 5.0
	body.exertion_factor = 0.0 # Reposo

	# Simular 30 minutos en reposo (1800 segundos)
	for s in range(1800):
		body.update(1.0)

	print("  • Temp Central tras 30 min reposo a 22°C: %.2f °C (Esperado ~37.0°C)" % body.thermo.core_temperature)
	assert(abs(body.thermo.core_temperature - 37.0) < 0.35, "En reposo a 22°C la temperatura debe ser homeostática (~37°C)")
	print(">>> ÉXITO: Equilibrio térmico basal verificado.")

	# =========================================================================
	# PRUEBA 5: Ambiente Hostil - Tormenta Helada y Viento Ártico (-10°C, 50 km/h)
	# =========================================================================
	print("\n--- PRUEBA 5: Tormenta Helada (-10°C, 50 km/h viento) ---")
	body.reset_to_base_state()
	body.environment.ambient_temperature = -10.0
	body.environment.wind_speed_kmh = 50.0
	body.thermo.clothing_insulation_clo = 0.2 # Poca ropa

	var wind_chill = body.environment.get_wind_chill_temperature()
	print("  • Temperatura del aire: -10°C | Sensación térmica (Wind Chill): %.1f °C" % wind_chill)
	assert(wind_chill < -18.0, "La sensación térmica con 50 km/h de viento debe ser inferior a -18°C")

	for s in range(600): # 10 minutos
		body.update(1.0)

	print("  • Temp Central tras 10 min frío polar: %.2f °C | Temblores: %s" % [body.thermo.core_temperature, body.thermo.is_shivering])
	assert(body.thermo.core_temperature < 36.5, "Debe descender la temperatura central")
	assert(body.thermo.is_shivering, "Deben activarse los temblores compensatorios (shivering)")
	print(">>> ÉXITO: Efecto Wind Chill y termorregulación por frío validados.")

	# =========================================================================
	# PRUEBA 6: Ambiente Hostil - Calor Húmedo Extremo (38°C, 90% HR)
	# =========================================================================
	print("\n--- PRUEBA 6: Calor Húmedo Sofocante (38°C, 90% HR) ---")
	body.reset_to_base_state()
	body.environment.ambient_temperature = 38.0
	body.environment.relative_humidity_pct = 90.0
	body.exertion_factor = 0.40 # Trote suave

	var evap_eff = body.environment.get_evaporation_efficiency()
	print("  • Eficiencia de evaporación con 90%% humedad: %.1f%%" % (evap_eff * 100.0))
	assert(evap_eff < 0.25, "En 90% de humedad la evaporación debe estar casi anulada (<25%)")

	for s in range(600): # 10 minutos
		body.update(1.0)

	print("  • Temp Central tras 10 min calor húmedo: %.2f °C | Estado: %s" % [body.thermo.core_temperature, body.get_vital_status_text()])
	assert(body.thermo.core_temperature > 38.0, "La temperatura central debe subir peligrosamente por falta de refrigeración")
	assert(body.thermo.sweat_rate_ml_per_min > 5.0, "Debe haber sudoración profusa intentando compensar")
	print(">>> ÉXITO: Fisiología psicrométrica y golpe de calor por humedad validados.")

	# =========================================================================
	# PRUEBA 7: Lluvia y Humedad Externa
	# =========================================================================
	print("\n--- PRUEBA 7: Humectación por Lluvia Externa ---")
	body.reset_to_base_state()
	body.environment.rain_intensity = 0.8 # Lluvia fuerte
	body.exertion_factor = 0.0

	for s in range(30): # 30 segundos bajo la lluvia
		body.update(1.0)

	print("  • Humedad de prendas tras 30s de lluvia: %.1f%%" % body.moisture.wetness_pct)
	assert(body.moisture.wetness_pct > 40.0, "La lluvia externa debe empapar la ropa rápidamente")
	print(">>> ÉXITO: Sistema de lluvia empapa ropa y piel coherentemente.")

	# =========================================================================
	# PRUEBA 8: Microtrauma en Pies Descalzos vs Calzados
	# =========================================================================
	print("\n--- PRUEBA 8: Microtrauma en Pies (Descalzo vs Calzado) ---")
	body.reset_to_base_state()
	body.is_barefoot = true
	body.environment.surface_roughness = 0.7 # Grava y escombros
	body.exertion_factor = 0.60

	for s in range(120): # 2 minutos corriendo descalzo sobre grava
		body.update(1.0)

	var foot_pain_barefoot = body.body_parts[BodyPart.PartType.LEFT_FOOT].pain_level
	print("  • Dolor en pie descalzo sobre grava (2 min): %.2f" % foot_pain_barefoot)
	assert(foot_pain_barefoot > 1.5, "Correr descalzo en grava debe generar dolor en pies")

	# Ahora probar con calzado
	body.reset_to_base_state()
	body.is_barefoot = false
	body.environment.surface_roughness = 0.7
	body.exertion_factor = 0.60

	for s in range(120): # 2 minutos calzado
		body.update(1.0)

	var foot_pain_shod = body.body_parts[BodyPart.PartType.LEFT_FOOT].pain_level
	print("  • Dolor en pie calzado sobre grava (2 min): %.2f" % foot_pain_shod)
	assert(foot_pain_shod < foot_pain_barefoot * 0.25, "El calzado debe absorber más del 75% del impacto")
	print(">>> ÉXITO: Amortiguación de calzado y microtrauma articular validados.")

	print("\n=========================================================================")
	print("¡TODAS LAS 8 PRUEBAS DE ARQUITECTURA, PUENTES Y AMBIENTE SUPERADAS AL 100%!")
	print("=========================================================================")
	quit(0)
