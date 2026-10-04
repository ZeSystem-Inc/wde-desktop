#include <gtk/gtk.h>
#include <gio/gio.h>
#include <vector>
#include <string>
#include <algorithm>

struct AppItem {
    std::string name;
    std::string exec;
    GIcon *icon;
};

std::vector<AppItem> app_list;
GtkWidget *list_box = nullptr;
GtkWidget *search_entry = nullptr;
GtkWidget *window = nullptr;

void load_applications() {
    app_list.clear();
    const char *dirs[] = {
        "/usr/share/applications",
        "/usr/local/share/applications",
        g_build_filename(g_get_user_data_dir(), "applications", NULL)
    };

    for (const char *dir_path : dirs) {
        GDir *dir = g_dir_open(dir_path, 0, NULL);
        if (!dir) continue;

        const char *filename;
        while ((filename = g_dir_read_name(dir))) {
            if (g_str_has_suffix(filename, ".desktop")) {
                char *full_path = g_build_filename(dir_path, filename, NULL);
                GKeyFile *key_file = g_key_file_new();

                if (g_key_file_load_from_file(key_file, full_path, G_KEY_FILE_NONE, NULL)) {
                    char *nodisplay = g_key_file_get_string(key_file, "Desktop Entry", "NoDisplay", NULL);
                    if (!nodisplay || g_strcmp0(nodisplay, "true") != 0) {
                        char *name = g_key_file_get_locale_string(key_file, "Desktop Entry", "Name", NULL, NULL);
                        char *exec = g_key_file_get_string(key_file, "Desktop Entry", "Exec", NULL);
                        char *icon_str = g_key_file_get_string(key_file, "Desktop Entry", "Icon", NULL);

                        if (name && exec) {
                            // Exec parametrelerini temizle (%u, %f vb.)
                            std::string exec_clean = exec;
                            size_t pos = exec_clean.find('%');
                            if (pos != std::string::npos) exec_clean = exec_clean.substr(0, pos);

                            GIcon *icon = icon_str ? g_themed_icon_new(icon_str) : NULL;
                            app_list.push_back({name, exec_clean, icon});
                        }
                        g_free(name); g_free(exec); g_free(icon_str);
                    }
                    g_free(nodisplay);
                }
                g_key_file_free(key_file);
                g_free(full_path);
            }
        }
        g_dir_close(dir);
    }

    std::sort(app_list.begin(), app_list.end(), [](const AppItem &a, const AppItem &b) {
        return a.name < b.name;
    });
}

static void on_app_clicked(GtkButton *btn, gpointer user_data) {
    const char *exec_cmd = (const char*)user_data;
    if (fork() == 0) {
        execl("/bin/sh", "sh", "-c", exec_cmd, NULL);
        exit(0);
    }
    gtk_widget_hide(window);
}

void populate_list(const std::string &filter = "") {
    GList *children = gtk_container_get_children(GTK_CONTAINER(list_box));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    std::string filter_lower = filter;
    std::transform(filter_lower.begin(), filter_lower.end(), filter_lower.begin(), ::tolower);

    for (const auto &app : app_list) {
        std::string name_lower = app.name;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);

        if (!filter.empty() && name_lower.find(filter_lower) == std::string::npos) continue;

        GtkWidget *btn = gtk_button_new();
        GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

        if (app.icon) {
            GtkWidget *img = gtk_image_new_from_gicon(app.icon, GTK_ICON_SIZE_BUTTON);
            gtk_box_pack_start(GTK_BOX(hbox), img, FALSE, FALSE, 0);
        }

        GtkWidget *lbl = gtk_label_new(app.name.c_str());
        gtk_label_set_xalign(GTK_LABEL(lbl), 0.0);
        gtk_box_pack_start(GTK_BOX(hbox), lbl, TRUE, TRUE, 0);

        gtk_container_add(GTK_CONTAINER(btn), hbox);
        
        char *exec_copy = g_strdup(app.exec.c_str());
        g_signal_connect(btn, "clicked", G_CALLBACK(on_app_clicked), exec_copy);

        gtk_box_pack_start(GTK_BOX(list_box), btn, FALSE, FALSE, 0);
    }
    gtk_widget_show_all(list_box);
}

static void on_search_changed(GtkSearchEntry *entry, gpointer data) {
    const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
    populate_list(text ? text : "");
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "WDE Başlat");
    gtk_window_set_default_size(GTK_WINDOW(window), 380, 520);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);

    search_entry = gtk_search_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Uygulama veya Wine programı ara...");
    g_signal_connect(search_entry, "search-changed", G_CALLBACK(on_search_changed), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), search_entry, FALSE, FALSE, 0);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    
    list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add(GTK_CONTAINER(scroll), list_box);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "window { background-color: #202020; border: 1px solid #383838; border-radius: 8px; }\n"
        "entry { background-color: #2d2d2d; color: #ffffff; border: 1px solid #3f3f3f; border-radius: 6px; padding: 8px; }\n"
        "button { background: transparent; color: #ffffff; border: none; padding: 8px; border-radius: 6px; text-align: left; }\n"
        "button:hover { background-color: #2c2c2c; }\n"
        "button:active { background-color: #0078d4; }\n", -1, NULL);

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    load_applications();
    populate_list();

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
