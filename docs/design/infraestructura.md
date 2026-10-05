# Infraestructura viva: energía, agua y fluidos por habitaciones

La ciudad tiene redes reales de electricidad y agua, acopladas entre sí y con el aire de los edificios. Se pueden cortar, explotar, saquear y restaurar. Es la apuesta de innovación del juego: casi ningún survival 3D lo hace, Project Zomboid solo apaga todo con un temporizador. Las redes son grafos y los interiores son un grafo de habitaciones. Todo se recalcula por eventos, nunca por fotograma. Un poste destruido cuesta una actualización de grafo, no una simulación continua.

Etiquetas: **Decidido** (la fuente lo da como regla del proyecto), **Propuesta** (no confirmado), **Abierto** (pregunta pendiente).

## Principios

| Regla | Estado |
| --- | --- |
| Todo elemento de la red puede repararse o fabricarse desde cero en el late game. | Decidido (la fuente lo llama "regla de oro" del proyecto) |
| Redes como grafos de nodos y aristas, no cables ni física. | Propuesta (prototipo e investigación coinciden) |
| Coste cero en reposo: cada receptor guarda su estado y no corre nada por tick. | Propuesta |
| Recalcular solo la parte del grafo afectada por un corte, reparación o sobrecarga. | Propuesta |
| Agua, gas, humo, calor, sonido y olor zombi viajan por un mismo grafo de habitaciones. | Propuesta (investigación) |
| Corte de servicios emergente y reversible, no un apagón fijo en el "día X". | Propuesta (investigación; reemplaza la idea del prototipo) |

Conflicto: el prototipo proponía apagón y corte de agua programados en un día fijo. La investigación prefiere decadencia emergente. Se adopta la segunda como Propuesta.

## Niveles de simulación (LOD de sim)

Propuesta (investigación). Reemplaza las zonas 0–80 m / 80–300 m / >300 m y sectores de 100 × 100 m del prototipo.

| Nivel | Dónde | Qué corre | Frecuencia |
| --- | --- | --- | --- |
| Caliente | ~50 m de un jugador | Superficie de agua por tile, chispas, fugas visibles, IA individual | Cada fotograma |
| Tibio | Distrito cargado | Presión por segmento, nivel de agua por habitación, energía por sector | Pocas veces por segundo o por evento |
| Frío | Resto del mundo | Estadísticas: "sector 12 sin energía", "edificio inundado a 1,2 m" | Por evento o tick lento (minutos) |

- Al bajar de nivel se conserva la cantidad (volumen de agua, carga). Al subir se reconstruye el detalle desde ese número.
- Solo se actualizan regiones marcadas como sucias.

## Red eléctrica

### Topología

Propuesta. Jerarquía: central → subestación → línea de media tensión → transformador de sector ("tacho gris") → postes de barrio → acometida → tablero de térmicas → circuitos (enchufes, lámparas, aparatos).

| Nodo | Rol | Atributos |
| --- | --- | --- |
| Fuente | Subestación, generador, transformador | Potencia máxima (W), condición, calidad |
| Conmutador | Poste, disyuntor, fusible | Condición, calidad, estado abierto/cerrado |
| Receptor | Lámpara, enchufe, heladera, bomba, alarma | Consumo (W), alimentado sí/no |
| Arista | Tramo de cable | Carga máxima según calidad y material |

- Al romperse una arista: BFS desde las fuentes solo sobre el subgrafo afectado, o union-find. Propuesta (investigación).
- Se emite un evento tipo `SectorSinEnergía(12)`. Luces, puertas, alarmas, heladeras y bombas están suscritas.
- Prototipo: 500 postes y 3.000 casas se recalculan en una fracción de milisegundo. Propuesta (estimación, no medida en el motor nuevo).

### Carga y sobrecarga

| Mecánica | Números | Estado |
| --- | --- | --- |
| Generador naftero | 3.000 W máximo | Propuesta |
| Foco LED | 10 W; motor en ralentí silencioso | Propuesta |
| Compresor o sierra de banco | 1.800 W | Propuesta |
| Brownout al superar el límite | Luces parpadean, humo negro, salta la térmica | Propuesta |
| Cable sobrecargado | Se rompe (estilo Oxygen Not Included) | Propuesta (investigación) |
| Calidad del cable | Q 60 aguanta más carga antes de sobrecalentarse que Q 30 | Propuesta (investigación) |
| Cerca electrificada 220 V | Frena la horda; consumo alto; puede prender pasto seco | Propuesta |

### Intervenir un poste

| Método | Herramienta | Consecuencia | Estado |
| --- | --- | --- | --- |
| Corte en la base | Motosierra (90 dB) o hacha | Cae según la cuña; riesgo de aplastamiento; cables cortados con chispas | Propuesta |
| Tracción | Eslinga o malacate + camioneta 4x4 | Desarraiga la base | Propuesta |
| Cortar tirantes | Cizalla | En esquinas el poste se curva o cae por el peso del tendido | Propuesta |
| Desmontaje limpio | Espuelas, arnés, alicate dieléctrico | Recupera transformador y aisladores intactos | Propuesta |
| Choque de vehículo | — | Rompe la arista y daña el vehículo | Propuesta |

## Red de agua

Propuesta (investigación). Jerarquía: planta potabilizadora → cañerías maestras → estaciones de bombeo → troncales de distrito → montantes de edificio → canillas.

- Se simulan segmentos de caño conectados con contenido compartido, no caños individuales.
- Presión por segmento = altura de suministro − demanda − fugas.
- Un caño reventado es un nodo de fuga con caudal. Ese caudal entra al grafo de habitaciones.
- Cerrar llaves de paso concentra presión en las ramas abiertas: mangueras, aspersores, turbinas. Propuesta.
- Q del caño fija la presión de reventón: Q 80 aguanta más que Q 30. Propuesta (investigación).

### Golpe de ariete

Propuesta. Cerrar una válvula en menos de 0,5 s genera una onda de sobrepresión con un "clang" metálico. Si el caño es viejo u oxidado, revienta un codo o junta en otro lugar. Cerrar despacio lo evita.

### Congelamiento

Propuesta. Antes de la primera helada hay que cerrar la llave general y abrir canillas para purgar. El agua atrapada congela y raja caños dentro de las paredes. En el deshielo la casa se inunda.

## Grafo de habitaciones (fluidos por espacios)

Propuesta (investigación). Cada habitación, pasillo, escalera y tramo de cloaca es un contenedor. Cada abertura es una arista con altura y ancho: puerta, escalera, desagüe, agujero, ventana rota. El agua pasa por diferencia de nivel. Cuesta cientos de números por edificio.

Conflicto: el prototipo describía cascadas físicas en tiempo real por escaleras y bocas de tormenta. Se mantiene como efecto visual del nivel caliente sobre el grafo. Propuesta.

| Fluido | Fuente | Efecto | Estado |
| --- | --- | --- | --- |
| Agua | Caño roto, lluvia, desagüe tapado, deshielo | Nivel por habitación; desborda umbrales y baja escaleras | Propuesta |
| Humo | Fogata sin chimenea, incendio | Sube al techo y desciende; a la altura de la cabeza: tos de 70 dB, daño pulmonar, ceguera temporal | Propuesta |
| CO | Generador o estufa bajo techo, chimenea tapada | Incoloro e inodoro; dolor de cabeza, somnolencia, muerte durmiendo; se evita con ventilación cruzada | Propuesta |
| Sonido | Disparos, generadores, golpes | Se propaga por aberturas; atrae zombis | Propuesta (investigación) |
| Olor zombi / olfato | Cuerpos, excrementos, comida | Viaja entre habitaciones | Propuesta (investigación) |
| Calor / fuego | Incendio | Va sobre el mismo grafo más adelante | Propuesta (investigación lo difiere) |

### Inundación de interiores

| Riesgo | Regla | Estado |
| --- | --- | --- |
| Agua electrificada | Si el nivel alcanza un enchufe con red activa, el agua electrocuta | Propuesta |
| Provisiones | Cartón, harina, munición suelta y libros a ras del piso se arruinan | Propuesta |
| Vadeo | Agua a la rodilla: 3× estamina y enfriamiento rápido | Propuesta |
| Ahogamiento | Inconsciente en más de 15 cm de agua: muere en minutos | Propuesta |
| Drenaje | Bomba de achique eléctrica, bomba de diafragma manual, zanjas con pala | Propuesta |
| Boca de tormenta | Tapada por basura junta agua en la calle; destaparla la vuelca a la cloaca de abajo | Propuesta |
| Cascada a la cloaca | Moja a quien esté abajo, apaga antorchas, reverbera y alerta al subsuelo | Propuesta |
| Continuidad | Sin pantallas de carga entre calle, interiores, vehículos y cloaca (Y = −4 m) | Propuesta |

### Escenario de referencia

Propuesta (investigación). Una granada explota en una sala de bombas subterránea. El daño rompe los caños que pasan por la sala según distancia y condición. Cada rotura es una fuga. El nivel sube, supera el desagüe y el umbral, y baja por la escalera. El jugador de abajo ve subir el agua y sufre vadeo, ruido y ahogamiento.

## Acoplamientos

Propuesta (investigación salvo indicación).

| Causa | Efecto encadenado |
| --- | --- |
| Sector sin energía | Bombas paradas; el tanque de techo se vacía y luego cae la presión |
| Sector sin energía | Heladeras se apagan; la comida se pudre |
| Sector sin energía | Alarmas a batería suenan y atraen zombis |
| Oscuridad | Cambia la percepción de zombis y jugadores |
| Agua sobre cable energizado | Charco electrificado letal (prototipo) |
| Explosión en túnel | Efecto pistón: más daño que al aire libre; revienta maestras, cae cableado al agua (prototipo) |
| Explosión grande bajo calle | Colapsa la losa y abre un socavón entre calle y cloaca (prototipo) |
| Pared con caños y cables | Una brecha corta un sector o inunda un sótano; robar cobre de una casa es una decisión real |
| Chispas | Pueden iniciar incendios (prototipo) |

## Decadencia y reparación

- Sin operarios, las subestaciones acumulan fallas en un timer del nivel frío. Propuesta (investigación).
- Reparar con piezas cuya Q decide cuánto dura el arreglo. Propuesta (investigación).
- Restaurar un distrito es una meta de mitad de partida. Propuesta (investigación).

| Componente | Reparación | Fabricación desde cero (late game) |
| --- | --- | --- |
| Poste | Apuntalar o volver a izar con malacate | Talar pino recto, descortezar, impregnar con brea de carbón vegetal |
| Aislador | Sacar de postes abandonados | Arcilla en torno, vitrificada a más de 1.100 °C (porcelana) |
| Cable | Empalmar con borneras y cinta | Fundir cobre, forjar varilla, trefilar; aislar con látex o resina |
| Generación | Bujías, aceite, filtros | Molino hidráulico o eólico con alternador; gasógeno de leña (CO + H₂) |
| Torre de agua | Parchar fugas con abrazaderas y goma | Andamio de madera con tanque; molino de bombeo o bomba de pistón |
| Plomería | Cambiar codos y niples | Caños de plomo o caña tratada con brea, juntas de estopa y grasa |
| Fusible de transformador | Cambiar cartucho con pértiga | Calibrar hilo de plomo/estaño que funda al amperaje crítico |

Toda la tabla es Propuesta en su detalle; la regla de que todo se puede reparar o fabricar es Decidido.

## Legibilidad

- Vista isométrica: overlays conmutables de energía (sectores muertos) y agua (presión, habitaciones inundadas). Propuesta (investigación).
- Tercera persona: la misma información, diegética. Ventanas oscuras, caños que silban, charcos bajo las puertas. Propuesta (investigación).

## Primer corte jugable

Propuesta (investigación). Una manzana con 3–4 edificios, uno con sala de bombas en el sótano. Una subestación y unos 20 postes. Victoria del corte:

1. Volar un poste y la manzana queda a oscuras.
2. Granada en el sótano: el agua llega al piso de abajo por la escalera.
3. Reparar el poste con una pieza fabricada; su Q decide cuánto dura.

## Abierto

- ¿El sistema héroe es la infraestructura o el cuerpo?
- ¿Tamaño del mundo: un pueblo, un distrito o una ciudad?
- ¿Cuántos días tarda la red en degradarse sola sin jugadores?
- ¿Se modela tensión y amperaje, o solo potencia en W por componente conectado?

## Fuentes del prototipo

- redes-infraestructura-servicios.md
- sistemas-supervivencia-mundo.md
- dinamicas-inmersivas-avanzadas.md
- quimica-combustibles-balistica-clima.md
- engine-research (investigación del motor nuevo)
