set(GMBP_SOURCES
	gmbp/geo.cpp
)

set(GMBP_HEADERS
	gmbp/geo.h
	gmbp/protocol.h
)

set(LOGGER_SOURCES
	logger/logger.cpp
)

set(LOGGER_HEADERS
	logger/logger.h
)

add_library(gmbp ${GMBP_SOURCES} ${GMBP_HEADERS})
add_library(logger ${LOGGER_SOURCES} ${LOGGER_HEADERS})