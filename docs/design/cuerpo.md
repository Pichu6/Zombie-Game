# Simulación del cuerpo

Cada humano y cada zombi tiene un cuerpo simulado. Es un árbol de partes con capas de tejido, masas en kg y un metabolismo lento. Las heridas, la infección y la descomposición son estados de ese mismo cuerpo. El juego lee capacidades derivadas (moverse, manipular, ver, respirar), no órganos sueltos. La vista dibuja el estado con morphs, máscaras de textura y capas internas que se revelan con daño. El estado es la autoridad; la malla nunca guarda fisiología. Jugador y NPC usan exactamente la misma simulación.

Procedencia: **Decidido** = el usuario lo confirmó en el prototipo. **Propuesta** = idea técnica sin confirmar. **Abierto** = pregunta sin respuesta.

## Principios

| Regla | Estado |
| --- | --- |
| La anatomía humana real es la referencia principal de huesos, músculos y proporciones. | Decidido |
| Personas reales filmadas o fotografiadas son referencia primaria; otros juegos, secundaria. | Decidido |
| Variedad realista: cuerpos delgados, gordos, musculosos, distintos pechos, glúteos y caras. | Decidido |
| La diversidad corporal es gameplay: el cuerpo cambia con ejercicio y alimentación durante la partida. | Decidido |
| El creador fija el estado corporal inicial. Fuerza, resistencia y demás surgen de ese estado. | Decidido |
| Etiquetas como "flaco" o "fit" describen el estado actual. No son bonus fijos. | Decidido |
| Músculos se deforman, crecen y se atrofian. La grasa crece, se reduce y se deforma pasivamente. | Decidido |
| La grasa también afecta cara y papada, sin escalar la cabeza ni borrar rasgos. | Decidido |
| Capas: esqueleto animable, piel exterior con morphs, interior ligero revelado por heridas o putrefacción. | Decidido |
| No simular músculos ni órganos por vértice en tiempo real. | Decidido |
| El esqueleto anatómico es estructura del rig y también el resultado final visible de la descomposición. | Decidido |
| Tejidos blandos y algo translúcidos. Corazón y pulmones visibles laten y respiran según el estado. | Decidido |
| Distinguir cuerpos blandos de fornidos, enrojecimiento, vascularización y reparto de grasa. | Decidido (pedido) |
| Mujer y hombre con anatomía propia; nunca un hombre escalado con pecho añadido. | Decidido |
| Ojos pertenecen a la capa de órganos, no al esqueleto. | Decidido |
| Población finita; jugador y NPC tienen la misma simulación y capacidades. | Decidido |
| Reutilizar soluciones existentes cuando sirvan (bibliotecas, bases abiertas). | Decidido |

## Arquitectura

Flujo único: estado persistente → capacidades → acciones permitidas → pose y presentación.

- El estado vive en el núcleo de simulación, sin dependencia del renderer. **Propuesta** (investigación de motor).
- Cambiar cámara, descargar la vista o alejarse no altera metabolismo, identidad ni lesiones. **Decidido** (paridad y población finita).
- La vista de juego y la vista médica leen el mismo cuerpo. Nunca crean cuerpos independientes. **Propuesta**.
- Actualización por eventos (golpe, comida) o por tick lento; nunca cada frame. **Propuesta** (investigación).
- Detalle por distancia: cerca, todo; distrito cargado, tick lento; resto, estadística conservando masas e identidad. **Propuesta**.
- Conflicto: el prototipo construía el cuerpo jugable desde un atlas anatómico de adentro hacia afuera. La investigación lo limita a la vista médica. **Propuesta**: árbol de partes para jugar, atlas sólo para inspección.

## Árbol de partes

- Árbol de partes con IDs estables compartidos por rig, heridas, ropa, órganos y guardado. **Propuesta**.
- El prototipo usaba 17 regiones lógicas. Sirve como punto de partida. **Propuesta**.
- Cada región define: lado anatómico, articulación, control de deformación, superficie, tejidos y anclajes. **Propuesta**.
- Separar hueso visible, articulación biomecánica y control de deformación. No son lo mismo. **Propuesta**.
- El lado izquierdo/derecho se valida con referencia anatómica, nunca por sufijo de nombre. **Propuesta**.
- Diseñar IDs, unidades, articulaciones y anclajes del cuerpo entero desde el inicio. Extender geometría por regiones. **Propuesta**.
- Reservar cavidades y anclajes para órganos, heridas y deterioro desde el principio. **Propuesta**.

## Capas de tejido

Modelo por capas al estilo Dwarf Fortress: cada parte tiene piel, grasa, músculo, hueso y órganos donde corresponda. **Propuesta** (investigación).

| Capa | Datos por región | Efecto |
| --- | --- | --- |
| Piel | Integridad, suciedad, sangre, perfusión (color) | Barrera, infección, aspecto |
| Grasa | kg y espesor de reposo por depósito | Protege, pesa, aísla, ralentiza |
| Músculo | kg, volumen de reposo, entrenamiento, fatiga | Fuerza, resistencia |
| Fascia | Límites de desplazamiento | Impide grasa como líquido libre |
| Hueso | Integridad, fractura | Soporte, movimiento |
| Órganos | Estado funcional individual | Capacidades vitales |

- Los golpes se resuelven capa por capa: trayectoria, profundidad y tejidos afectados. **Propuesta**.
- La grasa ocupa también espacios internos; no es una cáscara uniforme. **Propuesta**.

## Masas y composición

Tres compartimentos excluyentes en kg húmedos: grasa, músculo y estructural (resto de tejidos). **Propuesta** (implementado y probado en el prototipo).

- Peso total y porcentajes se derivan. Nunca se guardan ni se reconvierten. **Propuesta**.
- El agua ya está incluida. No sumar la hidratación otra vez. **Propuesta**.
- Edición atómica: grasa ≥ 0, músculo ≥ 0, resto > 0, suma ≤ 1000 kg (límite técnico, no clínico). **Propuesta**.
- Altura: dominio de datos 0,1–4 m. Cambiar altura conserva los kg. **Propuesta**.
- La altura escala piel, rig e interior juntos. Escalar sólo la piel rompe articulaciones. **Propuesta**.
- Masa corporal y masa móvil son distintas. La carga transportada nunca se convierte en tejido. **Propuesta**.
- Metabolismo basal y superficie térmica usan masa corporal. El trabajo activo usa masa móvil con equipo. **Propuesta**.
- Al redistribuir por regiones, conservar la masa total. No contar kg dos veces. **Propuesta**.
- Sangrado, deshidratación y amputación deben descontar masa. El prototipo no lo hacía. **Propuesta**.
- Contenido digestivo con masa explícita si cuenta en el total. Nunca convertir kcal en litros. **Propuesta**.

## Músculo y grasa: forma frente a cantidad

| Fenómeno | Músculo | Grasa |
| --- | --- | --- |
| Cambio rápido de forma | Por longitud, contacto y activación; conserva volumen aproximado | Se aplasta, cizalla y recupera; pasiva |
| Cambio lento de cantidad | Hipertrofia o atrofia regional por carga, descanso y nutrición | Acumulación o movilización regional por balance energético |
| Consecuencia | Fuerza separada de activación, fatiga y entrenamiento | Masa, inercia y protección; nunca actuador |

- Doblar un codo no cambia la masa. Comprimir grasa no la elimina. **Propuesta**.
- Separar fatiga transitoria de atrofia persistente. **Propuesta**.
- Separar acondicionamiento y adaptación neuromuscular del tamaño muscular. **Propuesta**.
- No derivar fuerza sólo del volumen visible ni destreza de músculos grandes. **Propuesta**.
- El reparto de grasa (androide, ginoide) es un sesgo. No lleva ninguna región a cero. **Propuesta**.

## Metabolismo

Calorías, macronutrientes, hidratación y digestión. La grasa y el músculo cambian en días, al estilo SCUM. **Propuesta** (investigación).

- Pérdidas y ganancias se aplican directamente en kg. **Propuesta**.
- Todos los coeficientes del prototipo eran de autor, sin calibrar. Pasar pruebas no los calibra. **Abierto**.
- Ritmo cardíaco, ventilación y recuperación son números de estado. La animación de órganos los lee. **Propuesta**.
- No usar pulso como sustituto de frecuencia y profundidad respiratoria. **Propuesta**.
- Circulación como reserva, perfusión y rutas vasculares principales. El prototipo usaba 5000 ml globales. **Propuesta**.
- Nervios como conexiones de regiones a centros: capacidad motora y sensibilidad. **Propuesta**.
- Estómago: volumen, llenado y vaciado cambian levemente la panza, sin crear grasa. **Propuesta** (idea del usuario).
- La panza depende también de intestinos, gases, diafragma, pared y grasa. No escalar sólo el estómago. **Propuesta**.

## Capacidades

Capacidades al estilo RimWorld: moverse, manipular, conciencia, ver, respirar, bombear sangre. Se calculan desde las partes. La IA y la interfaz leen capacidades. **Propuesta** (investigación).

- Cumple la decisión de que fuerza y resistencia surgen del cuerpo. **Decidido** (principio) / **Propuesta** (método).
- Las fórmulas y la calibración siguen pendientes. **Abierto**.

## Trauma, cirugía, infección y descomposición

Ver [cuerpo-trauma.md](cuerpo-trauma.md).

## Del estado a la imagen

| Estado | Representación |
| --- | --- |
| Altura | Escala común de piel, rig e interior |
| Grasa y músculo por región | Morphs como deltas dispersos; correctivos por combinación |
| Hinchazón, gases, infección | Morphs regionales |
| Suciedad, sangre, grasa, sudor, mojado | Máscaras en UV común del cuerpo |
| Heridas superficiales | Máscara y material |
| Heridas profundas, amputación | Piezas de herida y cortes regionales preparados |
| Interior | Capa ligera con los mismos IDs, visible sólo por daño o putrefacción |
| Ritmos vitales | Fases de animación de órganos según el estado |

- Todos los cuerpos comparten una topología y un UV; cambian por morphs. **Propuesta** (ropa-wrap).
- El mapeo kg → morph es experimental. Registrar su dominio y avisar cuando satura. **Propuesta**.
- La vista limita sólo su representación; nunca recorta los kg. **Propuesta**.
- Correctivos visuales fuera del estado de masa. **Propuesta**.
- Translucidez por dispersión subsuperficial, no transparencia uniforme. Distinto en piel, tela, hueso y órgano. **Propuesta**.
- Capas ocultas no actualizan deformación. Ocultar no libera memoria: cargar bajo demanda. **Propuesta**.
- Dinámica de tejido blando (física con unidades SI y gravedad) sólo cerca de cámara. **Decidido** (dirección) / **Propuesta** (sólo nivel cercano).
- Los coeficientes de tejido del prototipo no estaban calibrados. **Abierto**.

## Hombro y movimiento

- La escápula y la espalda se mueven con hombro y brazo. **Decidido**.
- Tres familias escapulares separadas: protracción/retracción, elevación/depresión, rotación al subir el brazo. **Propuesta**.
- Movimiento por animación e IK compatibles con articulaciones y capacidades. **Propuesta**.
- Músculos que generan movimiento por fuerzas: fuera de alcance. **Propuesta**.

## Mujer y variedad

- Lógica compartida; datos propios por base: pelvis, tórax, longitudes, reposos, inserciones, depósitos. **Propuesta**.
- Correspondencia de vértices dentro de cada familia de morphs, no entre hombre y mujer. **Decidido**.
- Sin multiplicadores universales de fuerza por sexo. Variación individual continua, no un booleano. **Propuesta**.
- Tejido mamario y órganos específicos como componentes posteriores. **Propuesta**.

## Abierto

- ¿El sistema héroe del juego es el cuerpo o la infraestructura?
- ¿Cuánta anatomía interior se ve en juego frente a la vista médica?
- Fórmulas y calibración de metabolismo, capacidades y tejido.
- Reglas de la infección zombi y si el zombi conserva identidad del habitante.

## Fuentes del prototipo

bases-simulador-corporal.md, bases-simulador-corporal-arquitectura-regional.md, bases-simulador-corporal-base-femenina.md, bases-simulador-corporal-contratos-masas.md, bases-simulador-corporal-tejidos-esqueleto.md, decisiones-tejidos-esqueletos.md, decisiones-generador-anatomia.md, evaluacion-humano-costo-realismo.md, conocimiento-produccion-personajes-reposo-movimiento.md, conocimiento-produccion-personajes-superficie-evidencia.md, movimiento-capas-rendimiento.md. Más: engine-research (investigación de motor) y research/ropa-wrap.md.
