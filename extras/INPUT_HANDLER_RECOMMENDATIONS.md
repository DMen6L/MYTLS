# Input Handler Recommendations

Goal: a Neovim-style input handler — modal editing, key sequences, command dispatch — integrated into FTXUI via `CatchEvent`.

---

## Bug to fix first

`extras/CMakeLists.txt` line 2 has two errors:

```cmake
# wrong
target_include_libraries(extras_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}))

# correct
target_include_directories(extras_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
```

- `target_include_libraries` is not a CMake command — it should be `target_include_directories`.
- There is a stray extra `)` at the end.

---

## Approach: Pure C++ (recommended)

No extra libraries needed. FTXUI already delivers every keystroke through `CatchEvent` as an `ftxui::Event`. Everything else is plain C++ state machines and data structures.

---

## Core concepts

### 1. Mode enum

```cpp
enum class Mode { Normal, Insert, Visual, Command };
```

The handler holds the current mode and routes keys differently depending on it.

### 2. Key buffer (for multi-key sequences)

Neovim handles sequences like `dd`, `gg`, `ci"`, `<leader>w` by accumulating characters in a pending buffer and flushing when a complete sequence is recognized or timed out.

```cpp
std::string key_buffer; // accumulates pending keystrokes
```

On each keypress in Normal mode:
1. Append the character to `key_buffer`.
2. Check `key_buffer` against a keymap (exact match → dispatch, prefix match → wait, no match → clear).
3. Optionally start a short timer to clear a dangling buffer (FTXUI has no built-in timer, but you can post a message from a thread or track a frame counter).

### 3. Action dispatch

Use a flat map from key sequence string to a callable:

```cpp
using Action = std::function<void()>;
std::unordered_map<std::string, Action> normal_keymap;
std::unordered_map<std::string, Action> command_keymap;
```

Register actions at construction time:

```cpp
normal_keymap["dd"] = [&]{ delete_current_line(); };
normal_keymap["gg"] = [&]{ move_to_top(); };
normal_keymap["i"]  = [&]{ set_mode(Mode::Insert); };
normal_keymap[":"]  = [&]{ set_mode(Mode::Command); };
```

This keeps dispatch O(1) and dead-simple to extend.

### 4. Command buffer (`:` mode)

When mode == `Command`, keystrokes go into a separate `std::string command_buf` that is rendered as a command line at the bottom of the screen. On `Enter`, parse and execute the command; on `Escape`, clear and return to Normal.

```cpp
std::string command_buf;
```

---

## FTXUI integration point

The right hook is `CatchEvent`. Wrap your top-level component:

```cpp
auto app = CatchEvent(inner_component, [&](Event e) -> bool {
    return input_handler.handle(e);  // returns true = event consumed
});
```

Inside `handle()`:
- `e.is_character()` — printable key, use `e.character()` to get it.
- `e == Event::Escape` — mode switch or cancel.
- `e == Event::Return` — confirm.
- `e == Event::Backspace` — delete in Insert/Command mode.
- `e == Event::ArrowUp/Down/Left/Right` — cursor movement.
- `e.input()` — raw byte string for anything else (e.g. `"\x1b[A"` for Up).

Returning `true` from the handler prevents FTXUI from processing the event further (same as Neovim consuming a key before it reaches the editor).

---

## Suggested struct layout for `InputHandler`

```
InputHandler
├── Mode current_mode
├── std::string key_buffer          (Normal mode pending sequence)
├── std::string command_buf         (Command mode line)
├── std::unordered_map normal_map   (sequence → Action)
├── std::unordered_map insert_map   (optional overrides in Insert)
├── std::unordered_map command_map  (built-in :commands)
└── bool handle(ftxui::Event)       (main entry point from CatchEvent)
```

---

## Prefix detection without a trie

For a small keymap (< 200 entries) a linear prefix scan over the map keys is fast enough:

```cpp
bool has_prefix_match(const std::string& buf,
                      const std::unordered_map<std::string, Action>& map) {
    for (auto& [key, _] : map)
        if (key.starts_with(buf)) return true;
    return false;
}
```

If the keymap grows large, replace with a **trie**. Each node stores a map of `char → child node` and an optional `Action`. Traversal is O(sequence length) regardless of keymap size.

---

## Timeout for dangling sequences

FTXUI's event loop blocks waiting for input, so you can't rely on a wall-clock timeout from within `CatchEvent`. Two options:

1. **No timeout** — clear `key_buffer` whenever a key doesn't extend any known prefix. Simple and predictable (Helix-style).
2. **Background thread** — spawn a thread that posts a custom `Event` (via `screen.PostEvent`) after N ms of inactivity to trigger a buffer clear (Neovim-style `timeoutlen`).

Option 1 is the right starting point. Add option 2 only if the UX demands it.

---

## Libraries worth knowing (but probably not needed)

| Library | What it adds | Worth using? |
|---|---|---|
| `libtermkey` | Parses raw terminal escape sequences into structured key events | No — FTXUI already does this |
| `libtickit` | Full terminal input + rendering | No — conflicts with FTXUI |
| `replxx` | Readline-like line editing with history | Only if you want a rich `:` command line with completion |
| `lua` / `sol2` | Scripted keymap config like Neovim | Only if you want a user-configurable keymap at runtime |

For this project, pure C++ + FTXUI's `CatchEvent` is sufficient for everything Neovim does at the input layer.

---

## Recommended build order

1. Fix the `extras/CMakeLists.txt` bug (see top).
2. Define `Mode` enum and basic `InputHandler` struct in `input_handler.hpp`.
3. Implement `handle(Event)` with Normal/Insert/Command routing in `input_handler.cpp`.
4. Wire into the todo app via `CatchEvent` to validate the design.
5. Extend the keymap table as needed per app.

---

## Code review of `extras/input_handler/` (first pass)

The direction is right — a horizontal bar at the top that shows mode + input text is a clean starting point. These are the bugs standing between the current code and something that compiles and runs.

### `input_handler.hpp`

**1. Typo in `toString` parameter type**
```cpp
// wrong — IncludeMode is not defined anywhere
std::string_view toString(const IncludeMode &mode) { ... }

// correct
std::string_view toString(const InputMode &mode) { ... }
```

**2. Function definition in a `.hpp` will cause ODR violations**
`toString` is *defined* (not just declared) in the header. If two `.cpp` files ever include it, you get a linker error ("multiple definition of toString"). Fix with `inline`:
```cpp
inline std::string_view toString(const InputMode &mode) { ... }
```
Or move the definition to `input_handler.cpp` and keep only the declaration in the header.

---

### `input_handler.cpp`

**3. Wrong include paths — missing `.hpp` extension**
```cpp
// wrong
#include <ftxui/component/component>
#include <ftxui/component/event>

// correct
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
```

**4. `text()` is a DOM Element, not a Component — can't go in `Container::Horizontal`**
`Container::Horizontal({...})` takes a `Components` list (i.e. `std::vector<Component>`).
`text(...)` returns an `Element` (the DOM side). Mixing them will not compile.

The mode label is display-only — it doesn't need to be a component. Put it only in the `Renderer` lambda, not in the container:
```cpp
// correct: only the interactive Input goes in the container
auto container = Container::Horizontal({ input });

auto renderer = Renderer(container, [input, &input_state] {
    return hbox({
        text(std::string(toString(input_state.mode))) | bold,
        text(" "),
        input->Render(),   // render the Input component directly
    });
});
```

**5. Missing `return` statement — function returns nothing**
`MakeInputHandler` is declared to return `ftxui::Component` but the body never returns. Add:
```cpp
return renderer;
```
at the end of the function.

---

### Design gap: no mode switching yet

The current code renders the mode label but nothing changes it. The `Input` component from FTXUI always stays in "insert" behaviour. To actually switch modes you need a `CatchEvent` layer that:
- Intercepts `Escape` → sets `input_state.mode = InputMode::Normal` and stops the Input from consuming keys
- Intercepts `i` in Normal mode → sets `InputMode::Insert` and lets the Input take over

Without this, the mode display is cosmetic only. That logic belongs in `MakeInputHandler` wrapping the renderer with `CatchEvent`. Example sketch:
```cpp
auto catchEvent = CatchEvent(renderer, [&input_state](Event e) -> bool {
    if (input_state.mode == InputMode::Normal) {
        if (e.is_character() && e.character() == "i") {
            input_state.mode = InputMode::Insert;
            return true;   // consumed
        }
        return true;  // in Normal mode, swallow all printable keys so Input doesn't see them
    }
    // Insert mode
    if (e == Event::Escape) {
        input_state.mode = InputMode::Normal;
        return true;
    }
    return false;  // let Input handle everything else
});
return catchEvent;
```

---

### Summary of required fixes before it compiles

| # | File | Issue |
|---|------|-------|
| 1 | `.hpp` | `IncludeMode` → `InputMode` typo |
| 2 | `.hpp` | `toString` needs `inline` (or move to `.cpp`) |
| 3 | `.cpp` | Add `.hpp` to both ftxui includes |
| 4 | `.cpp` | Remove `text(toString(...))` from `Container::Horizontal` |
| 5 | `.cpp` | Add `return renderer;` (or `return catchEvent;` once you add mode switching) |
