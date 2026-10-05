#include <gtk/gtk.h>

static void open_start_menu(GtkWidget *widget, gpointer data) {
    g_spawn_command_line_async("wde-startmenu", NULL);
}

#ifdef USE_GTK4
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "WDE Panel");
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_default_size(GTK_WINDOW(window), 1280, 40);

    GtkWidget *panel_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_window_set_child(GTK_WINDOW(window), panel_box);

    GtkWidget *start_btn = gtk_button_new_with_label(" 🪟 Start ");
    g_signal_connect(start_btn, "clicked", G_CALLBACK(open_start_menu), NULL);
    gtk_box_append(GTK_BOX(panel_box), start_btn);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.wde.panel", G_APPLICATION_DEFAULT_FLAGS);
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
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DOCK);
    gtk_window_set_default_size(GTK_WINDOW(window), 1280, 40);

    GtkWidget *panel_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(window), panel_box);

    GtkWidget *start_btn = gtk_button_new_with_label(" 🪟 Start ");
    g_signal_connect(start_btn, "clicked", G_CALLBACK(open_start_menu), NULL);
    gtk_box_pack_start(GTK_BOX(panel_box), start_btn, FALSE, FALSE, 2);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
#endif
