#include "BeginnerHelpHandler.hpp"
#include "MarkdownTip.hpp"
#include "OG_CustomCtrl.hpp"
#include "OptionsGroup.hpp"
#include "GUI_App.hpp"
#include "I18N.hpp"

#include "libslic3r/Utils.hpp"

#include <wx/window.h>
#include <wx/filename.h>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

namespace fs = boost::filesystem;

namespace Slic3r { namespace GUI {

BeginnerHelpHandler& BeginnerHelpHandler::instance()
{
    static BeginnerHelpHandler handler;
    return handler;
}

void BeginnerHelpHandler::init(wxWindow* main_frame)
{
    if (m_initialized) return;
    m_main_frame = main_frame;

    // Bind a global mouse hook on the main frame that captures all left-clicks
    main_frame->Bind(wxEVT_LEFT_DOWN, &BeginnerHelpHandler::on_mouse_down, this);

    // Pre-populate the option help map with key settings beginners need to know
    // These map ConfigOptionDef keys to beginner help markdown file names
    m_option_help_map = {
        // Print settings - Layers
        {"layer_height",              "layer_height"},
        {"initial_layer_print_height","first_layer_height"},
        // Print settings - Walls
        {"wall_loops",                "wall_loops"},
        {"wall_infill_order",         "wall_infill_order"},
        // Print settings - Top/Bottom
        {"top_shell_layers",          "top_shell_layers"},
        {"bottom_shell_layers",       "bottom_shell_layers"},
        // Print settings - Infill
        {"sparse_infill_density",     "infill_density"},
        {"sparse_infill_pattern",     "infill_pattern"},
        // Print settings - Support
        {"enable_support",            "support"},
        {"support_type",              "support_type"},
        {"support_threshold_angle",   "support_angle"},
        // Print settings - Adhesion
        {"brim_type",                 "brim"},
        {"skirt_loops",               "skirt"},
        {"raft_layers",               "raft"},
        // Print settings - Speed
        {"outer_wall_speed",          "print_speed"},
        {"inner_wall_speed",          "print_speed"},
        {"sparse_infill_speed",       "infill_speed"},
        {"travel_speed",              "travel_speed"},
        {"bridge_speed",              "bridge_speed"},
        // Print settings - Others
        {"ironing_type",              "ironing"},
        {"wall_generator",            "wall_generator"},
        {"timelapse_type",            "timelapse"},
        // Filament settings
        {"filament_type",             "filament_type"},
        {"nozzle_temperature",        "nozzle_temperature"},
        {"nozzle_temperature_initial_layer", "nozzle_temperature"},
        {"bed_temperature",           "bed_temperature"},
        {"fan_min_speed",             "cooling_fan"},
        {"fan_max_speed",             "cooling_fan"},
        {"filament_retract_length",   "retraction"},
        {"filament_retract_speed",    "retraction"},
        // Printer settings
        {"nozzle_diameter",           "nozzle_diameter"},
        {"printable_area",            "build_volume"},
    };

    // Widget name -> help key map for non-settings UI elements
    m_widget_help_map = {
        {"btn_slice",       "slice_button"},
        {"btn_print",       "print_button"},
        {"btn_add_model",   "add_model"},
        {"btn_delete",      "delete_model"},
        {"btn_arrange",     "arrange"},
        {"btn_orient",      "orient"},
        {"tab_3d_editor",   "3d_editor"},
        {"tab_preview",     "preview_tab"},
        {"tab_monitor",     "device_monitor"},
        {"tab_print",       "print_settings_tab"},
        {"tab_filament",    "filament_settings_tab"},
        {"tab_printer",     "printer_settings_tab"},
        {"canvas_3d",       "3d_canvas"},
        {"plate_tab",       "plate_tabs"},
    };

    m_initialized = true;
    BOOST_LOG_TRIVIAL(info) << "BeginnerHelpHandler initialized. Cmd/Ctrl+Click for help on "
                            << (m_option_help_map.size() + m_widget_help_map.size()) << " elements.";
}

void BeginnerHelpHandler::register_widget(wxWindow* widget, const std::string& help_key)
{
    if (widget) {
        widget->SetName(help_key);
        m_widget_help_map[help_key] = help_key;
    }
}

void BeginnerHelpHandler::on_mouse_down(wxMouseEvent& event)
{
    // Always skip so normal click handling continues
    event.Skip();

    // Check for Cmd (macOS) or Ctrl (Win/Linux) modifier
#ifdef __APPLE__
    bool modifier_held = event.CmdDown();
#else
    bool modifier_held = event.ControlDown();
#endif

    if (!modifier_held)
        return;

    wxPoint screen_pos = wxGetMousePosition();
    wxWindow* widget = wxFindWindowAtPoint(screen_pos);
    if (!widget)
        return;

    std::string help_key = resolve_help_key(widget);
    if (help_key.empty())
        return;

    BOOST_LOG_TRIVIAL(info) << "BeginnerHelp: showing help for key=" << help_key;
    show_help(help_key, screen_pos);
}

std::string BeginnerHelpHandler::resolve_help_key(wxWindow* widget)
{
    // Strategy 1: Check if this widget or any ancestor is a named widget in our map
    wxWindow* w = widget;
    for (int depth = 0; w && depth < 10; ++depth) {
        std::string name = w->GetName().ToStdString();
        auto it = m_widget_help_map.find(name);
        if (it != m_widget_help_map.end())
            return it->second;
        w = w->GetParent();
    }

    // Strategy 2: Check if this is inside an OG_CustomCtrl (settings panel)
    std::string opt_key = resolve_option_key(widget, wxGetMousePosition());
    if (!opt_key.empty()) {
        auto it = m_option_help_map.find(opt_key);
        if (it != m_option_help_map.end())
            return it->second;
    }

    return "";
}

std::string BeginnerHelpHandler::resolve_option_key(wxWindow* widget, const wxPoint& screen_pos)
{
    // Walk up to find an OG_CustomCtrl
    wxWindow* w = widget;
    OG_CustomCtrl* og_ctrl = nullptr;
    for (int depth = 0; w && depth < 10; ++depth) {
        og_ctrl = dynamic_cast<OG_CustomCtrl*>(w);
        if (og_ctrl)
            break;
        w = w->GetParent();
    }

    if (!og_ctrl || !og_ctrl->opt_group)
        return "";

    // Get the option group and find which option is under the cursor
    OptionsGroup* og = og_ctrl->opt_group;
    wxPoint local_pos = og_ctrl->ScreenToClient(screen_pos);

    // Iterate through the option group's lines to find the option under the cursor
    for (const auto& line : og->get_lines()) {
        for (const auto& opt : line.get_options()) {
            const std::string& opt_id = opt.opt_id;
            auto it = m_option_help_map.find(opt_id);
            if (it != m_option_help_map.end()) {
                Field* field = og->get_field(opt_id);
                if (field && field->getWindow()) {
                    wxRect field_rect = field->getWindow()->GetRect();
                    // Expand the hit area to include the label (to the left)
                    field_rect.x -= 300;
                    field_rect.width += 300;
                    field_rect.height += 10;
                    if (field_rect.Contains(local_pos))
                        return opt_id;
                }
            }
        }
    }

    // Fallback: if the OG has only one line with one option, return that
    const auto& lines = og->get_lines();
    if (lines.size() == 1 && lines.front().get_options().size() == 1)
        return lines.front().get_options().front().opt_id;

    return "";
}

void BeginnerHelpHandler::show_help(const std::string& help_key, const wxPoint& screen_pos)
{
    // Use the beginner/ prefix for our help files
    std::string tip_path = "beginner/" + help_key;
    std::string fallback_tooltip = "";

    // Try to load beginner help content; MarkdownTip will search:
    //   resources/tooltip/{language}/beginner/{help_key}.md
    //   resources/tooltip/beginner/{help_key}.md
    wxPoint popup_pos = screen_pos;
    popup_pos.x += 20;
    popup_pos.y -= 10;

    MarkdownTip::ShowTip(tip_path, fallback_tooltip, popup_pos);
}

bool BeginnerHelpHandler::has_help_content(const std::string& help_key)
{
    fs::path ph = resources_dir();
    ph /= "tooltip/beginner/" + help_key + ".md";
    return fs::exists(ph);
}

std::string BeginnerHelpHandler::load_help_content(const std::string& help_key)
{
    fs::path ph = resources_dir();
    ph /= "tooltip/beginner/" + help_key + ".md";

    if (!fs::exists(ph))
        return "";

    wxFile f;
    wxString file = wxString::FromUTF8(ph.string());
    if (f.Open(file)) {
        std::string content(f.Length(), 0);
        f.Read(&content[0], content.size());
        return content;
    }
    return "";
}

}} // namespace Slic3r::GUI
