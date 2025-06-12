#include "Phigros.h"

#include <iostream>

int main() {
    try {
        Phigros::Init("com.PigeonGames.Phigros");
        Phigros::Run();
    } catch(const std::exception &e) {
        std::cerr << "\033[91m错误: " << e.what() << std::endl;
    } catch(...) {
        std::cerr << "未知异常\n";
    }
}
