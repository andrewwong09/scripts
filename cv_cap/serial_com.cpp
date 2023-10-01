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


void serial_connection(std::string port) {
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
  usleep(5 * 1000 * 1000);

  // Send the command string
  std::string test_commands[6] = {"t50", "t-100", "t50", "p70", "p-140", "p70"};
  for (int i=0; i < sizeof(test_commands) / sizeof(test_commands[0]); i++) {
    std::cout << "Sending command: " << test_commands[i] << std::endl;
    std::string command = test_commands[i];
    write(serial_fd, command.c_str(), command.length());
    // Open loop assume all commands done within 4 seconds
    std::cout << "Sleeping" << std::endl;
    usleep(3 * 1e6);
  }

  // Close the serial port
  close(serial_fd);
  std::cout << "Serial closed; commands sent" << std::endl;

  return;
}

