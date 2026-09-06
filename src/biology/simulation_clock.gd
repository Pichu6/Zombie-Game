class_name SimulationClock
extends RefCounted

# =============================================================================
# RELOJ Y CALENDARIO DE SIMULACIÓN
# Gestiona el paso del tiempo en el mundo del juego con rigor astronómico/calendárico
# Formatos legibles sin decimales molestos:
#   • Día 03 — 14h 22m 18s
#   • Año 1, Mes 2, Día 14 — 18h 40m 12s
# =============================================================================

enum DisplayMode {
	MODE_AUTO = 0,         # Muestra "Día DD — HHh MMm SSs" y pasa a Año/Mes tras 30 días
	MODE_DAYS_ONLY = 1,    # "Día DDD — HHh MMm SSs" continuo
	MODE_FULL_CALENDAR = 2,# "Año A, Mes M, Día D — HHh MMm SSs"
	MODE_TOTAL_HMS = 3     # "HHHh MMm SSs" acumulado total
}

const SECONDS_PER_MINUTE: int = 60
const SECONDS_PER_HOUR: int = 3600
const SECONDS_PER_DAY: int = 86400
const DAYS_PER_MONTH: int = 30
const MONTHS_PER_YEAR: int = 12
const DAYS_PER_YEAR: int = 360

# Tiempo acumulado en segundos reales de simulación
# Por defecto inicializa en el Día 1 a las 08:00:00 (mañana despejada)
var accumulated_seconds: float = 8.0 * 3600.0 # 08h 00m 00s del Día 1
var simulation_speed: float = 1.0
var is_paused: bool = false
var current_mode: DisplayMode = DisplayMode.MODE_AUTO

# Tiempo transcurrido real desde inicio/reset de la simulación (empieza en 0)
var elapsed_seconds: float = 0.0

func advance(delta: float) -> void:
	if is_paused:
		return
	var dt = max(0.0, delta * simulation_speed)
	accumulated_seconds += dt
	elapsed_seconds += dt

func set_elapsed_seconds(seconds: float) -> void:
	accumulated_seconds = max(0.0, seconds)

func set_custom_date(year: int, month: int, day: int, hour: int, minute: int, second: int) -> void:
	var total_days: int = ((year - 1) * DAYS_PER_YEAR) + ((month - 1) * DAYS_PER_MONTH) + (day - 1)
	accumulated_seconds = (total_days * SECONDS_PER_DAY) + (hour * SECONDS_PER_HOUR) + (minute * SECONDS_PER_MINUTE) + second

func get_breakdown() -> Dictionary:
	var total_sec: int = int(accumulated_seconds)
	var sec: int = total_sec % SECONDS_PER_MINUTE
	var total_min: int = total_sec / SECONDS_PER_MINUTE
	var minute: int = total_min % 60
	var total_hours: int = total_min / 60
	var hour: int = total_hours % 24
	var total_days: int = total_hours / 24 # Días enteros transcurridos (base 0)
	
	var day_of_month: int = (total_days % DAYS_PER_MONTH) + 1
	var total_months: int = total_days / DAYS_PER_MONTH
	var month: int = (total_months % MONTHS_PER_YEAR) + 1
	var year: int = (total_months / MONTHS_PER_YEAR) + 1
	var display_day_number: int = total_days + 1

	return {
		"total_seconds": total_sec,
		"seconds": sec,
		"minutes": minute,
		"hours": hour,
		"total_hours": total_hours,
		"day_of_month": day_of_month,
		"display_day": display_day_number,
		"month": month,
		"year": year
	}

func get_formatted_time(mode: DisplayMode = current_mode) -> String:
	var b = get_breakdown()
	
	match mode:
		DisplayMode.MODE_DAYS_ONLY:
			return "Día %02d — %02dh %02dm %02ds" % [b.display_day, b.hours, b.minutes, b.seconds]
			
		DisplayMode.MODE_FULL_CALENDAR:
			return "Año %d, Mes %d, Día %02d — %02dh %02dm %02ds" % [b.year, b.month, b.day_of_month, b.hours, b.minutes, b.seconds]
			
		DisplayMode.MODE_TOTAL_HMS:
			return "%dh %02dm %02ds" % [b.total_hours, b.minutes, b.seconds]
			
		DisplayMode.MODE_AUTO:
			# Si han pasado menos de 30 días, muestra formato simple Día XX
			if b.display_day <= 30 and b.year == 1 and b.month == 1:
				return "Día %02d — %02dh %02dm %02ds" % [b.display_day, b.hours, b.minutes, b.seconds]
			else:
				return "Año %d, Mes %d, Día %02d — %02dh %02dm %02ds" % [b.year, b.month, b.day_of_month, b.hours, b.minutes, b.seconds]
				
	return "Día %02d — %02dh %02dm %02ds" % [b.display_day, b.hours, b.minutes, b.seconds]

func get_elapsed_formatted() -> String:
	var total_sec: int = int(elapsed_seconds)
	var sec: int = total_sec % 60
	var total_min: int = total_sec / 60
	var min: int = total_min % 60
	var hours: int = total_min / 60
	return "%02dh %02dm %02ds" % [hours, min, sec]

func reset_to_initial() -> void:
	elapsed_seconds = 0.0
	accumulated_seconds = 8.0 * 3600.0 # Día 1, 08:00:00
	simulation_speed = 1.0
	is_paused = false
