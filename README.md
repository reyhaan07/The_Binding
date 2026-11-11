# THE BINDING

*Every connection has a consequence.*

A first-person psychological horror game in Unreal Engine 5. The story, rules and characters are in
[`Docs/THE_BINDING_Story_Bible_v2.1.pdf`](Docs/THE_BINDING_Story_Bible_v2.1.pdf).

What works so far: you can walk around as Saskia in first person, crouch, run, use a flashlight, hold your breath
(Space), and switch a lamp on and off. Your breathing and footsteps already make "noise" that the Bound One will hear
once it is built.

---

## 1. Install (once)

1. **Unreal Engine 5**: Epic Games Launcher → Unreal Engine → Library → install 5.4 or newer.
2. **Visual Studio 2022 Community** (free). In the installer tick the workload **Game development with C++**.
   You need this even if you write code in VS Code: Unreal uses Visual Studio's C++ compiler to build the game.
3. **VS Code** with the **C/C++** extension (by Microsoft), plus Claude Code and/or Codex.
4. **Git**.

## 2. Get the project and build it

1. Clone this repository to a short path, for example `C:\Games\TheBinding`.
2. If your Unreal version is not 5.7: right-click `TheBinding.uproject` → **Switch Unreal Engine version** → pick yours.
3. Double-click `TheBinding.uproject`. Unreal says the **TheBinding** module is missing and asks to rebuild it →
   **Yes**. The first build takes a few minutes.
   If it fails, open `Saved/Logs/TheBinding.log`, copy the errors and paste them to Claude or Codex.

## 3. Set up VS Code

1. In Unreal: **Edit → Editor Preferences → General → Source Code → Source Code Editor → Visual Studio Code**.
2. **Tools → Refresh Visual Studio Code Project**, then open `TheBinding.code-workspace` in VS Code.
3. To build from VS Code (with Unreal **closed**): **Terminal → Run Build Task → TheBindingEditor Win64 Development Build**.

When the AI changes code:
- only `.cpp` files changed → in Unreal press **Ctrl+Alt+F11** (Live Coding);
- a `.h` file changed or a new class was added → close Unreal, build, reopen.

## 4. First test level

1. **File → New Level → Basic**, save it as `Content/Maps/L_Test`.
2. To make it dark: in the Outliner delete **DirectionalLight** and **SkyLight** (or turn them down).
3. Open **Place Actors** (Window → Place Actors), search **Binding Lamp**, drag one onto the floor.
4. Press **Play** (Alt+P) and click in the view.

| Key | Gamepad | Does |
|---|---|---|
| W A S D | left stick | walk |
| mouse | right stick | look |
| Shift (hold) | click left stick | run |
| C or Ctrl | B | crouch / stand |
| Space (hold) | left trigger | hold breath |
| F | Y | flashlight on/off |
| E | A | use (lamps, later doors and the phone) |

**Check these:**
- Crouching lowers the view smoothly; running is faster and the view bobs more.
- Look at the lamp from close by: a dot and **"E  Switch off the lamp"** appear. Press E.
- Hold Space: a thin pale-blue line appears at the bottom and shrinks. Let go and it refills. Hold it until it runs
  out: Saskia gasps and the line turns red and pulses while she gets her breath back.
- Run for a while: the line drops (she is out of breath).

## 5. Make Blueprints from the code

Blueprints here are for choosing models and sounds and tuning numbers. The logic stays in C++ so the AI can work on it.

1. In the Content Browser, open **C++ Classes → TheBinding**. (If you can't see it: Settings → Show C++ Classes.)
2. Right-click **SaskiaCharacter** → **Create Blueprint class based on SaskiaCharacter** → name it `BP_Saskia`, save
   it in `Content/Blueprints`.
3. Do the same for **BindingGameMode** → `BP_BindingGameMode`. Open it, set **Default Pawn Class** to `BP_Saskia`,
   **Compile**, **Save**.
4. **Edit → Project Settings → Maps & Modes → Default GameMode** → `BP_BindingGameMode`.
5. Open `BP_Saskia`. In the Details panel you can now:
   - set **Footstep Sound** and **Flashlight Click Sound**;
   - click the **Breath** component and set **Gasp Sound**, **Exhale Sound**, **Heartbeat Sound**;
   - tune numbers: **Max Hold Seconds** (how long she can hold her breath), **Walk Speed**, **Flashlight Lumens**...
6. For a real lamp: make a Blueprint from **BindingLamp** and swap the **Base** and **Shade** meshes for your lamp model.

## Folders

| Folder | What's in it |
|---|---|
| `Source/TheBinding/Player` | Saskia (movement, camera, flashlight) and her breath |
| `Source/TheBinding/Interaction` | the "use" component any object can have |
| `Source/TheBinding/World` | objects in the world (the lamp) |
| `Source/TheBinding/Core` | game mode and HUD |
| `Config` | project settings |
| `Docs` | the Story Bible and the Bound One concept art |
| `AGENTS.md`, `CLAUDE.md` | the brief Codex and Claude read before working on the code |

Large files (models, sounds, textures) belong in `Content/`. When those get big, set up **Git LFS** so GitHub accepts them
(GitHub refuses single files over 100 MB).
