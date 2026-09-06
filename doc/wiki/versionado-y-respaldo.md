# Qué se versiona y cómo recuperar el proyecto

## Git y Git LFS

- Git normal: `project.godot`, `src/`, `scenes/`, `addons/`, `doc/`, pruebas y herramientas. Conservar `.uid` cuyo script existe y `.import` de los assets incluidos: contienen identidad y opciones de importación, no el caché.
- Git LFS: modelos, mallas binarias, imágenes y otros binarios fuente. `.gitattributes` aplica reglas por extensión también a los assets nuevos. Instalar Git LFS antes de agregar archivos o clonar.
- Se conserva `char_human/rig/makehuman/` completo: modelo base, malla con morphs, modelos fuente de las variantes y texturas. Las variantes son fuentes de trabajo, aunque no todas se cargan directamente en ejecución.
- Se conserva `char_human/anim/` completo: las diez animaciones cargadas en `human_character.gd`, sus texturas y opciones de importación. Los FBX actuales incluyen geometría/texturas; reemplazarlos por exportaciones de solo animación exige otro cambio con validación de retargeting. No eliminarlas por una búsqueda textual únicamente.
- Se conserva `assets/ui/`, incluido el paperdoll utilizado por el laboratorio, y el plugin habilitado en `project.godot`.

## Fuera de Git, conservado localmente

- `char_human/rig/CC Character Base/`: paquete original ZIP, FBX/OBJ, texturas, mapas y fuentes ZBrush. Sin referencias desde scripts ni escenas actuales. La carpeta se ignora en Git y tiene `.gdignore` local para evitar importarla en Godot.
- `.backups/`: respaldo, manifiestos, resultados de validación y copias de prueba. Diez `.gd.uid` sin script y `test_vertex_only_blend.res` sin referencias se archivaron en `.backups/non-runtime-files-20260906/`.
- `.godot/`: caché reconstruible de Godot. `build/`, `dist/` y `exports/`: salidas generadas. Variables de entorno locales y metadatos del sistema también se ignoran.
- `_local_assets/` queda reservado e ignorado para futuros paquetes originales. Si se usa dentro del proyecto, agregar un `.gdignore` local. Los assets realmente integrados deben copiarse a una carpeta versionada con sus dependencias.

No se ignoran globalmente todos los ZIP ni todos los archivos grandes: un formato o tamaño no demuestra que un archivo sea prescindible.

## Respaldo separado

Antes de excluir archivos se compararon original, copia local y manifiesto SHA-256: 2.141 archivos, 1.775.944.611 bytes coincidentes. No se borraron los originales del paquete CC.

Los archivos excluidos necesitan respaldo en otra unidad o servicio. El respaldo local está en el mismo disco. El commit inicial publicado también los contiene, pero no debe tratarse como archivo externo permanente si se limpia el historial. No purgar copias locales ni objetos LFS antiguos hasta disponer de ese respaldo independiente.

## Recuperación y comprobación

1. Instalar Git, Git LFS y la versión de Godot usada por el proyecto (validación local: 4.7.2).
2. Clonar el repositorio y ejecutar `git lfs pull`; los archivos LFS deben ser binarios reales, no textos que empiecen por `version https://git-lfs.github.com/spec/v1`.
3. Ejecutar Godot con `--headless --editor --path RUTA_DEL_CLON --import` y esperar la finalización. No copiar `.godot` desde otro proyecto.
4. Ejecutar `--headless --path RUTA_DEL_CLON --script res://tools/verify_project_assets.gd --quit-after 600`. Exigir código 0 y `ASSET_CHECK_OK`: un límite de frames por sí solo no demuestra que la prueba terminó.
5. Ejecutar las pruebas existentes pertinentes y abrir `scenes/biology_showroom.tscn` para inspección visual cuando se cambien assets o UI.

Al agregar contenido, revisar `git status`, las dependencias, `git check-attr filter -- RUTA` y `git lfs status`. Los paquetes de descarga sin integrar pertenecen al respaldo separado. Los binarios necesarios pertenecen a LFS; el tamaño no es motivo para omitir una dependencia.

## Historial publicado

La exclusión mediante un commit normal limpia la versión actual, pero el commit inicial `9dbf972` sigue conservando el paquete CC. Reducir también el historial publicado requiere sustituir la rama remota con un historial limpio; es una operación distinta que debe aprobarse explícitamente. Los objetos LFS ya almacenados en GitHub no se liberan necesariamente al reescribir una rama.

## Verificación de la separación — 2026-09-06

- Copia limpia creada desde el índice de Git en `.backups/verify-git-clean-20260906`, sin `.godot` previo ni paquete CC. No es todavía una prueba de descarga desde otra máquina.
- Importación con Godot 4.7.2: código 0. La comprobación de assets terminó con `ASSET_CHECK_OK`; están el esqueleto, diez animaciones con tracks, al menos tres morphs y diez texturas requeridas.
- Pruebas existentes de lógica clínica/paperdoll y caminata de dos horas: código 0 y marcadores finales de éxito. Las pruebas clínicas reportan material nulo en el renderer headless y recursos sin liberar; se reprodujeron los mismos avisos al ejecutar el respaldo inicial. No se corrigieron en esta tarea.
- La importación informó cuatro avisos UTF-8 de FBX y que el plugin Tripo no pudo abrir su puerto 60650 durante esta segunda instancia. No hubo recursos faltantes ni errores de compilación de scripts. No se hizo inspección visual en esta verificación.
- Se eliminó de `project.godot` la referencia al `icon.svg` inexistente. No se modificaron scripts del juego ni los binarios fuente conservados.
- La versión de trabajo ocupa aproximadamente 716 MB frente a 2,49 GB. Estas cifras suman archivos materializados; no son el tamaño de transferencia ni el almacenamiento facturado por GitHub. LFS deduplica objetos iguales.
