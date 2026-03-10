cmake_minimum_required(VERSION 3.18)

project(ToolsCollection
    VERSION 0.1.0
    DESCRIPTION "Collection of Useful Tools"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



# find_package(jsoncpp REQUIRED)
find_package(SharedCppLib2 REQUIRED)

add_executable(mysha256 mysha256.cpp)
target_link_libraries(mysha256 PRIVATE SharedCppLib2::sha256)

add_executable(ftree ftree.cpp)
target_link_libraries(ftree PRIVATE SharedCppLib2::basic SharedCppLib2::regexfilter)

add_executable(csleep csleep.cpp)
target_link_libraries(csleep PRIVATE SharedCppLib2::basic)

add_executable(base64 base64_tool.cpp)
target_link_libraries(base64 PRIVATE SharedCppLib2::basic SharedCppLib2::Base64 SharedCppLib2::arguments)

if(WIN32)
    add_executable(grep windows/grep.cpp)

    add_executable(whoareyou windows/whoareyou.cpp)
    target_link_libraries(whoareyou PRIVATE psapi SharedCppLib2::basic SharedCppLib2::logt)

    add_executable(silentlaunch windows/silentlaunch.cpp)
    target_link_libraries(silentlaunch PRIVATE SharedCppLib2::basic)

    add_executable(pesigverify windows/pesigverify.cpp)
    target_link_libraries(pesigverify PRIVATE wintrust)

    add_executable(whaterr windows/whaterr.cpp)
    target_link_libraries(whaterr PRIVATE SharedCppLib2::platform)

    add_executable(Mousemover windows/mousemover.cpp)
endif()