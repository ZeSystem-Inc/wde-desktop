#include <gtk/gtk.h>
#include <iostream>
#include <ctime>

static gboolean update_clock(gpointer user_data) {
    GtkLabel *label = GTK_LABEL(user_data);
    std::time_t now = std::time(nullptr);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%d %b %a  %H:%M:%S", std::localtime(&now));
    gtk_label_set_text(label, buf);
    return TRUE;
}

static void on_menu_clicked(GtkWidget *widget, gpointer data) {
    g_spawn_command_line_async("rofi -show drun", NULL);
}

static void on_term_clicked(GtkWidget *widget, gpointer data) {
    g_spawn_command_line_async("xterm", NULL);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css = 
        "window { background-color: #242933; }"
        "button { background-color: #3b4252; color: #eceff4; border: none; padding: 4px 12px; border-radius: 4px; font-weight: bold; }"
        "button:hover { background-color: #434c5e; }"
        "#menu-btn { background-color: #5e81ac; color: #ffffff; }"
        "#menu-btn:hover { background-color: #81a1c1; }"
        "label { color: #88c0d0; font-size: 13px; font-weight: bold; }";

    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "XFCE5 Panel");
    gtk_window_set_default_size(GTK_WINDOW(window), 1920, 36);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(box), 4);
    gtk_container_add(GTK_CONTAINER(window), box);

    GtkWidget *menu_btn = gtk_button_new_with_label("❖ XFCE5 Menu");
    gtk_widget_set_name(menu_btn, "menu-btn");
    g_signal_connect(menu_btn, "clicked", G_CALLBACK(on_menu_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), menu_btn, FALSE, FALSE, 0);

    GtkWidget *term_btn = gtk_button_new_with_label("💻 Terminal");
    g_signal_connect(term_btn, "clicked", G_CALLBACK(on_term_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), term_btn, FALSE, FALSE, 0);

    GtkWidget *clock_label = gtk_label_new("");
    gtk_box_pack_end(GTK_BOX(box), clock_label, FALSE, FALSE, 10);

    g_timeout_add_seconds(1, update_clock, clock_label);
    update_clock(clock_label);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);

    gtk_main();
    return 0;
}
