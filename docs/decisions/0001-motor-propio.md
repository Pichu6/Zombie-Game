# 0001 · Motor propio desde cero

- **Fecha:** 2026-10-05
- **Estado:** Decidido por el usuario

## Decisión

El juego se construye desde cero con un motor propio en C++ y renderer Vulkan. El proyecto Godot existente (Godot, Mixamo, Blender, Tripo3D) es sólo referencia de ideas.

## Por qué

El usuario quiere innovar en simulación profunda sin las restricciones de un motor genérico.

## Consecuencias

- Se escribe el motor *de este juego*, no un motor general.
- Herramientas de autoría externas (Blender u otras) siguen siendo válidas para modelar; el motor no depende de ellas en runtime.
