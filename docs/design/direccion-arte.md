# Dirección de arte y producción visual

El objetivo visual es realismo con animación fluida y fisiología legible. Primero se lee silueta, postura y función; después el detalle. Todo lo que se ve corresponde a un estado real de la simulación: una textura no inventa un sistema. Personajes, ropa, ítems, animaciones y vistas de análisis comparten proporciones, materiales, esqueleto y reglas de luz. La producción avanza con puertas de revisión: un concepto atractivo no es una malla animable.

Etiquetas: **Decidido** (el usuario lo confirmó en el prototipo), **Propuesta** (no confirmado), **Abierto** (pregunta pendiente).

## Decisiones de dirección

| Regla | Estado |
| --- | --- |
| Realismo y animación fluida. Reemplaza la propuesta anterior de "realismo simplificado". | Decidido |
| Fisiología legible, consistencia y evolución corporal visibles. | Decidido |
| Consistencia entre imágenes guía, personajes, animaciones, ítems y vista de análisis. | Decidido |
| Análisis corporal claro e integrado: transparencia, inspección puntual, señales del estado simulado. | Decidido |
| Ítems con icono, representación en el cuerpo y referencias a modelos y materiales reales. | Decidido |
| No cambiar a semirrealismo para justificar un prototipo. | Decidido |
| Época, región, vestuario cultural, tono y violencia gráfica. | Abierto |
| Densidad de población visible y alcance gráfico. | Abierto |
| No deducir ambientación militar por referentes o por un ítem suelto. | Propuesta |

## Principios visuales

Propuesta, salvo indicación.

1. Silueta, postura y función antes que detalle de superficie.
2. Dimensiones, materiales y mecanismos plausibles. El desgaste tiene causa visible.
3. Vista normal y vista de análisis son distintas. Piel y ropa no brillan fuera del escáner.
4. Mostrar solo estados que existen en la simulación.
5. Reutilizar proporciones, materiales, esqueleto y luz dentro de cada familia de assets.

Evitar: ruido de costuras, suciedad uniforme, todo con aspecto plástico, proporciones heroicas por defecto, adornos tecnológicos sin función, texto generado dentro de imágenes.

## Personajes

| Regla | Estado |
| --- | --- |
| Todos los cuerpos comparten topología, UV y esqueleto; cambian por morphs. | Propuesta |
| Altura, estructura ósea, músculo y grasa son ejes separados con combinaciones válidas y correctivos. | Propuesta |
| No extrapolar morphs sin límites (el prototipo deformó hombros y pecho así). | Propuesta |
| Musculatura por volumen y movimiento, sin surcos exagerados. | Propuesta |
| Cuerpo, ropa, pelo y equipo separados; la ropa nunca es una región pintada del cuerpo. | Propuesta |
| Comparar personajes con la misma cámara y luz de revisión. | Propuesta |
| Revisar anatomía con ropa interior neutra ajustada. | Propuesta |
| Cuerpo base de prueba: adulto de 1,78 m; no fijar kilos a partir de una imagen. | Propuesta |
| Grasa, suciedad, sudor y sangre en máscaras de textura, no en geometría. | Propuesta |

Matriz de composición a revisar con la misma identidad y altura: delgado, medio, musculoso, adiposo, musculoso con grasa. Mirar abdomen, pecho, pelvis, cuello y extremidades. Propuesta.

### Ropa que sigue al cuerpo

Investigación del motor; prefiero esto a los ajustes por prenda del prototipo. Propuesta.

| Opción | Método | Uso |
| --- | --- | --- |
| A. Hornear morphs | Copiar cada morph del cuerpo a la prenda como shape key | Primer paso; costo de memoria por morph |
| B. Wrap en tiempo real | Cada vértice de prenda se ata a la superficie del cuerpo y se recoloca en GPU | Escala a cualquier morph; capas con grosor mínimo |
| C. Huesos de contorno | Huesos de vientre, pecho, cadera, muslo escalados por grasa y músculo | Primera pasada y LOD lejanos |

| Ítem | Método |
| --- | --- |
| Remeras, pantalones, ropa interior | A o B, ajustado |
| Abrigos, faldas, camisas sueltas | A o B; tela simulada solo cerca de cámara |
| Mochilas, cinturones, fundas | Rígido atado a hueso; correas con A o B |
| Placas y cascos | Rígido, tallas S, M, L |

- Las prendas tienen talla: si el personaje adelgaza, el pantalón L queda flojo. Propuesta.
- Capas de ropa del GDD: piel, ropa interior, prendas base, exteriores/armadura, equipo de carga, accesorios. Propuesta.
- La ropa cerrada oculta la piel que cubre, salvo heridas o vista anatómica. Abierto.

### Lámina de referencia

Propuesta. Frente, perfil y espalda del mismo diseño en pose A, cuerpo entero, manos visibles, pies alineados, escala constante. Fondo gris neutro, luz difusa, perspectiva mínima. Una vista tres cuartos aparte.

- Bolsillos, correas, cicatrices y cierres en el mismo lado en todas las vistas.
- Si dos vistas se contradicen, resolver antes de modelar. Una imagen generada es referencia, no medida.
- Cada candidato registra ID, prompt o fuente, referencias, fecha y estado (propuesto/aprobado).

## Materiales

Propuesta.

| Familia | Lectura | Regla |
| --- | --- | --- |
| Piel | Variación suave, volumen natural | Sin brillo plástico ni contraste excesivo |
| Tela | Mate, pliegues por tensión y gravedad | Costuras y desgaste a escala |
| Cuero y goma | Respuesta distinta de la tela | Diferenciar rugosidad y bordes, no solo color |
| Metal | Brillos localizados, construcción legible | Metalicidad solo en partes metálicas |
| Plástico | Volumen y acabado propios | No igual que metal pintado |

- Dimensiones visuales en metros. Masa y protección vienen de la simulación, nunca del aspecto. Propuesta.
- La calidad de fabricación puede verse en costuras, ajuste y acabado. Propuesta.
- No codificar calidad ni rareza por color. Propuesta.

## Ítems e iconos

- Cada ítem: ID estable, referencia visual, icono, modelo (o pendiente), materiales reales, puntos de montaje, dimensiones, piezas móviles o reemplazables. Propuesta.
- Icono legible a 32 y 64 px; encuadre y margen uniformes; fondo transparente; luz desde arriba a la izquierda. Propuesta.
- Tres cuartos por defecto; lateral si comunica mejor la función. Probar sobre fondo claro y oscuro. Propuesta.
- Sin texto pequeño en el icono. Propuesta.
- Icono, modelo equipado y modelo en el mundo salen del mismo diseño. Propuesta.

## Animación

- Prioridad: peso, apoyo y transiciones legibles. Propuesta.
- Pies sin deslizar a velocidad de referencia; rodillas y codos coherentes; manos sin atravesar equipo. Propuesta.
- Respiración sutil en reposo; sin oscilación de cámara para comunicar cada estado. Propuesta.
- Primera familia: reposo, caminar, correr, sentarse, levantarse, acostarse. Propuesta.
- Comer y beber exigen contacto real de mano y objeto. Propuesta.
- Cada clip documenta pose inicial y final, duración, bucle, velocidad y origen del desplazamiento (raíz o juego, sin mezclar). Propuesta.
- Locomoción separada de capas aditivas (respiración). Respuestas a lesión y agotamiento ligadas a datos reales. Propuesta.
- Estados que necesitarán animación: cojera por lado, herida de torso, tambaleo por pérdida de sangre, desmayo, vendar, torniquete, jadeo. Propuesta.
- Animaciones contextuales ante paredes, esquinas y puertas en pose de combate. Decidido.
- IK y motion matching solo ante problemas concretos de contacto o transición. Propuesta.

## Vista de análisis corporal

| Regla | Estado |
| --- | --- |
| Hover con transparencia hacia el interior; overlays por teclas; el daño ilumina localmente la zona. | Decidido |
| Separar diagnóstico médico de configuración de ítems. | Decidido |
| Evitar regiones cuadradas y columnas aisladas en la UI de análisis. | Decidido |
| Cuatro áreas: estado general, zona inspeccionada, equipo, acciones. | Propuesta |
| Capas: base, músculo, hueso, vascular, nervios, todo. | Propuesta |
| Huesos, músculos y vasos comparten posición y escala; sin dobles extremidades. | Propuesta |
| Dolor, infección y fractura con símbolos distintos; nunca solo color. | Propuesta |
| No mostrar fatiga por músculo ni coordenada exacta de lesión si la simulación no la tiene. | Propuesta |
| Anatomía real en la vista médica; el cuerpo jugable se simula como árbol de partes. | Propuesta |

Paleta de partida tomada de la UI del prototipo. Propuesta; revisar contraste.

| Uso | Color | Complemento |
| --- | --- | --- |
| Fondo | #0B1016 | Jerarquía por espacio |
| Texto principal | #D1EFE8 | Tamaño legible |
| Secundario | #AEC3C7 | No esconder datos por opacidad |
| Inspección / selección | #66C7CC | Punto, contorno o rótulo |
| Atención local | #F2B080 | Nombre del hallazgo |
| Sangrado / alarma | #E75A6A | Símbolo y texto |

## Mundo y legibilidad de sistemas

- En tercera persona los sistemas se ven en el mundo: ventanas oscuras, caños que silban, charcos bajo puertas. Propuesta.
- En vistas altas, capas superpuestas de electricidad, agua y presión muestran sectores muertos y salas inundadas. Propuesta.
- Agua de sala como plano que sube, con salpicaduras en escaleras y puertas. Propuesta.
- Niebla: color visible, gris recordado, negro desconocido. Luz e información son conceptos separados. Propuesta.
- Referencias a establecer: un exterior y un interior, día y noche, con persona y objeto de escala. Propuesta.

## Flujo con puertas de revisión

Propuesta.

| Puerta | Qué se aprueba |
| --- | --- |
| A | Dirección visual con láminas; distinguir realismo de nivel de detalle |
| B | Cuerpo base en gris: clavícula, tórax, pelvis, codo, rodilla, manos, pies |
| C | Estados corporales sobre topología común, combinaciones y correctivos |
| D | Deformación: brazos arriba, codo, giro de torso, sentadilla, sentado |
| E | Caminata corta con arranque, giro y parada; luego en toda la matriz corporal |
| F | Piel, pelo y ropa separada bajo luz neutra, exterior e interior |
| G | Escena pequeña de juego y análisis con el mismo asset; medir rendimiento |

Criterios de aceptación (Propuesta):

- Panel comparativo con idéntica cámara y luz, frente y perfil.
- Combinaciones corporales intermedias y extremas, no solo deslizadores aislados.
- Video de 10–15 s con apoyos, giro y flexión; sin saltos de volumen ni intersecciones.
- Textura de ropa no cuenta como prenda; pelo sin efecto casco a distancia de juego.
- Prueba técnica y revisión artística se registran por separado; si falla una, sigue siendo prototipo.
- No fijar presupuestos de polígonos o texturas sin cámara y escena representativas.

Primera secuencia visual (Propuesta): misma instalación subterránea, mismo habitante, misma luz en cuatro encuadres. Planta con techo cortado, habitante seleccionado con ruta, seguimiento junto a una puerta, tercera persona en el pasillo. Una quinta vista abre el análisis corporal.

## Fuentes del prototipo

- wiki/guia-de-arte.md
- wiki/bases-produccion-visual.md
- wiki/laboratorio.md
- wiki/decisiones-experiencia-control.md
- GDD_Survival_3D.md (secciones 4, 15, 19)
- art-direction/multi-team-blockout/README.md
- engine-research.txt
