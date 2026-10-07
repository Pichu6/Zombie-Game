# 0047 · One line of work: GitHub main is the only build

- **Date:** 2026-10-07
- **Status:** Decided by the user ("it cannot happen again; one build and one only; nothing duplicate; all work local and pushed to git").

## What happened

On 2026-10-06 and 07 two sessions worked in parallel without seeing each other: one on the user's PC, one in the cloud pushing to GitHub. Both built the stage 5 window and the stage 9 run, with different code, and both numbered decisions from 0022. The local line's code was dropped on merge; its documents were carried over and its decisions renumbered 0041–0046.

## Decisions

1. **`main` on GitHub is the only line.** There is one build and one implementation of each thing. Nothing is built twice in two places.
2. **Every session starts with `git fetch` and a read of `docs/status.md` from `origin/main`**, and ends with a push. Work that is not pushed does not exist for the next session.
3. **Before building anything, check it is not already built or being built:** `docs/status.md`, the open branches on GitHub (`git branch -r`), and the open pull requests. If a branch is already working on it, join that branch or wait.
4. **Decision numbers come from `origin/main`** at the moment of writing, after a fetch. A session that cannot push records its decisions as Proposals until they are merged.
5. **Cloud sessions build for Windows through CI** and hand over the zip; they say so in `docs/status.md` with the commit it was built from. Local sessions on the PC verify on MSVC and record the result. The user may have to use cloud tokens, so the cloud session must leave a Windows build behind, not a Linux-only one.
6. **A session that finds a conflict stops and asks**, as this one did, rather than merging two implementations by hand.

## Consequences

- `CLAUDE.md` carries the rule under "Before you work".
- The local work of 2026-10-07 is kept as the git tag `local-fork-2026-10-07` for reference, never merged.
