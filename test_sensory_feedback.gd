@tool
extends SceneTree

func _init():
	print("=== INICIANDO PRUEBAS DEL MOTOR DE PERCEPCIÓN SENSORIAL REAL ===")
	var body: BodySystem = BodySystem.new()
	body.reset_to_base_state()

	# 1. Estado Base Confortable
	var rep1 = body.get_sensory_report()
	print("\n1. Confort Base:")
	print("  Mood: ", rep1.mood_tag)
	print("  Pensamiento: ", rep1.primary_thought)
	print("  Tags: ", rep1.somatic_tags)
	assert("Confortable" in rep1.mood_tag, "Debe iniciar en estado confortable")

	# 2. Correr Descalzo en Grava
	body.is_barefoot = true
	body.environment.surface_roughness = 0.8
	body.body_parts[BodyPart.PartType.LEFT_FOOT].pain_level = 25.0
	body.body_parts[BodyPart.PartType.RIGHT_FOOT].pain_level = 25.0
	var rep2 = body.get_sensory_report()
	print("\n2. Pies Descalzos sobre Grava:")
	print("  Mood: ", rep2.mood_tag)
	print("  Pensamiento: ", rep2.primary_thought)
	assert("Plantas Laceradas" in rep2.mood_tag, "Debe sentir las plantas laceradas")

	# 3. Tormenta de Nieve con Ropa Mojada
	body.reset_to_base_state()
	body.thermo.core_temperature = 34.2
	body.moisture.wetness_pct = 80.0
	var rep3 = body.get_sensory_report()
	print("\n3. Hipotermia con Ropa Empapada:")
	print("  Mood: ", rep3.mood_tag)
	print("  Pensamiento: ", rep3.primary_thought)
	assert("Hipotermia" in rep3.mood_tag or "Helada" in rep3.mood_tag, "Debe reportar hipotermia / ropa helada")

	# 4. Sed Extrema
	body.reset_to_base_state()
	body.hydration.current_water_ml = 900.0
	var rep4 = body.get_sensory_report()
	print("\n4. Deshidratación Severa:")
	print("  Mood: ", rep4.mood_tag)
	print("  Pensamiento: ", rep4.primary_thought)
	assert("Sed" in rep4.mood_tag, "Debe reportar sed abrasadora")

	# 5. Taquicardia por Esfuerzo Máximo
	body.reset_to_base_state()
	body.cardio.heart_rate_bpm = 182.0
	var rep5 = body.get_sensory_report()
	print("\n5. Taquicardia:")
	print("  Mood: ", rep5.mood_tag)
	print("  Pensamiento: ", rep5.primary_thought)
	assert("Taquicardia" in rep5.mood_tag, "Debe reportar taquicardia")

	# 6. Amputación con Torniquete
	body.reset_to_base_state()
	body.amputate_part(BodyPart.PartType.LEFT_ARM, true)
	var rep6 = body.get_sensory_report()
	print("\n6. Miembro Amputado:")
	print("  Mood: ", rep6.mood_tag)
	print("  Pensamiento: ", rep6.primary_thought)
	assert("Amputado" in rep6.mood_tag, "Debe reportar muñón amputado / dolor fantasma")

	print("\n>>> ¡TODAS LAS PRUEBAS DE PERCEPCIÓN SENSORIAL REAL PASARON CON ÉXITO! <<<")
	quit(0)
