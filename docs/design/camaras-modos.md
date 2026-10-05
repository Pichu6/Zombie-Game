# Cámaras, modos y control

Las cámaras son lentes sobre una sola simulación: cambiar de vista nunca cambia reglas, daño ni estado. Hay tres modos: tercera persona (cuerpo propio), táctica isométrica (un personaje, su percepción) y gestión (el equipo, visión compartida por radio). El jugador es independiente del habitante que controla y puede tomar a otro sin reiniciar su estado. Primero se construyen tercera persona e isométrica sobre el mismo personaje; gestión, RTS y cooperativo esperan al multijugador (Propuesta de la investigación).

Etiquetas: **Decidido** (el usuario lo confirmó en el prototipo), **Propuesta** (no confirmado), **Abierto** (pregunta pendiente).

## Principios

| Regla | Estado |
| --- | --- |
| Misma simulación en todos los modos; un solo sistema de combate y daño. | Propuesta |
| El jugador es independiente del habitante controlado. | Decidido |
| Órdenes macro en gestión, táctica isométrica y control directo inmersivo. | Decidido |
| NPC autónomos cuando nadie los controla. | Decidido |
| Flujo de gestión a tercera persona, también en interiores y subterráneos. | Decidido |
| Táctica y gestión tienen funciones distintas; zoom y libertad no las igualan. | Decidido |
| Inventario, esfuerzo y síntomas persisten al cambiar control, cámara u orden. | Decidido |
| Mando y teclado/ratón son vías completas y cómodas. | Decidido |
| Empezar por tercera persona + isométrica sobre un personaje. | Propuesta |
| Diferir modo comandante/RTS y gestor de ciudad. | Propuesta |

## Los modos

| Modo | Atención | Información visible | Función |
| --- | --- | --- | --- |
| Tercera persona | Cuerpo propio y entorno inmediato | Percepción propia | Moverse, apuntar, interactuar, combatir |
| Táctica (isométrica) | Personaje elegido y alrededores | Solo percepción propia; nunca unión del equipo | Posicionar y decidir acciones individuales |
| Gestión | Operación del equipo | Unión de lo que reportan aliados con radio | Coordinar sectores, asignar tareas a NPC |
| Gestor de ciudad | Distritos, cobertura, suministro | Estado "frío" de la simulación | Diferido (Propuesta) |

Táctica muestra solo la percepción del personaje elegido y queda centrada en él. Gestión reúne la visión del equipo. Decidido.

Referentes del usuario (Decidido como referencia, no como copia de sistemas): tercera persona como GTA, Ghost Recon Breakpoint y Gears of War; táctica como Diablo y Door Kickers. No implican ambientación militar ni combate con coberturas.

## Parámetros de cámara

Valores de prueba del desarrollador, no decisiones del usuario. Propuesta.

| Modo | Parámetros probados |
| --- | --- |
| Tercera persona | ~1,85 m del pivote, desplazamiento lateral 12 cm, FOV 62°, se acorta ante obstáculos |
| Táctica | Objetivo en personaje + 0,9 m; inclinación fija 55°; ortográfica 10–17 m (inicial 13); giro libre, sin pan |
| Gestión | Inclinación 65–80° (inicial 72°); ortográfica 30–60 m (inicial 44); pan acotado al escenario |

- La rueda no cambia de modo; el cambio es explícito y cada modo conserva su encuadre. Propuesta.
- Conflicto: el GDD proponía zoom continuo 2,5–60 m que pasaba de micro a macro. El prototipo posterior lo descartó. Prefiero modos explícitos. Propuesta.
- Seleccionar un personaje en gestión no arrastra la cámara de gestión. Propuesta.

## Interiores y pisos

- Tercera persona: paredes y techos entre cámara y personaje se vuelven semitransparentes (tramado), sin acercar la cámara de golpe. Propuesta.
- Vistas altas: se ocultan los pisos superiores al del personaje (corte por piso). Propuesta.
- Retirar techos solo cuando gestión lo necesita; restituir la arquitectura al recorrerla. Propuesta.
- Cambiar de piso no debe revelar información oculta por accidente. Propuesta.
- La transparencia nunca modifica la percepción. Propuesta.

## Percepción y niebla

Cada observador tiene su propia percepción. Altura, pan o zoom de cámara no dan visión a través de muros. Decidido (binoculares y vigía); el resto Propuesta.

| Componente | Qué hace | Estado |
| --- | --- | --- |
| Detección | Línea de visión 3D, obstáculos, luz, alcance y atención. Detectar no es identificar. | Propuesta |
| Nitidez | Curva por distancia y contraste; lentes equipadas la corrigen. | Propuesta |
| Memoria | Última observación, antigüedad y confianza. Lo móvil no se actualiza dentro de la niebla. | Propuesta |
| Estabilidad | Temblor de mirada y precisión separados del efecto visual. Temblor de cámara opcional. | Propuesta |

- Niebla: visible en color, recordado en gris, desconocido en negro. Negro es ignorancia, no noche. Propuesta.
- Valores probados: cono 110°, alcance 12 m; vigía 24 m. Propuesta.
- En tercera persona el desenfoque puede expresar nitidez; en táctica y gestión se muestra incertidumbre del contacto, no blur de pantalla. Propuesta.
- Nunca mover ni desenfocar la interfaz. Propuesta.

## Cooperativo

Escenario de referencia confirmado por el usuario. Es dirección, no misión guionada. Decidido.

1. PJ1 pelea en tercera persona; PJ2 en táctica; se coordinan por radio.
2. PJ3 vigila desde una torre en gestión, selecciona NPC por área y les ordena saquear.
3. Los NPC con radio aportan sus conos de visión como reportes.
4. PJ3 usa binoculares y hace pan más lejos desde la altura; avisa por radio de refuerzos.
5. PJ1 y PJ2 se cruzan en la escalera y hablan por voz local.
6. PJ3 ordena evacuar al trote; un NPC sobrecargado se queja y la orden se mantiene.
7. Ya a salvo, un jugador toma ese NPC y encuentra dolor muscular. Puede tratarlo o delegarlo a otro NPC.

Reglas derivadas:

| Regla | Estado |
| --- | --- |
| Dos vías de voz: radio a distancia y voz local por proximidad. | Decidido |
| La radio permite voz y compartir conos como abstracción de lo que se comunica. | Decidido |
| Escuchar un aviso no muestra al enemigo a través de paredes en táctica o tercera persona. | Decidido |
| El coordinador puede evacuar su cuerpo mientras sigue en gestión. | Decidido |
| Selección por área y órdenes grupales; destino y ritmo son datos distintos. | Decidido |
| La queja no cancela la orden; separar aviso, voluntad y capacidad física. | Propuesta |
| Todo ligado a la simulación del personaje: menús, ítems, transferencias, órdenes. | Decidido |
| Órdenes a otros jugadores son indicaciones que aceptan; a NPC según permisos. | Propuesta |
| Separar personaje propio, selección para órdenes y foco de cámara. | Propuesta |
| Pertenencia a grupo y rol de líder condicionan el control de NPC. | Decidido |
| Informes de radio con emisor y antigüedad; sin conexión, envejecen. | Propuesta |
| Alcance, obstrucción, baterías y latencia de radio. | Abierto |
| Reconocimiento de voz para convertir charla en órdenes. | Abierto (no confirmado) |

## Control e interacción

- Selección contextual con ratón; selección al apuntar en tercera persona; selección grupal, waypoints y patrullas. Decidido.
- Animaciones contextuales ante paredes, esquinas y puertas en pose de combate. Decidido.
- Acciones compartidas: el menú, la transferencia manual y la orden a un NPC usan las mismas reglas. Propuesta.
- Herramientas de depuración siempre distinguibles de acciones del jugador. Decidido.
- Movimiento relativo a la cámara activa. Propuesta.

Esquema base del GDD (Propuesta; atajos definitivos Abierto):

| Acción | Mando | Teclado y ratón |
| --- | --- | --- |
| Mover | Stick izquierdo | WASD |
| Apuntar / mirar | Stick derecho | Cursor |
| Modo combate | Gatillo izquierdo | Clic derecho (mantener) |
| Atacar / empujar | Gatillo derecho | Clic izquierdo |
| Rotar cámara | LB / RB | Q / E o botón central |
| Interactuar | X / Cuadrado | E / F |
| Correr / agacharse | L3 / A | Shift / Ctrl |
| Inspeccionar cuerpo y ropa | Cruceta arriba | H |

## Reconocimiento antes de entrar

Del GDD. Propuesta.

- Espiar por rendija: el campo visual crece al acercar el ojo (θ = 2·arctan(w/2d)). A 1 m, ~2°; pegado, >50°.
- En tercera persona la cámara pasa a los ojos y se abre el sonido de la sala. En vistas altas, una cuña revela la niebla.
- Linterna y olor también cruzan la rendija: el riesgo es bidireccional.
- Oído en la pared: tabique transmite pasos y voces; ladrillo solo golpes; aislante casi nada.
- El propio pulso tapa lo que se oye. Quien escucha pierde visión trasera.
- Un golpe en la pared dispara sobresalto: pánico, paso atrás, posible caída del objeto en mano.
- Huecos de 3–5 cm en barricadas permiten atacar desde dentro; el arma puede quedar atascada.

## Tiempo

- La aceleración del tiempo es lo difícil, no las cámaras. Propuesta.
- Single player: acelerar solo sin amenazas cerca. Propuesta.
- Multijugador: una sola velocidad. Propuesta.
- El reloj de la simulación no depende de la UI ni de la cámara. Propuesta.
- ¿La isométrica permite acelerar? Abierto.

## Fuentes del prototipo

- GDD_Survival_3D.md (secciones 2, 16, 19, 20, 21)
- wiki/gameplay-cooperativo.md
- wiki/decisiones-experiencia-control.md
- wiki/decisiones-mundo-aprendizaje.md
- wiki/bases-produccion-visual.md
- art-direction/multi-team-blockout/README.md
- engine-research.txt
