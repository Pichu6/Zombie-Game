extends SceneTree

func _init() -> void:
	print("=== INICIANDO VALIDACIÓN REALISTA: CAMINATA DE 2 HORAS A 22°C (DESCALZO Y DESNUDO) ===")

	var BodySystemClass = load("res://src/biology/body_system.gd")
	var body = BodySystemClass.new()
	body.reset_to_base_state()

	# Configuración idéntica al escenario del usuario
	body.height_meters = 1.78
	body.muscle_mass_pct = 0.45
	body.body_fat_pct = 0.17
	body.total_weight_kg = 70.2
	body.is_barefoot = true
	body.thermo.clothing_insulation_clo = 0.0 # Desnudo
	body.exertion_factor = 0.20 # Caminar (20% esfuerzo)
	body.environment.ambient_temperature = 22.0
	body.environment.relative_humidity_pct = 50.0
	body.environment.wind_speed_kmh = 5.0
	body.environment.rain_intensity = 0.0
	body.environment.surface_roughness = 0.0 # Superficie plana normal

	print("\nEstado Inicial:")
	print("  • Peso: %.1f kg | Altura: %.2f m" % [body.total_weight_kg, body.height_meters])
	print("  • Hambre inicial: %.1f%% | Hidratación: %.0f mL" % [body.metabolism.hunger_pct, body.hydration.current_water_ml])

	# Simular 2 horas = 7200 segundos
	for s in range(7200):
		body.update(1.0)

	print("\nResultados tras 2 Horas y 0 Minutos de Caminata Continua:")
	var kcal_out = body.metabolism.calories_burned_today
	var hunger = body.metabolism.hunger_pct
	var wetness = body.moisture.wetness_pct
	var hr = body.cardio.heart_rate_bpm
	var temp = body.thermo.core_temperature
	var water_lost = 2500.0 - body.hydration.current_water_ml
	var foot_pain = body._get_foot_pain()
	var report = body.get_sensory_report()

	print("  1. Gasto Calórico (Out): %.1f kcal" % kcal_out)
	print("  2. Nivel de Apetito: %.1f%% (%s)" % [hunger, "Apetito Moderado" if hunger > 25.0 else "Saciado"])
	print("  3. Humedad Piel: %.1f%% (%s)" % [wetness, body.moisture.get_wetness_status()])
	print("  4. Frecuencia Cardíaca: %.1f BPM" % hr)
	print("  5. Temperatura Central: %.2f °C" % temp)
	print("  6. Pérdida de Agua: %.0f mL (Restante: %.0f mL)" % [water_lost, body.hydration.current_water_ml])
	print("  7. Dolor Podal en Pies Descalzos: %.2f%%" % foot_pain)
	print("  8. Estado Sensorial: %s | Pensamiento: %s" % [report.mood_tag, report.primary_thought])
	print("  9. Tags Somáticos: %s" % str(report.somatic_tags))

	# Validaciones
	assert(kcal_out >= 650.0 and kcal_out <= 750.0, "Gasto calórico debe situarse entre 650 y 750 kcal")
	print("  [✓] Gasto calórico realista validado (%.1f kcal)" % kcal_out)

	assert(hunger >= 25.0 and hunger <= 40.0, "El apetito tras 2h de marcha en ayunas debe ser moderado (25-40%), no 4%")
	print("  [✓] Progresión de apetito realista validada (%.1f%% - Apetito Moderado)" % hunger)

	assert(wetness >= 12.0 and wetness <= 35.0, "La humedad de piel desnuda a 22°C debe estabilizarse entre 12% y 35%, no saturar al 100%")
	print("  [✓] Biofísica de evaporación y humedad de piel validada (%.1f%%)" % wetness)

	assert(hr >= 85.0 and hr <= 100.0, "La frecuencia cardíaca en marcha ligera debe rondar 85-100 BPM")
	print("  [✓] Frecuencia cardíaca en caminata validada (%.1f BPM)" % hr)

	assert(temp >= 37.1 and temp <= 37.5, "La temperatura central debe estar termorregulada entre 37.1 y 37.5°C")
	print("  [✓] Termorregulación equilibrada validada (%.2f °C)" % temp)

	assert(foot_pain >= 6.0 and foot_pain <= 15.0, "Caminar 2 horas descalzo debe generar dolor plantar leve (6-15%)")
	print("  [✓] Microtrauma podal por caminata descalza validado (%.2f%% dolor)" % foot_pain)

	var tags_str = " ".join(report.somatic_tags)
	assert("Activo" in tags_str, "El tag cardíaco para 92 BPM debe indicar (Activo)")
	assert("Piel:" in tags_str, "El tag de humedad para sujeto desnudo debe indicar 'Piel:' y no 'Ropa:'")
	print("  [✓] Etiquetas somáticas precisas y contextuales validadas")

	print("\n>>> ¡TODAS LAS CALIBRACIONES REALISTAS DEL ESCENARIO PASARON CON ÉXITO! <<<")
	quit(0)
