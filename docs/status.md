# Estado

Actualizado: 2026-10-05.

## Qué hay que hacer

Construir un primer corte vertical jugable sobre el motor propio (Propuesta, ver [visión](vision.md)):

1. Volar un poste y que la manzana quede a oscuras.
2. Tirar una granada en el cuarto de bombas del sótano y que el agua llegue al piso de abajo por la escalera.
3. Reparar el poste con una pieza fabricada cuya calidad decide cuánto dura.
4. Un personaje con cuerpo que cambia de forma, ropa que lo sigue y las dos cámaras.

## Dónde estamos

- **Hay:** documentación de diseño destilada del prototipo, decisiones registradas y la investigación de ropa con herramientas probadas en un cuerpo sintético.
- **No hay:** código del motor. El repositorio todavía no está en GitHub (cuenta del usuario bloqueada por 2FA); el historial está en git local.

## Siguiente paso

1. El usuario confirma o corrige las decisiones marcadas para revalidar y responde las preguntas abiertas de la [visión](vision.md), sobre todo cuál es el sistema héroe.
2. Arrancar el núcleo de simulación sin ventana, con una vista de depuración 2D: red eléctrica por grafo y grafo de salas para el agua.
3. Subir el repositorio a GitHub cuando la cuenta vuelva.

## Pendiente de decisión del usuario

- Stack del motor ([0002](decisions/0002-stack-motor.md)).
- Ropa por wrap en GPU y si la ropa ceñida oculta la piel ([0003](decisions/0003-ropa-wrap.md)).
- Fórmula de calidad ([0006](decisions/0006-crafteo-calidad.md)).
- Si "todo en las redes se puede reparar o construir desde cero" es una regla confirmada ([infraestructura](design/infraestructura.md)).
- Zombis lentos con incubación larga o rápidos ([amenazas](design/amenazas.md)).
