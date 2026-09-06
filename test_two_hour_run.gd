@tool
extends SceneTree

func _init():
	print('=== TEST DE 2 HORAS DE CARRERA CONTINUA (55% ESFUERZO A 22°C) ===')
	var body = BodySystem.new()
	body.height_meters = 1.78
	body.muscle_mass_pct = 0.50
	body.body_fat_pct = 0.20
	body.recalculate_weight()
	body.reset_to_base_state()
	body.thermo.ambient_temperature = 22.0
	body.exertion_factor = 0.55 # Carrera de la captura del usuario
	
	print('Peso: ' + str(round(body.total_weight_kg * 10.0) / 10.0) + ' kg | Temp: 22°C | Esfuerzo: 55%')
	
	for m in range(120):
		for s in range(60):
			body.update(1.0)
		var min_num = m + 1
		if min_num % 15 == 0 or min_num == 1:
			var leg_p = body.body_parts[BodyPart.PartType.LEFT_LEG].pain_level
			print('Minuto ' + str(min_num) + ': Estado=[' + body.get_vital_status_text() + '] | Glucogeno=' + str(round(body.glycogen_pct)) + '% | Dolor Piernas=' + str(round(leg_p * 10.0) / 10.0) + ' | BPM=' + str(round(body.cardio.heart_rate_bpm)) + ' | Temp=' + str(round(body.thermo.core_temperature * 10.0) / 10.0) + '°C | Agua=' + str(round(body.hydration.current_water_ml)) + ' mL')
	
	assert(body.is_alive, 'El corredor entrenado sobrevive a las 2 horas de carrera')
	assert(body.glycogen_pct < 5.0, 'El glucogeno debe haberse agotado casi por completo tras 2 horas')
	assert(body.total_pain > 10.0, 'Las piernas deben registrar microtrauma por impacto')
	assert(body.cardio.heart_rate_bpm > 140.0, 'Debe haber deriva cardiovascular real')
	print('\n¡TEST DE 2 HORAS SUPERADO CON ÉXITO!')
	quit(0)
