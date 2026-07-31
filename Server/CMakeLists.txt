set(SOURCES 
	Server.cpp
	Session.cpp
	main.cpp
	Game/Game.cpp
	Game/JsonParser.cpp
	Game/GameStructures.cpp
	Game/CommandHandler.cpp
	utils.cpp
)

set(HEADERS 
	Server.h
	Session.h
	Game/Game.h
	Game/JsonParser.h
	Game/GameStructures.h
	Game/CommandHandler.h
	utils.h
)

add_executable (GigahrushServer ${SOURCES} ${HEADERS})

set(SHARED_DIR "${CMAKE_SOURCE_DIR}/Shared")

target_include_directories(GigahrushServer PRIVATE
	"${SHARED_DIR}/"
)

target_link_libraries(GigahrushServer PRIVATE
	gmbp
	logger
	asio
	nlohmann_json
)

target_compile_options(GigahrushServer PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)