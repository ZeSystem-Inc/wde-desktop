#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>

int main() {
    std::cout << "[XFCE5-Autoscale] Ekran ölçeklendirme daemonu aktif." << std::endl;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    return 0;
}
