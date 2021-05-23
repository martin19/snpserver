#include <iostream>
#include <network/SnpSocket.h>
#include "util/loguru.h"

//int main(int argc, char *argv[]) {
//    loguru::init(argc, argv);
//
//    LOG_F(INFO, "I'm hungry for some %.3f!", 3.14159);
//    LOG_F(WARNING, "I'm hungry for some %.3f!", 3.14159);
//    LOG_F(ERROR, "I'm hungry for some %.3f!", 3.14159);
//    LOG_F(FATAL, "I'm hungry for some %.3f!", 3.14159);
//    LOG_F(2, "Will only show if verbosity is 2 or higher");
//}

int main() {
    SnpSocket s;
    s.run();

//    while(1) {
//        for(auto& it : s.getClients()) {
//            std::string s = std::string("Hello Client");
//            it.second.send((uint8_t*)s.data(), s.size());
//        }
//        sleep(10);
//    }

//    VideoCaptureModesettingOptions options = {};
//    options.width = 100;
//    options.height = 100;
//    options.screenNumber = 10;
//
//    auto *videoCapture = new VideoCaptureModesetting(&options);
//    std::cout << "derived options:" << std::endl;
//    videoCapture->printOptions();
//
//    std::cout << "base options:" << std::endl;
//    videoCapture->VideoCapture::printOptions();

    return 0;
}