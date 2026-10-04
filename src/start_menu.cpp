#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <cstdlib>
#include <unistd.h>
#include <string>

static void launch_app(GtkWidget *widget, gpointer data) {
    const char *cmd = (const char *)data;
    if (fork() == 0) {
        execlp("sh", "sh", "-c", cmd, NULL);
        exit(0);
    }
}

void make_popup_x11(GtkWidget *widget) {
    GdkWindow *gdk_win = gtk_widget_get_window(widget);
    if (!gdk_win) return;

    Display *display = GDK_WINDOW_XDISPLAY(gdk_win);
    Window xid = GDK_WINDOW_XID(gdk_win);

    Atom net_wm_window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom net_wm_type_util = XInternAtom(display, "_NET_WM_WINDOW_TYPE_UTILITY", False);
    XChangeProperty(display, xid, net_wm_window_type, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *)&net_wm_type_util, 1);

    Atom net_wm_state = XInternAtom(display, "_NET_WM_STATE", False);
    Atom net_wm_state_above = XInternAtom(display, "_NET_WM_STATE_ABOVE", False);
    XChangeProperty(display, xid, net_wm_state, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *)&net_wm_state_above, 1);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GdkDisplay *gdk_display = gdk_display_get_default();
    GdkMonitor *monitor = gdk_display_get_primary_monitor(gdk_display);
    if (!monitor) monitor = gdk_display_get_monitor(gdk_display, 0);

    GdkRectangle geometry;
    gdk_monitor_get_geometry(monitor, &geometry);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "wde-startmenu");
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_POPUP_MENU);

    int menu_width = 320;
    int menu_height = 420;
    int panel_height = 36;

    gtk_window_set_default_size(GTK_WINDOW(window), menu_width, menu_height);
    gtk_window_move(GTK_WINDOW(window), 0, geometry.height - panel_height - menu_height);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);

    GtkWidget *title = gtk_label_new("<b>Windows Desktop Environment</b>");
    gtk_label_set_use_markup(GTK_LABEL(title), TRUE);
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 6);

    // Örnek Uygulamalar Listesi
    struct AppItem {
        const char *name;
        const char *cmd;
    } apps[] = {
        {"Terminal (Zsh)", "x-terminal-emulator -e zsh"},
        {"Dosya Yöneticisi", "thunar || pcmanfm || nautilus"},
        {"Web Tarayıcısı", "x-www-browser"},
        {"Yazılım Merkezi", "gnome-software || synaptics"},
        {"Ayarlar", "xfce4-settings-manager || gnome-control-center"}
    };

    for (const auto &app : apps) {
        GtkWidget *btn = gtk_button_new_with_label(app.name);
        g_signal_connect(btn, "clicked", G_CALLBACK(launch_app), (gpointer)app.cmd);
        gtk_box_pack_start(GTK_BOX(box), btn, FALSE, FALSE, 2);
    }

    GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(box), sep, FALSE, FALSE, 8);

    GtkWidget *btn_exit = gtk_button_new_with_label("Oturumu Kapat");
    g_signal_connect(btn_exit, "clicked", G_CALLBACK(+[](GtkWidget*, gpointer){
        system("pkill -u $USER");
    }), NULL);
    gtk_box_pack_start(GTK_BOX(box), btn_exit, FALSE, FALSE, 2);

    gtk_container_add(GTK_CONTAINER(window), box);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "window { background-color: #252526; border: 1px solid #3c3c3c; border-radius: 8px; }\n"
        "label { color: #ffffff; font-family: 'Segoe UI', sans-serif; }\n"
        "button { background-color: #333333; color: #ffffff; border-radius: 4px; padding: 8px; border: none; text-align: left; }\n"
        "button:hover { background-color: #0078d4; }\n", -1, NULL);

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_signal_connect(window, "realize", G_CALLBACK(make_popup_x11), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
