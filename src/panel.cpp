#include <gtk/gtk.h>
#include <ctime>

#ifdef USE_GTK4
#include <gdk/wayland/gdkwayland.h>
#include <gdk/x11/gdkx.h>
#else
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

static void open_start_menu(GtkWidget *widget, gpointer data) {
#ifdef USE_GTK4
    g_spawn_command_line_async("wde-startmenu-wayland", NULL);
#else
    g_spawn_command_line_async("wde-startmenu", NULL);
#endif
}

static void open_terminal(GtkWidget *widget, gpointer data) {
    g_spawn_command_line_async("gnome-terminal", NULL);
}

static void open_file_manager(GtkWidget *widget, gpointer data) {
    g_spawn_command_line_async("nautilus", NULL);
}

static void open_settings(GtkWidget *widget, gpointer data) {
    g_spawn_command_line_async("gnome-control-center", NULL);
}

static gboolean update_clock(gpointer label) {
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%H:%M - %d.%m.%Y", timeinfo);

    gtk_label_set_text(GTK_LABEL(label), buffer);
    return TRUE;
}

#ifdef USE_GTK4
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "WDE Panel");
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_default_size(GTK_WINDOW(window), 1280, 40);

    GtkWidget *panel_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_window_set_child(GTK_WINDOW(window), panel_box);

    GtkWidget *start_btn = gtk_button_new_from_icon_name("start-here-symbolic");
    g_signal_connect(start_btn, "clicked", G_CALLBACK(open_start_menu), NULL);
    gtk_box_append(GTK_BOX(panel_box), start_btn);

    GtkWidget *term_btn = gtk_button_new_from_icon_name("utilities-terminal-symbolic");
    g_signal_connect(term_btn, "clicked", G_CALLBACK(open_terminal), NULL);
    gtk_box_append(GTK_BOX(panel_box), term_btn);

    GtkWidget *files_btn = gtk_button_new_from_icon_name("system-file-manager-symbolic");
    g_signal_connect(files_btn, "clicked", G_CALLBACK(open_file_manager), NULL);
    gtk_box_append(GTK_BOX(panel_box), files_btn);

    GtkWidget *settings_btn = gtk_button_new_from_icon_name("emblem-system-symbolic");
    g_signal_connect(settings_btn, "clicked", G_CALLBACK(open_settings), NULL);
    gtk_box_append(GTK_BOX(panel_box), settings_btn);

    GtkWidget *spacer = gtk_label_new("");
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(panel_box), spacer);

    GtkWidget *clock_label = gtk_label_new("");
    update_clock(clock_label);
    g_timeout_add_seconds(1, update_clock, clock_label);
    gtk_box_append(GTK_BOX(panel_box), clock_label);

    gtk_window_present(GTK_WINDOW(window));

    GdkSurface *surface = gtk_native_get_surface(GTK_NATIVE(window));
    if (surface && GDK_IS_X11_SURFACE(surface)) {
        Display *xdisplay = gdk_x11_display_get_xdisplay(gdk_surface_get_display(surface));
        Window xwindow = gdk_x11_surface_get_xid(surface);
        Atom net_wm_window_type = XInternAtom(xdisplay, "_NET_WM_WINDOW_TYPE", False);
        Atom net_wm_type_dock = XInternAtom(xdisplay, "_NET_WM_WINDOW_TYPE_DOCK", False);
        XChangeProperty(xdisplay, xwindow, net_wm_window_type, XA_ATOM, 32,
                        PropModeReplace, (unsigned char *)&net_wm_type_dock, 1);
    }
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
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DOCK);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);

    GdkDisplay *display = gdk_display_get_default();
    GdkMonitor *monitor = gdk_display_get_primary_monitor(display);
    if (!monitor) monitor = gdk_display_get_monitor(display, 0);

    GdkRectangle geometry;
    gdk_monitor_get_geometry(monitor, &geometry);

    int panel_height = 40;
    gtk_window_set_default_size(GTK_WINDOW(window), geometry.width, panel_height);
    gtk_window_move(GTK_WINDOW(window), geometry.x, geometry.y + geometry.height - panel_height);

    GtkWidget *panel_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(window), panel_box);

    GtkWidget *start_btn = gtk_button_new_from_icon_name("start-here-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
    g_signal_connect(start_btn, "clicked", G_CALLBACK(open_start_menu), NULL);
    gtk_box_pack_start(GTK_BOX(panel_box), start_btn, FALSE, FALSE, 2);

    GtkWidget *term_btn = gtk_button_new_from_icon_name("utilities-terminal-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
    g_signal_connect(term_btn, "clicked", G_CALLBACK(open_terminal), NULL);
    gtk_box_pack_start(GTK_BOX(panel_box), term_btn, FALSE, FALSE, 2);

    GtkWidget *files_btn = gtk_button_new_from_icon_name("system-file-manager-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
    g_signal_connect(files_btn, "clicked", G_CALLBACK(open_file_manager), NULL);
    gtk_box_pack_start(GTK_BOX(panel_box), files_btn, FALSE, FALSE, 2);

    GtkWidget *settings_btn = gtk_button_new_from_icon_name("emblem-system-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
    g_signal_connect(settings_btn, "clicked", G_CALLBACK(open_settings), NULL);
    gtk_box_pack_start(GTK_BOX(panel_box), settings_btn, FALSE, FALSE, 2);

    GtkWidget *clock_label = gtk_label_new("");
    update_clock(clock_label);
    g_timeout_add_seconds(1, update_clock, clock_label);
    gtk_box_pack_end(GTK_BOX(panel_box), clock_label, FALSE, FALSE, 10);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);

    GdkWindow *gdk_win = gtk_widget_get_window(window);
    if (GDK_IS_X11_WINDOW(gdk_win)) {
        Display *xdisplay = GDK_WINDOW_XDISPLAY(gdk_win);
        Window xwindow = GDK_WINDOW_XID(gdk_win);
        Atom net_wm_window_type = XInternAtom(xdisplay, "_NET_WM_WINDOW_TYPE", False);
        Atom net_wm_type_dock = XInternAtom(xdisplay, "_NET_WM_WINDOW_TYPE_DOCK", False);
        XChangeProperty(xdisplay, xwindow, net_wm_window_type, XA_ATOM, 32,
                        PropModeReplace, (unsigned char *)&net_wm_type_dock, 1);
    }

    gtk_main();
    return 0;
}
#endif
