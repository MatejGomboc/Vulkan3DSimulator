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
		bool start(std::string& out_error_message);
		void stop();
		void logDebug(const std::string& message);
		void logWarning(const std::string& message);
		void logError(const std::string& message);

	private:
		enum class ThreadState {
			STOPPED,
			STARTING,
			RUNNING,
			STOPPING
		};

		static void logProcess(Logger* logger);
		void logWrite(const std::string& message);

		std::queue<std::string> m_message_fifo;
		std::mutex m_message_fifo_mutex;
		std::condition_variable m_message_fifo_wait_condition;

		std::thread m_worker_thread;
		ThreadState m_worker_thread_state = ThreadState::STOPPED;
		std::ofstream m_file;
		std::mutex m_worker_thread_mutex;
	};
}
