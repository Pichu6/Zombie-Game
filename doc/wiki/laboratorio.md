# Laboratorio: selección y acciones sobre el cuerpo

Estado: primera mejora implementada, 2026-09-06. Prioridad confirmada: mejorar el laboratorio antes de ampliar el juego single player y, posteriormente, cooperativo/multijugador.

## Recorrido implementado

Seleccionar con clic izquierdo una zona de la silueta muestra su nombre y habilita «Acciones de esta zona» debajo del cuerpo. El botón abre el menú existente: primeros auxilios, equipamiento, detalles y herramientas explícitamente rotuladas «Laboratorio (depuración)».

El clic derecho mantiene el comportamiento previo de acciones rápidas: opera sobre la zona apuntada sin cambiar automáticamente la selección del inspector. El encabezado del menú identifica su destino. «Examinar Detalles» lleva esa zona al inspector.

Cambiar o quitar la selección elimina el destino de un menú anterior y cierra ese menú. Sin zona seleccionada, el botón está deshabilitado. Esto evita que una acción posterior sobre la selección termine afectando la zona anterior.

## Archivos y límites

- `src/ui/biology_dashboard.gd`: controles, actualización de selección, menú y destino de acciones. Entrada: ID anatómico entero y posición del menú en píxeles. Salida: controles actualizados y ejecución de las acciones existentes.
- `tools/test_selection_actions.gd`: prueba del cambio de brazo a pierna, ausencia de efectos en el brazo anterior, deselección y compatibilidad del clic derecho. Ejecutar con Godot `--headless --path . --script res://tools/test_selection_actions.gd --quit-after 600`; exigir `SELECTION_ACTIONS_OK`, sin errores de script.
- No cambia las fórmulas fisiológicas, el coste/duración de tratamientos ni implementa red.

## Verificación

La prueba nueva y las existentes `test_ux_refinements_and_biomechanics.gd` y `test_clinical_logic_and_paperdoll.gd` alcanzaron sus marcadores finales de éxito. Se conservaron las expectativas de las pruebas existentes. Siguen los avisos de liberación de recursos previamente documentados.

Revisión visual mediante render real del laboratorio a 1600×900 y 1920×1080: nombre de zona, botón y encabezado del menú legibles. Capturas locales en `.backups/laboratory-selection.png` y `.backups/laboratory-actions-menu.png`. En esa primera revisión se observó recorte de controles de la columna derecha; se corrigió en la iteración siguiente, documentada abajo. No se considera completo un rediseño de toda la pantalla.

## Adaptación de paneles a la ventana — 2026-09-06

Implementado: el panel de simulación permanece dentro de la ventana con ancho mínimo de 370 píxeles. El divisor utiliza la distribución del contenedor y deja de recalcular una posición que no contemplaba los mínimos de los hijos. El usuario puede seguir arrastrándolo.

Los campos de formato de reloj, sustrato y vegetación colocan su selector debajo de la etiqueta. Las descripciones e información clínica admiten varias líneas; los valores numéricos de las filas se mantienen en una sola. Las barras superior e inferior pueden distribuir controles entre filas, manteniendo juntos etiqueta, slider y valor de cada control biométrico.

El área de trabajo dispone de desplazamiento horizontal y vertical automático. A 1280×720 hace falta desplazarla para acceder al inspector y los controles inferiores; el panel de simulación tiene su propio desplazamiento vertical. Es acceso completo mediante scroll, no una versión móvil ni una promesa de mostrar todas las columnas simultáneamente.

Verificación: render real a 1920×1080, 1600×900 y 1280×720; inspección visual de 1600 y 1280. La prueba `tools/test_laboratory_layout.gd` verifica ambos paneles dentro de la ventana, ancho del contenido derecho, valores sin fragmentar y posibilidad de desplazarse hasta el final del contenido. Marcador: `LABORATORY_LAYOUT_OK`. También pasaron selección/acciones, UX/biomecánica y lógica clínica/paperdoll. Persisten los avisos de recursos sin liberar ya documentados. Capturas y logs en `.backups/layout-*`, fuera de Git.
