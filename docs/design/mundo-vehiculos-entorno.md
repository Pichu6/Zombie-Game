# Mundo de supervivencia: vehículos, logística y entorno espacial

Edificios y vehículos se recorren a escala 1:1, sin pantallas de carga. Los vehículos son herramienta, refugio móvil y transporte de botín pesado. La percepción depende de lo que el personaje ve y oye, no de lo que muestra la cámara. Este doc completa `mundo-supervivencia.md`. Las redes eléctricas y de agua están en `infraestructura.md`.

Etiquetas: **Decidido** (la fuente dice que el usuario lo confirmó), **Propuesta** (no confirmado), **Abierto** (pregunta pendiente).

Nota general: ninguna fuente registra confirmación explícita del usuario para estos ítems. Todo es Propuesta salvo que se indique.

## Vehículos: física y daño

Conflicto: el prototipo usaba suspensión por raycast de un addon. La investigación indica Jolt Physics para física y colisión. Se adopta Jolt. Propuesta.

| Mecánica | Regla | Estado |
| --- | --- | --- |
| Suspensión | Por rueda: resorte, amortiguador, fricción longitudinal y lateral separadas | Propuesta |
| Tracción | Delantera, trasera o 4x4; manual o automática; ABS | Propuesta |
| Centro de masa | Se mueve con la carga; mala estiba o techo cargado vuelca en curvas rápidas | Propuesta |
| Daño por choque | Por componente: radiador perforado recalienta, alternador roto no carga, neumático reventado, dirección torcida | Propuesta |
| Muro de carga u hormigón | Frena el vehículo en seco; latigazo y trauma torácico a ocupantes | Propuesta |
| Tabiques, puertas, muebles | Se astillan y quedan como escombro transitable | Propuesta |
| Hielo | Derrapes en invierno | Propuesta |

Conflicto: deformación libre de chasis y paredes no encaja con el presupuesto. La investigación propone piezas prefracturadas con chequeo de conectividad. Propuesta.

## Interiores habitables (casas rodantes y camiones)

| Modo | Regla | Estado |
| --- | --- | --- |
| Estacionado | Motor apagado y freno de mano: se corta techo y paredes y se camina adentro | Propuesta |
| En marcha | Los ocupantes usan el marco local del chasis; aceleración y curvas aplican inercia relativa sin temblores | Propuesta |
| Puertas | Abrir conecta con el exterior: horda, fuego o gas entran de inmediato | Propuesta |

| Subsistema | Detalle | Estado |
| --- | --- | --- |
| Red de 12 V | Baterías de ciclo profundo separadas del motor; luces, bomba, heladerita para remedios y carne | Propuesta |
| Solar | Paneles flexibles en el techo; carga diurna | Propuesta |
| Toma externa 220 V | Enchufar a generador o poste al acampar; es un nodo de la red eléctrica | Propuesta |
| Agua potable | Tanque de 100–200 L con bomba de 12 V; beber y lavarse el olor | Propuesta |
| Aguas grises | Hay que vaciarlas o rebalsan | Propuesta |
| Cocina | Garrafa de GLP exterior; comida caliente mejora la moral | Propuesta |
| Calefacción | Calefactor diésel estacionario o estufa a leña; chimenea tapada acumula CO | Propuesta |
| Cortinas opacas | No escapa luz de noche; sigilo ante la horda | Propuesta |

## Logística de muebles y carga pesada

| Mecánica | Regla | Estado |
| --- | --- | --- |
| Desmontar del mundo | Palanca, destornillador, llave o maza; heladeras, cocinas, estanterías, camas, cajas fuertes, generadores | Propuesta |
| Resultado | La habilidad decide si sale entero o en piezas y materiales | Propuesta |
| Acarreo a dos manos | 60–90 kg al frente del torso; no se puede correr, trepar ni desenfundar | Propuesta |
| Fatiga | Estamina cae rápido; sin fuerza suficiente hay riesgo de lesión lumbar | Propuesta |
| Carretilla (diablito) | −75 % de esfuerzo; rueda hasta la rampa | Propuesta |
| Carga cooperativa | Dos personas comparten el peso a velocidad normal | Propuesta (requiere multijugador, diferido) |

| Transporte | Ventaja | Riesgo | Estado |
| --- | --- | --- | --- |
| Pickup | Carga rápida | Lluvia y zombis que trepan a la caja | Propuesta |
| Furgón o camión de caja | Mucho volumen protegido | — | Propuesta |
| Trailer | Amplía cualquier auto con enganche | — | Propuesta |

- Amarre con cinchas de trinquete o sogas a rieles o piso. Propuesta.
- Carga suelta: al frenar o doblar a 70 km/h se golpea y pierde condición, sale volando de la pickup o vuelca el camión. Propuesta.

## Mejoras artesanales de vehículos (late game)

| Mejora | Materiales | Función | Estado |
| --- | --- | --- | --- |
| Paragolpes reforzado | Soldadora + perfiles U | Embestir zombis y portones sin dañar radiador | Propuesta |
| Rejas de ventanillas | Barras o malla soldada | Zombis no rompen vidrios para morder | Propuesta |
| Plataforma de techo | Caños soldados + tablones | Vigía y tiro, fuera del alcance de zombis | Propuesta |
| Toldo lateral | Lona + caños | Cocinar y descansar bajo lluvia | Propuesta |
| Tanques auxiliares | Tanques + mangueras | Duplica la autonomía | Propuesta |

## Interacción espacial

Comportamiento implementado y probado en el prototipo, sin confirmación de diseño del usuario. Propuesta.

| Paso | Regla |
| --- | --- |
| 1 | Validar cuerpo, objeto, accesos, capacidad y reservas; hallar camino antes de aceptar la orden |
| 2 | Reservar actor y objeto; fase "acercamiento" |
| 3 | El objeto queda en origen mientras el actor camina |
| 4 | Al llegar, verificar alcance y línea de interacción; la manipulación empieza al paso siguiente |
| 5 | Completar la transferencia; si cambian los requisitos, interrumpir y liberar la reserva |

- El viaje nunca cuenta como trabajo de manipulación. Propuesta.
- Cancelar conserva el objeto en origen. Una puerta cerrada a mitad de camino impide cruzar. Propuesta.
- Alcance de interacción 1,5 m; radio corporal 0,25 m; velocidad máxima de prueba 1,2 m/s reducida por dolor. Propuesta (coeficientes de prueba, no calibrados).
- Guardar a mitad de recorrido restaura posición, camino y progreso. Propuesta.

Conflicto: el prototipo usaba grilla de 0,5 m en 2D. La investigación propone Recast y Detour para navegación. Propuesta.

## Barricadas y asedio

| Mecánica | Regla | Estado |
| --- | --- | --- |
| Puertas y ventanas | Tienen integridad estructural | Propuesta |
| Aporreo | Zombis que oyen ruido adentro sin verte golpean la puerta más cercana; varios la rompen rápido | Propuesta |
| Tablones | Clavar 1 a 4 tablones en puertas y ventanas | Propuesta |
| Muebles | Empujar armarios o mesas contra accesos suma tiempo de ruptura | Propuesta |
| Brecha | Cargas de demolición abren muros de mampostería | Propuesta |

## Ruido de superficies

| Superficie | Ruido al pisar | Estado |
| --- | --- | --- |
| Parquet o madera vieja | 55 dB; alerta la habitación contigua | Propuesta |
| Vidrio roto | 70 dB; alerta a 15 m | Propuesta |
| Chapa | 65 dB | Propuesta |
| Alfombra o césped | 30 dB | Propuesta |
| Charco de agua o sangre | Salpicadura y huellas | Propuesta |

## Percepción y cámaras

| Regla | Estado |
| --- | --- |
| La cámara no da información que el personaje no ve: espalda u oclusión por muros | Propuesta (listado como requisito del prototipo) |
| Tercera persona: ~120° periférico, ~60° focal, horizonte 200 m o más | Propuesta |
| Detrás del personaje no hay entidades activas salvo que suenen | Propuesta |
| Isométrica: niebla de guerra fuera del cono o tras muros; última posición conocida en gris | Propuesta |
| Edificios y vehículos 1:1 sin pantallas de carga, con corte de techo y paredes | Propuesta (listado como requisito del prototipo) |
| Tercera persona e isométrica sobre el mismo personaje | Propuesta (investigación: "mantener") |

### Agudeza visual

| Condición | Efecto | Estado |
| --- | --- | --- |
| Miopía | Lejos borroso; más dioptrías, menos distancia nítida (ej. zombis manchas a 4 m) | Propuesta |
| Hipermetropía o presbicia | Borroso a menos de 0,8 m; no se leen manuales ni se arman piezas finas | Propuesta |
| Astigmatismo | Aberración cromática y halos en faros | Propuesta |
| Anteojos | Anulan el desenfoque; se mojan, se empañan al pasar de frío a calor, se caen o rompen en combate | Propuesta |

## Niveles de detalle visual

Propuesta. Las distancias son del prototipo y deben ajustarse con el nivel caliente de ~50 m de la investigación.

| Distancia | Representación |
| --- | --- |
| Más de 80 m | Bloque prismático con textura bakeada; sin luces ni muebles |
| 30–80 m | Exterior completo; interiores simulados por shader |
| Menos de 30 m | Malla 1:1, interior completo, colisiones de muebles |

La transición es por fundido con dither. La niebla reduce el pop-in. Propuesta.

## Abierto

- Persistencia: ¿archivos por sector escritos en segundo plano, o una base embebida tipo SQLite?
- ¿La isométrica permite acelerar el tiempo sin amenazas cerca?
- ¿Los pasajeros caminan libremente dentro de un vehículo en marcha desde el primer corte?

## Fuentes del prototipo

- vehiculos-interiores-logistica.md
- vision-entorno-espacial.md
- interaccion-espacial.md
- sistemas-supervivencia-mundo.md
- redes-infraestructura-servicios.md
- dinamicas-inmersivas-avanzadas.md
- engine-research (investigación del motor nuevo)
