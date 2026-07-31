#include "logger.h"

namespace logger {

	Log* Log::log = nullptr;

	Log::Log() {}
	
	Log::Log(const std::string& filename) {
		stream.open(filename);
	}

	Log::~Log() {
		if (!stream.is_open()) return;
		stream.flush();
		stream.close();
	}

	Log* Log::init() {
		if (!log) log = new Log;
		return log;
	}

	Log* Log::init(const std::string& filename) {
		if (!log) log = new Log(filename);
		return log;
	}

	Log* Log::get() { return log; }

	void Log::destroy() {
		if (!log) return;
		delete log;
		log = nullptr;
	}

}