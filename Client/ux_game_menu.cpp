#include "ux_game_menu.h"

namespace ux {

	GameMenu::GameMenu() {
		serverList = ftxui::Renderer([&] {
			return ftxui::vbox(servers) | ftxui::focusPositionRelative(0.0f, 1.0f);
		});

		connectButton = ftxui::Button("Подключиться", Connect);
	}

}