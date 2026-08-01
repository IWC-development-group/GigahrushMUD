#pragma once

#include <string>
#include <gmbp/geo.h>

class ServerConfig {
private:
	std::string serverName;
	std::string gameName;
	gmbp::geo::Country country;
	uint32_t port;

public:
	ServerConfig();

	void load();

	const std::string& getServerName() const { return serverName; }
	const std::string& getGameName() const { return gameName; }
	gmbp::geo::Country getCountry() const { return country; }
	uint32_t getPort() const { return port; }

	static ServerConfig loadOrDefault();
};