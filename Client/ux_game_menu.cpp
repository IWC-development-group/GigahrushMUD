#include "ux_game_menu.h"
#include "connection_event.h"

namespace ux {

	using namespace lsid::literals;

	std::string serverEntryToString(const ServerEntry& entry) {
		return std::format(
			"[{}] {} | {}:{}",
			gmbp::geo::countryToString(entry.server.country),
			entry.server.name,
			entry.ip,
			entry.port);
	}

	GameMenu::GameMenu(EventManager& _events) : events(_events), selectedServer(0) {
		ftxui::MenuOption menuOption;

		rebuildList();

		serverList = ftxui::Menu(&elements, &selectedServer, menuOption);

		connectButton = ftxui::Button("Connect", [this]() {			
			if (servers.empty()) return;

			std::string nick("player");

			events.fire<ConnectionEvent, "ON_CONNECT_PRESSED"_sid32>(
				servers[selectedServer].ip,
				std::to_string(servers[selectedServer].port),
				nick
			);
		});

		ftxui::Component base = ftxui::Container::Vertical({
			serverList,
			connectButton
		});

		container = ftxui::Renderer(base, [this] {
			return ftxui::vbox({
				ftxui::text("Servers"),
				serverList->Render()
				|	ftxui::frame
				|	ftxui::vscroll_indicator
				|	ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, 8),
				ftxui::separator(),
				connectButton->Render()
			});
		});
	}

	void GameMenu::rebuildList() {
		elements.clear();
		
		for (auto& server : servers) {
			elements.push_back(serverEntryToString(server));
		}
	}

	void GameMenu::addServer(const gmbp::ServerBroadcast& server, const std::string& ip, int port) {
		servers.push_back({
			.server = server,
			.ip = ip,
			.port = port
		});

		rebuildList();
	}
}