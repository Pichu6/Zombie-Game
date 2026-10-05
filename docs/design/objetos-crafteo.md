# Objetos, ensamblaje y crafteo

Todo objeto del mundo es un ensamblaje de componentes con material, calidad (Q) y condición. Desmontar recorre ese grafo y devuelve piezas reales; craftear lo construye. La Q de un resultado nace de la Q de sus entradas, limitada por la habilidad del ejecutor y la Q de la herramienta. Calidad, condición y material son tres números separados. El mundo es finito: desmontar, fabricar o transferir nunca crea ni pierde masa sin una regla declarada. Inventario y contenedores guardan ejemplares con identidad, no iconos sueltos.

Etiquetas: **Decidido** (el usuario lo confirmó en el prototipo), **Propuesta** (no confirmado), **Abierto** (pregunta pendiente).

## Principios

| Regla | Estado |
| --- | --- |
| Los objetos existentes pueden desmontarse y construirse. | Decidido |
| La mayoría aparece ya ensamblada: herramientas, muebles, props, vehículos. | Decidido |
| Todo objeto tiene Q ligada a componentes, datos físicos y representación visual. | Decidido |
| Los componentes tienen vías de fabricación ligadas a conocimientos y habilidades. | Decidido |
| Haven & Hearth es la referencia canónica de conocimiento, profesión y Q. | Decidido |
| Mundo finito: sin respawn de loot, personas ni zombis. | Decidido |
| No copiar respawn ni reinicios de mundo de Haven & Hearth. | Decidido |
| Grafo de ensamblaje como modelo de datos de todo objeto. | Propuesta (investigación del motor) |
| Primer corte: desmontaje y Q sobre unas 30 familias de ítems. | Propuesta (investigación del motor) |

## Capas de datos de un objeto

Separar material, tipo de componente, ejemplar, ensamblaje y proceso (receta). Propuesta.

| Capa | Contenido mínimo |
| --- | --- |
| Definición | ID estable, versión, nombre, familia, capacidades, materiales permitidos, compatibilidades, modelo de Q, procesos |
| Físico | Masa en kg (o regla para derivarla), dimensiones en m, volumen y capacidad si aplica, colisión |
| Ejemplar | ID único, definición, Q, condición, cantidad, ubicación y dueño, componentes reales, estado funcional |
| Visual | Icono, silueta 2D, modelo 3D, materiales, puntos de montaje y agarre, LOD |
| Producción | Recetas, entradas, salidas, requisitos, tiempos, política de recuperación |
| Validación | Referencias válidas, unidades y rangos, masa coherente, propiedad sin ciclos |

- La definición comparte assets; cada ejemplar guarda su propio estado.
- No hace falta una textura por tornillo ni por valor de Q.
- Un placeholder se marca como tal; nunca se presenta como asset final.
- Una pieza oculta dentro de un ensamblaje necesita representación propia al extraerse.

## Grafo de ensamblaje

Propuesta. Un objeto es una lista de componentes; cada uno tiene material, Q y condición. Nada se simula físicamente hasta que se rompe.

- Las piezas ocupan ranuras funcionales y tienen un estado de unión.
- El contenido guardado no es una pieza montada. El botín de una mochila no es estructura.
- La masa no se cuenta dos veces: piezas montadas y contenido se suman por separado.
- Propiedad y ubicación forman una jerarquía sin ciclos.
- Uniones físicas y conexiones entre subsistemas se modelan aparte, sin dar dos dueños a una pieza.
- Un vehículo es un ensamblaje de subsistemas, no miles de nodos activos.
- La composición se fija al generar el objeto, de forma explícita o reproducible por semilla.
- Desmontar materializa esas mismas piezas; nunca se sortea botín nuevo.
- Tornillos iguales pueden apilarse si comparten Q, condición y procedencia.
- Edificios llevan su infraestructura: muros y suelos contienen aristas de cable y tubería. Propuesta (investigación).
- Romper un muro puede cortar un sector o inundar un sótano. Arrancar cobre de una casa es una decisión real. Propuesta.

## Calidad, condición y material

Tres valores separados. Una tubería de alta Q oxidada sigue valiendo la pena recuperarla.

| Valor | Qué mide | Cambia con el tiempo |
| --- | --- | --- |
| Q (calidad) | Qué tan bien se fabricó o ensambló | No; fija salvo un proceso real |
| Condición | Desgaste, rotura, corrosión | Sí; baja con uso, sube con reparación |
| Material | De qué está hecho | No |
| Propiedades funcionales | Carga, filo, tolerancia, aislamiento, presión de rotura | Derivadas de piezas relevantes, diseño, Q y condición |

### Reglas confirmadas (modelo Haven & Hearth)

Decidido (confirmado 2026-09-10):

- Todo ítem, material, herramienta y componente tiene Q.
- La Q de un ensamblaje o reparación hereda de la Q de sus piezas.
- Una pieza muy mala arrastra el conjunto: cabeza de hacha Q70 con mango podrido Q10 da una herramienta muy penalizada.
- La habilidad del ejecutor es un techo blando sobre la Q de herramientas y materiales.
- Resultados excelentes (Q > 70) exigen habilidad alta, buena herramienta y buenos componentes a la vez.
- Mayor Q efectiva reduce el tiempo de la acción hasta un 50 %.
- Herramientas de Q alta se desgastan menos por uso.
- Mayor Q efectiva reduce fallos críticos: tornillos partidos, armas encasquilladas, tuberías reventadas.

### Fórmula

Conflicto: el prototipo usaba media geométrica y techo `sqrt(Q·S)`; la investigación del motor propone otra regla. Se prefiere la de la investigación. Propuesta:

- `Q_salida = media ponderada de Q de entradas`, limitada por `min(habilidad, Q_herramienta)`, más una pequeña dispersión aleatoria.
- Los pesos representan función, no masa.
- Q es continua, sin escalones. Escala candidata 0–100. Abierto: el prototipo usó también [1, 100+].
- Ejemplo: cabeza Q40 y mango Q90 con pesos 70/30 dan Q55 antes del límite.

Ejemplos del prototipo con techo `sqrt(Q·S)`, solo como referencia: novato S10 con bisturí Q90 rinde ≈30; cirujano S80 con cuchillo Q15 rinde ≈34.

### Límites de uso de Q

Propuesta:

- Q no es un multiplicador universal de todas las estadísticas.
- Cada propiedad declara qué pieza la gobierna. El filo depende de la cabeza, no de la Q media del hacha.
- Unos frenos inútiles no se compensan con asientos excelentes.
- Q escala propiedades de forma continua: una tubería Q80 revienta a más presión que una Q30. Investigación.
- Un cable Q60 soporta más carga antes de sobrecalentarse. Así la Q alimenta las redes eléctricas y de agua.
- Piezas ausentes o rotas se tratan explícitamente; el promedio no autoriza a funcionar un objeto incompleto.
- La Q real pertenece al objeto. El personaje solo ve su evaluación, según conocimiento e instrumento.
- Estudiar no sube la Q de ningún objeto. Decidido.

## Desmontaje, crafteo y reparación

- Desmontar devuelve componentes con su Q, menos una pérdida que baja con habilidad y Q de herramienta. Propuesta (investigación).
- No todo desmontaje es reversible: atornillado, soldado, pegado y consumido exigen procesos distintos. Propuesta.
- Cada proceso declara recuperación intacta, rotura y residuos. Propuesta.
- Ensamblar, desmontar, reparar y fabricar material base no son la misma receta invertida. Propuesta.
- Desmontar y volver a montar no sube la Q. Propuesta.
- Ningún ciclo fabricar/desmontar puede generar masa, piezas o mejoras gratis. Propuesta.
- Un refinado con pérdidas puede elevar una propiedad si su receta lo define. Propuesta.
- Reparar recupera condición y puede sustituir piezas; no deja el objeto como nuevo. Propuesta.
- La Q de una pieza de reparación decide cuánto dura el arreglo. Propuesta (investigación).
- Un mal montaje limita el conjunto sin bajar la Q intrínseca de cada pieza. Propuesta.
- El resultado y sus defectos se registran; consultar Q no vuelve a sortearlos. Propuesta.
- Que una pieza tenga receta no implica que un superviviente pueda hacerla a mano. Puede requerir industria. Propuesta.
- Construir muebles y estructuras usa los mismos objetos, más soportes, fijaciones y espacio. Propuesta.

### Contenido de una receta

Propuesta. Entradas y alternativas; cantidades y unidades; herramientas y capacidades; estación y condiciones; conocimientos; dificultad y habilidad; fases y tiempo; recursos consumidos; salidas, subproductos y pérdidas; reglas de Q y recuperación; interrupciones y resultados parciales.

## Inventario

- Separar tipo, ejemplar, estado, ubicación y cantidad o pila. Propuesta.
- Familias candidatas: ropa, contenedor, herramienta, consumible, recurso. Propuesta.
- Las capacidades del objeto (abrir, guardar, equipar, consumir, manipular, desmontar) definen sus acciones. Propuesta.
- No diseñar primero una cuadrícula de iconos e inventar reglas después. Propuesta.
- Masa del portador = cuerpo + ropa + contenido real de sus contenedores. Propuesta.
- La carga corporal se deriva siempre del contenido real, nunca de un valor copiado. Propuesta.
- Un ejemplar nunca se duplica ni desaparece al transferirse o interrumpirse. Propuesta.

## Contenedores

- La capacidad del recipiente es distinta de la capacidad física del portador. Propuesta.
- Al contenedor pertenecen: volumen, boca de acceso, compatibilidad de contenidos y límite estructural. Propuesta.
- Al portador pertenece lo que puede levantar y transportar, según su estado. Propuesta.
- Una mochila no pierde volumen porque su portador esté cansado. Propuesta.
- Contenedor sin dueño pertenece al mundo; con dueño, su contenido pesa sobre ese habitante. Propuesta.
- Ver un contenedor no revela todo su contenido. Propuesta.
- Abierto: contenedores anidados, inventario volumétrico y apilado.

## Ropa y equipo

- Capas de ropa al estilo CDDA: interior, media y exterior. Propuesta.
- La ropa aporta aislamiento (clo) y estorbo (encumbrance). Propuesta.
- La ropa puede tener talla; al adelgazar, un pantalón L queda flojo. Propuesta (investigación).
- Mochilas, cinturones y fundas: cuerpo rígido unido a un hueso, correas que siguen el cuerpo. Propuesta (investigación).

## Preguntas abiertas

- Escala final de Q y si se muestra exacta o como rango con confianza.
- Pesos por familia y si se aplica dispersión aleatoria en todas las recetas.
- Reglas de producción renovable (cultivos, nacimientos) en un mundo sin respawn.
- Cómo evitar que inspeccionar muchos objetos parecidos se vuelva tedioso.

## Fuentes del prototipo

- catalogo-items.md
- fundamentos-mundo-personajes-produccion-habitantes-objetos.md
- aprendizaje-especializacion.md
- interaccion-control-inventario.md
- base-funcional-habitantes-contratos-acciones.md
- arquitectura-sistemas.md
- engine-research.txt (investigación del motor, autoridad más reciente)
