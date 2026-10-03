#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <iostream>
#include <map>
#include <cstdlib>

struct WindowFrame {
    Window frame;
    Window client;
    Window close_btn;
    Window maximize_btn;
    Window minimize_btn;
    int x, y, width, height;
    bool is_maximized;
};

class XFCE5WindowManager {
private:
    Display* display;
    Window root_window;
    std::map<Window, WindowFrame> clients;
    std::map<Window, Window> frame_to_client;
    
    int start_x, start_y;
    XWindowAttributes start_attr;

public:
    XFCE5WindowManager() : display(nullptr), root_window(0) {}

    ~XFCE5WindowManager() {
        if (display) XCloseDisplay(display);
    }

    bool init() {
        display = XOpenDisplay(NULL);
        if (!display) return false;

        root_window = DefaultRootWindow(display);
        XSelectInput(display, root_window, SubstructureRedirectMask | SubstructureNotifyMask);
        XSync(display, False);
        return true;
    }

    void run() {
        XEvent event;
        while (true) {
            XNextEvent(display, &event);

            switch (event.type) {
                case MapRequest:
                    frame_window(event.xmaprequest.window);
                    break;
                case ButtonPress:
                    on_button_press(event.xbutton);
                    break;
                case DestroyNotify:
                    unframe_window(event.xdestroywindow.window);
                    break;
                case ConfigureRequest: {
                    XWindowChanges changes;
                    changes.x = event.xconfigurerequest.x;
                    changes.y = event.xconfigurerequest.y;
                    changes.width = event.xconfigurerequest.width;
                    changes.height = event.xconfigurerequest.height;
                    changes.border_width = event.xconfigurerequest.border_width;
                    changes.sibling = event.xconfigurerequest.above;
                    changes.stack_mode = event.xconfigurerequest.detail;
                    XConfigureWindow(display, event.xconfigurerequest.window, event.xconfigurerequest.value_mask, &changes);
                    break;
                }
            }
        }
    }

private:
    void frame_window(Window w) {
        XWindowAttributes attrs;
        XGetWindowAttributes(display, w, &attrs);
        if (attrs.override_redirect) return;

        int title_height = 30;
        Window frame = XCreateSimpleWindow(display, root_window, attrs.x, attrs.y, attrs.width, attrs.height + title_height, 1, 0x2e3440, 0x3b4252);

        Window close_btn = XCreateSimpleWindow(display, frame, attrs.width - 25, 5, 20, 20, 0, 0xbf616a, 0xbf616a);
        Window max_btn = XCreateSimpleWindow(display, frame, attrs.width - 50, 5, 20, 20, 0, 0xebcb8b, 0xebcb8b);
        Window min_btn = XCreateSimpleWindow(display, frame, attrs.width - 75, 5, 20, 20, 0, 0xa3be8c, 0xa3be8c);

        XSelectInput(display, frame, SubstructureRedirectMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
        XSelectInput(display, close_btn, ButtonPressMask);
        XSelectInput(display, max_btn, ButtonPressMask);
        XSelectInput(display, min_btn, ButtonPressMask);

        XMapWindow(display, frame);
        XMapWindow(display, close_btn);
        XMapWindow(display, max_btn);
        XMapWindow(display, min_btn);

        XReparentWindow(display, w, frame, 0, title_height);
        XMapWindow(display, w);

        WindowFrame wf = {frame, w, close_btn, max_btn, min_btn, attrs.x, attrs.y, attrs.width, attrs.height, false};
        clients[w] = wf;
        frame_to_client[frame] = w;

        XSetInputFocus(display, w, RevertToParent, CurrentTime);
    }

    void unframe_window(Window w) {
        if (clients.count(w)) {
            XUnmapWindow(display, clients[w].frame);
            XDestroyWindow(display, clients[w].frame);
            frame_to_client.erase(clients[w].frame);
            clients.erase(w);
        }
    }

    void on_button_press(const XButtonEvent& e) {
        for (auto& pair : clients) {
            WindowFrame& wf = pair.second;
            if (e.window == wf.close_btn) {
                // Kapat Butonu
                XEvent ke;
                ke.type = ClientMessage;
                ke.xclient.window = wf.client;
                ke.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", True);
                ke.xclient.format = 32;
                ke.xclient.data.l[0] = XInternAtom(display, "WM_DELETE_WINDOW", True);
                ke.xclient.data.l[1] = CurrentTime;
                XSendEvent(display, wf.client, False, NoEventMask, &ke);
                return;
            } else if (e.window == wf.minimize_btn) {
                XUnmapWindow(display, wf.frame);
                return;
            } else if (e.window == wf.maximize_btn) {
                if (!wf.is_maximized) {
                    XMoveResizeWindow(display, wf.frame, 0, 40, DisplayWidth(display, DefaultScreen(display)), DisplayHeight(display, DefaultScreen(display)) - 40);
                    XResizeWindow(display, wf.client, DisplayWidth(display, DefaultScreen(display)), DisplayHeight(display, DefaultScreen(display)) - 70);
                    wf.is_maximized = true;
                } else {
                    XMoveResizeWindow(display, wf.frame, wf.x, wf.y, wf.width, wf.height + 30);
                    XResizeWindow(display, wf.client, wf.width, wf.height);
                    wf.is_maximized = false;
                }
                return;
            }
        }
    }
};

int main() {
    XFCE5WindowManager wm;
    if (!wm.init()) return 1;
    wm.run();
    return 0;
}
