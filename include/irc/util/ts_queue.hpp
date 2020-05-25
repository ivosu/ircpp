#ifndef TS_QUEUE_HPP
#define TS_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

/**
 * Simple thread safe queue
 * @tparam T of what type the queue is
 */
template<typename T>
class ts_queue {
  public:

	void push(T&& val) {
		{
			std::lock_guard lock(m_mutex);
			m_queue.push(val);
		}
		m_cond_var.notify_one();
	}

	void push(const T& val) {
		{
			std::lock_guard lock(m_mutex);
			m_queue.push(val);
		}
		m_cond_var.notify_one();
	}

	template<typename... Args>
	void emplace(Args&& ... args) {
		{
			std::lock_guard lock(m_mutex);
			m_queue.emplace(args...);
		}
		m_cond_var.notify_one();
	}

	template<typename Rep, typename Period>
	std::optional<T> pop(const std::chrono::duration<Rep, Period>& timeout) {
		std::unique_lock lock(m_mutex);
		if (!m_cond_var.wait_for(lock, timeout, [=]() -> bool { return !m_queue.empty(); }))
			return std::nullopt;
		else {
			T tmp(m_queue.front());
			m_queue.pop();
			return tmp;
		}
	}

	T pop() {
		std::unique_lock lock(m_mutex);
		m_cond_var.wait(lock, [=]() -> bool { return !m_queue.empty(); });
		T tmp(m_queue.front());
		m_queue.pop();
		return tmp;
	}

  private:
	std::queue<T> m_queue;
	std::mutex m_mutex;
	std::condition_variable m_cond_var;
};

#endif //TS_QUEUE_HPP
