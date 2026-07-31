#include <iostream>
#include <locale>
#include <thread>
#include <mutex>
#include <atomic>
#include <print>

#include <gmbp/protocol.h>
#include <broadcast/broadcaster.h>
#include <logger/logger.h>

#include "Server.h"
#include "Game/Game.h"

using Log = logger::Log;

std::mutex gameMutex;
std::atomic<bool> serverRunning = false;
std::atomic<bool> isExit = false;
std::atomic<bool> serverActive = false;
Server* srvv = nullptr;

std::string toLowerCase(std::string str) {
	std::string res = "";
	for (auto c : str) {
		res += std::tolower(c);
	}
	return res;
}

void StartServer() {
	asio::io_context io_context;
	Server srv(io_context, 15001);
	srvv = &srv;

	srv.startMapUpdate();
	srv.checkPlayersSamosbor();

	Gigahrush::Game::Instance().srv = &srv;

	while (!isExit) {
		while (!serverRunning) {
			if (isExit) { break; }
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		srv.async_accept();
		while (serverRunning == true) {
			serverActive = true;
			io_context.poll();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		if (serverActive == true) {
			io_context.stop();
			std::cout << "Server stopped\n";
			srv.stopSamosbor(true);
		}
		else {
			std::cout << "Server not started xd\n";
			srv.stopSamosbor(true);
		}
	}

	if (serverActive == true) {
		io_context.stop();
		std::cout << "Server stopped\n";
		srv.stopSamosbor(true);
	}
	else {
		std::cout << "Server not started xd\n";
		srv.stopSamosbor(true);
	}
}

void Terminal() {
	srand(time(0));
	Gigahrush::Game& Game = Gigahrush::Game::Instance();
	setlocale(LC_ALL, "ru_RU.utf8");

	std::thread serverThread(StartServer);

	while (true) {
		std::cout << "> ";
		std::string command;
		std::getline(std::cin, command);
		std::string lowCom = toLowerCase(command);

		if (lowCom == "loadconf") {
			gameMutex.lock();
			Game.LoadConfig();
			gameMutex.unlock();
		}
		else if (lowCom == "gen") {
			gameMutex.lock();
			bool x = Game.GenerateGame();
			gameMutex.unlock();
		}
		else if (lowCom == "start") {
			std::cout << "Starting server...\n";

			gameMutex.lock();
			if (Game.isGenerated) {
				serverRunning = true;
				Game.isReseted = false;
			}
			else {
				std::cout << "Can't start server. Game is not generated or loaded.\n";
			}
			gameMutex.unlock();
		}
		else if (lowCom == "stop") {
			std::cout << "Stopping server...\n";
			serverRunning = false;
		}
		else if (lowCom == "reset") {
			if (srvv != nullptr) {
				std::cout << "Stopping server...\n";
				serverRunning = false;

				std::cout << "Reseting game...\n";
				Game.isReseted = true;
				gameMutex.lock();
				Game.ResetGame();
				for (auto& it : srvv->allSessions) {
					auto p = it.lock();
					if (p != nullptr) {
						p->sessionPlayer.reset();
						std::cout << "Player reseted\n";
					}
				}
			}

			gameMutex.unlock();
		}
		else if (lowCom == "info") {
			gameMutex.lock();
			Game.Info();
			gameMutex.unlock();
		}
		else if (lowCom == "exit") {
			std::cout << "Stopping server...\n";
			serverRunning = false;
			isExit = true;

			if (serverThread.joinable()) {
				serverThread.join();
			}

			break;
		}
		else if (lowCom == "") {
			std::cout << "No command!\n";
		}
		else {
			std::stringstream ss(lowCom);
			std::vector<std::string> words;
			std::string word;
			while (ss >> word) {
				words.push_back(word);
			}

			if (words[0] == "save") {
				if (words.size() >= 2) {
					Game.SaveGame(words[1]);
				}
				else {
					std::cout << "Bad syntax!\n";
				}
			}
			else if (words[0] == "load") {
				if (words.size() >= 2) {
					Game.LoadGame(words[1]);
				}
				else {
					std::cout << "Bad syntax!\n";
				}
			}
			else if (words[0] == "autosave") {
				if (words.size() >= 2) {
					if (words[1] == "name") {
						if (words.size() >= 3) {
							if (srvv != nullptr) {
								srvv->autosave_filename = words[2];
								std::cout << "Autosave filename changed to " << words[2] << "\n";
							}
						}
						else {
							std::cout << "Bad syntax!\n";
						}
					}
					else if (words[1] == "start") {
						if (srvv != nullptr) {
							std::cout << "Autosave started\n";
							srvv->autosaveGoing = true;
							srvv->autosave();
						}
					}
					else if (words[1] == "stop") {
						if (srvv != nullptr) {
							srvv->autosaveGoing = false;
						}
					}
				}
				else {
					std::cout << "Bad syntax!\n";
				}
			}
		}
	}
}

using ServerBroadcaster = Broadcaster<gmbp::ServerBroadcast, gmbp::ClientBroadcast>;

void processServerBroadcast() {
	/* Wait for the other possible threads that will set global booleans to TRUE */
	while (!serverRunning.load() || !srvv) {
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}

	ServerBroadcaster broadcaster(srvv->getContext(), GMBP_DEFAULT_PORT);

	gmbp::ServerBroadcast response;
	response.header.type = gmbp::Type::SERVER_BROADCAST;
	response.country = gmbp::geo::RU;
	response.playerCount = 1;

	std::strcpy(response.game, "GigahrushMUD (RU)");
	std::strcpy(response.name, "Kupitman's daily v-rot server");

	broadcaster.onReceive([&](const gmbp::ClientBroadcast& request, size_t bytes, asio::ip::udp::endpoint endpoint) {
		std::string_view clientHeader(request.header.header);
		std::string_view gmbpHeader(GMBP_HEADER);

		if (clientHeader != gmbpHeader || request.header.type != gmbp::Type::CLIENT_BROADCAST) return;

		Log::important("Receiving packet with size {} from {}", bytes, endpoint.address().to_string());
		broadcaster.send(response);
	});

	while (serverRunning.load()) {
		broadcaster.poll();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	/*
		FIXME: 
		If the server was stopped it will stop all broadcast processing with no ability to resume it
	*/
}

int main() {
	Log::init();

	std::thread t1(Terminal);
	
	processServerBroadcast();
	
	if (t1.joinable()) {
		t1.join();
	}

	Log::destroy();
	return 0;
}