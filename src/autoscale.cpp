#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <iostream>
#include <cstdlib>

int main() {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        std::cerr << "X Display açılamadı!" << std::endl;
        return 1;
    }

    Window root = DefaultRootWindow(dpy);
    XRRScreenResources *resources = XRRGetScreenResources(dpy, root);

    if (resources) {
        for (int i = 0; i < resources->noutput; i++) {
            XRROutputInfo *output_info = XRRGetOutputInfo(dpy, resources, resources->outputs[i]);
            if (output_info->connection == RR_Connected && output_info->crtc) {
                XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(dpy, resources, output_info->crtc);
                if (crtc_info) {
                    std::cout << "Ekran Bulundu: " << output_info->name 
                              << " | Çözünürlük: " << crtc_info->width << "x" << crtc_info->height << std::endl;
                    XRRFreeCrtcInfo(crtc_info);
                }
            }
            XRRFreeOutputInfo(output_info);
        }
        XRRFreeScreenResources(resources);
    }

    XCloseDisplay(dpy);
    return 0;
}
