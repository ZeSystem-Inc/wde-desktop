#include <X11/Xlib.h>
#include <iostream>
#include <cstdlib>
#include <algorithm>

class XFCE5WindowManager {
private:
    Display* display;
    Window root_window;

public:
    XFCE5WindowManager() : display(nullptr), root_window(0) {}

    ~XFCE5WindowManager() {
        if (display) {
            XCloseDisplay(display);
        }
    }

    bool init() {
        display = XOpenDisplay(NULL);
        if (!display) {
            std::cerr << "[XFCE5-WM] Hata: Display sunucusuna bağlanılamadı!\n";
            return false;
        }

        root_window = DefaultRootWindow(display);

        XSelectInput(display, root_window, SubstructureRedirectMask | SubstructureNotifyMask);
        XSync(display, False);

        std::cout << "[XFCE5-WM] XFCE5 Window Manager başarıyla başlatıldı.\n";
        return true;
    }

    void run() {
        XEvent event;
        while (true) {
            XNextEvent(display, &event);

            switch (event.type) {
                case MapRequest:
                    on_map_request(event.xmaprequest);
                    break;
                case ConfigureRequest:
                    on_configure_request(event.xconfigurerequest);
                    break;
                default:
                    break;
            }
        }
    }

private:
    void on_map_request(const XMapRequestEvent& e) {
        XMapWindow(display, e.window);
        XSetInputFocus(display, e.window, RevertToParent, CurrentTime);
        std::cout << "[XFCE5-WM] Pencere haritalandı (Mapped): " << e.window << "\n";
    }

    void on_configure_request(const XConfigureRequestEvent& e) {
        XWindowChanges changes;
        changes.x = e.x;
        changes.y = e.y;
        changes.width = e.width;
        changes.height = e.height;
        changes.border_width = e.border_width;
        changes.above = e.above;
        changes.detail = e.detail;

        XConfigureWindow(display, e.window, e.value_mask, &changes);
    }
};

int main() {
    XFCE5WindowManager wm;
    if (!wm.init()) {
        return 1;
    }
    wm.run();
    return 0;
}
