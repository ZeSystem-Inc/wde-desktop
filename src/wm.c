#ifndef WLR_USE_UNSTABLE
#define WLR_USE_UNSTABLE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/render/allocator.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>

struct xfce5_server {
    struct wl_display *wl_display;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;
    struct wlr_scene *scene;
    struct wlr_output_layout *output_layout;
    struct wl_listener new_output;
};

struct xfce5_output {
    struct wl_list link;
    struct xfce5_server *server;
    struct wlr_output *wlr_output;
    struct wl_listener frame;
};

static void output_frame(struct wl_listener *listener, void *data) {
    struct xfce5_output *output = wl_container_of(listener, output, frame);
    struct wlr_scene *scene = output->server->scene;

    struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(scene, output->wlr_output);
    if (scene_output) {
        wlr_scene_output_commit(scene_output, NULL);
    }

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_send_frame_done(scene_output, &now);
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

    wlr_output_layout_add_auto(server->output_layout, wlr_output);
    wlr_scene_output_create(server.scene, wlr_output);
}

int main(int argc, char *argv[]) {
    wlr_log_init(WLR_DEBUG, NULL);

    struct xfce5_server server = {0};

    server.wl_display = wl_display_create();
    if (!server.wl_display) {
        fprintf(stderr, "[XFCE5-WM] HATA: Wayland Display sunucusu oluşturulamadı!\n");
        return 1;
    }

    server.backend = wlr_backend_autocreate(server.wl_display, NULL);
    if (!server.backend) {
        fprintf(stderr, "[XFCE5-WM] HATA: GPU/DRM Backend oluşturulamadı! (Sürücü veya DRM erişim izni yetersiz)\n");
        return 1;
    }
    server.renderer = wlr_renderer_autocreate(server.backend);
    if (!server.renderer) {
        fprintf(stderr, "[XFCE5-WM] HATA: GPU Renderer bağlamı oluşturulamadı!\n");
        return 1;
    }
    wlr_renderer_init_wl_display(server.renderer, server.wl_display);

    server.allocator = wlr_allocator_autocreate(server.backend, server.renderer);
    if (!server.allocator) {
        fprintf(stderr, "[XFCE5-WM] HATA: GBM Allocator oluşturulamadı!\n");
        return 1;
    }

    wlr_compositor_create(server.wl_display, 5, server.renderer);
    wlr_subcompositor_create(server.wl_display);
    wlr_data_device_manager_create(server.wl_display);

    server.output_layout = wlr_output_layout_create();
    server.scene = wlr_scene_create();
    wlr_scene_attach_output_layout(server.scene, server.output_layout);

    server.new_output.notify = server_new_output;
    wl_signal_add(&server.backend->events.new_output, &server.new_output);

    const char *socket = wl_display_add_socket_auto(server.wl_display);
    if (!socket) {
        fprintf(stderr, "[XFCE5-WM] HATA: Wayland soketi açılamadı!\n");
        return 1;
    }

    FILE *f = fopen("/tmp/xfce5_wayland_socket", "w");
    if (f) {
        fprintf(f, "%s", socket);
        fclose(f);
    }

    setenv("WAYLAND_DISPLAY", socket, 1);
    printf("[XFCE5-WM] GPU ile bağlantı kuruldu. Socket: %s\n", socket);

    if (!wlr_backend_start(server.backend)) {
        fprintf(stderr, "[XFCE5-WM] HATA: DRM Backend başlatılamadı!\n");
        return 1;
    }

    wl_display_run(server.wl_display);

    wl_display_destroy_clients(server.wl_display);
    wl_display_destroy(server.wl_display);
    return 0;
}
