#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "[XFCE5-Autoscale] Ekran ölçeklendirme servisi başlatıldı." << std::endl;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(15));
    }
    return 0;
}
