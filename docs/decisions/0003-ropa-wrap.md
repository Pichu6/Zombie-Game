# 0003 · Ropa por wrap en GPU

- **Fecha:** 2026-10-05
- **Estado:** Propuesta

## Propuesta

La ropa no tiene shape keys ni ajustes por prenda. Cada vértice de la prenda guarda un binding a la superficie del cuerpo (triángulo, baricéntricas, desplazamiento en base local). Un compute shader la recoloca cada frame después del morph y el skinning del cuerpo. Los detalles con relieve (bolsillos, tapas, correas) son módulos atados a la prenda base. La suciedad y la sangre van en máscaras sobre un UV común del cuerpo.

## Requisito

Todos los cuerpos comparten una topología, un UV y un esqueleto, y cambian por morphs.

## Abierto

¿La ropa ceñida oculta la piel que cubre (salvo heridas o vista anatómica)? Recomendación: sí.

Detalle: [docs/research/ropa-wrap.md](../research/ropa-wrap.md)
