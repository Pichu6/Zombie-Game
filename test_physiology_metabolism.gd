@tool
extends SceneTree

func _init():
	print('=== INICIANDO PRUEBAS DE FISIOLOGIA REAL Y METABOLISMO DINAMICO ===')
	var scn = load('res://scenes/biology_showroom.tscn')
	var inst = scn.instantiate()
	root.add_child(inst)
	for f in range(5): await process_frame
	
	var char_node = inst.find_child('HumanCharacter', true, false)
	var dash = inst.find_child('UI', true, false)
	var body = char_node.body
	
	# =========================================================================
	# PRUEBA 1: Persona Obesa No Entrenada (118 kg, 85% grasa, 35% músculo)
	# =========================================================================
	print('\n--- PRUEBA 1: Carrera de Persona Obesa No Entrenada (118 kg) ---')
	char_node.set_character_archetype('fat')
	body.recalculate_weight()
	print('Peso inicial: %.1f kg | Músculo: %.0f%% | Grasa: %.0f%%' % [body.total_weight_kg, body.muscle_mass_pct * 100, body.body_fat_pct * 100])
	assert(body.total_weight_kg > 110.0, 'Debe pesar más de 110 kg')
	
	dash._apply_effort(55.0, 'Running')
	
	var elapsed: float = 0.0
	var hit_170_bpm: bool = false
	var stamina_exhausted_time: float = -1.0
	
	for s in range(300): # 5 minutos de carrera
		body.update(1.0)
		elapsed += 1.0
		
		if body.cardio.heart_rate_bpm >= 170.0 and not hit_170_bpm:
			hit_170_bpm = true
			print('   [Minuto %.1f] Frecuencia Cardíaca alcanzó zona roja: %.1f BPM' % [elapsed / 60.0, body.cardio.heart_rate_bpm])
			
		if body.stamina <= 0.0 and stamina_exhausted_time < 0.0:
			stamina_exhausted_time = elapsed
			print('   [Minuto %.2f (%.0fs)] ESTAMINA AGOTADA POR COMPLETO! Estado: %s' % [elapsed / 60.0, elapsed, body.get_vital_status_text()])
			
	print('\nResultados Obeso (5 min):')
	print('BPM final: %.1f | Estamina final: %.1f | Dolor en piernas: %.1f | Estado: %s' % [
		body.cardio.heart_rate_bpm, body.stamina, body.total_pain, body.get_vital_status_text()
	])
	assert(hit_170_bpm, 'Una persona obesa debe alcanzar >170 BPM al correr')
	assert(stamina_exhausted_time > 0 and stamina_exhausted_time <= 300, 'La estamina debe agotarse en menos de 5 minutos')
	assert(body.total_pain > 5.0, 'Correr descalzo con 118 kg debe generar dolor significativo en piernas')
	print('>>> EXITO: Persona obesa ya NO corre como robot; experimenta sobreesfuerzo agudo real.')
	
	# =========================================================================
	# PRUEBA 2: Persona Entrenada y Normopeso (75 kg, 45% músculo, 18% grasa)
	# =========================================================================
	print('\n--- PRUEBA 2: Carrera de Persona Normal / En Forma (75 kg) ---')
	body.reset_to_base_state()
	char_node.set_character_archetype('normal')
	body.metabolism.cardio_conditioning = 60.0
	print('Peso normal: %.1f kg | Músculo: %.0f%% | Grasa: %.0f%%' % [body.total_weight_kg, body.muscle_mass_pct * 100, body.body_fat_pct * 100])
	
	for s in range(240): # 4 minutos de carrera a 55%
		body.update(1.0)
		
	print('Resultados Normal (4 min):')
	print('BPM: %.1f | Estamina: %.1f | Temp: %.2f°C | Estado: %s' % [
		body.cardio.heart_rate_bpm, body.stamina, body.thermo.core_temperature, body.get_vital_status_text()
	])
	assert(body.cardio.heart_rate_bpm < 160.0, 'El corazón de una persona en forma debe estar en zona aeróbica (<160 BPM)')
	assert(body.stamina > 50.0, 'La persona en forma debe tener más del 50% de estamina a los 4 minutos')
	print('>>> EXITO: Persona normal sostiene la carrera aeróbica eficientemente.')
	
	# =========================================================================
	# PRUEBA 3: Motor Metabólico - Déficit Calórico y Pérdida de Grasa
	# =========================================================================
	print('\n--- PRUEBA 3: Motor Metabólico - Déficit Calórico y Pérdida de Grasa ---')
	body.reset_to_base_state()
	char_node.set_character_archetype('fat')
	var initial_fat = body.body_fat_pct
	var initial_weight = body.total_weight_kg
	print('Grasa antes de déficit: %.3f (%.1f kg)' % [initial_fat, initial_weight])
	
	body.exertion_factor = 0.25
	for s in range(108): # 108 pasos de 100 segundos = 10800 segundos (3 horas)
		body.update(100.0)
		
	print('Grasa tras 3h caminata: %.3f (%.1f kg) | Kcal Quemadas: %.0f' % [
		body.body_fat_pct, body.total_weight_kg, body.metabolism.calories_burned_today
	])
	assert(body.body_fat_pct < initial_fat, 'La grasa corporal debe haber disminuido por el déficit')
	assert(body.total_weight_kg < initial_weight, 'El peso total debe haber disminuido')
	print('>>> EXITO: Déficit calórico oxida grasa y reduce el peso corporal dinámicamente.')
	
	# =========================================================================
	# PRUEBA 4: Alimentación y Superávit (Engordar con Comida Chatarra)
	# =========================================================================
	print('\n--- PRUEBA 4: Alimentación y Superávit (Comida Chatarra) ---')
	var fat_before_junk = body.body_fat_pct
	dash.btn_eat_junk.emit_signal('pressed')
	dash.btn_eat_junk.emit_signal('pressed')
	dash.btn_eat_junk.emit_signal('pressed')
	assert(body.metabolism.stomach_food_kcal >= 4000.0, 'Debe haber >4000 kcal en estómago')
	
	body.exertion_factor = 0.0
	for s in range(60): # 60 pasos de 360 segundos = 21600 segundos (6 horas)
		body.update(360.0)
		
	print('Grasa tras superávit chatarra: %.3f (%.1f kg) | Kcal Consumidas: %.0f' % [
		body.body_fat_pct, body.total_weight_kg, body.metabolism.calories_consumed_today
	])
	assert(body.body_fat_pct > fat_before_junk, 'La grasa debe aumentar tras consumir superávit de comida chatarra')
	print('>>> EXITO: Comer mal genera lipogénesis y aumenta el peso.')
	
	# =========================================================================
	# PRUEBA 5: Entrenamiento y Ganancia Muscular (Hipertrofia)
	# =========================================================================
	print('\n--- PRUEBA 5: Entrenamiento + Nutrición Proteica -> Hipertrofia ---')
	body.reset_to_base_state()
	char_node.set_character_archetype('skinny')
	var initial_muscle = body.muscle_mass_pct
	print('Músculo inicial: %.4f' % initial_muscle)
	
	# Ejercicio de fuerza
	body.exertion_factor = 0.85
	for s in range(30): body.update(60.0) # 30 minutos de entrenamiento intenso
	print('   Estímulo hipertrofia acumulado: %.2f' % body.metabolism.hypertrophy_stimulus)
	
	# Alimentación sana rica en proteínas (2 comidas de 650 kcal con 35g proteína)
	dash.btn_eat_healthy.emit_signal('pressed')
	dash.btn_eat_healthy.emit_signal('pressed')
	
	# Descanso / recuperación para síntesis muscular
	body.exertion_factor = 0.0
	for s in range(60): body.update(120.0) # 120 minutos de reposo anabólico
	
	print('Músculo tras entrenamiento + nutrición: %.4f (Ganancia: +%.4f)' % [
		body.muscle_mass_pct, body.muscle_mass_pct - initial_muscle
	])
	assert(body.muscle_mass_pct > initial_muscle, 'El porcentaje de músculo debe aumentar con entrenamiento y proteína')
	print('>>> EXITO: Entrenamiento y nutrición estimulan la síntesis muscular.')
	
	print('\n=== TODAS LAS PRUEBAS FISIOLOGICAS Y METABOLICAS PASARON EXITOSAMENTE! ===')
	inst.free()
	quit()
