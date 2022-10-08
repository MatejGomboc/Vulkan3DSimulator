#pragma once

#include <string>
#include <fstream>
#include <thread>
#include <queue>
#include <mutex>

namespace Simulator {
	class Logger {
	public:
		~Logger();
		bool start(const std::string& log_file_name, std::string& out_error_message);
		void logWrite(const std::string& message);
		void requestStop();
		void waitForStop();

	private:
		enum class ThreadState {
			STOPPED,
			STARTING,
			RUNNING,
			STOPPING
		};

		static void logProcess(Logger* logger);

		std::queue<std::string> m_message_fifo;
		std::mutex m_message_fifo_mutex;

		std::thread m_worker_thread;
		ThreadState m_worker_thread_state = ThreadState::STOPPED;
		std::mutex m_worker_thread_mutex;

		std::ofstream m_file;

		std::mutex m_worker_thread_wait_mutex;
		std::condition_variable m_worker_thread_wait_condition;

		std::mutex m_stop_wait_mutex;
		std::condition_variable m_stop_wait_condition;
	};
}
