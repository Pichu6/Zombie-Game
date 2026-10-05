# Map format

Maps are TOML files. Each `[[level]]` has a `z` (−1 basement, 0 ground, 1 first floor) and an ASCII `grid` drawn at double resolution: cells sit on odd rows and odd columns, the edges between them on the even ones, and `+` marks corners (ignored).

```
+-+E+      row 0: top edges of row 0 cells     ("-" wall, "E" exterior door)
|. ,#      row 1: left edges and cells          ("|" wall, "." wood floor, "," concrete, "#" masonry wall)
+ + +      row 2: top edges of row 1 cells      (" " no edge)
```

Cells are 1 m × 1 m. Spec: [09-site-map](../../../docs/tier0/specs/09-site-map.md).

| Cell | Floor | Indoor |
| --- | --- | --- |
| ` ` | none (not walkable) | — |
| `.` | wood | yes |
| `,` | concrete or tile | yes |
| `"` | grass | no |
| `_` | asphalt | no |
| `:` | dirt or gravel | no |

| Edge | Kind | Item definition |
| --- | --- | --- |
| ` ` | none | — |
| `-` or `\|` | interior stud wall | `wall_interior` |
| `#` | exterior masonry wall | `wall_exterior` |
| `D` | interior door | `door_interior` |
| `E` | exterior door | `door_exterior` |
| `W` | window | `window` |
| `=` | fence | `fence` |
| `G` | gate | `gate` |

`[[stairs]]` join two cells `[z, x, y]` on different levels. `[[object]]` places things by `kind` (pop starts, survivors, fixtures, stockpiles) with an optional `id`.

`block.toml` is the Tier 0 block. `tools/map/make_tier0_block.py` drew its first version; the TOML file is the source of truth.
