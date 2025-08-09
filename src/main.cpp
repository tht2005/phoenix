#include <CLI/CLI.hpp>
#include <gtkmm.h>
#include <gtkmm/application.h>
#include <gtkmm/window.h>

#include <stdlib.h>
#include <string>
#include <vector>

#include "logger.hpp"

#define APP_ID "org.tht2005.phoenix"
#define APP_TITLE "phoenix"
#define APP_DESCRIPTION "phoenix - lightweight DnD source/target"
#define APP_VERSION     "1.0.0"

#define APP_CONFIG_FILE "/.config/phoenix/phoenix.ini" // relative to home

#define APP_DEFAULT_LOG_LEVEL "info"
#define APP_LIST_LOG_LEVEL "trace", "debug", "info", "warn", "err", "critical", "off"

namespace cli_option {
    bool version = false;
    bool and_exit = false;
    bool keep = false;
    bool print_path = false;
    bool all = false;
    bool all_compact = false;
    std::string log_level = APP_DEFAULT_LOG_LEVEL;
    std::string config_file_path = std::string(getenv("HOME")) + APP_CONFIG_FILE;

    std::vector<std::string> file_and_uri_list;

    enum class Mode { SOURCE, TARGET };
    Mode selected_mode;
}

class DWindow : public Gtk::Window {
public:
    DWindow() {
        // load css
        auto css = Gtk::CssProvider::create();
        css->load_from_data(
            ".rounded-border {"
            "   border: 2px dashed currentColor;"
            "   border-radius: 15px;"
            "   padding: 12px;"
            "   color: @theme_unfocused_fg_color;"
            "}"
            ".target-btn-drop-hover {"
                "background-color: color-mix(in srgb, @theme_selected_bg_color 20%, transparent);"
                "border: 2px solid @theme_selected_bg_color;"
                "color: @theme_selected_fg_color;"
            "}"
        );
        Gtk::StyleContext::add_provider_for_display(
            Gdk::Display::get_default(),
            css,
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
        );

        this->set_title(APP_TITLE);
        this->set_default_size(400, 300);

        if (cli_option::selected_mode == cli_option::Mode::SOURCE) {
            this->construct_source_ui();
        }
        else {
            this->construct_target_ui();
        }

        auto key_controller = Gtk::EventControllerKey::create();
        key_controller->signal_key_pressed().connect(sigc::mem_fun(*this, &DWindow::on_key_pressed), false);
        this->add_controller(key_controller);
    }

private:
    const Glib::ustring pwd = Glib::get_current_dir();
    const Glib::RefPtr<Gio::File> pwdFile = Gio::File::create_for_path(pwd);

    std::vector<Glib::RefPtr<Gio::File>> _keep_gfile_ref; // make sure Gio::File created not be freed
    std::vector<GFile*> all_files_list;

    Gtk::Button* target_mode_drop_btn = Gtk::make_managed<Gtk::Button> ();

    void construct_source_ui() {

        for (const std::string& file_path_or_uri : cli_option::file_and_uri_list) {
            Glib::RefPtr<Gio::File> file = Gio::File::create_for_commandline_arg(file_path_or_uri);
            const std::string uri_scheme = file->get_uri_scheme();
            if (uri_scheme == "file") {
                if (file->query_exists()) {
                    this->_keep_gfile_ref.push_back(file);
                    this->all_files_list.push_back(file->gobj());
                }
                else {
                    log()->warn("Local file {} doesn't exist, drop", file->get_path());
                }
            }
            else {
                log()->info("Uri scheme {} not supported, drop {}", uri_scheme, file->get_uri());
            }
        }

        if (cli_option::all_compact) {
            auto* btn = Gtk::make_managed<Gtk::Button> ();
            btn->set_margin(10);
            btn->set_valign(Gtk::Align::FILL);
            btn->set_halign(Gtk::Align::FILL);
            btn->set_expand();
            btn->get_style_context()->add_class("rounded-border");
            this->set_child(*btn);

            auto label = Gtk::make_managed<Gtk::Label>(std::to_string(this->all_files_list.size()) + std::string(" file(s)"));
            label->set_margin(6);
            btn->set_child(*label);

            auto source = Gtk::DragSource::create();
            source->set_actions(Gdk::DragAction::COPY);
            source->signal_prepare().connect(
                sigc::mem_fun(*this, &DWindow::on_drag_all_data_prepare),
                false
            );
            source->signal_drag_end().connect(
                sigc::mem_fun(*this, &DWindow::on_drag_end),
                false
            );
            btn->add_controller(source);
        }
        else {
            auto box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
            box->set_spacing(8);
            box->set_margin(8);
            for (const auto& file : this->_keep_gfile_ref) {
                auto btn = create_button(file);
                if (btn)
                    box->append(*btn);
            }

            auto scroll = Gtk::make_managed<Gtk::ScrolledWindow>();
            scroll->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
            scroll->set_child(*box);

            this->set_child(*scroll);

        }
    }
    
    // ensure file is a valid local file
    Gtk::Button* create_button(const Glib::RefPtr<Gio::File>& file) {
        const Glib::ustring abs_path = file->get_path();
        const Glib::ustring uri_path = file->get_uri();

        Glib::RefPtr<Gio::FileInfo> info = file->query_info();
        Glib::ustring content_type = info->get_attribute_string("standard::content-type");

        Gtk::Image *image = Gtk::make_managed<Gtk::Image>();
        image->set_pixel_size(38);
        Glib::RefPtr<Gio::Icon> icon = Gio::content_type_get_icon(content_type);
        image->set(icon);

        Glib::ustring label_text = this->pwdFile->get_relative_path(file);
        if (label_text.empty()) {
            label_text = abs_path;
        }

        Gtk::Label *label = Gtk::make_managed<Gtk::Label>(label_text);
        label->set_halign(Gtk::Align::START);
        label->set_valign(Gtk::Align::CENTER);

        // consider tuning these
        label->set_hexpand(true);
        label->set_ellipsize(Pango::EllipsizeMode::END);

        Gtk::Box *hbox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
        hbox->set_margin(8);
        hbox->set_spacing(24);
        hbox->append(*image);
        hbox->append(*label);

        auto btn = Gtk::make_managed<Gtk::Button>();
        btn->set_size_request(-1, 68);
        btn->set_child(*hbox);

        auto source = Gtk::DragSource::create();
        source->set_actions(Gdk::DragAction::COPY);
        source->signal_prepare().connect(
            sigc::bind(
                sigc::mem_fun(*this, &DWindow::on_drag_btn_data_prepare),
                file
            ),
            false
        );
        source->signal_drag_end().connect(
            sigc::mem_fun(*this, &DWindow::on_drag_end),
            false
        );
        btn->add_controller(source);
        return btn;
    }

    void construct_target_ui() {
        auto btn = this->target_mode_drop_btn;
        btn->set_margin(10);
        btn->set_valign(Gtk::Align::FILL);
        btn->set_halign(Gtk::Align::FILL);
        btn->set_expand();
        btn->get_style_context()->add_class("rounded-border");

        auto label = Gtk::make_managed<Gtk::Label>("Drag something here...");
        label->set_margin(6);
        btn->set_child(*label);

        this->set_child(*btn);

        auto target = Gtk::DropTarget::create(GDK_TYPE_FILE_LIST, Gdk::DragAction::COPY | Gdk::DragAction::MOVE);
        target->signal_drop().connect(
            sigc::mem_fun(*this, &DWindow::on_drop_data),
            false
        );
        target->signal_enter().connect(
            sigc::mem_fun(*this, &DWindow::on_drop_enter),
            false
        );
        target->signal_leave().connect(
            sigc::mem_fun(*this, &DWindow::on_drop_leave),
            false
        );
        btn->add_controller(target);
    }

    Glib::RefPtr<Gdk::ContentProvider> on_drag_all_data_prepare (double, double) {
        std::vector<GFile*> files_vector = this->all_files_list;
        GdkFileList *file_list = gdk_file_list_new_from_array(files_vector.data(), files_vector.size());
        GdkContentProvider *contentProvider = gdk_content_provider_new_typed(GDK_TYPE_FILE_LIST, file_list);
        return Glib::wrap(contentProvider);
    }

    Glib::RefPtr<Gdk::ContentProvider> on_drag_btn_data_prepare (double, double, const Glib::RefPtr<Gio::File>& file) {
        std::vector<GFile*> files_vector;
        if (cli_option::all) {
            files_vector = this->all_files_list;
        }
        else {
            files_vector.push_back(file->gobj());
        }
        GdkFileList *file_list = gdk_file_list_new_from_array(files_vector.data(), files_vector.size());
        GdkContentProvider *contentProvider = gdk_content_provider_new_typed(GDK_TYPE_FILE_LIST, file_list);
        return Glib::wrap(contentProvider);
    }

    void on_drag_end (const Glib::RefPtr<Gdk::Drag>& drag, bool delete_data) {
        if (cli_option::and_exit) {
            this->close();
        }
    }

    Gdk::DragAction on_drop_enter (double, double) {
        this->target_mode_drop_btn->add_css_class("target-btn-drop-hover");
        return Gdk::DragAction::COPY;
    }

    void on_drop_leave() {
        this->target_mode_drop_btn->remove_css_class("target-btn-drop-hover");
    }

    bool on_drop_data (const Glib::ValueBase& value, double, double) {
        Glib::Value<GSList*> gslist_value;
        gslist_value.init(value.gobj());
        auto files = Glib::SListHandler<Glib::RefPtr<Gio::File>>::slist_to_vector(gslist_value.get(), Glib::OwnershipType::OWNERSHIP_NONE);
        for (const auto& file : files) {
            if (cli_option::print_path) {
                std::cout << file->get_path() << std::endl;
            }
            else {
                std::cout << file->get_uri() << std::endl;
            }
        }
        return true;
    }

    bool on_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state) {
        switch (keyval) {
            case 'q':
                this->close();
                return true;
            default:
                return false;
        }
    }

};

int main(int argc, char **argv) {

    // *** PARSE OPTION ***

    CLI::App cli11_app{APP_DESCRIPTION};
    argv = cli11_app.ensure_utf8(argv);

    cli11_app.require_subcommand(1);
    cli11_app.set_version_flag("-v,--version", APP_VERSION, "Show version");
    cli11_app.fallthrough();
    cli11_app.positionals_at_end();

    cli11_app.set_config("-c,--config", cli_option::config_file_path, "Config file")
        ->default_str("~" APP_CONFIG_FILE);
    // cli11_app.add_flag("--version", cli_option::version, "Show version");
    cli11_app.add_option("--log-level", cli_option::log_level, "Change log level")
        ->type_name("LEVEL")
        ->default_str(APP_DEFAULT_LOG_LEVEL)
        ->check(CLI::IsMember({ APP_LIST_LOG_LEVEL }));

    CLI::App* cli11_source = cli11_app.add_subcommand("source", "Act as a drag source");

    cli11_source->add_flag("-x,--and-exit", cli_option::and_exit, "Exit after a single completed drop");
    cli11_source->add_flag("-a,--all", cli_option::all, "Drag all files at once");
    cli11_source->add_flag("-A,--all-compact", cli_option::all_compact, "Drag all files at once, only displaying the number of files");
    cli11_source->add_option("FILES", cli_option::file_and_uri_list, "Files to drag")
        ->required()
        ->expected(-1);

    CLI::App* cli11_target = cli11_app.add_subcommand("target", "Act as a drop target");

    // cli11_target->add_flag("-k,--keep", cli_option::keep, "Keep files to drag out");
    cli11_target->add_flag("-p,--print-path", cli_option::print_path, "Print file paths instead of URIs");

    CLI11_PARSE(cli11_app, argc, argv);

    // *** INIT ***

    logger::init(cli_option::log_level);

    // *** RUN ***

    if (cli11_source->parsed()) {
        cli_option::selected_mode = cli_option::Mode::SOURCE;
    }
    else if (cli11_target->parsed()) {
        cli_option::selected_mode = cli_option::Mode::TARGET;
    }
    else {
        log()->critical("CLI11 failed to parse subcommand");
        exit (1);
    }
    auto gtk_app = Gtk::Application::create(APP_ID, Gio::Application::Flags::NON_UNIQUE);
    return gtk_app->make_window_and_run<DWindow> (0, NULL);
}

