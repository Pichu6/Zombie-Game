# Habitantes

Cada habitante es un individuo persistente con cuerpo, conocimientos, práctica, inventario, relaciones y pertenencia. La ciudad tiene una población inicial finita y nadie reaparece. Los habitantes aprenden estudiando y practicando, sin clases rígidas. Sus historias nacen de sucesos compartidos, recuerdos distintos y vínculos asimétricos. Lejos del jugador se simulan con menos detalle y por eventos, sin perder identidad ni conservación.

Etiquetas: **Decidido** (el usuario lo confirmó en el prototipo), **Propuesta** (no confirmado), **Abierto** (pregunta pendiente).

## Identidad, grupo y autoridad

Decidido:

- El jugador empieza con su personaje y puede hallar supervivientes o bandidos.
- Puede formar grupos, unirse a otros o reclutar.
- Controlar NPC depende de pertenencia a grupo o facción y de un rol de liderazgo.
- Habitantes individuales con rasgos, estados y personalización. Referentes: Sims/pops con elementos de XCOM.

Propuesta:

| Concepto | Significado |
| --- | --- |
| Facción | Pertenencia social y relaciones. Ser amistoso no da control. |
| Grupo operativo | Quienes viajan o trabajan juntos y comparten órdenes locales. |
| Rol | Líder y delegaciones explícitas dentro del grupo. |

- Un NPC reclutado es el mismo individuo, con sus objetos y estado previo.
- Liderar NPC no da control sobre personajes de otros jugadores.
- Abierto: sucesión del líder, transferencia de mando, abandono, lealtad, desobediencia.

## Población finita

Decidido:

- Personas, loot y zombis son finitos; sin respawn.
- Cada ciudad tiene una población inicial lógica y finita.
- Supervivencia exigente: uno o dos enemigos ya pueden ser peligrosos.
- Conversión, huida y muerte trasladan o transforman personas existentes.

Propuesta:

- La población sale de viviendas, hogares, trabajos y contexto del brote, no de un máximo técnico.
- Conservación: inicial = vivos + muertos + convertidos + huidos localizados.
- El mundo guarda el censo real; el personaje solo sabe lo observado o comunicado.
- El comercio no regenera mercancía. Las reservas naturales son cantidades finitas.
- Abierto: nacimientos, cultivos y crecimiento. No son excepción automática al «sin respawn».

## Necesidades y cuerpo

Propuesta:

- Las necesidades (hambre, sed, sueño, temperatura, dolor) salen de la simulación corporal, no de barras sueltas.
- Las ofertas de objetos describen cómo satisfacerlas; el cuerpo aplica el efecto real.
- La masa móvil (cuerpo + ropa + carga) gobierna gasto calórico y fatiga.
- Modelo corporal de la investigación: árbol de partes con capas de tejido → capacidades → acciones.
- Fatiga o lesión afectan la capacidad efectiva, nunca borran conocimiento.
- Abierto: modelo de nerviosismo y pánico; evaluación de utilidad para elegir qué necesidad atender.

## Aprendizaje y especialización

Decidido (2026-09-07):

- Cada jugador orienta la especialización de su personaje por lo que estudia y practica.
- Estudiar consume tiempo del personaje.
- Comprender un procedimiento, asociar una herramienta y ejecutarlo son avances distintos.
- Las primeras acciones novedosas dan experiencia; la repetición da menos o nada.
- Otro ejemplar de un tipo conocido puede enseñar por diferencias, piezas o fallas.
- Aplica a todos los ítems según sus capacidades, no a un tipo concreto.

Decidido (2026-09-10, modelo Haven & Hearth): conocimiento y profesión filtran qué acciones ve cada habitante. La habilidad es techo blando de la Q obtenida. Ver `objetos-crafteo.md`.

Propuesta:

- Especialidades abiertas, sin clases que bloqueen otras áreas.
- Conocimientos relacionados se transfieren entre procedimientos compatibles.
- Estudiar no aumenta la Q de ningún objeto.
- Un error enseña solo si revela algo nuevo. Repetir fallos vacíos no premia.

### Cuatro estados separados

| Estado | Pertenece a | Ejemplo |
| --- | --- | --- |
| Conocimiento | Habitante | Entiende una fijación roscada |
| Práctica | Habitante y ámbito | Ha desmontado muchos cierres de esa familia |
| Evaluación | Habitante, ejemplar y propiedad | Cree que esta pieza está dañada (con fecha, método y confianza) |
| Capacidad efectiva | Ejecutor al actuar | Mano herida o herramienta mala dificultan la tarea |

### Ciclo propuesto

1. El objeto expone propiedades observables, sin revelar piezas ocultas.
2. El habitante reconoce lo que sabe e inicia estudio si tiene acceso y medios.
3. La acción avanza en tiempo de simulación y registra hitos.
4. El conocimiento asocia herramientas y procedimientos.
5. Ejecutar da un resultado real y evidencia de práctica.
6. Una clave de novedad decide qué se aprendió, con rendimiento decreciente.

- Solo la simulación valida hitos; la UI nunca concede experiencia. Propuesta.
- El registro de novedad evita dobles premios al recargar o cambiar de cámara. Propuesta.
- Manual o mentor pueden dar conocimiento, no práctica equivalente. Propuesta.

Abierto: taxonomía de dominios; conocimientos iniciales por historia; duraciones y curvas; olvido (no confirmado, no introducir); visibilidad de Q; aprendizaje autónomo de NPC; conocimiento tras morir o reclutar.

## Historias y relaciones

Propuesta (investigación, no confirmada como hito).

Una buena creación responde a quién afectaste, dónde, qué cambió y qué quedó sin resolver.

### Hecho, recuerdo y relato

| Capa | Contenido |
| --- | --- |
| Hecho mundial | Qué pasó, cuándo, dónde, quién; se guarda una vez y todos lo referencian |
| Recuerdo personal | Lo que cada uno percibió, con confianza, detalle, emoción y errores |
| Evidencia | Objetos, heridas, cadáveres, registros, lugares, testimonios |
| Relato circulante | Rumor o versión oficial; muta sin cambiar la verdad |


### Vínculos

- Grafo disperso: solo hay arista si hubo parentesco, convivencia, contacto o información.
- Vista A sobre B: familiaridad, afinidad, confianza, respeto, miedo, resentimiento, dependencia, atracción opcional.
- También: hechos conocidos, recuerdos destacados, qué quiere A de B y qué lo impide.
- Bilateral y asimétrico: Ana confía en Bruno y lo teme; Bruno la quiere y cree que lo traicionó.
- Curar sube confianza o deuda, no afecto. Intimidar da obediencia, miedo y resentimiento.
- Regalar lo óptimo no compra amistad. En escasez importa necesidad, coste, intención y procedencia.
- Un vínculo no da telepatía; las noticias requieren observación, reporte o inferencia.
- Jugador e IA usan las mismas acciones sociales: observar, preguntar, prometer, mentir, ayudar, amenazar.
- De Stardew Valley: ritmo de convivencia, gustos descubribles, hitos relacionales, calendario social.
- Hitos relacionales cambian algo operable (acceso, ayuda, confesión), no solo diálogo.

### Trasfondo sin clase rígida

Cadena: hogar → experiencia formativa → oficio → crisis → vínculo decisivo → incidente previo → brote → objetivo actual.

- Cada paso deja residuos verificables: práctica inicial, contactos reales, accesos, bienes, deudas, lesiones, una pregunta abierta.
- Una profesión nunca da dominio perfecto. «Fue paramédica» no es «medicina 80».
- Todo beneficio trae un coste coherente: tiempo, obligación, lesión, edad.

### Origen del brote

- La verdad causal del brote está fija en la simulación, aunque oculta al jugador.
- Gramática: causa → primer incidente → testigos → ocultamiento → versión pública → fallos → experiencias personales.
- Sin misión principal obligatoria. Si muere la única viróloga, no aparece reemplazo; quedan sus notas.

### Generación

De mayor a menor: ciudad → hogares y organizaciones → esqueleto individual → detalle al entrar en foco.

- Expansión determinista; todo dato revelado queda fijado. «No detallado» no significa «contradecible».
- Fuera de escena se agendan próximos sucesos, no conversaciones cuadro a cuadro.

## Escala y simulación diferida

Propuesta. Una identidad atraviesa tres niveles de detalle; la investigación los llama caliente, templado y frío.

| Nivel | Dónde | Qué corre |
| --- | --- | --- |
| Completo (caliente) | ~50 m de un jugador, o en conflicto | Cuerpo, percepción, navegación, animación, acciones en pasos finos |
| Sectorial (templado) | Distrito cargado | Acciones reales, movimiento agregado, cuerpo a intervalos mayores |
| Dormido (frío) | Resto del mundo | Sin tick; planes por fases, próximo umbral, evolución analítica |

- Estado por habitante: tiempo resuelto, estado compacto, fase actual, próximo despertar, sector, semilla.
- El reloj del mundo es la autoridad; al bajar de nivel se agenda el próximo evento.
- Al subir, se resuelven eventos vencidos en orden; nunca tick a tick. Sangrado, hambre crítica o encuentros despiertan.
- La promoción empieza antes de ser visible y respeta un presupuesto por cuadro.
- Benchmark del prototipo: 1000 cuerpos completos a 10 Hz costaban ~30 ms por paso. Hace falta el nivel diferido.
- Guardado del prototipo: ~28 KB por habitante. Separar definición de estado y dividir por sector.
- Primer objetivo: decenas de humanos materializados y miles de identidades persistentes con conservación exacta.

Propuesta (intención del usuario, sin confirmar cifras): mapa grande, recorridos de horas a pie, referentes Arma/GTA. La investigación deja abierto el tamaño (pueblo, distrito o ciudad). Abierto: tamaño, población simultánea, hardware y aceleración de tiempo.

## Fuentes del prototipo

- aprendizaje-especializacion.md
- historias-relaciones-sandbox.md
- base-funcional-habitantes.md
- base-funcional-habitantes-contratos-acciones.md
- fundamentos-mundo-personajes-produccion.md
- fundamentos-mundo-personajes-produccion-habitantes-objetos.md
- fundamentos-mundo-personajes-produccion-aprendizaje-escala.md
- arquitectura-sistemas.md
- engine-research.txt (investigación del motor)
