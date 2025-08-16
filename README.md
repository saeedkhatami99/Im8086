# 8086

![EMU](docs/emu.png)

---

![TUI](docs/tui.png)

## Build

### Requirements

* C++17 compiler (g++ or clang++)
* make
* ncurses (for the TUI) – on Windows use PDCurses (or run under WSL)

### Build (release default)

```bash
make
```

Outputs: build/8086emu

Debug build:

```bash
make BUILD=debug
```
