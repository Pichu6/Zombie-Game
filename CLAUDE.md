# Juego Zombie

Simulador de supervivencia zombi realista para PC, sobre un motor propio en C++ con renderer Vulkan. La apuesta es la simulación profunda: redes de energía y agua, fluidos por habitaciones, cuerpo humano por capas y crafteo con calidad estilo Haven & Hearth. El mismo personaje se juega en tercera persona y en vista isométrica.

## Antes de trabajar

1. Leer `docs/status.md` (dónde estamos) y `docs/vision.md` (qué es el juego).
2. Leer el documento del sistema que se toca en `docs/design/` y las decisiones relacionadas en `docs/decisions/`.
3. Las instrucciones actuales del usuario mandan sobre cualquier documento.

## Dónde vive cada cosa

| Qué | Dónde | Regla |
| --- | --- | --- |
| Estado actual y siguiente paso | `docs/status.md` | Una pantalla. Se reescribe, no se acumula. |
| Visión del juego | `docs/vision.md` | Una página. |
| Diseño por sistema | `docs/design/<sistema>.md` | Una fuente de verdad por sistema. |
| Decisiones | `docs/decisions/NNNN-tema.md` | Una por archivo, con fecha. No se reescriben; una decisión nueva reemplaza a otra y lo dice. |
| Investigación | `docs/research/` | Notas fechadas; no son decisiones. |
| Lecciones del prototipo | `docs/lessons/` | Qué falló y por qué. |
| Términos | `docs/glosario.md` | |

## Reglas de trabajo

- **Distinguir siempre:** *Decidido* (lo confirmó el usuario), *Propuesta* (idea sin confirmar), *Implementado* (existe en código) y *Validado* (probado, con evidencia). Un test que pasa no es una validación visual.
- **El núcleo de simulación no depende del renderer.** Es una biblioteca C++ que corre y se prueba sin ventana. Renderer, cámaras y UI leen su estado y le mandan comandos.
- **Escribir sólo lo que hace distinto a este juego.** Para lo demás, usar bibliotecas probadas (ver `docs/decisions/0002-stack-motor.md`).
- **Toda tarea de motor responde a un escenario del vertical slice** (`docs/status.md`). Si ninguno la necesita, espera.
- Archivos chicos y con una sola responsabilidad, por diseño y no por un límite de líneas.
- No gastar créditos de servicios pagos ni hacer acciones externas sin pedirlo.
- El usuario escribe en español e inglés; responder en el idioma en que escribe.

## Prototipo anterior

`D:/Proyectos/juego zombie` (Godot, Mixamo, Blender, Tripo3D) es **sólo referencia**. No se continúa ni se copia código. Sus lecciones están destiladas en `docs/lessons/` y su diseño en `docs/design/`.

## Compilar y probar

Todavía no hay código. Cuando exista, esta sección dice cómo compilar, correr y testear.
