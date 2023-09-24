#include <unistd.h>
#include <iostream>
#include <string>
#include <fstream>


int main() {
    std::string serialPort = "/dev/ttyACM0";

    // Open the serial port as an output file stream
    std::ofstream serial(serialPort);

    if (!serial.is_open()) {
        std::cerr << "Error opening serial port." << std::endl;
        return 1;
    }

    // Send the command string
    serial << "t50";
    usleep(20 * 1000 * 1000);
    serial << "t-50";


    // Close the serial port
    serial.close();

    std::cout << "Commands sent" << std::endl;

    return 0;
}
