set(SOURCES
	gmbp/geo.cpp
)

set(HEADERS
	gmbp/geo.h
	gmbp/protocol.h
)

add_library(gmbp ${SOURCES} ${HEADERS})