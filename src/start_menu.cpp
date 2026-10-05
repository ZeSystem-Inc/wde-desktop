#include <gtk/gtk.h>
#include <gio/gio.h>
#include <vector>
#include <string>

#ifdef USE_GTK4
#include <gtk4-layer-shell/gtk4-layer-shell.h>
#endif

struct AppItem {
    std::string name;
    std::string icon;
    std::string exec;
};

static void launch_app(GtkWidget *widget, gpointer data) {
    char *exec_cmd = (char *)data;
    if (exec_cmd) {
        g_spawn_command_line_async(exec_cmd, NULL);
#ifdef USE_GTK4
        GtkWidget *win = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "parent-window"));
        if (win) {
            gtk_window_destroy(GTK_WINDOW(win));
        }
#else
        gtk_main_quit();
#endif
    }
}

static std::vector<AppItem> load_system_applications() {
    std::vector<AppItem> apps;
    GList *app_list = g_app_info_get_all();

    for (GList *l = app_list; l != NULL; l = l->next) {
        GAppInfo *info = G_APP_INFO(l->data);
        if (!g_app_info_should_show(info)) continue;

        const char *app_id = g_app_info_get_id(info);

        if (app_id && (g_str_has_prefix(app_id, "org.kde.") || g_str_has_prefix(app_id, "kde-"))) {
            continue;
        }

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

#ifdef USE_GTK4
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "WDE Start Menu");
    gtk_window_set_default_size(GTK_WINDOW(window), 420, 520);

    gtk_layer_init_for_window(GTK_WINDOW(window));
    gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, 40);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    GtkWidget *middle_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *scroll_window = gtk_scrolled_window_new();
    GtkWidget *app_list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_window), app_list_box);

    std::vector<AppItem> apps = load_system_applications();
    for (const auto &app_data : apps) {
        GtkWidget *btn = gtk_button_new();
        GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        GtkWidget *icon = gtk_image_new_from_icon_name(app_data.icon.c_str());
        GtkWidget *label = gtk_label_new(app_data.name.c_str());

        gtk_box_append(GTK_BOX(btn_box), icon);
        gtk_box_append(GTK_BOX(btn_box), label);
        gtk_button_set_child(GTK_BUTTON(btn), btn_box);
        
        g_object_set_data(G_OBJECT(btn), "parent-window", window);
        g_signal_connect(btn, "clicked", G_CALLBACK(launch_app), g_strdup(app_data.exec.c_str()));
        gtk_box_append(GTK_BOX(app_list_box), btn);
    }

    gtk_box_append(GTK_BOX(middle_box), scroll_window);
    gtk_box_append(GTK_BOX(main_box), middle_box);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.wde.startmenu", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
#else
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_POPUP_MENU);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);

    GdkDisplay *display = gdk_display_get_default();
    GdkMonitor *monitor = gdk_display_get_primary_monitor(display);
    if (!monitor) monitor = gdk_display_get_monitor(display, 0);

    GdkRectangle geometry;
    gdk_monitor_get_geometry(monitor, &geometry);

    int menu_width = 420;
    int menu_height = 500;
    int pos_x = geometry.x;
    int pos_y = geometry.y + geometry.height - menu_height - 40;

    gtk_window_set_default_size(GTK_WINDOW(window), menu_width, menu_height);
    gtk_window_move(GTK_WINDOW(window), pos_x, pos_y);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    GtkWidget *middle_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *scroll_window = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *app_list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_add(GTK_CONTAINER(scroll_window), app_list_box);

    std::vector<AppItem> apps = load_system_applications();
    for (const auto &app : apps) {
        GtkWidget *btn = gtk_button_new();
        GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        GtkWidget *icon = gtk_image_new_from_icon_name(app.icon.c_str(), GTK_ICON_SIZE_LARGE_TOOLBAR);
        GtkWidget *label = gtk_label_new(app.name.c_str());

        gtk_box_pack_start(GTK_BOX(btn_box), icon, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(btn_box), label, TRUE, TRUE, 0);
        gtk_container_add(GTK_CONTAINER(btn), btn_box);
        g_signal_connect(btn, "clicked", G_CALLBACK(launch_app), g_strdup(app.exec.c_str()));
        gtk_box_pack_start(GTK_BOX(app_list_box), btn, FALSE, FALSE, 2);
    }

    gtk_box_pack_start(GTK_BOX(middle_box), scroll_window, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), middle_box, TRUE, TRUE, 5);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
#endif
