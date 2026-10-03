#ifndef WLR_USE_UNSTABLE
#define WLR_USE_UNSTABLE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>

#if __has_include(<wlr/render/allocator.h>)
#include <wlr/render/allocator.h>
#elif __has_include(<wlr/allocator/allocator.h>)
#include <wlr/allocator/allocator.h>
#elif __has_include(<wlr/types/wlr_allocator.h>)
#include <wlr/types/wlr_allocator.h>
#endif

#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/log.h>

struct xfce5_server {
    struct wl_display *wl_display;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;
    struct wl_listener new_output;
};

struct xfce5_output {
    struct wl_listener frame;
    struct wlr_output *wlr_output;
    struct xfce5_server *server;
};

static void output_frame(struct wl_listener *listener, void *data) {
    struct xfce5_output *output = wl_container_of(listener, output, frame);
    struct wlr_renderer *renderer = output->server->renderer;

    if (!wlr_output_attach_render(output->wlr_output, NULL)) {
        return;
    }

    wlr_renderer_begin(renderer, output->wlr_output->width, output->wlr_output->height);
    
    float color[4] = {0.12f, 0.22f, 0.30f, 1.0f};
    wlr_renderer_clear(renderer, color);
    
    wlr_renderer_end(renderer);
    wlr_output_commit(output->wlr_output);
}

static void server_new_output(struct wl_listener *listener, void *data) {
    struct xfce5_server *server = wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_output = (struct wlr_output *)data;

    wlr_output_init_render(wlr_output, server->allocator, server->renderer);

    if (!wl_list_empty(&wlr_output->modes)) {
        struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
        wlr_output_set_mode(wlr_output, mode);
        wlr_output_enable(wlr_output, true);
        wlr_output_commit(wlr_output);
    }

    struct xfce5_output *output = calloc(1, sizeof(struct xfce5_output));
    output->wlr_output = wlr_output;
    output->server = server;
    output->frame.notify = output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);
}

int main(int argc, char *argv[]) {
    wlr_log_init(WLR_INFO, NULL);

    struct xfce5_server server = {0};

    server.wl_display = wl_display_create();
    if (!server.wl_display) {
        fprintf(stderr, "[XFCE5-WM] Wayland Display oluşturulamadı!\n");
        return 1;
    }

    server.backend = wlr_backend_autocreate(server.wl_display, NULL);
    if (!server.backend) {
        fprintf(stderr, "[XFCE5-WM] Backend oluşturulamadı!\n");
        return 1;
    }

    server.renderer = wlr_renderer_autocreate(server.backend);
    if (!server.renderer) {
        fprintf(stderr, "[XFCE5-WM] Renderer oluşturulamadı!\n");
        return 1;
    }
    wlr_renderer_init_wl_display(server.renderer, server.wl_display);

    server.allocator = wlr_allocator_autocreate(server.backend, server.renderer);
    if (!server.allocator) {
        fprintf(stderr, "[XFCE5-WM] Allocator oluşturulamadı!\n");
        return 1;
    }

    wlr_compositor_create(server.wl_display, 5, server.renderer);
    wlr_subcompositor_create(server.wl_display);
    wlr_data_device_manager_create(server.wl_display);
    wlr_xdg_shell_create(server.wl_display, 3);

    server.new_output.notify = server_new_output;
    wl_signal_add(&server.backend->events.new_output, &server.new_output);

    const char *socket = wl_display_add_socket_auto(server.wl_display);
    if (!socket) {
        fprintf(stderr, "[XFCE5-WM] Wayland soketi açılamadı!\n");
        wlr_backend_destroy(server.backend);
        return 1;
    }

    FILE *f = fopen("/tmp/xfce5_wayland_socket", "w");
    if (f) {
        fprintf(f, "%s", socket);
        fclose(f);
    }

    setenv("WAYLAND_DISPLAY", socket, 1);
    printf("[XFCE5-WM] Compositor başlatıldı. Socket: %s\n", socket);

    if (!wlr_backend_start(server.backend)) {
        fprintf(stderr, "[XFCE5-WM] Backend başlatılamadı!\n");
        wlr_backend_destroy(server.backend);
        wl_display_destroy(server.wl_display);
        return 1;
    }

    wl_display_run(server.wl_display);

    wl_display_destroy_clients(server.wl_display);
    wl_display_destroy(server.wl_display);
    return 0;
}
