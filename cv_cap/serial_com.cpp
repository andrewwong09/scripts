#include <iostream>
#include <string>
#include <fstream>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#include "serial_com.h"


int get_response(int serial_fd) {
  char response_buffer[256]; // Adjust the buffer size as needed
  int response_index = 0;
  char current_char;

  while (true) {
    ssize_t bytes_read = read(serial_fd, &current_char, 1);

    if (bytes_read == -1) {
      // Handle read error
      std::cerr << "Error reading from serial port." << std::endl;
      break;
    }

    if (bytes_read == 0) {
      // No more data available
      break;
    }

    response_buffer[response_index++] = current_char;

    // Check for a termination character or a maximum response length
    if (current_char == '\n' || response_index >= sizeof(response_buffer) - 1) {
      response_buffer[response_index] = '\0'; // Null-terminate the string
      break;
    }
  }

  // Handle the response (response_buffer contains the response)
  std::cout << "Received response: " << response_buffer << std::endl;
  return 0;
}


void serial_connection(std::string port, ThreadSafeStringQueue& cmd_queue, bool* stop) {
  std::string serialPort = port;
  int serial_fd = open(serialPort.c_str(), O_WRONLY);

  if (serial_fd == -1) {
    std::cerr << "Error opening serial port." << std::endl;
    return;
  }

  // Configure the serial port (baud rate: 9600, 8N1)
  struct termios tty;
  tcgetattr(serial_fd, &tty);
  cfsetospeed(&tty, B9600);
  cfsetispeed(&tty, B9600);
  tty.c_cflag &= ~PARENB; // No parity
  tty.c_cflag &= ~CSTOPB; // 1 stop bit
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8; // 8 data bits
  tty.c_cflag &= ~CRTSCTS; // Disable hardware flow control
  tty.c_cflag |= CREAD | CLOCAL; // Enable receiver, ignore modem control lines

  // Set input and output modes to non-canonical (raw)
  tty.c_lflag = 0;
  tty.c_oflag = 0;

  tcsetattr(serial_fd, TCSANOW, &tty);

  // Wait for Arduino to setup; serial connection opened defaults to restarting
  std::cout << "Serial Open. Waiting..." << std::endl;
  usleep(ARDUINO_RESET_TIME);

  // Send the command string
  std::chrono::milliseconds timeout(100);
  while(!*stop) {
    std::string cmd = cmd_queue.removeString(timeout);
    if (cmd != "") {
	write(serial_fd, cmd.c_str(), cmd.length());
    	std::cout << "Consumed: " << cmd << std::endl;
	usleep(100000);
    }
  }

  // Close the serial port
  close(serial_fd);
  std::cout << "Serial closed; commands sent" << std::endl;

  return;
}

