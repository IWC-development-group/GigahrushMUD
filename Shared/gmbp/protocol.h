#pragma once

#include <stdint.h>
#include "geo.h"

/*
============================================================
------------- GIGAHRUSH MUD BROADCAST PROTOCOL -------------
============================================================
*/

#define GMBP_DEFAULT_PORT			50478
#define GMBP_HEADER					"GMBP v1.0_"
#define GMBP_SRV_NAME_LENGTH		128
#define GMBP_SRV_GAME_LENGTH		32

namespace gmbp {

	enum class Type : uint8_t {
		CLIENT_BROADCAST,
		SERVER_BROADCAST
	};

	struct GmbpHeader {
		const char header[11] = GMBP_HEADER;
		Type type;
	};

	struct ClientBroadcast {
		GmbpHeader header;
	};

	struct ServerBroadcast {
		GmbpHeader header;
		geo::Country country;
		char name[GMBP_SRV_NAME_LENGTH];
		char game[GMBP_SRV_GAME_LENGTH];
		uint32_t playerCount;
	};

	struct ServerListHeader {
		uint32_t playerCount;
	};

	struct PlayerHeader {
		uint32_t nicknameLength;
	};

}