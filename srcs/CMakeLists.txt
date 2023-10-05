cmake_minimum_required(VERSION 3.25)
project(srcs)

set(CMAKE_CXX_STANDARD 14)

include_directories(AMethod)
include_directories(cgi)
include_directories(networking)
include_directories(util)

add_executable(srcs
        AMethod/AMethod.cpp
        AMethod/AMethod.hpp
        AMethod/main.cpp
        AMethod/MethodGet.cpp
        AMethod/MethodGet.hpp
        cgi/cgi.cpp
        cgi/cgi.hpp
        cgi/HttpRequest.cpp
        cgi/HttpRequest.hpp
        networking/ChangeList.cpp
        networking/ChangeList.hpp
        networking/dataSet.hpp
        networking/httpRequestParsing.cpp
        networking/initServer.cpp
        networking/main.cpp
        networking/main_WebServerSample.cpp
        networking/test.cpp
        networking/UserData.cpp
        networking/UserData.hpp
        networking/waitForClientConnection.cpp
        networking/WebServer.cpp
        networking/WebServer.hpp
        networking/webServerParseFileOrNull.cpp
        util/Colors.cpp
        util/Colors.hpp
        util/Error.cpp
        util/Error.hpp
        util/MultiTree.cpp
        util/MultiTree.hpp
        util/MultiTreeNode.cpp
        util/MultiTreeNode.hpp
        util/test.cpp)
