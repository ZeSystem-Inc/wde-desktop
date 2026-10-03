#include <iostream>
#include <fstream>
#include <cstdlib>

int get_scale_factor() {
    return 2;
}

int main() {
    int scale = get_scale_factor();

    std::cout << "[XFCE5-Autoscale] Wayland Ölçek Değeri: " << scale << "x\n";

    std::ofstream env_file("/tmp/xfce5_env");
    if (env_file.is_open()) {
        env_file << "export GDK_SCALE=" << scale << "\n";
        env_file << "export QT_AUTO_SCREEN_SCALE_FACTOR=1\n";
        env_file << "export QT_SCALE_FACTOR=" << scale << "\n";
        env_file.close();
    }

    return 0;
}
