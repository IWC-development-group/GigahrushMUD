/*
	TODO: replace it with spdlog
*/

#include <fstream>
#include <string>
#include <print>
#include <array>
#include <chrono>

#define LOGGER_STACK_BUFF_SIZE		1024

namespace logger {

	class Log {
	private:
		std::ofstream stream;
		static Log* log;

		Log();
		Log(const std::string& filename);
		~Log();

		Log(const Log&) = delete;
		Log& operator=(const Log&) = delete;

		template <typename ...Args>
		void writeFileLine(std::string_view level, std::format_string<Args...> fmt, Args&&... args);

	public:
		static Log* init();
		static Log* init(const std::string& filename);
		static Log* get();
		static void destroy();

		template <typename ...Args>
		void _info(std::format_string<Args...> fmt, Args&&... args);

		template <typename ...Args>
		void _warn(std::format_string<Args...> fmt, Args&&... args);

		template <typename ...Args>
		void _error(std::format_string<Args...> fmt, Args&&... args);

		template <typename ...Args>
		void _important(std::format_string<Args...> fmt, Args&&... args);

		template <typename ...Args>
		static void info(std::format_string<Args...> fmt, Args&&... args) {
			log->_info(fmt, std::forward<Args>(args)...);
		}

		template <typename ...Args>
		static void warn(std::format_string<Args...> fmt, Args&&... args) {
			log->_warn(fmt, std::forward<Args>(args)...);
		}

		template <typename ...Args>
		static void error(std::format_string<Args...> fmt, Args&&... args) {
			log->_error(fmt, std::forward<Args>(args)...);
		}

		template <typename ...Args>
		static void important(std::format_string<Args...> fmt, Args&&... args) {
			log->_important(fmt, std::forward<Args>(args)...);
		}
	};

	template <typename ...Args>
	void Log::writeFileLine(std::string_view level, std::format_string<Args...> fmt, Args&&... args) {
		if (!stream.is_open()) return;

		std::array<char, LOGGER_STACK_BUFF_SIZE> buffer;
		char* it = buffer.data();
		char* end = buffer.data() + buffer.size();

		auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());

		it = std::format_to_n(it, end - it, "[{:%Y-%m-%d %H:%M:%S}][{}] ", now, level).out;
		it = std::format_to_n(it, end - it, fmt, std::forward<Args>(args)...).out;
		if (it < end) *it++ = '\n';

		stream.write(buffer.data(), it - buffer.data());
	}

	template <typename ...Args>
	void Log::_info(std::format_string<Args...> fmt, Args&&... args) {
		writeFileLine("INFO", fmt, std::forward<Args>(args)...);
	}

	template <typename ...Args>
	void Log::_warn(std::format_string<Args...> fmt, Args&&... args) {
		writeFileLine("WARN", fmt, std::forward<Args>(args)...);
	}

	template <typename ...Args>
	void Log::_error(std::format_string<Args...> fmt, Args&&... args) {
		writeFileLine("ERROR", fmt, std::forward<Args>(args)...);
		stream.flush();
	}

	template <typename ...Args>
	void Log::_important(std::format_string<Args...> fmt, Args&&... args) {
		writeFileLine("IMPORTANT", fmt, std::forward<Args>(args)...);
		stream.flush();
	}

}