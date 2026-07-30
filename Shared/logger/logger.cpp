#include "logger.h"

namespace logger {

	Log* Log::log = nullptr;

	Log::Log() {}
	
	Log::Log(const std::string& filename) {
		stream.open(filename);
	}

	Log::~Log() {
		if (stream.is_open()) stream.close();
	}

	Log* Log::init() {
		if (!log) log = new Log;
	}

	Log* Log::init(const std::string& filename) {
		if (!log) log = new Log(filename);
	}

	Log* Log::get() { return log; }

	void Log::destroy() {
		if (log != nullptr) {
			delete log;
			log = nullptr;
		}
	}

}