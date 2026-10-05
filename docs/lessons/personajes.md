# Lecciones de producción de personajes

El prototipo gastó la mayor parte de su esfuerzo en personajes: mallas generadas por IA, rigs heredados, hombros y axilas, ropa y anatomía encajada a mano. Casi todos los fallos vinieron de mezclar responsabilidades: forma en reposo, rig, pesos, correctivos, composición y prenda. Estas lecciones resumen síntoma, causa y qué hacer en el motor nuevo. Las cifras describen assets concretos del prototipo; no son tolerancias universales. La regla de fondo: un cuerpo base con una topología, un UV y un esqueleto, variado por morphs; ropa atada al cuerpo; y revisión visual desde muchas vistas.

Procedencia: la columna "Qué hacer" es **Propuesta** salvo que diga **Decidido** (confirmado por el usuario).

## Mallas generadas por IA y fuentes

| Síntoma | Causa | Qué hacer |
| --- | --- | --- |
| Al engordar aparecían cortes en torso y short. | La malla IA era torso, short, piernas y cara desconectados; la prenda solapaba la piel. No había cuerpo debajo. | Inventariar componentes, bordes abiertos y solapes antes de crear morphs. No usar mallas IA como cuerpo base. |
| Mezclar variante normal y gorda perdía volumen y rompía axilas. | Cada generación IA tenía otra topología; se emparejó por proximidad. | Una topología común; variantes sólo por morphs. **Decidido** (misma topología e identidad entre variantes). |
| Remera IA con aspecto de arpillera, cuello irregular y mangas estiradas. | Geometría y textura malas; pesos y morphs correctos no lo compensan. | No usar prendas IA como prendas deformables. Patrón propio con aberturas limpias y material mate. |
| Regenerar un personaje por cada peso o cara. | Topologías incompatibles entre sí. | Nunca. Un cuerpo base y morphs. |
| Exportación anatómica cargaba bien pero faltaban vértebra, pómulo y dientes. | Filtro por nombre de pieza omitió partes. | Clasificar por ID explícito, auditar lo excluido y revisar región por región. |
| Pie de otra fuente no cabía; escalarlo lo dejaba en 17 cm. | Piezas reales de fuentes distintas no comparten articulación. | Usar conjuntos regionales coherentes. No escalar huesos para encajar. Medir la unión. |
| Cráneo de tomografía con tamaño alterado. | Cortes apilados sin metadatos físicos de escala. | Exigir espaciado físico. Si faltan paredes o dientes, no darlo por cabeza sana. |
| Atlas forzado dentro de la piel con escalas no uniformes. | Se intentó conservar una mala silueta deformando huesos. | La anatomía guía la piel; corregir la piel. **Decidido**. Un atlas es una persona, no una norma. |

## Rig, reposo y retarget

| Síntoma | Causa | Qué hacer |
| --- | --- | --- |
| Pies girados ~85° con las plantas al frente. | El cambio entre matrices de reposo trasladó el roll técnico como torsión. | Rotación mínima entre segmentos, separada del roll. |
| El cuello desaparecía al mover un pivote. | Piel del trapecio con más de 90 % de peso cervical; subió 6–7 cm. | Tras cambiar un reposo, invalidar pesos, correctivos y contactos ligados al anterior. |
| Clavícula y escápula se separaban 25 mm al subir brazos. | El rig conservaba huesos pero no la conexión articular. | Ancla común calculada en reposo y corrección de traslación tras rotar. |
| Radio y cúbito giraban juntos; codo 12 mm fuera de eje. | Un solo control para el antebrazo; pivote heredado mal puesto. | Radio independiente con la mano. Separar orientación, centro articular real y origen técnico. |
| Rotar sobre un pivote desplazado movía la articulación. | Se aplicaba sólo el cuaternión. | Rotar alrededor de un punto exige compensar traslación. |
| Ancla sin deriva pero cúbito dentro del húmero al flexionar. | Una deriva cero de un punto no impide penetración. | Medir profundidad de cruce durante el movimiento. |
| Hombros en V y piernas invertidas al posar. | El motor esperaba rotación local completa, no un delta de la herramienta de autoría. | Componer reposo por delta. Test: delta cero conserva el reposo. Revisar posiciones globales. |
| Brazos cruzados al reproducir clips. | Delta de T-pose aplicado sobre un rig en A-pose. | Retarget en un marco común; comparar direcciones animadas con la fuente. |
| Codo y descendientes desalineados entre capas. | Se copiaban rotaciones entre rigs con reposos distintos. | Transferir el delta completo (actual por reposo inverso), con traslación de pivote. |
| Rodilla en punta; pantorrilla se iba atrás al engordar. | Pivotes delante del volumen; el morph de grasa movía el centro de secciones. | Revisar centros articulares desde ambos perfiles. La composición no mueve centros articulares. |
| Sentadilla que flota. | Ángulos fijos sin apoyo. | Medir apoyo en toda la trayectoria; compensar pelvis o IK. |
| Hombro artificialmente cuadrado. | Se aplicó la orientación del control como rotación del tejido en reposo. | Orientación del control y forma del tejido son cosas distintas. |
| Lesiones del lado equivocado posibles. | En un asset ".R" era la izquierda anatómica. | Validar lados con referencia anatómica, nunca por sufijo. |
| Músculos del cuello no seguían cráneo ni mandíbula. | Rig sin hueso de mandíbula; anclajes con 49 % de influencia ajena. | Mandíbula en el rig desde el inicio. Pesos de anclaje por identidad muscular. |
| Tendones del pie estirados al caminar. | Faltaba un hueso en la lista permitida; los vértices caían a la pelvis. | Prohibir caídas silenciosas a otro hueso. Prueba de pesos por región y lado. |

## Hombro y axila

| Síntoma | Causa | Qué hacer |
| --- | --- | --- |
| Hombro defectuoso al elevar, con poca o mucha grasa. | No había escápula independiente; sólo clavícula y brazo. | Escápula y espalda se mueven con el brazo. **Decidido**. |
| Un solo control escapular no basta. | Hay tres familias: protracción/retracción, elevación/depresión, rotación al subir brazo. | Probarlas por separado antes de combinarlas. Revisar también el descenso. |
| Subir el ángulo de clavícula de 16° a 32° creó otra salida de escápula. | Se buscaba un número, no coherencia articular. | Corregir trayectoria articular y tejidos regionales, no más ángulo ni huesos menores. |
| Envolvente para cubrir la escápula produjo "alas" de 51 mm. | Un correctivo tapaba un fallo articular inflando tejido. | Arreglar primero la articulación; el soporte superficial quedó en 4 mm. |
| Axila rellenada se veía falsa. | Una axila elevada real mantiene un hueco. | No rellenar el hueco. Pliegue pectoral y pliegue dorsal son transiciones distintas. |
| Hombro parecía piezas pegadas. | Mezcla de geometría, normales y material sin relieve ni dispersión. | Aislar geometría, normales y textura. Una sombra no prueba un agujero. |
| Definición muscular de un sujeto delgado aplicada al gordo. | Referencia de otra constitución. | Referencias de la misma constitución; personas reales primero. **Decidido**. |

## Skinning, correctivos y tejido

| Síntoma | Causa | Qué hacer |
| --- | --- | --- |
| Correctivos automáticos agrandaron deltoides y axilas. | Conservar volumen matemático no conoce inserciones ni pliegues. | Comparar rig con y sin correctivo en las mismas vistas. Retirarlo si empeora alguna. |
| Región plegada al mezclar dual quaternion y lineal. | Mezcla espacial de dos métodos. | Un método por región o parche con borde fijo y transporte del detalle. |
| Puntas en los pesos. | Gradientes calculados por islas pequeñas de malla. | Agrupar por identidad muscular antes de suavizar. |
| Ombligo plegado al flexionar. | Anillos vecinos con pesos distintos. | Continuidad local de pesos. Comparar caso por caso, no totales. |
| Short se movía con los brazos. | 10 de 280 vértices tenían influencia de brazo; se movía 3,5 cm. | Mover una cadena aislada y comprobar que lo demás queda quieto. |
| Resorte de tejido empujaba en dirección errónea (20 mm sobre 10 pedidos). | Conversión en el marco del hueso y no del padre. | Medir la posición mundial efectiva, no el estado del resorte. |
| "Activación" muscular que no movía nada. | Sólo endurecía resortes de piel. | Rigidez no es fuerza. Inventariar tejido por función, no por número de mallas. |
| Subcutáneo despegado del músculo. | Lámina separada por aire. | El subcutáneo ocupa su volumen hasta el soporte. |
| Cerrar un hueco proyectando vértices invirtió 65 normales. | Proyección independiente por vértice. | Transporte suave del parche; verificar normales y áreas. |
| Cabeza delgada sobre cuerpo gordo. | La cara se congeló para conservar identidad. | Depósitos faciales regionales que protegen ojos, nariz, labios y cráneo. **Decidido** (grasa en cara y papada). |
| Grasa extrema sólo en abdomen o sólo en cadera. | Reparto regional excluyente llevó regiones a cero. | El reparto es un sesgo; la grasa compartida sigue en todas partes. |
| Morphs nuevos volvían a atravesar la piel. | Se añadieron después del ajuste. | Correctivos por combinación de composición, interpolados con los mismos pesos. |
| Física de tejido no arreglaba la pose. | La física no repara la cinemática. | Separar soporte, inercia y visualización; corregir cinemática primero. |

## Ropa

| Síntoma | Causa | Qué hacer |
| --- | --- | --- |
| Ropa rígida; cada remera fue un proyecto (9 variantes, 18 correctivos de pecho). | La prenda conocía el esqueleto pero no los morphs ni correctivos del cuerpo. | Wrap en GPU tras morph y skinning (ver research/ropa-wrap.md). |
| Vértices fuera del cuerpo pero caras que lo cortan. | Se medían vértices, no caras. | Muestrear caras; revisar tres cuartos y pecho máximo. |
| Escalón en el pecho de la remera. | Caída de tela desde un solo punto del pecho. | Patrones por sexo y superficie frontal continua. |
| Pliegues rotos tras un correctivo global. | Mezclaba conflictos incompatibles (manga contra torso, bajo contra muslo). | Corregir por zona. No inflar toda la prenda. |
| Mangas desplazadas 67–82 mm al bajar brazos. | El ajuste de caída incluía influencias de brazo. | Excluir influencias que la prenda no debe seguir. |
| Short separado del abdomen al posar. | Estanqueidad de una malla no prueba su ajuste con otra. | Separar deformación por composición, ajuste entre piezas y skinning. |
| Prenda base con textura cambiable no daba bolsillos creíbles. | Una textura no cambia silueta; UV sin lugares estables. | Prenda base con UV de molde y módulos de detalle atados a ella. |
| Cruces milimétricos perseguidos a mano bajo la ropa. | El prototipo prohibía ocultar piel cubierta. | Conflicto con la investigación: ocultar piel cubierta por región. **Propuesta** (pendiente de decisión). |
| Prenda que pasaba pruebas fue rechazada en el pecho. | Pruebas funcionales y aprobación visual son cosas distintas. | Registrar ambas por separado. |

## Revisión y evidencia

| Síntoma | Causa | Qué hacer |
| --- | --- | --- |
| Fallos invisibles de frente. | Vista única. | Ambos perfiles, espalda, ambos tres cuartos, rotación continua. **Decidido** (exigido por el usuario). |
| Fallos con grasa baja, no sólo alta. | Se probaban sólo extremos. | Composiciones extremas e intermedias, misma cámara, luz y escala. |
| Métrica mejorada ocultó 399 caras invertidas. | La métrica medía cobertura, no orientación. | "Carga", "pesos suman uno", "morph cambia" y "se ve bien" son pruebas distintas. |
| Diferencias falsas de 39 µm entre autoría y motor. | Pesos cuantizados en 16 bits en un lado; precisión float32. | Comparar con la misma cuantización y tolerancias realistas. |
| Animación aparentaba no reproducirse. | Muestreo cada 3 s casi igual al período de 3,14 s. | Muestrear bastante más rápido que el período. |
| Capturas con etiquetas de otra pose. | Controles no sincronizados al cambiar por código. | La captura registra pose, vista, composición y tiempo. |
| Translucidez evaluada en un modo sin dispersión. | Renderer distinto al del juego. | Validar material en el renderer real. |
| Clip importado sin errores pero sin movimiento. | La importación descartó curvas. | Verificar excursión de curvas y reproducción real. |

## Rendimiento

| Síntoma | Causa | Qué hacer |
| --- | --- | --- |
| 150 ms por cuadro al flexionar el codo; 3,8 ms tras corregir. | Sin LOD, validación repetida por consulta, dos relojes y dos esqueletos. | Un esqueleto compartido por capas, un reloj, validar una vez por pose, LOD real. |
| Capas anatómicas de ~1,6 M triángulos frente a 40 k del cuerpo exterior. | Interior completo siempre cargado. | Interior ligero, cargado bajo demanda. Ocultar no libera memoria. |
| Copias de código en carpetas de assets rompieron la carga. | El importador escaneaba respaldos. | El importador lee sólo rutas declaradas. |

## Costo frente a realismo

| Síntoma | Causa | Qué hacer |
| --- | --- | --- |
| Iteraciones muy caras y lentas. | Piel construida sobre un atlas; cada pose genera contactos nuevos; criterios mezclados. | Base exterior estable más simulación e interior reducidos. |
| Ningún cierre claro de hitos. | Anatomía, cruces, naturalidad y calibración evaluadas a la vez. | Criterios observables por separado: silueta, cara, movimiento, ropa, herida. |
| Explosión de casos de prueba. | Producto cartesiano de todos los sliders. | Extremos, intermedios y combinaciones problemáticas. Ampliar una dimensión por vez. |
| Capas nuevas sin uso jugable. | Se añadían por inercia. | Añadir órganos o regiones sólo si cambian una decisión o una imagen que importa. |

## Fuentes del prototipo

conocimiento-produccion-personajes.md, conocimiento-produccion-personajes-superficie-evidencia.md, conocimiento-produccion-personajes-anatomia-tejidos.md, conocimiento-produccion-personajes-reposo-movimiento.md, conocimiento-produccion-personajes-soporte-rigs.md, personajes-realistas.md, evaluacion-humano-costo-realismo.md, movimiento-capas-rendimiento.md, decisiones-generador-anatomia.md, decisiones-tejidos-esqueletos.md, bases-simulador-corporal-base-femenina.md. Más: research/ropa-wrap.md y engine-research (investigación de motor).
