#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TITLEBAR_HEIGHT 28
#define BUTTON_WIDTH 30

Display *display;
Window root;

Atom wm_protocols, wm_delete_window;

void draw_frame(Window frame, Window child, int width, int height, const char *title) {
    GC gc = XCreateGC(display, frame, 0, NULL);

    XSetForeground(display, gc, 0x2D2D2D);
    XFillRectangle(display, frame, gc, 0, 0, width, TITLEBAR_HEIGHT);

    XSetForeground(display, gc, 0x444444);
    XDrawRectangle(display, frame, gc, 0, 0, width - 1, height - 1);

    if (title) {
        XSetForeground(display, gc, 0xFFFFFF);
        XDrawString(display, frame, gc, 10, 18, title, strlen(title));
    }

    int btn_x = width - (BUTTON_WIDTH * 3);

    XSetForeground(display, gc, 0x3C3C3C);
    XFillRectangle(display, frame, gc, btn_x, 2, BUTTON_WIDTH - 2, TITLEBAR_HEIGHT - 4);
    XSetForeground(display, gc, 0xFFFFFF);
    XDrawString(display, frame, gc, btn_x + 11, 18, "-", 1);

    btn_x += BUTTON_WIDTH;
    XSetForeground(display, gc, 0x3C3C3C);
    XFillRectangle(display, frame, gc, btn_x, 2, BUTTON_WIDTH - 2, TITLEBAR_HEIGHT - 4);
    XSetForeground(display, gc, 0xFFFFFF);
    XDrawRectangle(display, frame, gc, btn_x + 9, 8, 10, 10);

    btn_x += BUTTON_WIDTH;
    XSetForeground(display, gc, 0xC42B1C);
    XFillRectangle(display, frame, gc, btn_x, 2, BUTTON_WIDTH - 2, TITLEBAR_HEIGHT - 4);
    XSetForeground(display, gc, 0xFFFFFF);
    XDrawString(display, frame, gc, btn_x + 10, 18, "X", 1);

    XFreeGC(display, gc);
}

void frame_window(Window w) {
    XWindowAttributes attrs;
    XGetWindowAttributes(display, w, &attrs);

    if (attrs.override_redirect || attrs.map_state == IsViewable) return;

    Atom type_atom;
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop = NULL;
    
    Atom net_wm_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", True);
    Atom net_wm_dock = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", True);
    
    if (XGetWindowProperty(display, w, net_wm_type, 0, 1, False, XA_ATOM,
                           &actual_type, &actual_format, &nitems, &bytes_after, &prop) == Success && prop) {
        if (*(Atom*)prop == net_wm_dock) {
            XFree(prop);
            XMapWindow(display, w);
            return;
        }
        XFree(prop);
    }

    int frame_w = attrs.width;
    int frame_h = attrs.height + TITLEBAR_HEIGHT;

    Window frame = XCreateSimpleWindow(
        display, root,
        attrs.x, attrs.y,
        frame_w, frame_h,
        1, 0x444444, 0x1E1E1E
    );

    XSelectInput(display, frame, SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | ExposureMask);
    XSelectInput(display, w, PropertyChangeMask);

    XReparentWindow(display, w, frame, 0, TITLEBAR_HEIGHT);
    XMapWindow(display, frame);
    XMapWindow(display, w);

    char *name;
    XFetchName(display, w, &name);
    draw_frame(frame, w, frame_w, frame_h, name ? name : "WDE Window");
    if (name) XFree(name);
}

void handle_button_press(XButtonEvent *e) {
    // Fare başlık çubuğuna ve butonlara tıklandı mı?
    if (e->y <= TITLEBAR_HEIGHT) {
        XWindowAttributes attrs;
        XGetWindowAttributes(display, e->window, &attrs);

        int close_x_start = attrs.width - BUTTON_WIDTH;
        int max_x_start = attrs.width - (BUTTON_WIDTH * 2);
        int min_x_start = attrs.width - (BUTTON_WIDTH * 3);

        if (e->x >= close_x_start) {
            Window child = None;
            Window root_return, parent_return, *children;
            unsigned int num_children;
            if (XQueryTree(display, e->window, &root_return, &parent_return, &children, &num_children) && num_children > 0) {
                child = children[0];
                XFree(children);
            }
            if (child != None) {
                XEvent ke;
                memset(&ke, 0, sizeof(ke));
                ke.type = ClientMessage;
                ke.xclient.window = child;
                ke.xclient.message_type = wm_protocols;
                ke.xclient.format = 32;
                ke.xclient.data.l[0] = wm_delete_window;
                ke.xclient.data.l[1] = CurrentTime;
                XSendEvent(display, child, False, NoEventMask, &ke);
            }
        }

        else if (e->x >= max_x_start && e->x < close_x_start) {
            XDisplayWidth(display, DefaultScreen(display));
            int sw = XDisplayWidth(display, DefaultScreen(display));
            int sh = XDisplayHeight(display, DefaultScreen(display)) - 36; // Panel payı
            XMoveResizeWindow(display, e->window, 0, 0, sw, sh);
        }

        else if (e->x >= min_x_start && e->x < max_x_start) {
            XUnmapWindow(display, e->window);
        }
    }
}

int main() {
    display = XOpenDisplay(NULL);
    if (!display) return 1;

    root = DefaultRootWindow(display);
    wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
    wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);

    XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);

    XEvent ev;
    while (1) {
        XNextEvent(display, &ev);

        if (ev.type == MapRequest) {
            frame_window(ev.xmaprequest.window);
        } else if (ev.type == ButtonPress) {
            handle_button_press(&ev.xbutton);
        } else if (ev.type == Expose) {
            char *name = NULL;
            draw_frame(ev.xexpose.window, None, ev.xexpose.width, ev.xexpose.height + TITLEBAR_HEIGHT, "WDE Window");
        }
    }

    XCloseDisplay(display);
    return 0;
}
