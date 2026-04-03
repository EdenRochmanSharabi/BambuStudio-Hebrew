# Beginner Help Popup System

## Summary
Add Cmd+Click (macOS) / Ctrl+Click (Windows/Linux) help popups to ~30-50 key UI elements in BambuStudio. Targets users who have never used a slicer before. Content is based on code (ConfigOptionDef tooltips, button purposes) and rendered via the existing MarkdownTip system.

## Architecture

### Components

1. **BeginnerHelpHandler** (`src/slic3r/GUI/BeginnerHelpHandler.hpp/.cpp`)
   - Global wxEvtHandler filter registered on MainFrame
   - Intercepts mouse clicks where Cmd/Ctrl modifier is held
   - Uses `wxFindWindowAtPoint()` to identify widget under cursor
   - Resolves widget → help key via: widget name, parent ConfigOptionsGroup key, or explicit registration
   - Calls `MarkdownTip::ShowTip()` with the resolved content

2. **Help Content Map** — `resources/tooltip/beginner/_index.json`
   - Maps help keys to markdown file basenames
   - Example: `{"layer_height": "layer_height", "btn_slice": "slice_button"}`

3. **Markdown Help Files** — `resources/tooltip/beginner/*.md`
   - English beginner-friendly explanations
   - Hebrew versions in `resources/tooltip/he/beginner/*.md`
   - Format: title + what it does + why it matters + typical values

4. **MarkdownTip NDEBUG fix** — remove `#ifdef NDEBUG return false;` guard

### Widget Identification Strategy

- **Settings controls**: Walk parent chain to find `OG_CustomCtrl`, then resolve the `ConfigOptionDef` key (e.g., "layer_height", "infill_density"). These keys are stable.
- **Toolbar/buttons**: Set `wxWindow::SetName("btn_slice")` on key buttons at creation time. ~15 buttons.
- **Tabs/panels**: Set names on tab pages. ~5 tabs.
- **Fallback**: If no help entry found, do nothing (no popup).

### Target Elements (~40)

**Toolbar (8):** Slice, Print, Add Model, Delete, Arrange, Orient, Undo, Redo
**Settings tabs (3):** Print, Filament, Printer
**Key print settings (20):** layer_height, first_layer_height, wall_loops, top_shell_layers, bottom_shell_layers, sparse_infill_density, sparse_infill_pattern, support_type, support_threshold_angle, brim_type, skirt_loops, print_speed, travel_speed, temperature, bed_temperature, fan_min_speed, bridge_speed, overhang_speed, retract_length, retract_speed
**Key filament settings (5):** filament_type, filament_diameter, temperature, bed_temperature, fan_speed
**Key UI areas (4):** 3D canvas, plate tabs, device panel, preview mode

## Content Format

Each `.md` file:
```markdown
#### Layer Height / גובה שכבה

**What it does:** Controls how thick each printed layer is.

**Why it matters:** Lower values = smoother surface but slower print. Higher values = faster but rougher.

**Typical values:** 0.2mm (standard), 0.12mm (fine), 0.28mm (draft)

**Tip:** Start with 0.2mm for most prints.
```

## Implementation Steps

1. Remove NDEBUG guard from MarkdownTip
2. Create BeginnerHelpHandler class
3. Register it in MainFrame::MainFrame()
4. Add SetName() calls to key toolbar buttons
5. Create beginner/_index.json
6. Write ~40 markdown help files (English)
7. Write ~40 markdown help files (Hebrew)
8. Test Cmd+Click on various elements
