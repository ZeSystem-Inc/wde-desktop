#include <iostream>
#include <fstream>
#include <cstdlib>

#if defined(__linux__)
#include <X11/Xlib.h>
#endif

int get_primary_width() {
    int width = 1920; // Varsayılan fallback
#if defined(__linux__)
    Display* display = XOpenDisplay(NULL);
    if (display) {
        int screen = DefaultScreen(display);
        width = DisplayWidth(display, screen);
        XCloseDisplay(display);
    }
#endif
    return width;
}

int calculate_scale(int width) {
    if (width >= 3840) {
        return 2;
    } else if (width >= 2560) {
        return 2;
    } else {
        return 1;
    }
}

int main() {
    int width = get_primary_width();
    int scale = calculate_scale(width);

    std::cout << "[XFCE5-Autoscale] Algılanan Ekran Genişliği: " << width << "px\n";
    std::cout << "[XFCE5-Autoscale] Uygulanan Ölçek (Scale): " << scale << "x\n";

    std::ofstream env_file("/tmp/xfce5_env");
    if (env_file.is_open()) {
        env_file << "export GDK_SCALE=" << scale << "\n";
        env_file << "export QT_AUTO_SCREEN_SCALE_FACTOR=1\n";
        env_file << "export QT_SCALE_FACTOR=" << scale << "\n";
        env_file.close();
    }

    return 0;
}
