#include "server_config.h"
#include "Game/JsonParser.h"
#include <logger/logger.h>

using JSONParser = Gigahrush::JSONParser;
using Log = logger::Log;

ServerConfig::ServerConfig()
:	serverName("GigahrushMUD server"),
	gameName("GigahrushMUD"),
	country(gmbp::geo::RU),
	port(15001U) {
}

void ServerConfig::load() {
	JSONParser& parser = JSONParser::Instance();
	nlohmann::json config = parser.readFile("Config/Server.json");

	serverName = config["ServerName"].get<std::string>();
	gameName = config["GameName"].get<std::string>();
	country = config["Country"].get<gmbp::geo::Country>();
	port = config["Port"].get<uint32_t>();
}

ServerConfig ServerConfig::loadOrDefault() {
	ServerConfig config;

	try {
		config.load();
	}
	catch (std::exception& exc) {
		Log::error("Can't load server config: {}", exc.what());
	}

	return config;
}