#include <thread>
#include <iostream>
#include <string>
#include <unistd.h>

#include "serial_com.h"


int main() {
	ThreadSafeStringQueue cmd_queue;
	std::string port = "/dev/ttyACM0";
	bool stop = false;
	std::thread t1 (serial_connection, port, std::ref(cmd_queue), &stop);
	usleep(ARDUINO_RESET_TIME);

	std::string test_commands[6] = {"t50", "t-100", "t50", "p70", "p-140", "p70"};
	for (int i=0; i < sizeof(test_commands) / sizeof(test_commands[0]); i++) {
		std::cout << "Sending command: " << test_commands[i] << std::endl;
		std::string command = test_commands[i];
		cmd_queue.addString(command.c_str());
		
		// Open loop assume all commands done within 4 seconds
		std::cout << "Sleeping" << std::endl;
		usleep(3 * 1e6);
	}
	stop = true;
	t1.join();
}
