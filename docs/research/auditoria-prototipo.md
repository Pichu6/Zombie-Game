# Documentación y herramientas de agentes: qué conservar para el proyecto nuevo

Auditoría de sólo lectura de `D:/Proyectos/juego zombie`, hecha el 2026-10-05. No se modificó nada.

## Respuesta corta

El prototipo tiene **mucha infraestructura para compensar agentes sin memoria**: RAG vectorial con Ollama, mapas de código, presupuestos de líneas, reglas de "no leer más de 150 líneas" y cuatro skills atadas a Godot, Tripo y Blender. Casi todo eso sobra en el proyecto nuevo:

- Yo busco y leo archivos directamente (búsqueda de texto, lectura por rangos), así que con unos ~100 documentos cortos un índice vectorial no aporta y cuesta mantenimiento.
- Este proyecto ya tiene **memoria compartida** entre todas las sesiones de Claude, para preferencias, decisiones y hechos del proyecto.
- Las skills y reglas describen Godot, Tripo y Blender, que ya no son la base.

Lo que **sí vale oro** es el contenido: el diseño del juego, las decisiones que confirmaste y las lecciones de lo que falló. Eso hay que destilarlo, no copiarlo tal cual.

## Inventario y veredicto

| Qué hay | Para qué servía | Veredicto |
| --- | --- | --- |
| `AGENTS.md` (7,5 KB) | Reglas para Codex/Antigravity: Godot, presupuesto de tokens, escalera "Senior perezoso" con 8 addons de Godot | **Reemplazar** por un `CLAUDE.md` corto. Conservar la idea de "qué hay que hacer / dónde estamos / siguiente paso" y la de distinguir propuesta, implementación y validación. |
| `llms.txt` | Mapa para modelos con comandos de Godot | **Archivar.** |
| `.agents/skills/` (4 skills) | crear-asset (Tripo, Godot, CDDA), revisar-humano, validar-godot, retomar-proyecto | **Archivar las 4.** `retomar-proyecto` ya lo hago yo con la memoria y los hilos. Si en el motor nuevo se repite un procedimiento (compilar shaders, importar assets), se crea una skill nueva para eso. |
| `.codex/config.toml` | MCP de Blender para Codex | **Archivar.** |
| RAG: `wiki_search.py`, `wiki_embeddings.py`, `wiki_vectors.py`, `wiki_corpus.py`, `wiki_lexical.py`, `wiki_retrieval.py`, `evaluate_wiki_retrieval.py`, 5 tests, Ollama + EmbeddingGemma, SQLite | Buscar en la wiki por significado | **No migrar.** Con documentos cortos y bien nombrados, la búsqueda de texto y un índice bastan, y no dependen de que Ollama esté corriendo. Si algún día hay miles de documentos, se reevalúa. |
| `repo_map.py`, `file_budget.py` + excepciones, `wiki_integrity.py`, `check_project.py` | Mapa AST de GDScript/Python, límite de 200 líneas, enlaces | **No migrar.** El límite de 200 líneas por archivo no tiene sentido en C++. Lo que conviene es que cada módulo tenga una sola responsabilidad. Un chequeo de enlaces rotos en la documentación sí puede volver, más simple. |
| Reglas "no leer más de 150–200 líneas" | Ahorrar tokens en otros agentes | **Eliminar.** A mí me hace entender peor (leo a medias). El ahorro real viene de documentos chicos y bien separados. |
| `local_llm.py`, Qwen 2.5 Coder, plugin Ziva | LLM local en el editor de Godot | **No migrar** para el desarrollo. (Un LLM local podría tener sentido dentro del juego, para diálogos de NPC, pero eso es otra decisión.) |
| `.superpowers/` y `docs/superpowers/` | Planes y specs de la ropa y la pelvis del prototipo | **Archivar.** Las lecciones ya están resumidas en la nota de ropa nueva. |
| `doc/wiki/` (101 documentos: 31 de continuidad, 43 de implementación del prototipo, 22 de diseño, 15 de reglas) | Memoria del proyecto | **Destilar** los ~22 de diseño y las decisiones. Archivar continuidad, auditorías e historial. |
| `doc/GDD_Survival_3D.md` (74 KB) | Documento de diseño | **Destilar** en documentos por sistema. |
| `video transcripts/` (10 transcripciones de videos sobre "GPT-6 Astra") | Inspiración | **Archivar o borrar.** Son videos promocionales, sin información técnica útil para el juego. |
| PDFs de *Zombie Survival Guide* y *Zombie Combat Manual* | Referencia de ambientación | Mantenerlos **fuera del repositorio** (son libros con copyright). Sirven como lectura personal. |
| `references/cdda_reference/` | Datos de Cataclysm DDA (masas, aislamiento, recetas) | **Conservar como referencia.** CDDA es CC BY-SA 3.0, así que copiar sus datos al juego obligaría a esa licencia. Se usan como guía para calibrar valores propios. |

## Qué hay que destilar del prototipo

Son ideas, decisiones y lecciones, no código ni assets:

- **Visión y gameplay:** GDD, `gameplay-cooperativo`, `decisiones-experiencia-control`, cámaras y modos (tercera persona, táctico, gestión).
- **Simulación del mundo:** `redes-infraestructura-servicios` (energía, agua), `sistemas-supervivencia-mundo`, `quimica-combustibles-balistica-clima`, `vehiculos-interiores-logistica`, `ecologia-fauna-agricultura`, `amenazas-sandbox*`.
- **Cuerpo:** `bases-simulador-corporal*`, `cirugia-trauma-descomposicion`, `decisiones-tejidos-esqueletos`.
- **Objetos y crafteo:** `catalogo-items`, `interaccion-control-inventario`, `ofertas-interaccion`.
- **NPC y sociedad:** `aprendizaje-especializacion`, `historias-relaciones-sandbox`, `base-funcional-habitantes*`.
- **Lecciones:** `conocimiento-produccion-personajes*` (qué falló con cuerpos, rigs y ropa) y `decisiones*.md`.

## Estructura propuesta para el proyecto nuevo

```
CLAUDE.md                 ← lo que cualquier sesión lee primero (menos de 100 líneas)
docs/
  vision.md               ← el juego en una página
  status.md               ← tablero: frentes activos, siguiente paso (una pantalla)
  design/<sistema>.md     ← energía, agua, cuerpo, crafteo y calidad, cámaras, NPC...
  decisions/NNNN-tema.md  ← una decisión por archivo, corta y con fecha (no se reescribe)
  research/               ← investigaciones como la de la ropa
engine/ ... game/ ... tools/
```

Reglas, pocas:

1. **Una verdad por cosa.** El estado va en `status.md`, el porqué en `decisions/` y el cómo en `design/`. Nada de entradas acumuladas: `continuidad.md` del prototipo llegó a 20 KB porque sólo crecía.
2. **Propuesto, implementado y validado se distinguen siempre.** Esa regla del prototipo era buena y se conserva.
3. **Las preferencias de trabajo** ("respondé en español", "no gastar créditos sin preguntar") van a la memoria del proyecto, que todas las sesiones leen.
4. **Un repositorio en GitHub** conectado al proyecto, para que cada hilo pueda trabajar en una rama y abrir PRs revisables.

## Siguiente paso sugerido

1. Crear el repositorio nuevo con este esqueleto y un `CLAUDE.md` inicial.
2. Destilar el diseño del prototipo en `docs/design/` y `docs/decisions/`, revisando con vos las decisiones que ya no aplican (todas las atadas a Godot).
3. El prototipo queda intacto en tu disco como referencia de sólo lectura.
