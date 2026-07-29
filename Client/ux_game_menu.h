#pragma once

#include <string>
#include <vector>

#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component_options.hpp"

namespace ux {

	class GameMenu {
	private:
		ftxui::Component serverList;
		ftxui::Component connectButton;
		ftxui::Component serverListWindow;
				
		std::vector<ftxui::Element> servers;

	public:
		GameMenu();

		void addServer(const std::string& server);

		ftxui::Component getComponent() const;
		ftxui::Element getElement() const;
	};

} 