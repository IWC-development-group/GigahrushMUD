set(SOURCES
	Client.cpp
	main.cpp
	"gigamud_proto.cpp"
	"ux_game_menu.cpp")

set(HEADERS
	Client.h
	Config.h
	Parser.h
	"gigamud_proto.h"
	"broadcaster.h"
	"ux_game_menu.h" "connection_event.h")

add_executable(GigahrushClient ${SOURCES} ${HEADERS})

set(LSID_DIR "${CMAKE_SOURCE_DIR}/thirdparty/lsid")

target_include_directories(GigahrushClient PRIVATE "${LSID_DIR}/")

target_link_libraries(GigahrushClient PRIVATE asio)
target_link_libraries(GigahrushClient PRIVATE nlohmann_json)

target_link_libraries(GigahrushClient PRIVATE 
	ftxui::component
	ftxui::dom
	ftxui::screen
)

target_compile_options(GigahrushClient PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)