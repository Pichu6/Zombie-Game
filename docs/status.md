# Status

Updated: 2026-10-05.

## What we're building

A first playable vertical slice on our own engine (Proposal, see [vision](vision.md)):

1. Blow up a pole and the block goes dark.
2. Throw a grenade into the basement pump room and the water reaches the floor below down the stairs.
3. Repair the pole with a crafted part whose quality decides how long it lasts.
4. One character whose body changes shape, clothing that follows it, and both cameras.

## Where we are

- **Have:** design docs distilled from the prototype, recorded decisions, and the clothing research with tools tested on a synthetic body.
- **Don't have:** any engine code. The repo is not on GitHub yet (the user's account is locked by a 2FA issue); history is in local git.

## Next step

1. The user confirms or corrects the decisions marked for revalidation, and answers the open questions in the [vision](vision.md), above all which system is the hero.
2. Start the simulation core without a window, with a 2D debug view: a graph-based power grid and a room graph for water.
3. Push the repo to GitHub once the account is back.

## Waiting on the user

- Engine stack ([0002](decisions/0002-engine-stack.md)).
- Clothing via GPU wrap, and whether tight clothing hides the skin under it ([0003](decisions/0003-clothing-wrap.md)).
- Quality formula ([0006](decisions/0006-crafting-quality.md)).
- Whether "everything in the grids can be repaired or built from scratch" is a confirmed rule ([infrastructure](design/infrastructure.md)).
- Slow zombies with a long incubation, or fast ones ([threats](design/threats.md)).
