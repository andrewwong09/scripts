#ifndef SERIAL_COM_H
#define SERIAL_COM_H

#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <vector>
#include <chrono>

#define ARDUINO_RESET_TIME 5000000

class ThreadSafeStringQueue {
	private:
		std::vector<std::string> data;
		std::mutex mutex;
		std::condition_variable condVar;

	public:
		// Add a string to the end of the vector
		void addString(const std::string& str) {
			std::lock_guard<std::mutex> lock(mutex);
			data.push_back(str);
			// Notify waiting threads that new data is available
			condVar.notify_one();
		}

		// Remove and return the first string in the vector (FIFO)
		std::string removeString(std::chrono::milliseconds timeout) {
			std::unique_lock<std::mutex> lock(mutex);
			// Wait until there is data to process or the timeout is reached
			if (condVar.wait_for(lock, timeout, [this] { return !data.empty(); })) {
				std::string result = data.front();
				data.erase(data.begin());
				return result;
			} else {
				// Timeout reached, handle accordingly (return an empty string or throw an exception)
				return "";  // Modify this line based on your error handling strategy
			}
		}
};

void serial_connection(std::string port, ThreadSafeStringQueue& cmd_queue, bool* stop);

#endif // SERIAL_COM_H
