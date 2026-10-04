#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <ctime>
#include <cstdlib>
#include <unistd.h>

void make_x11_dock(GtkWidget *widget) {
    GdkWindow *gdk_win = gtk_widget_get_window(widget);
    if (!gdk_win) return;

    Display *display = GDK_WINDOW_XDISPLAY(gdk_win);
    Window xid = GDK_WINDOW_XID(gdk_win);

    Atom net_wm_window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom net_wm_window_type_dock = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
    XChangeProperty(display, xid, net_wm_window_type, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *)&net_wm_window_type_dock, 1);

    Atom net_wm_strut_partial = XInternAtom(display, "_NET_WM_STRUT_PARTIAL", False);
    
    GdkDisplay *gdk_display = gdk_display_get_default();
    GdkMonitor *monitor = gdk_display_get_primary_monitor(gdk_display);
    if (!monitor) monitor = gdk_display_get_monitor(gdk_display, 0);

    GdkRectangle geometry;
    gdk_monitor_get_geometry(monitor, &geometry);

    int panel_height = 36;
    long strut[12] = {0, 0, 0, panel_height, 0, 0, 0, 0, 0, 0, 0, (long)geometry.width};
    
    XChangeProperty(display, xid, net_wm_strut_partial, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char *)strut, 12);
}

static gboolean update_clock(gpointer user_data) {
    GtkLabel *label = GTK_LABEL(user_data);
    time_t now = time(0);
    struct tm *ltm = localtime(&now);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%H:%M  |  %d.%m.%Y", ltm);
    gtk_label_set_text(label, buffer);
    return TRUE;
}

static void launch_start_menu(GtkWidget *widget, gpointer data) {
    if (fork() == 0) {
        const char *rofi_theme = 
            "* { background-color: transparent; text-color: #ffffff; font: 'Segoe UI 10'; }"
            "window { location: south west; anchor: south west; x-offset: 6px; y-offset: -42px; "
            "width: 380px; height: 500px; background-color: #202020; border: 1px; border-color: #383838; "
            "border-radius: 8px; padding: 12px; }"
            "mainbox { children: [ entry, listview ]; spacing: 10px; }"
            "entry { background-color: #2d2d2d; placeholder: 'Uygulama ara...'; placeholder-color: #888888; "
            "padding: 10px; border-radius: 6px; border: 1px; border-color: #3f3f3f; }"
            "listview { lines: 10; columns: 1; fixed-height: false; spacing: 4px; cycle: true; scrollbar: false; }"
            "element { padding: 8px 12px; border-radius: 6px; }"
            "element selected { background-color: #0078d4; text-color: #ffffff; }"
            "element-text { vertical-align: 0.5; }";

        execlp("rofi", "rofi", "-show", "drun", "-theme-str", rofi_theme, NULL);
        exit(0);
    }
}

static void launch_terminal(GtkWidget *widget, gpointer data) {
    if (fork() == 0) {
        // Zsh çalıştıran terminal
        execlp("x-terminal-emulator", "x-terminal-emulator", "-e", "zsh", NULL);
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GdkDisplay *gdk_display = gdk_display_get_default();
    GdkMonitor *monitor = gdk_display_get_primary_monitor(gdk_display);
    if (!monitor) monitor = gdk_display_get_monitor(gdk_display, 0);

    GdkRectangle geometry;
    gdk_monitor_get_geometry(monitor, &geometry);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_default_size(GTK_WINDOW(window), geometry.width, 36);
    gtk_window_move(GTK_WINDOW(window), 0, geometry.height - 36);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(box), 2);

    GtkWidget *btn_start = gtk_button_new_with_label(" Başlat ");
    g_signal_connect(btn_start, "clicked", G_CALLBACK(launch_start_menu), NULL);
    gtk_box_pack_start(GTK_BOX(box), btn_start, FALSE, FALSE, 0);

    GtkWidget *btn_term = gtk_button_new_with_label(" Terminal ");
    g_signal_connect(btn_term, "clicked", G_CALLBACK(launch_terminal), NULL);
    gtk_box_pack_start(GTK_BOX(box), btn_term, FALSE, FALSE, 0);

    GtkWidget *lbl_spacer = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(box), lbl_spacer, TRUE, TRUE, 0);

    GtkWidget *lbl_clock = gtk_label_new("");
    gtk_box_pack_end(GTK_BOX(box), lbl_clock, FALSE, FALSE, 8);
    g_timeout_add_seconds(1, update_clock, lbl_clock);
    update_clock(lbl_clock);

    gtk_container_add(GTK_CONTAINER(window), box);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "window { background-color: #1c1c1c; color: #ffffff; font-family: 'Segoe UI', sans-serif; font-size: 13px; }\n"
        "button { background-color: #2b2b2b; color: #ffffff; border-radius: 4px; border: 1px solid #3a3a3a; padding: 4px 12px; font-weight: bold; }\n"
        "button:hover { background-color: #0078d4; border-color: #0078d4; }\n"
        "label { color: #cccccc; }\n", -1, NULL);

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_signal_connect(window, "realize", G_CALLBACK(make_x11_dock), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
