# Cs2 External


CS2 external cheat with aimbot, triggerbot, and ESP (some features incomplete) — an external cheat base!


External C++ overlay built with D3D11 and ImGui.

## Requirements

- Windows 10/11
- Visual Studio 2022 with the **Desktop development with C++** workload
- ImGui (downloaded separately)

## Dependencies

Download ImGui from https://github.com/ocornut/imgui and copy the files:

Into `imgui/`:

```
imgui.h
imgui.cpp
imgui_internal.h
imgui_draw.cpp
imgui_tables.cpp
imgui_widgets.cpp
imconfig.h
```

Into `imgui/backends/`:

```
imgui_impl_win32.h
imgui_impl_win32.cpp
imgui_impl_dx11.h
imgui_impl_dx11.cpp
```

Expected layout:

```
CS2External/
├── CS2External.sln
├── CS2External.vcxproj
├── main.cpp
├── ...
└── imgui/
    ├── imgui.h
    ├── imgui.cpp
    ├── imgui_draw.cpp
    ├── imgui_tables.cpp
    ├── imgui_widgets.cpp
    ├── imgui_internal.h
    ├── imconfig.h
    └── backends/
        ├── imgui_impl_win32.h
        ├── imgui_impl_win32.cpp
        ├── imgui_impl_dx11.h
        └── imgui_impl_dx11.cpp
```

## Build

1. Open `CS2External.sln`.
2. Select **Release** and **x64**.
3. **Build > Build Solution** (`Ctrl+Shift+B`).
4. The executable is written to `bin\Release\CS2External.exe`.

## Usage

1. Launch Counter-Strike 2.
2. Run `CS2External.exe` as administrator.
3. Press `INSERT` to open and close the menu.

## Common issues

- `Cannot open include file: 'imgui.h'`: the ImGui files are not in the folders shown above.
- `unresolved external symbol`: a required ImGui `.cpp` file is missing from the build.
- Platform Toolset error: under **Properties > General**, select the installed version (for example `v143`).
- The program closes by itself: run it as administrator and start the game first.
