# Decisiones confirmadas — 2026-09-06

## Modalidad

Objetivo final: multijugador. Es aceptable comenzar con single player, continuar con cooperativo y después ampliar multijugador. Escala de habitantes, zombis y jugadores aún pendiente.

Consecuencia técnica propuesta: separar estado de simulación, órdenes de acciones y presentación. Evitar que el reloj global o las reglas dependan del dashboard. No implementar networking todavía ni prometer determinismo de red sin pruebas.

## Prioridad de experiencia

Primero mejorar el laboratorio de simulación. Este trabajo alimentará la futura interfaz de creación y uso del personaje. Priorizar legibilidad, selección de partes, acciones contextuales y comprensión de causas y consecuencias. Las herramientas de depuración deben poder distinguirse de las acciones del jugador.

## Estado de conservación comunicado por el usuario

La carpeta actual es la versión más reciente. El repositorio privado https://github.com/Pichu6/Zombie-Game está vacío según el usuario. No existía otra copia de respaldo al comenzar esta etapa. El usuario sufrió anteriormente una pérdida del disco de trabajo.

El respaldo local de esta sesión se verifica por hashes, pero sigue en la misma unidad. El respaldo externo, la autenticación de GitHub y la autoría de commits requieren datos pendientes del usuario.

## Orden de trabajo propuesto

1. Verificar copia local y acordar destino independiente.
2. Preparar versionado con exclusión de cachés y estrategia para binarios grandes; verificar recuperación desde una copia limpia.
3. Documentar unidades y corregir pruebas que pueden aprobar sin ejecutar la actividad esperada.
4. Corregir inconsistencias de anatomía, composición corporal y masa transportada en cambios pequeños.
5. Evaluar visualmente el laboratorio y rediseñar un recorrido completo de inspección y acción.
6. Definir pipeline común de esqueleto, morphs y animaciones antes de expandir assets.

Este orden es una propuesta técnica; no significa que los pasos estén terminados.

## Actualización del estado de conservación — 2026-09-06

Autoría y autenticación ya comprobadas; el primer commit se publicó en GitHub. La separación de contenido y sus límites se documentan en [versionado y respaldo](versionado-y-respaldo.md). El estado anterior de repositorio vacío describe el comienzo de esta etapa. Sigue faltando un respaldo independiente para los paquetes que se excluyen del repositorio de desarrollo.
