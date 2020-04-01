cmake_minimum_required(VERSION 3.10)
project(Stonefish)

add_definitions(-DDATA_DIR_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/Data/\")

add_executable(ConsoleTest ConsoleTest/main.cpp ConsoleTest/ConsoleTestApp.cpp ConsoleTest/ConsoleTestManager.cpp)
target_link_libraries(ConsoleTest Stonefish)

add_executable(FallingTest FallingTest/main.cpp FallingTest/FallingTestApp.cpp FallingTest/FallingTestManager.cpp)
target_link_libraries(FallingTest Stonefish)

add_executable(FloatingTest FloatingTest/main.cpp FloatingTest/FloatingTestManager.cpp)
target_link_libraries(FloatingTest Stonefish)

add_executable(FlyingTest FlyingTest/main.cpp FlyingTest/FlyingTestManager.cpp)
target_link_libraries(FlyingTest Stonefish)

add_executable(JointsTest JointsTest/main.cpp JointsTest/JointsTestManager.cpp)
target_link_libraries(JointsTest Stonefish)

add_executable(SlidingTest SlidingTest/main.cpp SlidingTest/SlidingTestApp.cpp SlidingTest/SlidingTestManager.cpp)
target_link_libraries(SlidingTest Stonefish)

add_executable(UnderwaterTest UnderwaterTest/main.cpp UnderwaterTest/UnderwaterTestApp.cpp UnderwaterTest/UnderwaterTestManager.cpp)
target_link_libraries(UnderwaterTest Stonefish)