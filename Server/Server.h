#pragma once
#include <optional>
#include <asio.hpp>
#include "Session.h"
#include "server_config.h"

class Server {
	private:
		asio::io_context& io_context;
		asio::ip::tcp::acceptor acceptor;
		std::optional<asio::ip::tcp::socket> socket;

		asio::steady_timer timer1;
		asio::steady_timer samosborIntervalTimer;
		asio::steady_timer samosborDuringTimer;
		asio::steady_timer timer2;
		asio::steady_timer timer3;

		asio::steady_timer autosave_timer;

		ServerConfig config;

		Server(asio::io_context& io_context, ServerConfig config);

	public:
		std::string autosave_filename;
		bool autosaveGoing;

		std::vector<std::weak_ptr<Session>> allSessions;
		Server(asio::io_context& io_context);
		void newPlayerNotify(std::string);
		void disconnectPlayerNotify(std::string);
		void sendChatMessage(std::shared_ptr<Gigahrush::Player>, std::string);

		void notifyAll(const std::string&);

		//Timers
		void mapUpdate(const asio::error_code&);
		void waitSamosbor();
		void startSamosbor();
		void stopSamosbor(bool);

		void autosave();

		void checkPlayersSamosbor();

		void async_accept();
		void startMapUpdate();

		ServerConfig& getConfig() { return config; }
		asio::io_context& getContext() { return io_context; }
};