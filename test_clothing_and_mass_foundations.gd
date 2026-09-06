extends SceneTree

func _init() -> void:
	print("=== INICIANDO PRUEBAS DE BASES: PRENDAS MULTI-ZONA, MOCHILAS Y MASA DINÁMICA ===")

	var BodySystemClass = load("res://src/biology/body_system.gd")
	var BodyPartClass = load("res://src/biology/body_part.gd")
	var body = BodySystemClass.new()
	body.reset_to_base_state()
	body.total_weight_kg = 70.0

	# -------------------------------------------------------------------------
	# PRUEBA 1: Equipamiento Multi-Zona (Campera cubre 4 partes)
	# -------------------------------------------------------------------------
	print("\n--- PRUEBA 1: Equipamiento Multi-Zona ---")
	var equipped = body.equip_item("tactical_jacket")
	assert(equipped, "Debe equipar la campera militar con éxito")
	assert(body.equipped_items.has("tactical_jacket"), "Debe registrarse en equipped_items")

	# Verificar que cubra Torso Sup, Torso Inf, Brazo Izq, Brazo Der
	for p_type in [BodyPartClass.PartType.TORSO_UPPER, BodyPartClass.PartType.TORSO_LOWER, BodyPartClass.PartType.LEFT_ARM, BodyPartClass.PartType.RIGHT_ARM]:
		var part: BodyPart = body.body_parts[p_type]
		assert(part.clothing_layers.has(BodyPartClass.ClothingLayer.LAYER_3_OUTER), "Cada parte debe tener la capa 3 asignada")
		assert(part.clothing_layers[BodyPartClass.ClothingLayer.LAYER_3_OUTER]["id"] == "tactical_jacket", "El ID debe ser tactical_jacket")
		assert(part.base_insulation_clo >= 0.85, "Aislamiento base de la parte debe reflejar la campera")

	print("  • Partes cubiertas por la campera: Torso Sup, Torso Inf, Brazo Izq, Brazo Der.")
	print(">>> ÉXITO: Prenda multi-zona aplicada correctamente a todas sus partes dependientes.")

	# -------------------------------------------------------------------------
	# PRUEBA 2: Control de Masa Seca (No duplicar peso)
	# -------------------------------------------------------------------------
	print("\n--- PRUEBA 2: Control de Masa Seca ---")
	var dry_weight = body.get_gear_dry_weight_kg()
	print("  • Peso seco calculado de la campera: %.2f kg (Esperado: 1.40 kg)" % dry_weight)
	assert(abs(dry_weight - 1.40) < 0.01, "El peso de la campera (1.4 kg) debe sumarse UNA SOLA VEZ, sin duplicar por zona")
	print(">>> ÉXITO: Masa seca unificada sin duplicaciones por zonas múltiples.")

	# -------------------------------------------------------------------------
	# PRUEBA 3: Calzado y Alternancia de Estado Descalzo
	# -------------------------------------------------------------------------
	print("\n--- PRUEBA 3: Calzado y Estado Descalzo ---")
	assert(body.is_barefoot, "Inicialmente debe estar descalzo")
	body.equip_item("combat_boots")
	assert(not body.is_barefoot, "Al equipar botas de combate debe marcar is_barefoot = false")
	assert(body.get_gear_dry_weight_kg() == (1.40 + 1.65), "Peso total seco debe ser campera (1.4) + botas (1.65) = 3.05 kg")

	# Desequipar botas
	body.unequip_item("combat_boots")
	assert(body.is_barefoot, "Al desequipar botas debe volver a quedar descalzo (is_barefoot = true)")
	assert(abs(body.get_gear_dry_weight_kg() - 1.40) < 0.01, "El peso debe volver a ser solo el de la campera")
	print(">>> ÉXITO: Calzado detectado, mitigación podal y masa gestionados.")

	# -------------------------------------------------------------------------
	# PRUEBA 4: Absorción de Agua y Masa Líquida en Prendas Mojadas
	# -------------------------------------------------------------------------
	print("\n--- PRUEBA 4: Física de Agua Absorbida en Prendas ---")
	# Equipar jeans (dry: 0.75 kg, max water: 1.10 kg) + campera (dry: 1.40 kg, max water: 0.70 kg)
	body.equip_item("jeans_denim")
	var dry_total = body.get_gear_dry_weight_kg() # 1.40 + 0.75 = 2.15 kg
	print("  • Ropa en seco: %.2f kg" % dry_total)

	# Simular 50% humedad en prendas
	body.moisture.set_wetness_pct(50.0)
	for part in body.body_parts.values():
		part.wetness_pct = 50.0

	var water_weight = body.get_gear_absorbed_water_kg()
	# Esperado: 0.70 * 0.50 + 1.10 * 0.50 = 0.35 + 0.55 = 0.90 kg
	print("  • Agua absorbida a 50%% humedad: %.2f kg (Esperado ~0.90 kg)" % water_weight)
	assert(abs(water_weight - 0.90) < 0.05, "El agua absorbida calculada debe rondar 0.90 kg")

	var total_gear_mass = dry_total + water_weight
	print("  • Peso total de prendas mojadas: %.2f kg" % total_gear_mass)
	assert(abs(total_gear_mass - 3.05) < 0.05, "Masa total de ropa debe ser seca + agua")
	print(">>> ÉXITO: Masa de agua absorbida en tejidos validada físicamente.")

	# -------------------------------------------------------------------------
	# PRUEBA 5: Mochilas y Carga Dinámica Adaptativa
	# -------------------------------------------------------------------------
	print("\n--- PRUEBA 5: Mochila y Carga Dinámica ---")
	# Equipar mochila de asalto (dry: 1.65 kg, cap max: 25.0 kg)
	body.equip_backpack("backpack_assault", 18.0)
	var bp_weight = body.get_backpack_total_weight_kg()
	print("  • Peso total de mochila (vacía 1.65 kg + 18 kg carga): %.2f kg" % bp_weight)
	assert(abs(bp_weight - 19.65) < 0.01, "Mochila debe pesar base + carga")

	# Intentar sobrepasar capacidad
	body.set_backpack_content_weight(35.0) # Máx es 25
	assert(body.backpack_content_weight_kg == 25.0, "La carga debe clampearse a la capacidad máxima de la mochila")

	var moving_mass = body.get_total_moving_mass_kg()
	# 70.0 (cuerpo) + 2.15 (ropa seca) + ~0.90 (agua ropa) + (1.65 + 25.0 = 26.65 mochila) + ~0.40 (agua mochila)
	print("  • Masa total en movimiento calculada: %.2f kg" % moving_mass)
	assert(moving_mass > 98.0 and moving_mass < 101.0, "Masa total debe rondar 99-100 kg")
	print(">>> ÉXITO: Sistema de mochilas con límites de capacidad y masa total verificado.")

	# -------------------------------------------------------------------------
	# PRUEBA 6: Demanda Fisiológica y Fatiga por Sobrecarga de Carga
	# -------------------------------------------------------------------------
	print("\n--- PRUEBA 6: Impacto Fisiológico de la Carga en Marcha ---")
	body.reset_to_base_state()
	body.total_weight_kg = 70.0
	body.exertion_factor = 0.20 # Caminata
	body.environment.ambient_temperature = 22.0

	# 1. Caminata sin carga durante 10 minutos (600s)
	for s in range(600):
		body.update(1.0)
	var kcal_unladen = body.metabolism.calories_burned_today
	print("  • Kcal quemadas en 10 min marcha sin carga: %.2f kcal" % kcal_unladen)

	# 2. Caminata con mochila pesada de 25 kg durante 10 minutos (600s)
	body.reset_to_base_state()
	body.total_weight_kg = 70.0
	body.exertion_factor = 0.20
	body.environment.ambient_temperature = 22.0
	body.equip_backpack("backpack_assault", 22.0) # Mochila pesada

	for s in range(600):
		body.update(1.0)
	var kcal_laden = body.metabolism.calories_burned_today
	var report = body.get_sensory_report()

	print("  • Kcal quemadas en 10 min marcha con 22 kg de mochila: %.2f kcal" % kcal_laden)
	print("  • Incremento calórico por carga: +%.1f%%" % (((kcal_laden / kcal_unladen) - 1.0) * 100.0))
	assert(kcal_laden > kcal_unladen * 1.25, "Llevar mochila pesada debe incrementar el gasto calórico más de un 25%")

	print("  • Pensamiento del personaje bajo carga: %s | Mood: %s" % [report.primary_thought, report.mood_tag])
	assert(report.mood_tag == "🎒 Sobrecarga en Espalda", "Debe disparar pensamiento somático de sobrecarga en espalda")
	print(">>> ÉXITO: Repercusión calórica y somática de la carga verificada.")

	print("\n=========================================================================")
	print("¡TODAS LAS PRUEBAS DE PRENDAS, MOCHILAS Y MASA DINÁMICA SUPERADAS AL 100%!")
	print("=========================================================================")
	quit(0)
