#include <gtk/gtk.h>
#include <gtk-layer-shell/gtk-layer-shell.h>
#include <ctime>
#include <iostream>

static gboolean update_clock(gpointer user_data) {
    GtkLabel *label = GTK_LABEL(user_data);
    time_t now = time(0);
    struct tm *ltm = localtime(&now);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "  %H:%M:%S  |  %d %b %Y  ", ltm);
    gtk_label_set_text(label, buffer);
    return TRUE;
}

static void launch_rofi(GtkWidget *widget, gpointer data) {
    if (fork() == 0) {
        execlp("rofi", "rofi", "-show", "drun", NULL);
        exit(0);
    }
}

static void logout_session(GtkWidget *widget, gpointer data) {
    system("pkill -f xfce5-session");
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    if (gtk_layer_is_supported()) {
        gtk_layer_init_for_window(GTK_WINDOW(window));
        gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_TOP);
        gtk_layer_auto_exclusive_zone_enable(GTK_WINDOW(window));
        
        gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
        gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
        gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
    }

    GtkWidget *header_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(header_bar), 6);

    GtkWidget *btn_menu = gtk_button_new_with_label("  XFCE5 Menü ");
    g_signal_connect(btn_menu, "clicked", G_CALLBACK(launch_rofi), NULL);
    gtk_box_pack_start(GTK_BOX(header_bar), btn_menu, FALSE, FALSE, 0);

    GtkWidget *lbl_clock = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(header_bar), lbl_clock, TRUE, TRUE, 0);
    g_timeout_add_seconds(1, update_clock, lbl_clock);
    update_clock(lbl_clock);

    GtkWidget *btn_logout = gtk_button_new_with_label(" Çıkış ");
    g_signal_connect(btn_logout, "clicked", G_CALLBACK(logout_session), NULL);
    gtk_box_pack_end(GTK_BOX(header_bar), btn_logout, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(window), header_bar);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "window { background-color: #11111b; color: #cdd6f4; font-family: sans-serif; font-weight: bold; }\n"
        "button { background-color: #313244; color: #cdd6f4; border-radius: 6px; border: none; padding: 4px 10px; }\n"
        "button:hover { background-color: #45475a; }\n", -1, NULL);

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
