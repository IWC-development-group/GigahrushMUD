#include <locale>
#include <codecvt>
#include <thread>
#include <mutex>
#include <atomic>
#include <deque>

#include <asio.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component_options.hpp>
#include <nlohmann/json.hpp>
#include <gmbp/protocol.h>
#include <broadcast/broadcaster.h>
#include <logger/logger.h>

#include "Client.h"
#include "Config.h"
#include "Parser.h"
#include "events.h"
#include "ux_game_menu.h"

using Log = logger::Log;

enum class State {CONNECTED, DISCONNECTED};

std::atomic<bool> broadcastPollRunning = true;
std::atomic<bool> bgRunning = false;
std::atomic<bool> refreshNeeded = false;

std::mutex mtx;

std::atomic<State> state = State::DISCONNECTED;

std::vector<ftxui::Element> logs;
std::vector<ftxui::Element> serverMessages;

std::string map;

std::string ip;
std::string port;
std::string nick;

EventManager events;

asio::io_context io_context;
Client client(io_context, ip, port);

using ClientBroadcaster = Broadcaster<gmbp::ClientBroadcast, gmbp::ServerBroadcast>;
ClientBroadcaster broadcaster(io_context, GMBP_DEFAULT_PORT, false);

std::string lastCommand;

std::thread inGameUpdater;
std::thread broadcastPoller;

auto screen = ftxui::ScreenInteractive::Fullscreen();

void UpdateMsgThread();

/* !!! Runs UpdateMsgThread !!! */
void Connect(const std::string& ip, const std::string& port, std::string& nick) {
	std::lock_guard<std::mutex> lock(mtx);

	if (state == State::CONNECTED) { return; }

	try {
		client.ip = ip;
		client.port = port;

		client.Connect();
		state = State::CONNECTED;
		bgRunning = true;
		inGameUpdater = std::thread(UpdateMsgThread);
		client.Send(nick);
	}
	catch (const std::exception& ec) {
		state = State::DISCONNECTED;
	}
}

void SendServ(std::string request) {
	std::lock_guard<std::mutex> lock(mtx);

	try {
		client.Send(request);
	}
	catch (const std::exception& e) {
		return;
	}
}

void broadcastPoll() {
	while (broadcastPollRunning) {
		if (broadcaster.poll() && state == State::DISCONNECTED) {
			screen.PostEvent(ftxui::Event::Special("refresh"));
		}
	}
}

bool inGameUpdate() {
	if (state == State::DISCONNECTED) return false;

	asio::error_code ec;
	client.recv_buffer_server.resize(4096);
	size_t br = client.socket.read_some(asio::buffer(client.recv_buffer_server), ec);

	if (ec) { return false; }

	client.recv_buffer_server.resize(br);

	try {
		Log::important("Some hueta detected");

		nlohmann::json js = nlohmann::json::parse(client.recv_buffer_server);
		if (js["type"] == "ANSWER") {
			addLog(logs, js);
		}
		else if (js["type"] == "MAP") {
			map = js["content"];
		}
		else if (js["type"] == "SERVER") {
			addServerMsg(serverMessages, js);
		}
	}
	catch (std::exception& er) {
		//Пока уберу для релиза
		logs.push_back(ftxui::text(er.what()) | ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 50));
	}

	return true;
}

void UpdateMsgThread() {
	while (bgRunning) {
		if (inGameUpdate()) screen.PostEvent(ftxui::Event::Special("refresh"));
	}
}

/* Runs broadcastPoll thread */
void MainThread() {
	broadcastPollRunning = true;
	broadcastPoller = std::thread(broadcastPoll);

	ConnectionEvent connectPressed;
	connectPressed.onEvent([&](const std::string& ip, const std::string& port, std::string& nick) {
		Connect(ip, port, nick);
	});

	RefreshEvent refreshPressed;
	refreshPressed.onEvent([&] {
		gmbp::ClientBroadcast clientBroadcast;
		clientBroadcast.header.type = gmbp::Type::CLIENT_BROADCAST;
		broadcaster.send(clientBroadcast);
	});

	using namespace lsid::literals;

	events.add<"ON_CONNECT_PRESSED"_sid32>(connectPressed);
	events.add<"ON_REFRESH_PRESSED"_sid32>(refreshPressed);

	/* FTXUI */

	/* Menu elements */

	ux::GameMenu gameMenu(events);
	ftxui::Component firstField = gameMenu.getComponent();

	Log::info("Main thread started");

	broadcaster.onReceive([&](const gmbp::ServerBroadcast& server, size_t bytes, asio::ip::udp::endpoint endpoint) {
		std::lock_guard<std::mutex> guard(mtx);

		int port = endpoint.port();

		std::string_view serverHeader(server.header.header);
		std::string_view gmbpHeader(GMBP_HEADER);

		if (serverHeader != gmbpHeader || server.header.type != gmbp::Type::SERVER_BROADCAST) {
			return;
		}

		Log::important("{} from {}:{} ([{}] {})",
			bytes,
			endpoint.address().to_string(),
			port,
			gmbp::geo::countryToString(server.country),
			server.name
		);
		
		gameMenu.addServer(server, endpoint.address().to_string());
	});

	/* Main box elements */

	std::string userCommand;
	ftxui::Component commandInput = ftxui::Input(&userCommand);
	int selected_log = 0;

	ftxui::Component logWindow = ftxui::Renderer([&] {
		return ftxui::vbox(logs) | ftxui::focusPositionRelative(0.0f, 1.0f);
	});

	ftxui::Component serverWindow = ftxui::Renderer([&] {
		return ftxui::vbox(serverMessages) | ftxui::focusPositionRelative(0.0f, 1.0f);
	});

	ftxui::Component mapWindow = ftxui::Renderer([&] {
		std::vector<ftxui::Element> elements;

		elements.push_back(ftxui::paragraph(map));

		return ftxui::vbox(elements) | ftxui::focusPositionRelative(0.0f, 1.0f);
	});

	ftxui::Component mainInputHandler = ftxui::CatchEvent(commandInput, [&](ftxui::Event event) {
		if (event == ftxui::Event::Return) {
			if (userCommand == "") { return true; }

			logs.push_back(ftxui::text(""));
			logs.push_back(ftxui::text("---------------------"));
			logs.push_back(ftxui::text(""));

			logs.push_back(ftxui::text(userCommand) | ftxui::color(DECORATE_COLOR) | ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 50));

			SendServ(userCommand);
			lastCommand = userCommand;
			userCommand = "";
			return true;
		}

		if (event == ftxui::Event::ArrowUp) {
			userCommand = lastCommand;
			return true;
		}

		return false;
	});

	ftxui::Component mainBox = ftxui::Container::Vertical({
			firstField,
			logWindow,
			mainInputHandler,
			serverWindow,
			mapWindow
	});

	ftxui::Component renderer = ftxui::Renderer(mainBox, [&] {
		/* Logs */

		auto login_form = ftxui::vbox({ firstField->Render() });

		//auto centered_content = ftxui::vbox({ login_form }) | ftxui::flex;

		if (state == State::DISCONNECTED) {
			return ftxui::window(ftxui::text("Серверы") | ftxui::bold, login_form)
				|	ftxui::flex;
		}

		auto game_box = ftxui::vbox({
			logWindow->Render() | ftxui::frame | ftxui::border | ftxui::flex,
			ftxui::hbox(ftxui::text("Команда: "), commandInput->Render()) }) | ftxui::flex;
		auto game_window = ftxui::window(ftxui::text("Гигахрущ"), game_box) | ftxui::color(MAIN_COLOR);

		auto server_box = ftxui::vbox({ serverWindow->Render() | ftxui::frame | ftxui::flex }) | ftxui::flex;
		auto server_window = ftxui::window(ftxui::text("Оповещения сервера"), server_box) | ftxui::color(ADD_COLOR2) | ftxui::flex;

		auto map_box = ftxui::vbox({ mapWindow->Render() | ftxui::frame | ftxui::flex }) | ftxui::flex;
		auto map_window = ftxui::window(ftxui::text("Карта"), map_box) | ftxui::color(ADD_COLOR1) | ftxui::flex;

		auto right_column = ftxui::vbox({
			server_window | ftxui::flex_grow | ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, 14),
			map_window | ftxui::flex_grow
		}) | ftxui::flex;  

		auto main_layout = ftxui::hbox({
			game_window | ftxui::flex_grow,
			right_column | ftxui::flex_grow  
		}) | ftxui::flex;

		return main_layout;
	});

	events.fire<RefreshEvent, "ON_REFRESH_PRESSED"_sid32>();
	screen.Loop(renderer);

	return;
}

int main() {
	Log::init("debug.log");

	MainThread();

	/* Stopping all tasks */

	if (inGameUpdater.joinable()) {
		inGameUpdater.join();
	}

	broadcastPollRunning = false;
	broadcaster.close();

	if (broadcastPoller.joinable()) {
		broadcastPoller.join();
	}

	Log::destroy();
	return 0;
}