#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "X11 Ekranı Açılamadı!\n");
        return 1;
    }

    Window root = DefaultRootWindow(display);
    XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);

    // Kısayol Tuşlarını Dinle
    XGrabKey(display, XKeysymToKeycode(display, XStringToKeysym("F4")), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(display, XKeysymToKeycode(display, XStringToKeysym("F10")), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(display, XKeysymToKeycode(display, XStringToKeysym("F9")), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);

    XEvent ev;
    while (1) {
        XNextEvent(display, &ev);

        if (ev.type == MapRequest) {
            XMapWindow(display, ev.xmaprequest.window);
        } 
        else if (ev.type == KeyPress) {
            Window focused;
            int revert_to;
            XGetInputFocus(display, &focused, &revert_to);

            if (focused != None && focused != root) {
                KeySym keysym = XLookupKeysym(&ev.xkey, 0);

                if (keysym == XStringToKeysym("F4")) {
                    Atom wm_delete = XInternAtom(display, "WM_DELETE_WINDOW", False);
                    XEvent kill_ev;
                    kill_ev.type = ClientMessage;
                    kill_ev.xclient.window = focused;
                    kill_ev.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", True);
                    kill_ev.xclient.format = 32;
                    kill_ev.xclient.data.l[0] = wm_delete;
                    kill_ev.xclient.data.l[1] = CurrentTime;
                    XSendEvent(display, focused, False, NoEventMask, &kill_ev);
                    XDestroyWindow(display, focused);
                }

                else if (keysym == XStringToKeysym("F10")) {
                    int screen = DefaultScreen(display);
                    XMoveResizeWindow(display, focused, 0, 0, DisplayWidth(display, screen), DisplayHeight(display, screen) - 36);
                }

                else if (keysym == XStringToKeysym("F9")) {
                    XUnmapWindow(display, focused);
                }
            }
        }
    }

    XCloseDisplay(display);
    return 0;
}
