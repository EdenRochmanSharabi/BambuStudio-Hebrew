#ifndef slic3r_BeginnerHelpHandler_hpp_
#define slic3r_BeginnerHelpHandler_hpp_

#include <wx/event.h>
#include <string>
#include <map>

namespace Slic3r { namespace GUI {

// Intercepts Cmd+Click (macOS) / Ctrl+Click (Win/Linux) on key UI elements
// and shows beginner-friendly help popups via MarkdownTip.
//
// Widget identification strategy:
//   1. For settings controls (OG_CustomCtrl): walks parent chain, resolves opt_id
//   2. For named widgets: uses wxWindow::GetName()
//   3. Falls back to checking parent names up the hierarchy
//
// Help content lives in resources/tooltip/beginner/*.md files.

class BeginnerHelpHandler
{
public:
    static BeginnerHelpHandler& instance();

    // Call once from MainFrame constructor
    void init(wxWindow* main_frame);

    // Register a widget for beginner help by name -> help_key mapping
    void register_widget(wxWindow* widget, const std::string& help_key);

private:
    BeginnerHelpHandler() = default;

    // The global mouse event filter
    void on_mouse_down(wxMouseEvent& event);

    // Try to resolve a help key from the widget under the cursor
    std::string resolve_help_key(wxWindow* widget);

    // Try to find opt_id from an OG_CustomCtrl at the given screen position
    std::string resolve_option_key(wxWindow* widget, const wxPoint& screen_pos);

    // Show the beginner help popup
    void show_help(const std::string& help_key, const wxPoint& screen_pos);

    // Check if a help markdown file exists for this key
    bool has_help_content(const std::string& help_key);

    // Build the markdown content for a help key
    std::string load_help_content(const std::string& help_key);

    // Map of widget name -> help key
    std::map<std::string, std::string> m_widget_help_map;

    // Map of config option key -> help key (for settings)
    std::map<std::string, std::string> m_option_help_map;

    wxWindow* m_main_frame = nullptr;
    bool m_initialized = false;
};

}} // namespace Slic3r::GUI

#endif // slic3r_BeginnerHelpHandler_hpp_
