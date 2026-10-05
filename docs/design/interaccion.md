# Interacción con el mundo

Jugadores y NPC actúan sobre el mundo con las mismas reglas. Cada objeto ofrece interacciones según sus capacidades; un ejecutor común las valida, las ejecuta en tiempo de simulación y aplica sus efectos. La interfaz solo formula intenciones: actuar en persona, ordenar al propio personaje o delegar en una unidad autorizada. Hay tres modos de control (tercera persona, táctica individual y gestión) y todos llegan al mismo ejecutor. Teclado, ratón y mando son vías completas.

Etiquetas: **Decidido** (el usuario lo confirmó en el prototipo), **Propuesta** (no confirmado), **Abierto** (pregunta pendiente).

## Paridad jugador–IA

Decidido: cada humano sobrevive, se desplaza, trabaja, se equipa, come, descansa, combate o muere con las mismas acciones y reglas, lo controle un jugador o una IA.

- La IA elige intenciones de alto nivel. No imita pulsaciones ni salta el ejecutor común. Decidido.
- Jugador e IA pasan por los mismos requisitos, reservas, tiempos, consumo y efectos. Propuesta.
- La IA no recibe ataques gratis ni atajos distintos. Propuesta.
- Una acción fuera de cámara no tiene éxito automático. Conserva ruta, carga, fase, riesgos e interrupciones. Propuesta.
- Cambiar quién controla a un habitante no cambia tiempos, costes ni resultados. Propuesta.

## Ofertas de interacción (Smart Objects)

Propuesta, con base parcial probada en el prototipo.

- La definición de un objeto declara sus interacciones por identificador (p. ej. `transfer`).
- Otro tipo que declare la misma capacidad recibe el mismo comportamiento, sin código propio.
- Una interacción no implementada no produce acciones ficticias.
- Cada oferta contiene: identificador, etiqueta, ejecutor, objetivo, disponibilidad, motivo de bloqueo, fases y efecto descrito.
- Una oferta puede mostrarse bloqueada con su motivo.
- El identificador incluye origen y destino. Un menú viejo no actúa sobre un objeto que se movió.
- La oferta describe una posibilidad; el proceso validado produce las consecuencias.
- Nada de efectos directos tipo «energía +80» que salten la simulación corporal.

### Filtrado por conocimiento

Decidido (modelo Haven & Hearth, 2026-09-10):

- Las ofertas no son universales. Dependen del conocimiento y la profesión del actor.
- Un electricista sabe cambiar un cartucho de transformador; un cirujano, hacer suturas vasculares.
- Sin conocimiento solo aparecen acciones torpes de alto riesgo, como arrancar cables a la fuerza.
- La Q de la herramienta condiciona velocidad, desgaste y tasa de éxito.

Propuesta: las acciones cotidianas están disponibles desde los conocimientos iniciales; no todo exige estudio previo.

### Consultar no es ejecutar

- La vista previa usa las mismas comprobaciones que la ejecución y no cambia nada. Propuesta.
- Comprobaciones: actor disponible, reserva, origen, capacidad, accesos y camino. Propuesta.
- Al ejecutar, el sistema vuelve a validar todo. Propuesta.
- Los efectos ocurren en el ejecutor, nunca en el menú ni por reproducir una animación. Propuesta.
- El llamador solo pasa objetivos que el personaje conoce o percibe. Propuesta.

## Acciones temporizadas

Propuesta. Toda acción tiene requisitos, duración, recursos, fases, interrupción y efecto.

- Una acción por ejecutor y una reserva por ejemplar.
- La propiedad cambia solo al completar.
- Cancelar no devuelve el tiempo gastado.
- Pausa detiene progreso y cuerpo.
- Guardar conserva acceso, reservas y progreso.
- Si una acción cargada perdió requisitos, la siguiente validación la interrumpe.
- Interrupciones: muerte, falta de reserva física, pérdida de alcance, origen cambiado, falta de capacidad.
- Distancia excesiva genera un acercamiento previo; nunca una transferencia remota silenciosa.

### Transferencias

Fases propuestas: acceder al origen → extraer y manipular → acceder al destino → acomodar y cerrar.

- Antes de extraer, el objeto sigue en el origen. Durante la manipulación está en la mano.
- Una interrupción no duplica ni evapora el objeto ni completa un lote entero.
- El acceso que se paga una vez (abrir la mochila) se separa del coste por unidad.
- Influyen tamaño, masa, cantidad, agarre, manos libres, postura y distancia.
- Ir de un bolsillo a una mochila cerrada a la espalda no cuesta lo mismo que a una caja.

Coeficientes de prueba del prototipo, no calibrados:

| Parámetro | Valor |
| --- | --- |
| Trabajo | s origen + s destino + 0,5 + 0,5 por kg |
| Acceso a caja | 1 s |
| Acceso a contenedor personal | 3 s |
| Alcance máximo a cada extremo | 1,5 m |
| Esfuerzo corporal mínimo durante la acción | 0,15 |
| Modulación | Reserva de esfuerzo y dolor, con velocidad mínima mientras siga capaz |

### Cuerpo y capacidad

- Fatiga, lesión, dolor y nerviosismo afectan solo las fases pertinentes. Decidido (relevancia), reglas en Propuesta.
- No existe un multiplicador único de «salud» que penalice todo. Propuesta.
- Un cuerpo delgado no es incapaz por apariencia: cuentan fuerza, composición, entrenamiento y estado. Propuesta.
- La duración de una transferencia depende del objeto, origen, destino y cuerpo. Decidido.

## Selección, objetivo y control

Decidido: interacción con objetos y contenedores, consulta de estado e ítems de cada personaje.

Decidido: esquema base de los tres modos.

| Modo | Seleccionar | Acción rápida / mover | Opciones ampliadas |
| --- | --- | --- | --- |
| Tercera persona | Apuntar a un objeto visible | Botón interactuar; locomoción directa | Mantener interactuar o botón dedicado |
| Táctica individual | Clic izquierdo sobre objetivo | Clic derecho en suelo mueve al personaje | Clic derecho sobre objeto abre su menú |
| Gestión | Clic izquierdo o arrastre por área | Clic derecho asigna destino a seleccionados | Clic central abre órdenes, poses y acciones |

- Asignar selección por área y órdenes grupales a gestión es interpretación provisional. Propuesta.
- La acción «guardar objeto» da igual resultado en los tres modos. Propuesta.
- Se separan cuatro cosas: personaje controlado, unidades seleccionadas, objeto objetivo y foco de cámara. Propuesta.
- Seleccionar una caja no cambia de personaje controlado. Propuesta.
- Inspeccionar a un compañero no autoriza tocar su inventario. Propuesta.
- El objetivo se fija al abrir el menú y se revalida al ejecutar. Propuesta.
- Alcance, acceso, visibilidad y conocimiento son restricciones distintas. Propuesta.
- Mostrar objetivo, ejecutor y motivo de bloqueo. Propuesta.
- Abierto: permisos de control e intercambio cooperativo.

### Mando y entradas

Decidido: mando y teclado/ratón soportados desde el diseño.

Propuesta:

- Acciones abstractas: seleccionar, ampliar selección, confirmar, contextual, cancelar, cambiar foco, inventario, encolar, postura.
- Las reglas de juego nunca viven en manejadores de botones físicos.
- Mando: selección asistida entre objetivos válidos, menú radial con alternativa en lista.
- Toda operación esencial se completa sin ratón, hover, arrastrar ni clic central.
- Teclas reasignables, alternativa a mantener pulsado, foco visible, cancelar vuelve atrás.
- Abierto: distribución física final; requiere prototipo con mando.

## Interfaz de inventario y estado

Propuesta. Una presentación común, abierta desde cualquier modo:

- Estado funcional: actividad, fatiga, dolor, afecciones conocidas. Lo perceptible se separa del diagnóstico detallado.
- Cuerpo y equipo: manos, ropa colocada, puntos de almacenamiento.
- Contenedores: bolsillos, mochila, bolsas, con contenido real.
- Transferencia: origen, destino, cantidad, espacio, carga resultante, duración estimada.
- Vista de dos inventarios, o seleccionar objeto → elegir destino → confirmar.
- Cola de acciones: ejecutor, objeto, destino, progreso, interrupción, motivo de bloqueo.
- El menú pide acciones; nunca concede experiencia directamente.
- Feedback de conocimiento: «falta herramienta compatible», «procedimiento conocido, sin práctica», «componente desconocido».

## Movimiento, waypoints y patrullas

Decidido: órdenes de movimiento, waypoints y posible patrulla.

Propuesta:

- Los waypoints expresan intención; la navegación respeta colisiones, puertas, escaleras y conectividad.
- Distinguir sustituir destino de encolar puntos.
- Una ruta se ve, edita, cancela y retoma. Un tramo imposible avisa; nunca teletransporta.
- Patrulla: ruta reutilizable con repetición, esperas y orientación.
- Abierto: formaciones, prioridades y respuesta a amenazas.

## Postura, entorno y combate

Decidido: animaciones sensibles a paredes, esquinas y puertas, sobre todo en pose de combate. Referente: Ghost Recon Breakpoint.

Propuesta:

- Separar detección de entorno, decisión de postura y animación.
- No pegar al personaje a toda pared cercana. Combinar intención, orientación, espacio libre y estado del cuerpo.
- La animación representa una acción viable; no autoriza atravesar colisiones.
- Mantener un botón entra en modo combate: más lento, torso hacia la amenaza, listo para atacar o defender.
- Separar entrada, intención, resolución y representación.
- Los objetos declaran agarres y usos, no solo «una mano / dos manos».
- Cambiar agarre, recuperar equilibrio o liberar una herramienta trabada consume tiempo.
- Distancias: fuera de alcance, alcance de arma, contacto y forcejeo son estados distintos.
- Apuntar no garantiza acertar. La asistencia muestra el foco y nunca corrige a través de paredes.
- Tercera persona: retícula libre contra partes del cuerpo, afectada por retroceso, fatiga, dolor y respiración.
- Isométrica: selector de zona (cabeza, torso, piernas, brazos) y cono de dispersión que se cierra al estabilizar.
- El cono depende de habilidad, Q del arma y calma. Moverse, temblar o recibir un golpe lo abre.
- Abierto: bindings, lista de técnicas, mantener frente a alternar.

## Orden de construcción

Propuesta: estado del habitante → objetos e inventarios → ejecutor de acciones → selección y controles → navegación y órdenes → autonomía y cooperativo.

## Fuentes del prototipo

- interaccion-control-inventario.md
- ofertas-interaccion.md
- base-funcional-habitantes-contratos-acciones.md
- aprendizaje-especializacion.md
- fundamentos-mundo-personajes-produccion-aprendizaje-escala.md
- fundamentos-mundo-personajes-produccion-habitantes-objetos.md
