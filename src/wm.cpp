#include <iostream>
#include <cstdlib>
#include <wayland-server-core.h>

extern "C" {
    #include <wlr/backend.h>
    #include <wlr/render/wlr_renderer.h>
    #include <wlr/types/wlr_compositor.h>
    #include <wlr/types/wlr_data_device.h>
    #include <wlr/types/wlr_subcompositor.h>
    #include <wlr/types/wlr_xdg_shell.h>
    #include <wlr/util/log.h>
}

int main(int argc, char *argv[]) {
    wlr_log_init(WLR_INFO, NULL);

    struct wl_display *display = wl_display_create();
    if (!display) {
        std::cerr << "[XFCE5-WM] Wayland Display oluşturulamadı!\n";
        return 1;
    }

    struct wlr_backend *backend = wlr_backend_autocreate(wl_display_get_event_loop(display), NULL);
    if (!backend) {
        std::cerr << "[XFCE5-WM] Backend oluşturulamadı!\n";
        return 1;
    }

    struct wlr_renderer *renderer = wlr_renderer_autocreate(backend);
    wlr_renderer_init_wl_display(renderer, display);

    wlr_compositor_create(display, 5, renderer);
    wlr_subcompositor_create(display);
    wlr_data_device_manager_create(display);

    struct wlr_xdg_shell *xdg_shell = wlr_xdg_shell_create(display, 3);

    const char *socket = wl_display_add_socket_auto(display);
    if (!socket) {
        wlr_backend_destroy(backend);
        return 1;
    }

    setenv("WAYLAND_DISPLAY", socket, 1);
    std::cout << "[XFCE5-WM] Wayland Compositor başlatıldı. Socket: " << socket << "\n";

    if (!wlr_backend_start(backend)) {
        wlr_backend_destroy(backend);
        wl_display_destroy(display);
        return 1;
    }

    wl_display_run(display);

    wl_display_destroy_clients(display);
    wl_display_destroy(display);
    return 0;
}
