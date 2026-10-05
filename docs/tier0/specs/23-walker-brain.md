# 23 · Walker brain

**Status:** Proposal. **Stage:** 7. **Needs:** [13](13-capabilities.md), [18](18-noise-field.md), [19](19-light-field.md), [20](20-damage.md), [22](22-census.md).

## Purpose

The policy that drives a walker's body. It perceives, weighs drives and requests actions from the same body and executor as everyone ([threats](../../design/threats.md#general-rules)).

## State

| Field | Notes |
| --- | --- |
| mode | idle, investigate, pursue, attack, bash, feed |
| stimulus memory | last heard or seen position, strength, age |
| target | living body or corpse |

## Rules

1. Idle: stand, sway, drift slowly; low cost.
2. Investigate: move to the remembered stimulus; memory fades with time, so walkers that hear one loud bang mill around and wander off.
3. Pursue: a living target seen ([19](19-light-field.md)) or heard close; path to it.
4. Bash: if the path is blocked by an edge (door, window, barricade) between it and a live stimulus, push and hit it ([10](10-fortification.md)). Several walkers at one door sum their push.
5. Attack: in reach, grab then bite ([20](20-damage.md)).
6. Feed: on a fresh kill or corpse; guards it; distant noise does not distract it, a living person nearby does.
7. Followers: a walker that sees another walker move with purpose may follow it (herding). Starting chance to tune.
8. Speed is body-driven by default (Decided): body walk speed × walker coordination factor, capped below a run; fresh lean bodies faster, fat or broken ones slower, broken legs crawl. Walkers do not tire by default.
9. Uses the action executor; no teleporting, no seeing through walls.

## Invariants

- A walker only knows what its senses gave it.

## Tests

- A generator started at night after grid failure draws idle walkers within range; they reach the nearest door. (A3)
- Breaking line of sight and staying quiet loses a pursuer over time. (A3)
- Walker speeds vary with body: lean fresh > fat > broken leg. (A3)

## Calibration

| Value | Source |
| --- | --- |
| Walk speeds, coordination | Body readouts ([13](13-capabilities.md)) × preset factor; tuned for "mostly slow" |
| Senses | Same rules as humans, from the noise and light fields |

## Not in Tier 0

Smell, swimming, climbing, other threat types.
