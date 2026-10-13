cmake_minimum_required(VERSION 3.20)

project(controller LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_program(CONTROLLER_PROTOC_EXECUTABLE
    NAMES protoc
    PATHS /usr/bin /usr/sbin
    NO_DEFAULT_PATH)
if(CONTROLLER_PROTOC_EXECUTABLE)
    set(Protobuf_PROTOC_EXECUTABLE
        ${CONTROLLER_PROTOC_EXECUTABLE}
        CACHE FILEPATH "Protocol Buffers compiler" FORCE)
endif()

find_package(OpenSSL REQUIRED)
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost 1.90 REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/proto/messages.proto)

add_library(controller_proto ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(controller_proto PUBLIC protobuf::libprotobuf)
target_include_directories(controller_proto PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(controller
    src/main.cc
    src/Server.cc
    src/TCPServer.cc
    src/UDPServer.cc
    src/SessionManager.cc
    src/DeviceSession.cc
    src/ControllerManager.cc
    src/Controller.cc
)

target_include_directories(controller PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
)

target_compile_definitions(controller PRIVATE
    BOOST_ERROR_CODE_HEADER_ONLY
)

target_link_libraries(controller PRIVATE
    controller_proto
    OpenSSL::Crypto
    Threads::Threads
)