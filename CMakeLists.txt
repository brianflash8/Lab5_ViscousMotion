cmake_minimum_required(VERSION 3.25)
project(Lab5_ViscousMotion LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

# Фізична модель (без SFML)
add_library(physics STATIC physics/ViscousBody.cpp)
target_include_directories(physics PUBLIC physics)

# Графічна програма
find_package(SFML COMPONENTS Graphics Window System CONFIG REQUIRED)
add_executable(viscous_app app/main.cpp)
target_link_libraries(viscous_app PRIVATE physics SFML::Graphics SFML::Window SFML::System)

# Обчислювальний експеримент (консольна програма)
add_executable(experiment app/experiment.cpp)
target_link_libraries(experiment PRIVATE physics)

# Модульні тести
add_executable(lab5_tests tests/test_main.cpp tests/PhysicsTests.cpp)
target_include_directories(lab5_tests PRIVATE tests)
target_link_libraries(lab5_tests PRIVATE physics)
add_test(NAME lab5_unit_tests COMMAND lab5_tests)

if(MSVC)
    foreach(target physics viscous_app experiment lab5_tests)
        target_compile_options(${target} PRIVATE /utf-8 /W4)
        target_compile_definitions(${target} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endforeach()
endif()