#pragma once
#include "events/event.h"

class ConnectionEvent : public Event<
	const std::string&,
	const std::string&,
	std::string&> {
public:
	ConnectionEvent() {}
};

class RefreshEvent : public Event<> {
public:
	RefreshEvent() {}
};