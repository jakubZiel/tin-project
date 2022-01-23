cmake_minimum_required(VERSION 3.15)
project(tin_21z)
include_directories(include)

set(CMAKE_CXX_STANDARD 14)

add_executable(server
        server/Server.cpp
        )
add_executable(tin-client
        client/Client.cpp
        )

add_executable(server-adm
        server/AdminServer.cpp
        )

add_executable(client-adm
        client/AdminClient.cpp
        server/admin-server/ChannelManager.cpp)

add_executable(json
        main.cpp
)