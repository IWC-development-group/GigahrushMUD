#pragma once

#include <string>
#include <vector>

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component_options.hpp>
#include <gmbp/protocol.h>

#include "events/event_manager.h"

namespace ux {

	struct ServerEntry {
		gmbp::ServerBroadcast server;
		std::string ip;
		int port;
	};

	class GameMenu {
	private:
		EventManager& events;

		ftxui::Component serverList;
		ftxui::Component connectButton;
		ftxui::Component refreshButton;
		ftxui::Component container;

		int selectedServer;
		std::vector<ServerEntry> servers;
		std::vector<std::string> elements;

		void rebuildList();
		void connectToSelected();

	public:
		GameMenu(EventManager& events);

		void addServer(const gmbp::ServerBroadcast& server, const std::string& ip);
		ftxui::Component getComponent() const { return container; }

		const ServerEntry& getSelectedServer() const { return servers[selectedServer]; }
		const std::string& getSelectedElement() const { return elements[selectedServer]; }
	};

} 