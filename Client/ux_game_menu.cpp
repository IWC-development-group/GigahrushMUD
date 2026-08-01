#include "ux_game_menu.h"
#include "events.h"

namespace ux {

	using namespace lsid::literals;

	std::string serverEntryToString(const ServerEntry& entry) {
		return std::format(
			"[{}] {} | {}:{}",
			gmbp::geo::countryToString(entry.server.country),
			entry.server.name,
			entry.ip,
			entry.server.port);
	}

	GameMenu::GameMenu(EventManager& _events) : events(_events), selectedServer(0) {
		ftxui::MenuOption menuOption;
		menuOption.on_enter = [&] {
			connectToSelected();
		};

		serverList = ftxui::Menu(&elements, &selectedServer, menuOption);

		connectButton = ftxui::Button("Connect", [this]() {
			connectToSelected();
		});

		refreshButton = ftxui::Button("Refresh", [this]() {
			servers.clear();
			elements.clear();
			events.fire<RefreshEvent, "ON_REFRESH_PRESSED"_sid32>();
		});

		ftxui::Component base = ftxui::Container::Vertical({
			serverList,
			connectButton,
			refreshButton
		});

		container = ftxui::Renderer(base, [this] {
			return ftxui::vbox({
				ftxui::hbox({
					connectButton->Render(),
					refreshButton->Render(),
					ftxui::filler()
				}),

				//ftxui::filler(),
				ftxui::separator(),

				serverList->Render()
				| ftxui::frame
				| ftxui::vscroll_indicator
			}) | ftxui::flex;
		});
	}

	void GameMenu::connectToSelected() {
		if (servers.empty()) return;

		std::string nick("player");

		events.fire<ConnectionEvent, "ON_CONNECT_PRESSED"_sid32>(
			servers[selectedServer].ip,
			std::to_string(servers[selectedServer].server.port),
			nick
		);
	}

	void GameMenu::rebuildList() {
	}

	void GameMenu::addServer(const gmbp::ServerBroadcast& server, const std::string& ip) {
		servers.push_back({
			.server = server,
			.ip = ip
		});

		elements.push_back(serverEntryToString(servers[servers.size() - 1]));
	}
}