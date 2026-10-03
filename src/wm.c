#ifndef WLR_USE_UNSTABLE
#define WLR_USE_UNSTABLE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>

// wlroots sürüm farklarını (0.15 / 0.16 / 0.17+) otomatik çözen Include bloğu
#if __has_include(<wlr/allocator.h>)
    #include <wlr/allocator.h>
#elif __has_include(<wlr/types/wlr_allocator.h>)
    #include <wlr/types/wlr_allocator.h>
#endif

#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xdg_shell.h>

struct wde_server {
    struct wl_display *wl_display;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;
    struct wlr_output_layout *output_layout;
    struct wl_listener new_output;
};

static void server_new_output(struct wl_listener *listener, void *data) {
    struct wde_server *server = wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_output = data;

    wlr_output_init_render(wlr_output, server->allocator, server->renderer);

    struct wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);

    struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
    if (mode != NULL) {
        wlr_output_state_set_mode(&state, mode);
    }

    wlr_output_commit_state(wlr_output, &state);
    wlr_output_state_finish(&state);

    wlr_output_layout_add_auto(server->output_layout, wlr_output);
}

int main(int argc, char *argv[]) {
    printf("[WDE-WM] Windows Desktop Environment Compositor Başlatılıyor...\n");

    struct wde_server server = {0};

    server.wl_display = wl_display_create();
    if (!server.wl_display) return 1;

    server.backend = wlr_backend_autocreate(wl_display_get_event_loop(server.wl_display), NULL);
    if (!server.backend) return 1;

    server.renderer = wlr_renderer_autocreate(server.backend);
    if (!server.renderer) return 1;

    wlr_renderer_init_wl_display(server.renderer, server.wl_display);

    server.allocator = wlr_allocator_autocreate(server.backend, server.renderer);
    if (!server.allocator) return 1;

    wlr_compositor_create(server.wl_display, 5, server.renderer);
    wlr_subcompositor_create(server.wl_display);
    wlr_data_device_manager_create(server.wl_display);

    server.output_layout = wlr_output_layout_create(server.wl_display);
    server.new_output.notify = server_new_output;
    wl_signal_add(&server.backend->events.new_output, &server.new_output);

    const char *socket = wl_display_add_socket_auto(server.wl_display);
    if (!socket) return 1;

    if (!wlr_backend_start(server.backend)) return 1;

    printf("[WDE-WM] Başarıyla çalıştı. Wayland Socket: %s\n", socket);

    wl_display_run(server.wl_display);

    wl_display_destroy_clients(server.wl_display);
    wlr_backend_destroy(server.backend);
    wl_display_destroy(server.wl_display);
    return 0;
}
