#pragma once
#include <any>
#include <unordered_map>
#include <lsid.h>

class EventManager {
private:
	std::unordered_map<uint32_t, std::any> eventTable;

public:
	EventManager() {}

	template <uint32_t Id, typename EventT>
	void add(const EventT& event) {
		eventTable[Id] = event;
	}

	template <typename EventT, uint32_t Id>
	EventT* get() {
		auto it = eventTable.find(Id);
		return std::any_cast<EventT>(&it->second);
	}

	template <typename EventT, uint32_t Id, typename ...Args>
	void fire(Args&&... args) { get<EventT, Id>()->fire(std::forward<Args>(args)...); }
};