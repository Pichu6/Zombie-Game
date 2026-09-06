# Auditoría inicial — 2026-09-06

Alcance: lectura de documentación y scripts principales; dos pruebas existentes ejecutadas con Godot 4.7.2 en modo headless. No se hizo evaluación visual de la UI ni auditoría completa de todos los assets. El código del juego no fue modificado en esta revisión.

## Mapa comprobado

| Área | Código | Estado observado |
| --- | --- | --- |
| Cuerpo | `src/biology/body_system.gd`, `body_part.gd` | 17 zonas; heridas, tratamientos, postura y equipo |
| Fisiología | `src/biology/*_system.gd` | Subsistemas de metabolismo, circulación, hidratación, humedad y temperatura |
| Ambiente | `src/biology/environment_system.gd` | Parámetros y modificadores de clima/sustrato; no demuestra un mundo espacial persistente |
| Personaje | `src/entities/human_character.gd`, `character_master_loader.gd` | Modelo base MakeHuman, morphs y remapeo de animaciones Mixamo |
| Laboratorio | `src/ui/biology_dashboard.gd` | 3.479 líneas al revisar; controles, telemetría, inspección y acciones |
| Paperdoll | `src/ui/silhouette_paperdoll.gd` | Representación e interacción corporal |
| Reloj | `src/biology/simulation_clock.gd` | Instanciado por dashboard; el personaje avanza BodySystem por separado |
| Cámara | `src/ui/orbit_camera.gd` | Cámara orbital de inspección; no equivale al sistema de órdenes RTS |

La visión de construcción universal, autonomía de colonos y redes de servicios no tiene implementaciones equivalentes identificadas en los scripts revisados. No confundir las clases de ejemplo del GDD con archivos existentes.

## Hallazgos para reproducir y resolver

- `BodySystem.get_total_clothing_clo()` divide por 13 aunque se inicializan 17 zonas. Definir agregación por cobertura/superficie antes de sustituir mecánicamente el divisor.
- `BodySystem.update()` pasa masa corporal más equipo a `MetabolismSystem.update()`, que utiliza esa masa también para estimar BMR. Separar carga mecánica y masa corporal.
- `recalculate_weight()` convierte controles de grasa/músculo mediante fórmulas de morph, mientras otros subsistemas los interpretan como fracciones de masa. El arquetipo fat utiliza 0,85 grasa y 0,35 músculo. Definir magnitudes físicas y controles visuales independientes.
- `is_female` aparece declarado sin usos adicionales encontrados en biología. Esto no prueba ausencia de assets femeninos en disco.
- La simulación y el reloj avanzan desde nodos distintos. Evaluar controlador temporal común y pruebas de equivalencia entre pasos y velocidades.
- Dashboard extenso con creación corporal, inspección clínica, ropa, ambiente y animaciones. Propuesta: estado prioritario → selección → causas → acciones → detalle desplegable.

## Pruebas ejecutadas

- `test_two_hour_walk_realism.gd`: completó las aserciones. Resultado: 713,8 kcal, 97,4 BPM, 37,29 °C, pérdida de 723 mL tras 7.200 pasos de 1 segundo.
- `test_physiology_metabolism.gd`: completó las aserciones. La supuesta carrera normal terminó a 64,9 BPM y 100 % de estamina. Las cotas unilaterales permiten aprobar ese resultado; verificar esfuerzo real y aislamiento entre escenarios antes de usarlo como evidencia de calibración.
- El primer intento de la segunda prueba se limitó a 3 frames y no llegó a ejecutar los escenarios; se repitió con límite de 600 y alcanzó el mensaje final de todos los casos.
- Godot informó un error de lectura del almacén de certificados. La prueba que instancia el showroom también reportó recursos sin liberar al cierre. No se investigaron todavía.

## Referencias investigadas

- The Sims: comportamientos, precondiciones y necesidades anunciadas por objetos. [Forbus y Wright](https://users.cs.northwestern.edu/~forbus/c95-gd/2001/Programming%20Objects%20in%20The%20Sims.pdf).
- Project Zomboid: propiedades de fluidos y crafting extensible. [Build 42](https://projectzomboid.com/blog/upcoming-features-b42/).
- Bannerlord: IA individual, de formación y táctica. [TaleWorlds](https://www.taleworlds.com/en/Games/Bannerlord/Blog/83).
- Haven & Hearth: eliminación de calidad triple por complejidad sin suficiente valor de juego. [World 10](https://www.havenandhearth.com/forum/viewtopic.php?f=39&t=52822).

Las aplicaciones a este proyecto son propuestas propias, no afirmaciones sobre código interno de esos juegos.

## Preparación de Git — 2026-09-06

Autor y remoto configurados. Se corrigió el helper de credenciales solo en la configuración local del repositorio: se vacía la lista heredada y se utiliza `manager`. La prueba de lectura no interactiva del remoto sigue sin autenticación disponible; no se subieron archivos.

Git LFS instalado localmente. `.gitattributes` registra por ruta los 35 archivos existentes de al menos 10 MiB, incluido el ZIP de 639 MiB. Los assets nuevos no reciben seguimiento LFS automáticamente: revisar su tamaño antes de agregarlos. Cachés y `.backups` siguen excluidos.

El respaldo local y su manifiesto están presentes; en esta continuación no se repitió la verificación de hashes. Sigue pendiente un respaldo externo y comprobar recuperación desde una copia limpia con los objetos LFS.

## Actualización de conservación y Git — 2026-09-06

La autenticación quedó resuelta y el commit inicial `9dbf972` se publicó y verificó en `origin/main`. Se preparó una separación posterior de fuentes activas, assets LFS y paquetes locales; ver [versionado y respaldo](versionado-y-respaldo.md) para alcance y pruebas. Esta actualización reemplaza el estado pendiente de autoría/autenticación de los apartados históricos anteriores. El respaldo externo separado de los archivos excluidos sigue pendiente.
