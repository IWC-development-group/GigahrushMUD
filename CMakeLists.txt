cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 23)
project(GigahrushMUD)

add_subdirectory("Shared")
add_subdirectory("Server")
add_subdirectory("Client")

# Third-party
add_subdirectory("thirdparty/asio")
add_subdirectory("thirdparty/nlohmann")
add_subdirectory("thirdparty/ftxui")