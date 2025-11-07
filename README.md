# DebugDraw

![debugdraw2](https://github.com/user-attachments/assets/cefaa5fc-f26e-4dde-a5e1-ed4c2a22c1a4)

This mod re-implements the disabled `sm.debugDraw` Lua API in Scrap Mechanic.

## How to use

To use the debug draw features, simply add the `-debugDraw` flag to the game's launch options, then inject the DebugDraw DLL using a DLL injector of your choice.

The mod's Lua API documentation is hosted [here](https://scrapmechanictools.com/customApis/DebugDraw).  
In case the website is down, the source markdown file for it can be found in the root of this repository (DebugDraw.md).

After injection, you can use the `sm.debugDraw` API as stated in the [API Documentation](https://scrapmechanicdocs.com/docs/Game-Script-Environment/Static-Functions/sm.debugDraw).

## Notes
- Debug drawing features are **disabled by default**. If nothing is being drawn, **make sure you added the `-debugDraw` launch option.**
- In the game script environment, the vanilla debugDraw functions stated in the API documentation **can be safely used and will not cause errors if the DLL is removed.**
- Debug draw names are not shown in the world. This is a known issue and may be fixed in an update. In the meantime, this can be emulated in Lua using nametag GUIs.

## Extra Features

This mod adds three extra features:
- `sm.debugDraw.enabled`:
  This is a boolean flag which indicates the state of the mod and can be one of three things:
  - `true`: DebugDraw DLL is present and debug drawing features are enabled.
  - `false`: DebugDraw DLL is present but not enabled (launch option not set).
  - `nil`: DebugDraw DLL is not present (debugDraw functions do nothing, extra features below are not available).  

- `sm.debugDraw.drawLine(begin, end, color)`:  
  This custom function can be used to draw a single line between two positions for a single frame.  
  To draw a line without disappearing, the function needs to be called every frame.  
  **This function is not available without the DLL, check `sm.debugDraw.enabled`.**  
  Its parameters are:
  - `begin`: `Vec3`, the start world position of the line.
  - `end`: `Vec3`, the end world position of the line.
  - `color`: `Color`, the color of the line.

- **Terrain Script Environment Support**  
  The DLL adds the `sm.debugDraw` API to the terrain script environment.  
  While this is already stated in the API documentation, the API is not actually present by default.  
  **This is not available if the DLL is removed, check `sm.debugDraw.enabled`!**

## Screenshots

Here are some extra showcasing screenshots, visualizing enemy pathfinding.  
(Note: Debug draw calls are commented out in pathfinding Lua scripts, you need to un-comment them in order to see this)  

<img width="476" height="498" alt="pathfind1" src="https://github.com/user-attachments/assets/371dd073-3437-4cfe-af6e-662ce6848535" />

<img width="473" height="497" alt="pathfind2" src="https://github.com/user-attachments/assets/71f267b7-babb-4de2-9e09-7888a14596f1" />

<img width="1083" height="594" alt="pathfind3" src="https://github.com/user-attachments/assets/18c1ee91-cad1-46c1-85ae-490574a100f6" />

