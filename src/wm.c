#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    Display *display;
    Window root;

    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "HATA: X Display açılamadı!\n");
        return 1;
    }

    root = DefaultRootWindow(display);

    printf("[WDE-WM] X11 Window Manager Başarıyla Başlatıldı!\n");

    XSelectInput(display, root, SubstructureNotifyMask | SubstructureRedirectMask | KeyPressMask);

    XEvent ev;
    while (1) {
        XNextEvent(display, &ev);

        switch (ev.type) {
            case MapRequest: {
                XMapRequestEvent *e = &ev.xmaprequest;
                XMapWindow(display, e->window);
                printf("[WDE-WM] Yeni pencere eklendi: ID %lu\n", e->window);
                break;
            }
            case ConfigureRequest: {
                XConfigureRequestEvent *e = &ev.xconfigurerequest;
                XWindowChanges changes;
                changes.x = e->x;
                changes.y = e->y;
                changes.width = e->width;
                changes.height = e->height;
                changes.border_width = e->border_width;
                changes.sibling = e->above;
                changes.stack_mode = e->detail;
                XConfigureWindow(display, e->window, e->value_mask, &changes);
                break;
            }
            default:
                break;
        }
    }

    XCloseDisplay(display);
    return 0;
}
