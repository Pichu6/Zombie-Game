@tool
extends SceneTree

func _init():
	print('=== INICIANDO TEST DE RESETEO INSTANTANEO ===')
	var scn = load('res://scenes/biology_showroom.tscn')
	var inst = scn.instantiate()
	root.add_child(inst)
	
	for f in range(2):
		await process_frame
		
	var char_node = inst.find_child('HumanCharacter', true, false)
	var body: BodySystem = char_node.body
	print('Char node and body loaded: ', body != null)
	
	# 1. Simular desgaste masivo (correr 2 horas a 50 C para degradar todo al maximo)
	print('\n1. Degradando el cuerpo (corriendo en calor extremo)...')
	body.thermo.ambient_temperature = 50.0
	body.exertion_factor = 0.90
	char_node.set_locomotion_effort(0.90)
	for m in range(60):
		for s in range(60):
			body.update(1.0)
			
	print('Estado degradado antes de resetear:')
	print('  Glucogeno: ' + str(round(body.glycogen_pct)) + ' %')
	print('  Dolor Total: ' + str(round(body.total_pain)))
	print('  Dolor Pierna Izq: ' + str(round(body.body_parts[BodyPart.PartType.LEFT_LEG].pain_level)))
	print('  Agua: ' + str(round(body.hydration.current_water_ml)) + ' mL')
	print('  Humedad: ' + str(round(body.moisture.wetness_pct)) + ' %')
	print('  Temp Central: ' + str(round(body.thermo.core_temperature * 10.0) / 10.0) + ' C')
	print('  Estamina: ' + str(round(body.stamina)))
	print('  BPM: ' + str(round(body.cardio.heart_rate_bpm)))
	print('  Estado: ' + body.get_vital_status_text())
	
	assert(body.glycogen_pct < 50.0, 'El glucogeno debio degradarse')
	assert(body.total_pain > 5.0, 'El dolor debio acumularse')
	assert(body.hydration.current_water_ml < 2000.0, 'El agua debio descender')
	
	# 2. Presionar el boton de Resetear
	print('\n2. Presionando boton Resetear Vitalidad...')
	var btn_reset: Button = inst.find_child('BtnResetVitality', true, false)
	assert(btn_reset != null, 'Boton reset no encontrado')
	btn_reset.emit_signal('pressed')
	
	# 3. Comprobar que en ese EXACTO instante TODO volvio al valor base
	print('\n3. Verificando estado post-reset:')
	print('  Glucogeno: ' + str(round(body.glycogen_pct)) + ' % (Esperado 100.0)')
	assert(body.glycogen_pct == 100.0, 'Glucogeno no se reseteo a 100')
	
	print('  Dolor Total: ' + str(round(body.total_pain)) + ' (Esperado 0.0)')
	assert(body.total_pain == 0.0, 'Dolor no se reseteo a 0')
	
	print('  Dolor Piernas: ' + str(round(body.body_parts[BodyPart.PartType.LEFT_LEG].pain_level)) + ' (Esperado 0.0)')
	assert(body.body_parts[BodyPart.PartType.LEFT_LEG].pain_level == 0.0, 'Dolor pierna no se reseteo')
	
	print('  Agua: ' + str(round(body.hydration.current_water_ml)) + ' mL (Esperado 2500.0)')
	assert(body.hydration.current_water_ml == 2500.0, 'Agua no se reseteo')
	
	print('  Humedad: ' + str(round(body.moisture.wetness_pct)) + ' % (Esperado 0.0)')
	assert(body.moisture.wetness_pct == 0.0, 'Humedad no se reseteo')
	
	print('  Temp Central: ' + str(round(body.thermo.core_temperature * 10.0) / 10.0) + ' C (Esperado 37.0)')
	assert(body.thermo.core_temperature == 37.0, 'Temperatura central no se reseteo')
	
	print('  Tasa Sudor: ' + str(round(body.thermo.sweat_rate_ml_per_min)) + ' mL/min (Esperado 0.0)')
	assert(body.thermo.sweat_rate_ml_per_min == 0.0, 'Sudor no se reseteo')
	
	print('  Estamina: ' + str(round(body.stamina)) + ' / ' + str(round(body.max_stamina)) + ' (Esperado 100.0)')
	assert(body.stamina == 100.0 and body.max_stamina == 100.0, 'Estamina no se reseteo')
	
	print('  BPM: ' + str(round(body.cardio.heart_rate_bpm)) + ' (Esperado 70.0)')
	assert(body.cardio.heart_rate_bpm == 70.0, 'BPM no se reseteo')
	
	print('  Esfuerzo: ' + str(round(body.exertion_factor)) + ' (Esperado 0.0)')
	assert(body.exertion_factor == 0.0, 'Esfuerzo no se reseteo')
	
	print('  Estado Vital: ' + body.get_vital_status_text())
	assert(body.get_vital_status_text() == 'VITALIDAD ÓPTIMA', 'Estado no volvio a Vitalidad Optima')
	
	# 4. Comprobar Sliders y Labels de la UI
	var slider_exertion: HSlider = inst.find_child('SliderExertion', true, false)
	assert(slider_exertion.value == 0.0, 'Slider esfuerzo no se reseteo')
	
	var slider_time: HSlider = inst.find_child('SliderTimeScale', true, false)
	assert(slider_time.value == 1.0, 'Slider velocidad no se reseteo')
	
	var slider_temp: HSlider = inst.find_child('SliderAmbientTemp', true, false)
	assert(slider_temp.value == 22.0, 'Slider temperatura no se reseteo')
	
	var bar_glycogen: ProgressBar = inst.find_child('BarGlycogen', true, false)
	assert(bar_glycogen.value == 100.0, 'Barra de glucogeno no se reseteo a 100')
	
	var label_glycogen: Label = inst.find_child('LabelGlycogenVal', true, false)
	print('  UI Glucogeno Label: ' + label_glycogen.text)
	assert('100%' in label_glycogen.text, 'Label glucogeno no dice 100%')
	
	var label_pain: Label = inst.find_child('LabelPainVal', true, false)
	print('  UI Dolor Label: ' + label_pain.text)
	assert('0 / 100' in label_pain.text, 'Label dolor no dice 0 / 100')
	
	var label_status: Label = inst.find_child('LabelStatusVal', true, false)
	print('  UI Estado Vital Label: ' + label_status.text)
	assert('VITALIDAD ÓPTIMA' in label_status.text, 'Label estado no dice VITALIDAD OPTIMA')
	
	inst.free()
	print('\n=== TODAS LAS COMPROBACIONES DE RESETEO INSTANTANEO PASARON AL 100%! ===')
	quit(0)
