set(SOURCES
	Client.cpp
	main.cpp
	ux_game_menu.cpp)

set(HEADERS
	Client.h
	Config.h
	Parser.h
	ux_game_menu.h
	connection_event.h
)

add_executable(GigahrushClient ${SOURCES} ${HEADERS})

set(LSID_DIR "${CMAKE_SOURCE_DIR}/thirdparty/lsid")
set(SHARED_DIR "${CMAKE_SOURCE_DIR}/Shared")

target_include_directories(GigahrushClient PRIVATE
	"${LSID_DIR}/"
	"${SHARED_DIR}/"
)

target_link_libraries(GigahrushClient PRIVATE
	asio
	nlohmann_json
	gmbp

	# FTXUI
	ftxui::component
	ftxui::dom
	ftxui::screen
)

target_compile_options(GigahrushClient PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)