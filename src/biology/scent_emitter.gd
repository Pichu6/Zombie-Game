class_name ScentEmitter
extends RefCounted

var scent_intensity: float = 0.0 # 0.0 a 100.0 (Detectable por Zombis principalmente por sangre/heridas)

func update(_delta: float, _sweat_rate: float, total_bleeding_rate: float, total_infection: float) -> void:
	# El sangrado fresco e infecciones emiten el rastro biológico detectable por la IA
	var blood_scent: float = clamp(total_bleeding_rate * 6.0, 0.0, 80.0)
	var infection_scent: float = clamp(total_infection * 0.4, 0.0, 20.0)
	
	scent_intensity = clamp(blood_scent + infection_scent, 0.0, 100.0)

func wash_scent() -> void:
	scent_intensity = 0.0
