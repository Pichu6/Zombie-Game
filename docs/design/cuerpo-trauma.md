# Cuerpo: trauma, cirugía, infección y descomposición

Parte de [cuerpo.md](cuerpo.md). Las heridas viven en el árbol de partes como lista por parte, con capas afectadas. Sangrado, dolor e infección salen de cada herida y bajan capacidades. La cirugía de campo resuelve lo que el vendaje no puede, con ruido y riesgo. Los cadáveres no desaparecen: se pudren por fases, atraen moscas, contaminan con olor y terminan como esqueleto visible. Casi todo es Propuesta; sólo la descomposición hasta el esqueleto visible está Decidida.

Procedencia: **Decidido** = el usuario lo confirmó. **Propuesta** = sin confirmar. **Abierto** = pregunta sin respuesta.

## Trauma

- Lista de heridas por parte, cada una con tipo, trayectoria, profundidad y capas. El prototipo guardaba una sola herida activa por región. **Propuesta**.
- Tipos mínimos: rasguño, laceración, mordida, bala, fractura, quemadura, amputación. **Propuesta**.
- Cada herida acumula sangrado, dolor e infección. **Propuesta**.
- Cadena completa a cerrar primero: impacto → lesión → efecto → limitación de acción → presentación → guardado. **Propuesta**.
- Daño legible de un vistazo, como en Project Zomboid. **Propuesta**.

## Cirugía de campo

Todo **Propuesta**.

| Procedimiento | Requisito | Riesgo |
| --- | --- | --- |
| Sutura de laceración profunda | Hilo y aguja curva esterilizados (fuego o alcohol) | Sin anestesia: dolor 100 % y gritos de 75–80 dB que atraen zombis |
| Amortiguar gritos | Morder cinturón o paño grueso | Baja el ruido a nivel seguro |
| Extracción de bala o esquirla | Pinzas desinfectadas | Si la bala tapaba una arteria, hemorragia masiva súbita |
| Hemorragia súbita | Torniquete o compresión inmediata | Muerte si no se controla |

- Balas lentas alojadas sin extraer: necrosis e intoxicación por plomo. **Propuesta**.
- Cirugía libre con cortes arbitrarios es otro producto. Si importa, sólo en un modo médico separado. **Propuesta**.

## Fármacos

- Insulina, antibióticos reconstituidos y antídotos pierden efecto tras 24 h a más de 20 °C. **Propuesta**.
- Cadena de frío con heladera de 12 V en base o vehículo. **Propuesta**.
- Pastillas secas pierden 10 % de eficacia por año en ambiente húmedo. **Propuesta**.

## Infección

- Infección bacteriana y septicemia progresan desde heridas abiertas, como condición del mismo sistema. **Propuesta**.
- La hinchazón por infección mueve morphs; la ropa la sigue por wrap. **Propuesta** (ver ropa-wrap).
- Mecánica de la infección zombi (contagio, tiempos, conversión) no está definida en las fuentes. **Abierto**.

## Muerte y descomposición

Los cadáveres, humanos o zombis, no desaparecen. Todo **Propuesta**.

| Fase | Tiempo | Efecto |
| --- | --- | --- |
| Fresco | 0–24 h | Rigidez: articulaciones bloqueadas temporalmente |
| Hinchazón por gases | Días 2–5 | Abdomen hinchado; un disparo o corte al torso libera gases y salpicaduras infecciosas |
| Licuefacción | Día 6+ | Tejidos blandos se disuelven y exponen el esqueleto |

- Moscas atraídas a más de 100 m. Su zumbido revela cuerpos ocultos. **Propuesta**.
- Larvas arruinan comida destapada de la misma habitación en pocas horas. **Propuesta**.
- Olor (miasma) en el refugio: náuseas, vómitos, menos apetito, peor ánimo. Viaja por el grafo de habitaciones. **Propuesta**.
- Fosa común con pala: hasta 10 cuerpos, cuesta estamina y tiempo, elimina el olor. **Propuesta**.
- Pira con leña y combustible: destruye patógenos; humo negro visible a kilómetros. **Propuesta**.
- La descomposición avanza por capas hasta el esqueleto anatómico visible. **Decidido**.

## Fuentes del prototipo

cirugia-trauma-descomposicion.md, evaluacion-humano-costo-realismo.md, decisiones-generador-anatomia.md, conocimiento-produccion-personajes-superficie-evidencia.md. Más: engine-research (investigación de motor) y research/ropa-wrap.md.
