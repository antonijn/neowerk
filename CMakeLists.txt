cmake_minimum_required(VERSION 3.14)
project(werk)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
)
FetchContent_Declare(
	range-v3
	GIT_REPOSITORY git@github.com:ericniebler/range-v3.git
	GIT_TAG "0.12.0"
	GIT_SHALLOW TRUE
	GIT_PROGRESS ON
	SYSTEM
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest range-v3)

add_executable(werk src/gbuffer.cpp src/main.cpp)

target_link_libraries(werk range-v3)


enable_testing()

add_executable(gbuffer_test src/gbuffer.cpp src/gbuffer_test.cpp)
target_link_libraries(gbuffer_test range-v3 GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(gbuffer_test)
