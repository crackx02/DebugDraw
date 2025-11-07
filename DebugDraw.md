---
title: DebugDraw
hide_title: true
---

<br></br>

Documentation for the [DebugDraw](https://github.com/crackx02/DebugDraw) DLL mod's Lua API functions.

## sm.debugDraw

The DebugDraw DLL re-implements and enables the disabled (but already existing) `sm.debugDraw` functions.  
Documentation for these functions already exists [here](https://scrapmechanictools.com/lua/Game-Script-Environment/Static-Functions/sm.debugDraw).  
However, the DLL also expands the API a little bit - these extra features are documented below.

### sm.debugDraw.enabled

```lua
local enabled = sm.debugDraw.enabled
```

Not a function, but a boolean flag indicating the state of the mod:
- `true` means the DLL is loaded and enabled
- `false` means the DLL is loaded but disabled (game launch flag not set)
- `nil` means the DLL is not loaded (vanilla `debugDraw` functions do nothing; extra features below are not available)

### drawLine

```lua
sm.debugDraw.drawLine(from, to, color)
```

Draws a single debug line with the given color for a single frame.  
To keep the line from immediately disappearing, the function needs to be called every frame.

<strong>Parameters:</strong> <br></br>

- `from` (**[Vec3](https://scrapmechanictools.com/lua/Game-Script-Environment/Userdata/Vec3)**): The start position of the line.
- `to` (**[Vec3](https://scrapmechanictools.com/lua/Game-Script-Environment/Userdata/Vec3)**): The end position of the line.
- `color` (**[Color](https://scrapmechanictools.com/lua/Game-Script-Environment/Userdata/Color)**): The color of the line.

### Terrain Script Environment

The DLL also enables the debugDraw API to be used from the terrain script environment.  
The API there is the same as in the game script environment.
