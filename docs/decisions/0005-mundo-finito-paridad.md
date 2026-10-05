# 0005 · Mundo finito y mismas reglas para todos

- **Fecha original:** registrado en el prototipo (2026-09); traído al proyecto nuevo el 2026-10-05
- **Estado:** Decidido por el usuario en el prototipo. Revalidar si algo cambió con el motor propio.

## Decisiones

- El mundo es finito: personas, objetos y zombis no reaparecen.
- La población inicial de cada ciudad es finita.
- Jugador e IA usan las mismas acciones y reglas; la IA sólo elige intenciones.
- El jugador es independiente del habitante: puede dar órdenes de gestión, hacer táctica isométrica, controlar directo o dejar NPC autónomos.
- El estado del habitante (inventario, esfuerzo, dolor) persiste al cambiar de control o de cámara.
- Se empieza con un personaje; se encuentran supervivientes o bandidos y se forman o se unen grupos. Mandar sobre NPC depende del grupo y del rol de líder.
- El combate es peligroso incluso contra pocos.

Diseño: [habitantes](../design/habitantes.md), [interacción](../design/interaccion.md).
