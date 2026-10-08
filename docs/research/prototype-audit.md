# Docs and agent tooling: what to keep for the new project

**Date:** 2026-10-05. **Status:** research note, not a decision. **Feeds:** [0004](../decisions/0004-docs-without-rag.md). **Evidence:** read-only audit of the old prototype at `D:/Proyectos/juego zombie`, done on 2026-10-05 by reading its files directly; nothing was changed. File sizes and counts are from that audit; the verdicts are our judgement. **IDs:** PA.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| PA-1 | The prototype carried a lot of infrastructure built to make up for agents without memory: vector RAG on Ollama, code maps, line budgets, "don't read more than 150 lines" rules, and four skills tied to Godot, Tripo and Blender | sourced | `D:/Proyectos/juego zombie` |
| PA-2 | The prototype's `AGENTS.md` (7.5 KB) held rules for Codex/Antigravity: Godot, token budget and a "lazy senior" ladder built on 8 Godot addons | sourced | `D:/Proyectos/juego zombie` `AGENTS.md` |
| PA-3 | The prototype's RAG was seven Python modules plus 5 tests, on Ollama with EmbeddingGemma and SQLite, for semantic search over its wiki | sourced | `D:/Proyectos/juego zombie` |
| PA-4 | The prototype enforced a 200-line limit per file with `file_budget.py`, plus a GDScript/Python AST map and link checks | sourced | `D:/Proyectos/juego zombie` |
| PA-5 | The prototype's `doc/wiki/` held 101 documents: 31 continuity, 43 prototype implementation, 22 design, 15 rules | sourced | `D:/Proyectos/juego zombie` `doc/wiki/` |
| PA-6 | The prototype's design document `doc/GDD_Survival_3D.md` is 74 KB | sourced | `D:/Proyectos/juego zombie` |
| PA-7 | The prototype's continuity file reached 20 KB because it was append-only | sourced | `D:/Proyectos/juego zombie` |
| PA-8 | `references/cdda_reference/` holds Cataclysm DDA data (masses, insulation, recipes); CDDA is CC BY-SA 3.0, so copying its data would bring that licence along: use it only to calibrate our own values | sourced | `D:/Proyectos/juego zombie` `references/cdda_reference/` |
| PA-9 | The *Zombie Survival Guide* and *Zombie Combat Manual* PDFs are copyrighted and stay out of the repo | estimate | this note |
| PA-10 | With about 100 short documents, a vector index adds upkeep and nothing else; direct text search and ranged reads are enough; re-evaluate at thousands of documents | estimate | this note |
| PA-11 | "Don't read more than 150-200 lines" rules make the agent read partially and understand worse; savings come from small, well-split documents | estimate | this note |
| PA-12 | A per-file line limit makes no sense in C++; the goal is one responsibility per module | estimate | this note |
| PA-13 | Replace `AGENTS.md` with a short `CLAUDE.md`, keeping the what to do / where we are / next step format and the split between proposal, implementation and validation | estimate | this note |
| PA-14 | Archive the four prototype skills, `llms.txt`, `.codex/config.toml`, `.superpowers/` and the video transcripts; don't carry over the RAG, the code map, the line budget or the local LLM | estimate | this note |
| PA-15 | Distill, don't copy: the ~22 design docs, the GDD and the confirmed decisions go into per-system docs; continuity, audits and history are archived | estimate | this note |
| PA-16 | One truth per thing: state in `status.md`, the why in `decisions/`, the how in `design/`; no append-only logs | estimate | this note |
| PA-17 | A simpler broken-link check for the docs could come back | estimate | this note |

## Short answer

The prototype has **a lot of infrastructure built to make up for agents without memory**: vector RAG on Ollama, code maps, line budgets, "don't read more than 150 lines" rules, and four skills tied to Godot, Tripo and Blender. Almost all of it is unnecessary in the new project:

- Claude searches and reads files directly (text search, ranged reads). With ~100 short documents, a vector index adds upkeep and nothing else.
- The project already has **shared memory** across all Claude sessions for preferences, decisions and project facts.
- The skills and rules describe Godot, Tripo and Blender, which are no longer the base.

What **is** valuable is the content: the game design, the decisions the user confirmed, and the lessons from what failed. That gets distilled, not copied as-is.

## Inventory and verdict

| What | What it was for | Verdict |
| --- | --- | --- |
| `AGENTS.md` (7.5 KB) | Rules for Codex/Antigravity: Godot, token budget, a "lazy senior" ladder built on 8 Godot addons | **Replace** with a short `CLAUDE.md`. Keep the "what to do / where we are / next step" format and the habit of separating proposal, implementation and validation. |
| `llms.txt` | Map for models, with Godot commands | **Archive.** |
| `.agents/skills/` (4 skills) | create-asset (Tripo, Godot, CDDA), review-human, validate-godot, resume-project | **Archive all 4.** Memory and threads already cover resume-project. If a procedure repeats in the new engine (compiling shaders, importing assets), create a new skill for it. |
| `.codex/config.toml` | Blender MCP for Codex | **Archive.** |
| RAG: `wiki_search.py`, `wiki_embeddings.py`, `wiki_vectors.py`, `wiki_corpus.py`, `wiki_lexical.py`, `wiki_retrieval.py`, `evaluate_wiki_retrieval.py`, 5 tests, Ollama + EmbeddingGemma, SQLite | Semantic search over the wiki | **Don't carry over.** With short, well-named documents, text search and an index are enough, and they don't need Ollama running. Re-evaluate if there are ever thousands of documents. |
| `repo_map.py`, `file_budget.py` + exceptions, `wiki_integrity.py`, `check_project.py` | GDScript/Python AST map, 200-line limit, link checks | **Don't carry over.** A 200-line limit per file makes no sense in C++. The goal is one responsibility per module. A simpler broken-link check for the docs could come back. |
| "Don't read more than 150–200 lines" rules | Saving tokens in other agents | **Remove.** They make Claude read partially and understand worse. Real savings come from small, well-split documents. |
| `local_llm.py`, Qwen 2.5 Coder, Ziva plugin | Local LLM in the Godot editor | **Don't carry over** for development. (A local LLM could make sense inside the game, for NPC dialogue, but that is a separate decision.) |
| `.superpowers/` and `docs/superpowers/` | Plans and specs for the prototype's clothing and pelvis | **Archive.** The lessons are already summarised in the clothing note. |
| `doc/wiki/` (101 documents: 31 continuity, 43 prototype implementation, 22 design, 15 rules) | Project memory | **Distill** the ~22 design docs and the decisions. Archive continuity, audits and history. |
| `doc/GDD_Survival_3D.md` (74 KB) | Design document | **Distill** into per-system docs. |
| `video transcripts/` (10 transcripts of videos about "GPT-6 Astra") | Inspiration | **Archive or delete.** Promotional videos with no technical value for the game. |
| *Zombie Survival Guide* and *Zombie Combat Manual* PDFs | Setting reference | Keep **out of the repo** (copyrighted books). Fine as personal reading. |
| `references/cdda_reference/` | Cataclysm DDA data (masses, insulation, recipes) | **Keep as reference.** CDDA is CC BY-SA 3.0, so copying its data into the game would bring that licence along. Use it as a guide to calibrate our own values. |

## What to distill from the prototype

Ideas, decisions and lessons, not code or assets:

- **Vision and gameplay:** the GDD, co-op gameplay, control-experience decisions, cameras and modes (third person, tactical, management).
- **World simulation:** infrastructure networks (power, water), world survival systems, chemistry/fuels/ballistics/climate, vehicles and logistics, ecology and agriculture, threats.
- **Body:** body-simulator foundations, surgery/trauma/decomposition, tissue and skeleton decisions.
- **Items and crafting:** item catalogue, interaction and inventory, interaction offers.
- **NPCs and society:** learning and specialisation, sandbox stories and relationships, inhabitant foundations.
- **Lessons:** character-production knowledge (what failed with bodies, rigs and clothing) and the decision logs.

## Structure for the new project

```
CLAUDE.md                 ← what every session reads first (under 100 lines)
docs/
  vision.md               ← the game on one page
  status.md               ← board: active fronts, next step (one screen)
  design/<system>.md      ← power, water, body, crafting and quality, cameras, NPCs...
  decisions/NNNN-topic.md ← one decision per file, short and dated (never rewritten)
  research/               ← investigations like the clothing one
engine/ ... game/ ... tools/
```

A few rules:

1. **One truth per thing.** State goes in `status.md`, the why in `decisions/`, the how in `design/`. No append-only logs: the prototype's continuity file reached 20 KB because it only grew.
2. **Proposed, implemented and validated are always kept apart.** That prototype rule was good and stays.
3. **Working preferences** ("don't spend credits without asking") go in the project memory, which every session reads.
4. **A GitHub repository** attached to the project, so each thread can work on a branch and open reviewable PRs.
