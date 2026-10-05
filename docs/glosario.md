# Glosario

Vocabulario común del proyecto para documentos de diseño y código del nuevo motor. Recoge los términos del prototipo que siguen vigentes, sin los ligados a Godot o a assets concretos, y añade los de la investigación del motor. Las definiciones describen intención de diseño; el estado de cada regla (Decidido, Propuesta, Abierto) vive en su documento de diseño.

## Mundo

- **Mundo finito persistente**: sin respawn de personas, loot ni zombis. Cada ciudad tiene censo inicial finito.
- **Conservación**: población inicial = vivos + muertos + convertidos + huidos localizados. Lo mismo para masa de objetos.
- **Sector**: porción del mapa que se carga, guarda y simula como unidad.
- **Nivel de simulación (caliente / templado / frío)**: detalle completo cerca del jugador, cálculo de redes en el distrito, estadísticas en el resto.
- **Simulación diferida**: un habitante o sector dormido avanza por eventos agendados, no por ticks.
- **Grafo de salas**: salas y pasillos como contenedores unidos por aberturas; transporta agua, gas, humo, sonido y olor.
- **Red de servicios**: electricidad y agua como grafos de nodos y aristas que se recalculan solo al cambiar.

## Objetos

- **Definición**: datos compartidos de un tipo de objeto (masa, capacidades, materiales, procesos).
- **Ejemplar**: objeto concreto con ID único, Q, condición, ubicación y componentes reales.
- **Grafo de ensamblaje**: lista de componentes de un objeto, cada uno con material, Q y condición.
- **Q (calidad)**: qué tan bien se hizo algo. Fija salvo un proceso real.
- **Condición**: desgaste, daño o corrosión. Cambia con el uso y la reparación.
- **Material**: de qué está hecho un componente.
- **Q efectiva**: Q que rinde una acción tras limitarla por habilidad y herramienta.
- **Receta / proceso**: entradas, herramientas, conocimientos, tiempo, salidas, pérdidas y reglas de Q.
- **Contenedor**: objeto o lugar que guarda ejemplares; su capacidad es distinta de la del portador.

## Cuerpo

- **Compartimentos (kg)**: el cuerpo guarda masas en kg: grasa, músculo y estructural. Los porcentajes son derivados.
- **Masa biológica**: cuerpo desnudo (grasa + músculo + estructural).
- **Masa móvil**: masa biológica más ropa y carga. Gobierna gasto calórico y fatiga.
- **Capacidades**: moverse, manipular, consciencia, etc., calculadas desde las partes del cuerpo.
- **Capas de ropa**: interior, media y exterior, al estilo CDDA.
- **Clo**: unidad de aislamiento térmico de la ropa.
- **Estorbo (encumbrance)**: penalización de movimiento o precisión por ropa gruesa, rígida o cargada.
- **BodyParts3D**: atlas anatómico (CC BY 4.0) de referencia para la vista médica detallada.

## Habitantes

- **Habitante**: individuo persistente con identidad, cuerpo, conocimientos, práctica, inventario, relaciones y pertenencia.
- **Paridad jugador–IA**: jugador e IA usan las mismas acciones y reglas; la IA solo elige intenciones.
- **Facción / grupo operativo / rol**: pertenencia social; quienes actúan juntos; liderazgo y delegaciones.
- **Conocimiento**: qué procedimiento comprende un habitante.
- **Práctica / habilidad**: capacidad acumulada de ejecutarlo.
- **Evaluación**: lo que un habitante cree sobre un ejemplar, con confianza y fecha.
- **Clave de novedad**: identifica qué se aprendió, para no premiar dos veces lo mismo.
- **Hecho mundial / recuerdo / relato**: lo que pasó; lo que cada uno percibió; lo que circula como rumor.
- **Trasfondo (lifepath)**: cadena de etapas previas que deja contactos, práctica, bienes y deudas reales.

## Interacción

- **Oferta (Smart Object)**: acción que un objeto expone a un ejecutor, con disponibilidad y motivo de bloqueo.
- **Ejecutor común**: sistema que valida, temporiza y aplica toda acción, venga de jugador o IA.
- **Reserva**: bloqueo de un ejemplar mientras una acción lo usa.
- **Modo combate**: postura sostenida, más lenta, lista para atacar o defender.
- **Tercera persona / táctica / gestión**: tres modos de control sobre el mismo mundo y las mismas reglas.

## Cámaras y presentación

- **Niebla de guerra**: oculta lo que el personaje no ve en vista táctica y deja un recuerdo en gris.
- **Corte de techos y paredes (cutaway)**: oculta techo o paredes frontales cuando el personaje está dentro.
- **Superposición (overlay)**: capa visual de red eléctrica, agua o gas en vista isométrica.
- **LOD / proxies**: geometría simplificada a distancia para ahorrar coste de render.

## Fuentes del prototipo

- glosario.md
- arquitectura-sistemas.md
- aprendizaje-especializacion.md
- historias-relaciones-sandbox.md
- ofertas-interaccion.md
- fundamentos-mundo-personajes-produccion-habitantes-objetos.md
- engine-research.txt (investigación del motor)
