# Catálogo de ítems, componentes y representación

Estado: base implementada con un prototipo 3D, 2026-09-06. No equivale a un sistema terminado de inventario, fabricación o calidad física.

## Datos y archivos responsables

`src/items/item_catalog.gd` unifica los IDs ya existentes en `ClothingDatabase`: devuelve nombre, categoría, propiedades de simulación, representación, componentes y calidad. No reemplaza ni recalibra la biología.

`assets/items/catalog.json` contiene los datos visuales y de composición redactados para cada ítem. Los demás IDs se exponen con icono genérico identificado como tal, modelo vacío, componentes pendientes y calidad sin definir. No se generan nombres de `.glb` ficticios.

Cada definición tiene:

| Campo | Contenido y estado |
| --- | --- |
| `simulation` | Copia de propiedades físicas existentes: masa en kg, capacidad de carga en kg, aislamiento en clo y resistencias normalizadas cuando corresponden. |
| `visual.icon_path` | Archivo real; `icon_is_generic` distingue marcador de icono específico. |
| `visual.model_path` | Ruta `res://` a una escena 3D/malla real, o cadena vacía si falta. |
| `visual.material_paths`, `texture_paths` | Referencias a archivos; pueden estar vacías. Un material sin textura externa no equivale a una textura faltante. |
| `visual.attachment` | Socket/hueso, desplazamiento en metros, escala y tipo de overlay de la silueta, provenientes del template existente. |
| `components` | Lista de componentes y materiales; por ahora descriptiva, sin agregación automática de masa o resistencia. |
| `quality` | Grado y estado explícito `metadata_only`; no modifica protección, vida útil ni desgaste. |

`ItemCatalog.validate()` comprueba las referencias visuales declaradas y materiales de componentes. Un modelo pendiente se representa como ausencia explícita, no como asset verificado.

## Primer ítem 3D real

`backpack_school`: mochila urbana de prueba. Tiene icono SVG específico, modelo con cuerpo redondeado, bolsillo, tapa, correas y hebillas, y material de lona. La representación de correas sobre la silueta usa el overlay de mochila existente. La vista 3D muestra el mismo personaje y el modelo equipado.

- Modelo: `assets/items/backpack_school/model.tscn`.
- Material: `assets/items/backpack_school/canvas.tres` (color y rugosidad, sin texturas externas).
- Icono: `assets/items/backpack_school/icon.svg`.
- Fuente reproducible: `tools/generate_backpack_prototype.gd`; ejecutar con Godot `--headless --path . --script res://tools/generate_backpack_prototype.gd`.

Es modelado procedural generado para este proyecto: no es una descarga de terceros ni una promesa de calidad final. La mochila es rígida, fijada a `mixamorig_Spine2`; no implementa tela, deformación de correas o ajuste automático contra todos los morphs extremos. La geometría utiliza dimensiones visuales en metros; la masa/capacidad continúa gobernada por la ficha de simulación, no se calcula desde el volumen de la malla.

## Montaje real

`src/items/equipment_visuals.gd` valida el archivo, carga una `PackedScene` o `Mesh`, verifica el hueso y crea un `BoneAttachment3D` con geometría como hijo. Un error conserva el modelo anterior. Cada ID tiene su propio attachment, aunque comparta hueso con otro ítem. La sincronización desde `HumanCharacter` equipa y elimina automáticamente los modelos disponibles cuando cambia el inventario.

El botón de adjuntar del laboratorio ahora ejecuta esa carga; dejó de declarar éxito por crear solo un socket vacío. Una ruta escrita manualmente se aplica a la sesión actual. Para conservar una asignación entre ejecuciones, editar el catálogo JSON y los recursos del proyecto; todavía no existe un editor completo de guardado de templates en la UI.

## Simulación actual y trabajo pendiente

| Sistema | Alcance comprobado / pendiente |
| --- | --- |
| Zonas, heridas, sangrado, dolor y tratamientos | Reglas existentes consultadas por el nuevo informe y el escáner. La imagen interior es ilustrativa; no geometría anatómica de órganos simulados. |
| Metabolismo, hidratación, temperatura, circulación | Implementación parcial existente; se mantienen los hallazgos de unidades, composición corporal y reloj de `estado-inicial.md`. |
| Ropa y carga | Masa, humedad, aislamiento y equipo siguen en el sistema existente. No se implementó en esta iteración inventario volumétrico ni contenido individual del bolso. |
| Componentes, materiales y calidad | Esquema descriptivo inicial. Pendientes desgaste, daño de componentes, reparación, fabricación y efectos de calidad calibrados. |
| Catálogo visual | Todos los IDs existentes tienen una definición accesible. Solo la mochila urbana tiene un modelo nuevo; los demás modelos siguen pendientes. |
| Multiplayer/cooperativo | Pendiente. La nueva presentación no introduce networking ni garantiza determinismo. |

No utilizar `quality` como multiplicador genérico de todas las propiedades: el diseño futuro debe establecer qué componente afecta masa, aislamiento, impermeabilidad o resistencia, con unidades y pruebas separadas.

## Pruebas

`tools/test_station_and_catalog.gd`: referencias del catálogo, geometría real, error por modelo inexistente, ausencia de duplicados, retirada al desequipar, máscara local de lesión, fondo no seleccionable, teclas X/Alt+3 e informe clínico. Marcador `STATION_AND_CATALOG_OK`.

La prueba clínica anterior exigía el texto `sidebag_tactical.glb` aunque ese archivo no existía. Ahora exige ruta vacía y aviso de recurso faltante; la prueba nueva acredita por separado una instancia 3D real.
