#include <thread>

#include "serial_com.h"


int main() {
    std::string port = "/dev/ttyACM0";
    std::thread t1 (serial_connection, port);
    t1.join();
}
