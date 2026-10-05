# Ropa que sigue al cuerpo, en un motor propio

Investigación y propuesta para Juego Zombie. Versión 2, 2026-10-05: reescrita para un motor nuevo desde cero (Vulkan). El proyecto Godot/Mixamo/Tripo se usa sólo como referencia de lo que falló.

## Respuesta corta

No ajustar cada prenda. **Cada vértice de una prenda se ata una vez a un punto de la superficie del cuerpo, y en cada frame la GPU lo vuelve a colocar sobre el cuerpo ya deformado.** Así la ropa sigue cualquier cambio de grasa, músculo, hinchazón o pose sin shape keys propias, sin correctivos por prenda y sin volver a tocarla en Blender.

En el prototipo, cada remera fue un proyecto entero: nueve variantes de composición, 18 correctivos de pecho y rondas de arreglos en la axila. Eso es justo el trabajo "prenda por prenda" que no escala. Con un motor propio podemos hacer lo que los juegos grandes hacen fuera de línea, pero en tiempo real y gratis por prenda.

## Cómo lo resuelven otros

| Juego / herramienta | Cómo la ropa sigue al cuerpo | Qué copiar |
| --- | --- | --- |
| **The Sims 4** | Los sliders del cuerpo se guardan como *deformer maps*: texturas en un espacio UV común al cuerpo con el desplazamiento de forma. Cada prenda lee el mapa en su segundo UV (proyectado desde el cuerpo) y se mueve igual. Un mapa por slider sirve para todas las prendas, incluso las de la comunidad. | El desplazamiento vive en el cuerpo, no en cada prenda. |
| **Roblox (layered clothing)** | El cuerpo tiene una *jaula exterior* y cada prenda una *jaula interior* y otra exterior. En runtime un deformador "wrap" coloca la prenda sobre cualquier avatar, y una chaqueta se apila sobre la remera porque usa la jaula exterior de ésta como cuerpo. Funciona con millones de prendas hechas por usuarios. | Wrap en runtime y capas apiladas. |
| **MakeHuman / MPFB** | Formato `.mhclo`: cada vértice de prenda guarda tres vértices del cuerpo, baricéntricas y un desplazamiento. Es el mismo binding que propongo, hecho fuera de línea. | El formato del binding. |
| **Unreal Mutable / MetaHuman** | *Mesh Reshape* deforma la prenda del cuerpo base al cuerpo actual, y además *borra las partes del cuerpo que no se ven* bajo la ropa. MetaHuman ya trae ropa que se adapta sola. | Reshape más el ocultamiento de piel cubierta. |
| **Marvelous, Wrap, Houdini** | Fuera de línea: simulan o envuelven la prenda una vez por forma del cuerpo y guardan blend shapes. | Útil sólo para prendas héroe; no hace falta como base. |

El patrón común: atar una vez, transferir la forma, empujar hacia afuera, ocultar la piel cubierta y sólo corregir a mano el caso extremo raro.

## Arquitectura propuesta en el motor

### Pipeline por personaje en la GPU (cada frame, o cuando algo cambia)

1. **Morph** (compute): base + deltas dispersos según los pesos de la simulación corporal (grasa, músculo, hidratación, hinchazón, heridas).
2. **Skinning** (compute): esqueleto, máximo 4 huesos por vértice.
3. **Normales** del cuerpo deformado (compute, o recalculadas desde los deltas).
4. **Wrap** (compute), una pasada por capa: ropa interior, remera, chaqueta y equipo. Cada capa puede usar como "cuerpo" la salida de la capa anterior, como las jaulas de Roblox.
5. **Render**: cuerpo con regiones ocultas según lo que lleve puesto, y prendas.

Como el wrap corre después del morph y el skinning, la ropa hereda **todo**: forma, pose y los correctivos de pose del cuerpo. Los cruces de axila del prototipo venían en gran parte de que la ropa no tenía los correctivos que sí tenía el cuerpo; con wrap eso desaparece por construcción.

### El binding (lo que exporta la herramienta, una vez por prenda)

Por cada vértice de la prenda:

| Campo | Tipo | Para qué |
| --- | --- | --- |
| `tri_id` | u32 | Triángulo del cuerpo en reposo más cercano |
| `bary` | 2 × f32 | Posición dentro de ese triángulo |
| `offset_tbn` | 3 × f32 | Desplazamiento en la base local (tangente, bitangente, normal): grosor y holgura |
| `follow` | f32 | 1 = ceñido; ~0.5 = holgado (no abraza la panza) |
| `island` | u32 | Isla rígida o ninguna |
| `bone_index` / `bone_weight` | 4 × u16 / 4 × f32 | Pesos interpolados del cuerpo, para el modo sólo-esqueleto |
| `body_uv` | 2 × f32 | Coordenada en el UV común del cuerpo (suciedad, sangre, ocultamiento) |

Son unos 60 bytes por vértice, y se pueden comprimir a la mitad con f16 y baricéntricas en 16 bits.

### El shader de wrap (un hilo por vértice de prenda)

```glsl
uvec3 t = bodyTris[tri_id];
vec3 a = P[t.x], b = P[t.y], c = P[t.z];          // cuerpo ya deformado
float u = bary.x, v = bary.y, w = 1.0 - u - v;
vec3 p = a*w + b*u + c*v;
vec3 n = normalize(N[t.x]*w + N[t.y]*u + N[t.z]*v);
vec3 e = b - a;
vec3 T = normalize(e - n*dot(e, n));
vec3 B = cross(n, T);
vec3 o = offset_tbn;
if (island == NONE) o.z = max(o.z, minGap);       // la tela nunca entra en la piel
vec3 wrapped = p + T*o.x + B*o.y + n*o.z;
out_pos = mix(skinnedOnly, wrapped, follow);
```

Son 6 lecturas y unas 40 operaciones por vértice. Estimación sin medir: 50 personajes con 20.000 vértices de ropa cada uno dan un millón de vértices, una fracción de milisegundo en una GPU tipo RTX 3060.

### Tres clases de objetos, el mismo código

- **Blandos** (remera, pantalón, chaqueta): wrap completo.
- **Mixtos** (mochila, cinturón, funda, botas): las correas y la caña de la bota hacen wrap; las piezas duras (cuerpo de la mochila, hebillas, suela) son **islas rígidas**. Todos los vértices de una isla comparten el ancla de su centro, así que se mueven como un sólido que "cabalga" sobre la superficie. En el shader no hay caso especial.
- **Rígidos** (casco, reloj, armas): un único ancla (un "rivet") sobre la superficie, o un hueso. Un casco sobre una cara más gorda sube con la cara.

### Red de seguridad

- **Holgura mínima** (en el shader de arriba): ningún vértice baja de `minGap` sobre su punto de anclaje.
- **Ocultar la piel cubierta**: el cuerpo se etiqueta por regiones (torso, brazo superior, muslo...) y cada prenda declara qué regiones cubre del todo. En el vertex shader, los triángulos de regiones ocultas se colapsan con una máscara de bits por personaje. Las heridas o la vista anatómica apagan la máscara en esa zona. El prototipo prohibía esto; aquí recomiendo usarlo (ver la decisión al final).
- **Contacto entre partes del cuerpo** (manga contra torso con el brazo bajado): pocas cápsulas por personaje (torso, muslos) y empuje en el mismo compute. Es la única colisión necesaria, y es barata.

### Prendas base + módulos de detalle (relieve, bolsillos)

En el prototipo se probó una malla base de prenda deformable con textura intercambiable. Falló por relieve (bolsillos, tapas) por tres razones: una textura no cambia la silueta, la UV cilíndrica no daba lugares estables y los detalles no estaban atados a la prenda. Propuesta:

- **Prenda base**: pocas mallas genéricas (remera, buzo, pantalón, campera) con **UV de molde de costura** por paneles (delantero, trasero, manga...). Una coordenada significa lo mismo en todas las variantes. Se ata al cuerpo.
- **Módulos de detalle**: bolsillo, tapa, cierre, correa, rodillera, capucha. Son mallas chicas con grosor real, atadas a la **superficie de la prenda base** con el mismo binding y ubicadas en coordenadas de molde. Un cargo es pantalón base más cuatro módulos.
- **Material**: tejido repetible, normal map sólo para costuras, pespuntes y tejido, y las máscaras de suciedad y desgaste.
- **Crafteo**: la prenda es una lista de piezas, cada una con material y calidad. Desarmar devuelve las piezas y fabricar las combina. El render y el sistema de crafteo leen la misma descripción.

### Cambio corporal continuo

Como la ropa no tiene shape keys, la simulación puede mover la composición de forma continua: adelgazar durante semanas de hambre, hincharse por una infección, ganar músculo trabajando. La ropa siempre queda bien sin preparar nada. **Talla** sale casi gratis: una prenda de talla menor que el cuerpo se nota tirante (`follow` y holgura negativa limitada), y una mayor queda floja.

### Cámara isométrica / gestión

Desde arriba no se ve el grosor de la tela. Para personajes lejanos, saltar el wrap: hornear la ropa puesta en la textura del cuerpo cuando cambia el equipo y dibujar una sola malla. Es la gran ganancia de rendimiento para multitudes en modo RTS o gestión.

## Suciedad, grasa, sangre y humedad

Mismo truco que los deformer maps de Sims: un **espacio UV común del cuerpo**.

- Cada personaje lleva máscaras pequeñas en ese espacio (por ejemplo 256 × 256 RGBA: barro, grasa/aceite, sangre, mojado). El juego pinta en ellas: arrastrarse ensucia rodillas y antebrazos, una salpicadura pinta donde pegó, la lluvia moja de arriba hacia abajo.
- Cada prenda tiene `body_uv` por vértice (sale del binding) y lee **las mismas** máscaras, así que el barro en las piernas aparece en la piel y en el pantalón que la cubre, en el mismo lugar.
- Cada objeto guarda además su propia copia de las máscaras en ese mismo espacio. Si el pantalón pasa a otro personaje, conserva sus manchas en el lugar correcto porque todos los cuerpos comparten el espacio UV. Encaja con la idea de Haven & Hearth: calidad, desgaste y suciedad viven en el objeto.
- El aspecto viene de pocos materiales repetibles (barro, aceite, sangre seca, oscurecido por agua) mezclados por las máscaras. No hay que pintar texturas por prenda.

Requisito para todo esto: **todos los cuerpos comparten una topología y un UV** (cambian por morphs, no por mallas distintas). Esa regla ya era correcta en el prototipo y conviene fijarla desde el día uno del motor nuevo.

## Herramientas incluidas (probadas)

- [export_garment_binding.py](../../tools/garment/export_garment_binding.py): Blender sólo como herramienta de autoría. Lee el cuerpo en reposo y las prendas de una colección. Escribe por prenda `<nombre>.bind.json` (cabecera y layout) y `<nombre>.bind.bin` (little-endian), más `body.tris.bin`. Lee los grupos opcionales `fit_follow` y `fit_rigid`.
  ```
  blender escena.blend --background --factory-startup --python export_garment_binding.py -- --body Body --collection Garments --out-dir export/
  ```
- [wrap_reference.py](../../tools/garment/wrap_reference.py): el paso de wrap en numpy. Es la especificación ejecutable del compute shader; sirve para comparar la versión GPU vértice a vértice cuando exista.

Prueba hecha con Blender 5.0 sin interfaz, sobre un cuerpo sintético (cilindro con forma gorda y delgada, dos huesos) y una prenda con un bolsillo rígido. Resultados:
- Ida y vuelta en reposo con error máximo de 0,00002 mm.
- Con cuerpo gordo y delgado, la tela queda siempre fuera de la piel (separación mínima de 9,8 mm).
- El bolsillo rígido no se deforma (0 mm de distorsión).
- Los pesos de hueso suman exactamente 1.

**No se probó con un cuerpo humano real ni en la GPU todavía.**

## Orden sugerido para el motor nuevo

1. Fijar el cuerpo base: una topología, un UV, un esqueleto y morphs como deltas dispersos. La base CC0 de MakeHuman/MPFB es legalmente reutilizable como punto de partida si te sirve, o se modela una nueva.
2. Primer hito visible: cuerpo con morph y skinning en compute más una remera con wrap, y un slider de grasa en vivo. Si eso se ve bien, el problema de la ropa está resuelto en lo esencial.
3. Después: mochila (islas rígidas), capas, ocultamiento de piel, máscaras de suciedad y el modo lejano horneado para la cámara isométrica.

## Decisión para vos

**¿La ropa ceñida oculta la piel que cubre?** Recomiendo que sí, por región cubierta, salvo donde haya una herida o la vista anatómica. Es lo que hacen todos los juegos citados y evita perseguir cruces milimétricos a mano.

## Fuentes

- [Deformer maps de The Sims 4 (guía TS4MorphMaker)](https://modthesims.info/showthread.php?t=613057) y [UV_1 en ropa de Sims 4](https://sims4studio.com/thread/19464/uv-mapping)
- [Jaulas de Roblox](https://create.roblox.com/docs/art/modeling/cage-meshes) y [layered accessories](https://create.roblox.com/docs/art/accessories/layered-clothing)
- [Mutable: quitar partes del cuerpo que no se ven](https://work.anticto.com/w/mutable/unreal-engine-4/user-documentation/remove-unseen-parts/) y [hilo sobre Mesh Reshape](https://forums.unrealengine.com/t/is-mutable-mesh-reshape-possible-with-cloth-simulation/2689499)
- [Ropa redimensionable para MetaHumans (tutorial de Epic)](https://dev.epicgames.com/community/learning/tutorials/KZ2r/unreal-engine-how-to-create-resizable-clothing-for-metahumans-auto-adapts-to-body-proportions)
- [Paquete Shirts01 de MakeHuman](https://static.makehumancommunity.org/assets/assetpacks/shirts01.html) (formato `.mhclo`)
