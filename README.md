# mwm

A minimal X11 tiling/floating window manager written in C, using Xlib.

Unlike the earlier compile-time version, this build reads its configuration
at runtime from `~/.config/mwm/mwmrc`, so you can change keybinds, colors,
and layout behavior without recompiling.

## Features

- Tiling layout (master + stack), monocle layout, and floating windows
- Runtime config file — no recompilation needed for keybind/appearance changes
- Per-application rules (force a class to float, or open on a given workspace)
- 9 workspaces
- Focus-follows-mouse
- Simple, dependency-free config parser (no external libraries)

## Build

Requires Xlib development headers (`libx11-dev` on Debian/Ubuntu,
`libx11` on Arch).

```sh
make
```

## Install

```sh
sudo make install
```

This installs the binary to `/usr/local/bin/mwm` and copies a default
config to `~/.config/mwm/mwmrc` if one doesn't already exist.

## Configuration

Edit `~/.config/mwm/mwmrc`. See `mwmrc.default` for the full set of
options and example keybinds. General format:

```
key = value
bind = mod+key, action, arg
rule = ClassName, workspace=N, floating=0|1
```

Available bind actions: `spawn`, `kill`, `focus_next`, `focus_prev`,
`toggle_float`, `toggle_fullscreen`, `set_layout`, `goto_workspace`,
`move_to_workspace`, `resize_master`, `quit`, `reload_config`.

## Testing without a real session

Use Xephyr to run mwm in a nested window instead of your real X session:

```sh
Xephyr -screen 1280x800 :1 &
DISPLAY=:1 ./mwm
```

## Project layout

```
src/mwm.c      — X connection and event loop
src/config.c   — runtime config file parser
src/client.c   — client (window) list management
src/layout.c   — tiling/monocle layout engine
src/actions.c  — keybind action implementations
```
