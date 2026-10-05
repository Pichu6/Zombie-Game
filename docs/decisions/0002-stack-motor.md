# 0002 · Stack del motor

- **Fecha:** 2026-10-05
- **Estado:** Propuesta (de la investigación de simulación profunda; falta confirmación del usuario)

## Propuesta

| Área | Propio | Biblioteca |
| --- | --- | --- |
| Núcleo de simulación | Sí, es la innovación | Ninguna |
| Renderer | Sí, chico: forward clustered, skinning, morphs y wrap de ropa en compute | vk-bootstrap, Vulkan Memory Allocator, meshoptimizer |
| Ventana e input | No | SDL3 |
| Física y colisión | No | Jolt Physics |
| Animación esquelética | Capa fina | ozz-animation |
| Navegación | No | Recast y Detour |
| Importación de assets | Conversor a formato propio | cgltf o fastgltf |
| Herramientas y editor | Sí, dentro del motor | Dear ImGui |
| Audio | No | miniaudio, Steam Audio para oclusión |

## Regla de arquitectura

El núcleo de simulación es una biblioteca sin dependencia del renderer, que corre y se prueba sin ventana.
