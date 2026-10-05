# 0005 · Finite world, same rules for everyone

- **Original date:** recorded in the prototype (2026-09); brought into the new project on 2026-10-05
- **Status:** Decided by the user in the prototype. Revalidate if anything changed with our own engine.

## Decisions

- The world is finite: people, items and zombies don't respawn.
- Each city's starting population is finite.
- Player and AI use the same actions and rules; the AI only chooses intentions.
- The player is independent of the inhabitant: they can give management orders, play isometric tactics, control directly, or leave NPCs autonomous.
- An inhabitant's state (inventory, effort, pain) persists when switching control or camera.
- You start with one character, meet survivors or bandits, and form or join groups. Commanding NPCs depends on the group and the leader role.
- Combat is dangerous even against a few.

Design: [inhabitants](../design/inhabitants.md), [interaction](../design/interaction.md).
