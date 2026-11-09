cmake_minimum_required(VERSION 3.16)

project(BrainBox VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt 6 Widgets
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Enable automatic handling of Qt meta-object code, resources, etc.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Create the executable
add_executable(BrainBox
    main.cpp
)

# Link against Qt 6 Widgets
target_link_libraries(BrainBox
    PRIVATE Qt6::Widgets
)

configure_file(questions_single.json questions_single.json COPYONLY)
configure_file(questions_multi.json questions_multi.json COPYONLY)