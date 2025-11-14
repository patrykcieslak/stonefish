add_definitions(-DDATA_DIR_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/Data/\")

add_executable(ConsoleTest ConsoleTest/main.cpp ConsoleTest/ConsoleTestApp.cpp ConsoleTest/ConsoleTestManager.cpp)
target_link_libraries(ConsoleTest Stonefish_test)

add_executable(CameraTest CameraTest/main.cpp CameraTest/CameraTestManager.cpp)
target_link_libraries(CameraTest Stonefish_test)

add_executable(FallingTest FallingTest/main.cpp FallingTest/FallingTestApp.cpp FallingTest/FallingTestManager.cpp)
target_link_libraries(FallingTest Stonefish_test)

add_executable(FloatingTest FloatingTest/main.cpp FloatingTest/FloatingTestManager.cpp)
target_link_libraries(FloatingTest Stonefish_test)

add_executable(FlyingTest FlyingTest/main.cpp FlyingTest/FlyingTestManager.cpp)
target_link_libraries(FlyingTest Stonefish_test)

add_executable(JointsTest JointsTest/main.cpp JointsTest/JointsTestManager.cpp JointsTest/JointsTestApp.cpp)
target_link_libraries(JointsTest Stonefish_test)

add_executable(SlidingTest SlidingTest/main.cpp SlidingTest/SlidingTestApp.cpp SlidingTest/SlidingTestManager.cpp)
target_link_libraries(SlidingTest Stonefish_test)

add_executable(UnderwaterTest UnderwaterTest/main.cpp UnderwaterTest/UnderwaterTestApp.cpp UnderwaterTest/UnderwaterTestManager.cpp)
target_link_libraries(UnderwaterTest Stonefish_test)

add_executable(FluidDynamicsTest FluidDynamicsTest/main.cpp FluidDynamicsTest/FluidDynamicsTestApp.cpp FluidDynamicsTest/FluidDynamicsTestManager.cpp)
target_link_libraries(FluidDynamicsTest Stonefish_test)

add_executable(LearningTest LearningTest/main.cpp LearningTest/LearningTestManager.cpp)
target_link_libraries(LearningTest Stonefish_test)

add_executable(CableTest CableTest/main.cpp CableTest/CableTestApp.cpp CableTest/CableTestManager.cpp)
target_link_libraries(CableTest Stonefish_test)