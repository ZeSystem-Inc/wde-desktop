#include <gtk/gtk.h>
#include <gtk-layer-shell/gtk-layer-shell.h>
#include <ctime>
#include <cstdlib>
#include <unistd.h>

static gboolean update_clock(gpointer user_data) {
    GtkLabel *label = GTK_LABEL(user_data);
    time_t now = time(0);
    struct tm *ltm = localtime(&now);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%H:%M\n%d.%m.%Y", ltm);
    gtk_label_set_text(label, buffer);
    return TRUE;
}

static void launch_start_menu(GtkWidget *widget, gpointer data) {
    if (fork() == 0) {
        execlp("rofi", "rofi", "-show", "drun", "-theme-str", "window {location: bottom left; anchor: bottom left; x-offset: 5px; y-offset: -45px;}", NULL);
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    if (gtk_layer_is_supported()) {
        gtk_layer_init_for_window(GTK_WINDOW(window));
        gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_TOP);
        gtk_layer_auto_exclusive_zone_enable(GTK_WINDOW(window));
        
        // Windows Görev Çubuğu Gibi Alt Kısma Sabitleme
        gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
        gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
        gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
    }

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 2);

    // Windows Başlat Butonu
    GtkWidget *btn_start = gtk_button_new_with_label(" 🪟 Başlat ");
    g_signal_connect(btn_start, "clicked", G_CALLBACK(launch_start_menu), NULL);
    gtk_box_pack_start(GTK_BOX(box), btn_start, FALSE, FALSE, 0);

    // Orta Alan (Boşluk)
    GtkWidget *lbl_spacer = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(box), lbl_spacer, TRUE, TRUE, 0);

    // Sağ Alt Saat
    GtkWidget *lbl_clock = gtk_label_new("");
    gtk_box_pack_end(GTK_BOX(box), lbl_clock, FALSE, FALSE, 4);
    g_timeout_add_seconds(1, update_clock, lbl_clock);
    update_clock(lbl_clock);

    gtk_container_add(GTK_CONTAINER(window), box);

    // Windows Koyu Tema Stili
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "window { background-color: #1f1f1f; color: #ffffff; font-family: 'Segoe UI', sans-serif; font-size: 12px; }\n"
        "button { background-color: #2d2d2d; color: #ffffff; border-radius: 4px; border: 1px solid #3d3d3d; padding: 4px 12px; font-weight: bold; }\n"
        "button:hover { background-color: #0078d4; border-color: #0078d4; }\n"
        "label { color: #cccccc; text-align: center; }\n", -1, NULL);

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
