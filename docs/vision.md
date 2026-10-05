# Visión

Juego de supervivencia zombi en 3D sobre un motor propio (C++/Vulkan). El jugador encarna a un habitante de un mundo finito: un cuerpo humano simulado que se cansa, se hiere, aprende y carga peso real. La ciudad también está simulada: luz, agua y aire son redes que se rompen, se aprovechan y se reparan. Se juega en tercera persona y en vista isométrica sobre el mismo personaje; la gestión de grupo y el cooperativo llegan después. Todo objeto se desmonta y se fabrica, y su calidad depende de piezas, herramienta y habilidad.

Etiquetas: **Decidido** (el usuario lo confirmó en el prototipo), **Propuesta** (no confirmado), **Abierto** (pregunta pendiente).

## Fantasía

Sobrevivir mucho tiempo en una ciudad que se apaga. Cada pelea es peligrosa, incluso contra pocos. Lo que encuentras no vuelve a aparecer. Para durar hay que entender el cuerpo, los objetos y la infraestructura, y organizar a otros.

## Pilares

| Pilar | Qué significa | Estado |
| --- | --- | --- |
| Mundo finito | Personas, botín y zombis no reaparecen. Supervivencia larga y difícil. | Decidido |
| Cuerpo simulado | Lesiones por zona, metabolismo, composición corporal y carga real. Nada de barra de vida genérica. | Propuesta (versión reducida) |
| Todo se desmonta y fabrica | Objetos como ensamblajes; Q heredada de piezas, limitada por habilidad y herramienta. | Decidido |
| Conocimiento y profesión | Las acciones disponibles dependen de saber, profesión y Q de herramientas (modelo Haven & Hearth). | Decidido |
| Infraestructura viva | Red eléctrica, agua y aire acoplados; romper un poste apaga un sector. | Propuesta (sistema héroe) |
| Un mundo, varias lentes | Tercera persona, táctica y gestión sobre el mismo mundo (Decidido). Reglas idénticas en todas (Propuesta). | Decidido / Propuesta |
| Estado único por habitante | Inventario, esfuerzo y dolor persisten al cambiar control, cámara u orden. | Decidido |
| Realismo | Realismo visual y animación fluida; fisiología legible. | Decidido |

## Qué lo hace distinto

- **Infraestructura viva en 3D.** Juegos 2D (Oxygen Not Included, Space Station 13) lo hacen; casi ningún survival 3D. Project Zomboid corta luz y agua por temporizador. Propuesta.
- **Grafo de salas.** Agua, humo, gas, sonido y olor zombi viajan entre salas por puertas, escaleras y desagües. Propuesta.
- **Calidad que alimenta sistemas físicos.** Un caño Q80 aguanta más presión que uno Q30. Propuesta.
- **Cuerpo que cambia silueta y ajuste de la ropa.** Al adelgazar, el pantalón queda flojo. Propuesta.
- **Cooperativo asimétrico.** Un jugador coordina desde gestión mientras otros pelean y saquean. Decidido como intención; diferido por la investigación (Propuesta).

Escenario demostración (Propuesta): una granada en el cuarto de bombas del sótano rompe caños; el agua sube y baja por la escalera al piso inferior. Volar un poste deja la manzana a oscuras; repararlo con una pieza fabricada, cuya Q decide cuánto dura.

## Jugador objetivo

Propuesta: jugadores de survival sistémico (Project Zomboid, Dwarf Fortress, RimWorld, Oxygen Not Included) que quieren eso en 3D y con acción directa. Disfrutan entender causas y consecuencias más que seguir un guion.

## Modalidad

- Objetivo final multijugador; se acepta empezar single player, luego cooperativo, luego multijugador. Decidido.
- Empezar con un personaje, encontrar supervivientes o bandidos, formar o unirse a grupos. Decidido.
- Escala de habitantes, zombis y jugadores. Abierto.

## Alcance: recortar, diferir, mantener

Tabla de la investigación del motor. **Propuesta**: el usuario no la confirmó todavía.

| Capa | Decisión | Por qué |
| --- | --- | --- |
| Tercera persona + isométrica sobre un personaje | Mantener | Identidad central |
| Red eléctrica en grafo con sectores | Mantener | Barata, visible; demo del poste |
| Red de agua + inundación por grafo de salas | Mantener | Demo de la granada |
| Cuerpo por capas + metabolismo | Mantener, reducido | Árbol de partes y composición |
| Ropa que sigue los morphs del cuerpo | Mantener, ya | Bloquea todo asset de personaje |
| Desmontaje + calidad en ~30 ítems | Mantener | Prueba el ciclo H&H sin catálogo enorme |
| Modo comandante / RTS | Diferir | Necesita multijugador |
| Modo gestor de ciudad | Diferir | Necesita una ciudad entera de contenido |
| Multijugador | Diferir | Multiplica el costo de cada sistema |
| Fuego, temperatura, clima | Diferir | Irán sobre el grafo de salas |
| Simulación de tela completa | Diferir | Solo pulido cerca de cámara |
| Destrucción vóxel, fluidos a escala mundo | Recortar | Costo excesivo para la ganancia |

Primer corte vertical (Propuesta): una manzana con 3–4 edificios, uno con cuarto de bombas en el sótano; una subestación y ~20 postes; un personaje con ambas cámaras.

## Preguntas abiertas

- ¿El sistema héroe es la infraestructura o el cuerpo? Abierto.
- ¿Cooperativo central desde el día uno o después? Abierto (el usuario aceptó empezar single player).
- ¿Tamaño del mundo: un pueblo, un distrito o una ciudad? Abierto.
- ¿La vista isométrica permite acelerar el tiempo? Abierto.
- Época, región, tono y nivel de violencia gráfica. Abierto.

## Fuentes del prototipo

- GDD_Survival_3D.md
- wiki/decisiones-experiencia-control.md
- wiki/decisiones-mundo-aprendizaje.md
- wiki/gameplay-cooperativo.md
- wiki/guia-de-arte.md
- wiki/bases-produccion-visual.md
- engine-research.txt (investigación del motor, más reciente)
