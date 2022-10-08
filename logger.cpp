#include "logger.h"

using namespace Simulator;

Logger::~Logger()
{
	requestStop();
	waitForStop();
}

bool Logger::start(const std::string& log_file_name, std::string& out_error_message)
{
	out_error_message.clear();
	m_worker_thread_mutex.lock();

	if (m_worker_thread_state != ThreadState::STOPPED) {
		out_error_message = "Log file is already open.";
		m_worker_thread_mutex.unlock();
		return false;
	}

	if (m_worker_thread.joinable()) {
		m_worker_thread.join();
	}

	m_file.open(log_file_name, std::ofstream::out | std::ofstream::trunc);

	if (!m_file.is_open()) {
		out_error_message = "Failed to create log file.";
		m_worker_thread_mutex.unlock();
		return false;
	}

	m_worker_thread_state = ThreadState::STARTING;
	m_worker_thread = std::thread(logProcess, this);
	m_worker_thread_mutex.unlock();
	return true;
}

void Logger::logWrite(const std::string& message)
{
	m_worker_thread_mutex.lock();

	if (m_worker_thread_state != ThreadState::RUNNING) {
		m_worker_thread_mutex.unlock();
		return;
	}

	m_worker_thread_mutex.unlock();

	m_message_fifo_mutex.lock();
	m_message_fifo.push(message);
	m_message_fifo_mutex.unlock();

	m_worker_thread_wait_condition.notify_all();
}

void Logger::requestStop()
{
	m_worker_thread_mutex.lock();

	if ((m_worker_thread_state == ThreadState::STOPPING) ||
		(m_worker_thread_state == ThreadState::STOPPED)) {
		m_worker_thread_mutex.unlock();
		return;
	}

	m_worker_thread_state = ThreadState::STOPPING;
	m_worker_thread_mutex.unlock();

	m_worker_thread_wait_condition.notify_all();
}

void Logger::waitForStop()
{
	m_worker_thread_mutex.lock();

	if (m_worker_thread_state == ThreadState::STOPPED) {
		if (m_worker_thread.joinable()) {
			m_worker_thread.join();
		}
		m_worker_thread_mutex.unlock();
		return;
	}

	m_worker_thread_mutex.unlock();

	{
		std::unique_lock<std::mutex> wait_lock(m_stop_wait_mutex);
		m_stop_wait_condition.wait(wait_lock);
	}

	m_worker_thread_mutex.lock();

	if (m_worker_thread_state == ThreadState::STOPPED) {
		if (m_worker_thread.joinable()) {
			m_worker_thread.join();
		}
		m_worker_thread_mutex.unlock();
		return;
	}

	m_worker_thread_mutex.unlock();
}

void Logger::logProcess(Logger* logger)
{
	logger->m_worker_thread_mutex.lock();

	if (logger->m_worker_thread_state == ThreadState::STOPPING) {
		logger->m_file.close();
		logger->m_worker_thread_state = ThreadState::STOPPED;
		logger->m_worker_thread_mutex.unlock();
		return;
	}

	logger->m_worker_thread_state = ThreadState::RUNNING;
	logger->m_worker_thread_mutex.unlock();

	while (true) {
		while (true) {
			logger->m_message_fifo_mutex.lock();

			if (!logger->m_message_fifo.empty()) {
				logger->m_file << logger->m_message_fifo.front();
			}

			bool is_fifo_empty = logger->m_message_fifo.empty();
			logger->m_message_fifo_mutex.unlock();

			if (is_fifo_empty) {
				break;
			}
		}

		logger->m_worker_thread_mutex.lock();
		bool should_thread_stop = false;

		if (logger->m_worker_thread_state == ThreadState::STOPPING) {
			logger->m_file.close();
			should_thread_stop = true;
			logger->m_worker_thread_state = ThreadState::STOPPED;
		}

		logger->m_worker_thread_mutex.unlock();

		if (should_thread_stop) {
			break;
		}

		{
			std::unique_lock<std::mutex> wait_lock(logger->m_worker_thread_wait_mutex);
			logger->m_worker_thread_wait_condition.wait(wait_lock);
		}
	}

	logger->m_stop_wait_condition.notify_all();
}