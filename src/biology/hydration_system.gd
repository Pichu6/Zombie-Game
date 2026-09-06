class_name HydrationSystem
extends RefCounted

var current_water_ml: float = 2500.0 # Depósito de agua funcional (2.5L)
var max_water_ml: float = 2500.0
var dehydration_level: float = 0.0 # 0.0 (Óptimo) a 100.0 (Muerte renal)

func update(delta: float, sweat_rate_ml_per_min: float) -> void:
	# 1. Pérdida basal por respiración y metabolismo (~1.5L / 24h = ~0.018 mL/s = ~65 mL/hora)
	var baseline_drain: float = 0.02 # mL/s en reposo
	var sweat_drain: float = sweat_rate_ml_per_min / 60.0 # mL/s exactos proporcionales 1:1

	var total_water_loss: float = (baseline_drain + sweat_drain) * delta
	current_water_ml = max(0.0, current_water_ml - total_water_loss)

	# 2. Porcentaje de deshidratación
	dehydration_level = (1.0 - (current_water_ml / max_water_ml)) * 100.0

func drink_water(amount_ml: float) -> void:
	current_water_ml = min(max_water_ml, current_water_ml + amount_ml)
	dehydration_level = (1.0 - (current_water_ml / max_water_ml)) * 100.0
