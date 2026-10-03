#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <iostream>
#include <unistd.h>

int main() {
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        std::cerr << "[WDE-Autoscale] HATA: X Display'e bağlanılamadı!" << std::endl;
        return 1;
    }

    Window root = DefaultRootWindow(display);
    int event_base, error_base;

    // XRandR eklentisi var mı kontrol et
    if (!XRRQueryExtension(display, &event_base, &error_base)) {
        std::cerr << "[WDE-Autoscale] XRandR desteklenmiyor!" << std::endl;
        XCloseDisplay(display);
        return 1;
    }

    XRRSelectInput(display, root, RRScreenChangeNotifyMask);
    std::cout << "[WDE-Autoscale] Otomatik Çözünürlük Servisi Aktif." << std::endl;

    XEvent ev;
    while (true) {
        XNextEvent(display, &ev);
        if (ev.type == event_base + RRScreenChangeNotify) {
            XRRUpdateConfiguration(&ev);
            std::cout << "[WDE-Autoscale] Ekran boyutu değişti, düzen güncellendi." << std::endl;
        }
    }

    XCloseDisplay(display);
    return 0;
}
