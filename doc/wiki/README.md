# Wiki del proyecto

Esta wiki es la memoria técnica revisable del proyecto. El GDD describe la visión; no acredita que una función esté implementada.

## Lectura inicial

1. [Decisiones confirmadas](decisiones.md).
2. [Estado y auditoría inicial](estado-inicial.md).
3. [GDD](../GDD_Survival_3D.md), para consultar objetivos de diseño.

## Convención de documentación

Cada sistema debe indicar: estado (propuesto, implementado parcialmente o implementado), archivos responsables, unidades de sus variables, entradas y salidas, pruebas realizadas, limitaciones y decisiones pendientes. Una prueba aprobada no acredita precisión científica.

Registrar las decisiones del usuario separadas de las recomendaciones técnicas. Actualizar la documentación cuando cambie el código; no convertir hipótesis en hechos.

## Recuperación de conocimiento — propuesta, no implementada

Mantener Markdown como fuente de verdad y un índice reconstruible fuera del contenido fuente. Dividir por secciones y símbolos; asociar ruta, título, estado, hash y versión del documento. Combinar búsqueda textual con similitud vectorial y devolver siempre el fragmento con su procedencia. Invalidar entradas modificadas o eliminadas. Excluir cachés, respaldos, credenciales y binarios.

Antes de elegir motor de embeddings, acordar si documentos/código pueden enviarse a un proveedor externo. El índice vectorial no es un respaldo ni sustituye esta wiki. Por ahora se utiliza búsqueda textual del repositorio.

## Versionado y recuperación

Ver [qué entra en Git, Git LFS y el respaldo separado](versionado-y-respaldo.md).
