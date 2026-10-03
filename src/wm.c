#include <stdio.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_xdg_shell.h>

int main(int argc, char *argv[]) {
    printf("[XFCE5-WM] Native Wayland Compositor Başlatılıyor...\n");

    struct wl_display *display = wl_display_create();
    if (!display) {
        fprintf(stderr, "HATA: Wayland display oluşturulamadı!\n");
        return 1;
    }

    struct wlr_backend *backend = wlr_backend_autocreate(wl_display_get_event_loop(display), NULL);
    if (!backend) {
        fprintf(stderr, "HATA: wlroots backend oluşturulamadı!\n");
        wl_display_destroy(display);
        return 1;
    }

    struct wlr_renderer *renderer = wlr_renderer_autocreate(backend);
    if (!renderer) {
        fprintf(stderr, "HATA: Renderer oluşturulamadı!\n");
        wlr_backend_destroy(backend);
        wl_display_destroy(display);
        return 1;
    }

    wlr_renderer_init_wl_display(renderer, display);

    wlr_compositor_create(display, 5, renderer);
    wlr_subcompositor_create(display);
    wlr_data_device_manager_create(display);

    const char *socket = wl_display_add_socket_auto(display);
    if (!socket) {
        fprintf(stderr, "HATA: Wayland socket açılamadı!\n");
        wlr_backend_destroy(backend);
        wl_display_destroy(display);
        return 1;
    }

    if (!wlr_backend_start(backend)) {
        fprintf(stderr, "HATA: Backend başlatılamadı!\n");
        wlr_backend_destroy(backend);
        wl_display_destroy(display);
        return 1;
    }

    printf("[XFCE5-WM] Başarıyla çalıştı. Socket: %s\n", socket);

    wl_display_run(display);

    wl_display_destroy_clients(display);
    wlr_backend_destroy(backend);
    wl_display_destroy(display);
    return 0;
}
