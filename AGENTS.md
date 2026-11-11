# THE BINDING: notes for AI coding assistants

Read this before changing anything. It is the shared brief for every AI tool working on this project (Codex reads
`AGENTS.md`, Claude Code reads `CLAUDE.md`, which points here). The full canon is `Docs/THE_BINDING_Story_Bible_v2.1.pdf`;
when this file and the Story Bible disagree, the Story Bible wins and you should tell the developer.

## The project

| | |
|---|---|
| Game | THE BINDING, first-person psychological horror, single player |
| Engine | Unreal Engine 5 (5.4 or newer), C++ project with Blueprints for tuning |
| Platform | Windows PC first |
| Developer | A beginner solo developer. Explain what you change in plain words, and say exactly what to click or test. |

## The story in one minute

One night of a great flood in the fens. **Saskia** (38, a former freediver who can hold her breath for a very long
time) is in the family cottage. Her sister **Tess** is trapped across the water in the old engine house (the pumping
station). Their only contact is the old wall telephone.

As children they played **the Binding**: write `THING = THING` on a paper tag, tie it on, touch both things, count to
three, and the two things are joined: whatever happens to one happens to the other. It is real. Saskia uses it all
night to open Tess's way out, room by room.

**Mum's four rules**, painted on the kitchen wall:
1. NO MATCHING. NOT EVEN FOR FUN.
2. NEVER PUT A NAME ON A TAG.
3. IF YOU HEAR COUNTING, LET GO.
4. DON'T LET TESS SLEEP.

**The secret:** when Tess was a baby she kept stopping breathing in her sleep, and six-year-old Saskia wrote
`SASKIA = TESS`. Their breathing has been joined ever since: when Tess sleeps, Saskia breathes for her. In the 1994
flood that tag fell into the water, and something drowned in the fen found it and their names.

**The Bound One** (the family call it "the Thing"): eyeless, starved, wound in grave linen, crawling on its hands
with two extra pairs of long limbs out of its back, its whole head splitting open to breathe. It has no breath of its
own and wants theirs. It is blind and **hunts by the sound of breathing**. It moves **through linked things**. It knows
the game and **counts to three**. If it takes enough of your breath, it **walks you into the water**.

## Design pillars (do not break these)

1. **Linking is the tool; breaking a link is the good act.** Every link helps Tess and is also a road for the Bound One.
   Tearing or burning a tag is always safe.
2. **Breath is the game.** Holding breath hides Saskia, keeps her alive, and ends the game (one held breath in the
   flooded engine pit).
3. **Heard before it is seen.** Show the Bound One rarely, in the dark, lit only by the torch or a failing lamp:
   glimpses at the edge of the beam, a limb on a wall, the steam of its breath. Full reveals only at the big moments.
4. **Realism above everything.** Real places, real water, real sound. No gore. No jump-scare spam.
5. **One night, two sisters, one telephone.** Their relationship is the heart of the game.

Readability rules: every threat has a tell before it acts (a dimming lamp, knocking pipes, "One..."); every rule is
shown once safely before it can hurt the player; linked objects always visibly carry a tag. **The game never lets the
player write a name on a tag.**

## Systems: what exists and what comes next

Status: **done** = in the code now; **next** = build in roughly this order.

| System | Status | Where / how |
|---|---|---|
| First-person Saskia: walk, run, crouch, look, head bob, breathing camera | done | `Player/SaskiaCharacter` |
| Flashlight with hand lag and `FlickerFlashlight` for scares | done | `Player/SaskiaCharacter` |
| Breath: air meter, hold (silent, drains), gasp at 0 (loud), panting, heart rate with diving reflex, `ExtraDrainPerSecond` hook | done | `Player/BreathComponent` |
| Noise: breath and footsteps reported to AI hearing with tags `Breath`, `Gasp`, `Footstep` | done | `UAISense_Hearing::ReportNoiseEvent` |
| Use things (E): look at an actor with an `InteractableComponent` | done | `Interaction/InteractableComponent` |
| Test lamp: on/off, `SetListening` (the Bound One's tell), `OnLitChanged` | done | `World/BindingLamp` |
| Quiet HUD: use prompt, breath line | done | `Core/BindingHUD` |
| **Linkable component**: states on/off, open/closed, wet/dry, sound; applies state changes | next | `Binding/LinkableComponent` |
| **Link graph**: world subsystem; nodes = linkables, edges = tags (words, whole/broken); propagates state both ways; the Bound One travels along edges | next | `Binding/LinkSubsystem` (UWorldSubsystem) |
| **Tags**: write (never a name), tie, touch-and-hold count of three, tear (instant), burn (needs matches) | next | `Binding/` |
| **Phone**: wall crank phone, branching voice lines, Tess's wakefulness falls over time and rises when Saskia talks or rings | next | `Story/` |
| **The Bound One presence AI**: hears noise near linked nodes, moves node to node, drives the tells (lamp dimming, pipe knocks, water) | next | `Thing/` |
| **Counting event**: "One... Two..." in Saskia's six-year-old voice; let go / tear / burn before "three" | next | `Thing/` |
| **Third breath**: Tess asleep -> her breath runs through Saskia's meter, a third breath behind your ear | next | `Story/` + `BreathComponent::ExtraDrainPerSecond` |
| **Blackout and sleepwalk**: fade to black, wake waist-deep in the flood outside the open front door, wade back | next | `Story/` |
| **Swimming and diving** using the breath meter | later | `Player/` |
| **Story flags and save** at each phone call | later | `Story/` |

## Code rules

- **All logic in C++** under `Source/TheBinding/<Area>/`. Blueprints are only for choosing assets (meshes, sounds,
  animations) and tuning numbers. AI tools cannot read or edit `.uasset` files, so never put logic in them.
- Expose tunable numbers as `UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "...")` with a comment saying what
  the number means in the game (units in cm, seconds, 0-1).
- Expose moments that need sound or effects as `BlueprintAssignable` delegates (`OnGasp`, `OnLitChanged`), so the
  developer can hook sounds up in Blueprint without code.
- Sound and asset references are optional `UPROPERTY` fields that may be empty: the code must work with none set.
- Unreal naming: `A` actors, `U` objects/components, `F` structs, `E` enums. Game classes start with `Binding` or name
  what they are (`SaskiaCharacter`, `BreathComponent`).
- Log with `UE_LOG(LogBinding, ...)` (declared in `TheBinding.h`).
- Do not add plugins, change engine settings or rename existing classes without asking the developer first. Renaming
  a class breaks every Blueprint made from it.
- Comments explain *why* in game terms (point to the Story Bible section when a rule comes from the story).
- Input is built in code at runtime in `ASaskiaCharacter::EnsureInput` (no input assets needed). Controls: WASD/left
  stick move, mouse/right stick look, Shift run, C/Ctrl crouch, Space hold breath, F flashlight, E use.

## Building and testing (you cannot run the editor; the developer does)

- After a change to a **`.h` file** or a new class: the developer must **close Unreal**, then build
  (`TheBindingEditor Win64 Development`) and reopen. After changing only **`.cpp` files**: Live Coding
  (`Ctrl+Alt+F11` in the editor) is enough.
- End every change with a short **"How to test"** list: what to place in the level, what to press, what should happen.
- If the developer pastes a build error, fix the cause, not the symptom. Never "fix" an error by deleting a feature.

## Working with two AI tools on one project

- One tool, one task at a time. The developer commits with git when a step works, before the next task.
- Read the current file before editing; never undo another tool's work without saying so.
- Keep each change small enough to test in a few minutes.

## Never

- Commit `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/` (see `.gitignore`).
- Let the player write a name on a tag.
- Show the Bound One in full light, or add gore.
