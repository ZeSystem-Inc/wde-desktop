#include <iostream>
#include <fstream>

int main() {
    int scale = 1;

    std::cout << "[XFCE5-Autoscale] GTK Scale Faktörü: " << scale << "x\n";

    std::ofstream env_file("/tmp/xfce5_env");
    if (env_file.is_open()) {
        env_file << "export GDK_SCALE=" << scale << "\n";
        env_file << "export GDK_DPI_SCALE=1\n";
        env_file.close();
    }

    return 0;
}
