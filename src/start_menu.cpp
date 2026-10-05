#include <gtk/gtk.h>
#include <gio/gio.h>
#include <vector>
#include <string>

struct AppItem {
    std::string name;
    std::string icon;
    std::string exec;
};

static void launch_app(GtkWidget *widget, gpointer data) {
    char *exec_cmd = (char *)data;
    if (exec_cmd) {
        g_spawn_command_line_async(exec_cmd, NULL);
        gtk_main_quit();
    }
}

static std::vector<AppItem> load_system_applications() {
    std::vector<AppItem> apps;
    GList *app_list = g_app_info_get_all();

    for (GList *l = app_list; l != NULL; l = l->next) {
        GAppInfo *info = G_APP_INFO(l->data);
        if (!g_app_info_should_show(info)) continue;

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
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_default_size(GTK_WINDOW(window), 520, 580);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *btn_power = gtk_button_new_from_icon_name("system-shutdown-symbolic");
    gtk_box_append(GTK_BOX(sidebar), btn_power);
    gtk_box_append(GTK_BOX(main_box), sidebar);

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
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_POPUP_MENU);
    gtk_window_set_default_size(GTK_WINDOW(window), 520, 580);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *btn_power = gtk_button_new_from_icon_name("system-shutdown-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_end(GTK_BOX(sidebar), btn_power, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(main_box), sidebar, FALSE, FALSE, 0);

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
