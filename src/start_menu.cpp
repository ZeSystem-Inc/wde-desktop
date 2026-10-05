#include <gtk/gtk.h>
#include <gio/gio.h>
#include <vector>
#include <string>

struct AppItem {
    std::string name;
    std::string icon;
    std::string exec;
};

void launch_app(GtkWidget *widget, gpointer data) {
    char *exec_cmd = (char *)data;
    if (exec_cmd) {
        g_spawn_command_line_async(exec_cmd, NULL);
        gtk_main_quit(); // Menüyü kapat
    }
}

std::vector<AppItem> load_system_applications() {
    std::vector<AppItem> apps;
    GList *app_list = g_app_info_get_all();

    for (GList *l = app_list; l != NULL; l = l->next) {
        GAppInfo *info = G_APP_INFO(l->data);
        if (!g_app_info_should_show(info)) continue; // Gizli uygulamaları atla

        AppItem item;
        item.name = g_app_info_get_name(info) ? g_app_info_get_name(info) : "Uygulama";
        
        GIcon *icon = g_app_info_get_icon(info);
        if (icon) {
            char *icon_str = g_icon_to_string(icon);
            item.icon = icon_str ? icon_str : "application-x-executable";
            g_free(icon_str);
        } else {
            item.icon = "application-x-executable";
        }

        item.exec = g_app_info_get_executable(info) ? g_app_info_get_executable(info) : "";
        apps.push_back(item);
    }

    g_list_free_full(app_list, g_object_unref);
    return apps;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "WDE Start Menu");
    
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_POPUP_MENU);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
    
    gtk_window_set_default_size(GTK_WINDOW(window), 520, 580);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(sidebar, 48, -1);
    gtk_style_context_add_class(gtk_widget_get_style_context(sidebar), "sidebar");

    GtkWidget *btn_power = gtk_button_new_from_icon_name("system-shutdown-symbolic", GTK_ICON_SIZE_BUTTON);
    GtkWidget *btn_settings = gtk_button_new_from_icon_name("preferences-system-symbolic", GTK_ICON_SIZE_BUTTON);
    
    gtk_box_pack_end(GTK_BOX(sidebar), btn_power, FALSE, FALSE, 5);
    gtk_box_pack_end(GTK_BOX(sidebar), btn_settings, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(main_box), sidebar, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, FALSE, 0);

    GtkWidget *middle_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_size_request(middle_box, 240, -1);

    GtkWidget *search_entry = gtk_search_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Uygulama ara...");
    gtk_box_pack_start(GTK_BOX(middle_box), search_entry, FALSE, FALSE, 5);

    GtkWidget *scroll_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    
    GtkWidget *app_list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_add(GTK_CONTAINER(scroll_window), app_list_box);

    std::vector<AppItem> apps = load_system_applications();
    for (const auto &app : apps) {
        GtkWidget *btn = gtk_button_new();
        GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

        GtkWidget *icon = gtk_image_new_from_icon_name(app.icon.c_str(), GTK_ICON_SIZE_DOCK);
        GtkWidget *label = gtk_label_new(app.name.c_str());
        gtk_label_set_xalign(GTK_LABEL(label), 0.0);

        gtk_box_pack_start(GTK_BOX(btn_box), icon, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(btn_box), label, TRUE, TRUE, 0);
        gtk_container_add(GTK_CONTAINER(btn), btn_box);

        g_signal_connect(btn, "clicked", G_CALLBACK(launch_app), g_strdup(app.exec.c_str()));
        gtk_box_pack_start(GTK_BOX(app_list_box), btn, FALSE, FALSE, 2);
    }

    gtk_box_pack_start(GTK_BOX(middle_box), scroll_window, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), middle_box, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(main_box), gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, FALSE, 0);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);

    GtkWidget *tile1 = gtk_button_new_with_label("Terminal");
    GtkWidget *tile2 = gtk_button_new_with_label("Dosyalar");
    GtkWidget *tile3 = gtk_button_new_with_label("Ağ / İnternet");
    GtkWidget *tile4 = gtk_button_new_with_label("Ayarlar");

    gtk_widget_set_size_request(tile1, 100, 80);
    gtk_widget_set_size_request(tile2, 100, 80);
    gtk_widget_set_size_request(tile3, 100, 80);
    gtk_widget_set_size_request(tile4, 100, 80);

    gtk_grid_attach(GTK_GRID(grid), tile1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), tile2, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), tile3, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), tile4, 1, 1, 1, 1);

    g_signal_connect(tile1, "clicked", G_CALLBACK(launch_app), g_strdup("x-terminal-emulator"));
    g_signal_connect(tile2, "clicked", G_CALLBACK(launch_app), g_strdup("thunar"));

    gtk_box_pack_start(GTK_BOX(main_box), grid, TRUE, TRUE, 5);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
