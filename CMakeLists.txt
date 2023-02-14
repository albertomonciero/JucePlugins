cmake_minimum_required(VERSION 3.16)
project(JucePlugins)

set(CMAKE_CXX_STANDARD 17)

set_property(GLOBAL PROPERTY USE_FOLDER YES)

add_subdirectory (Ext/JUCE)
add_subdirectory(Modules)

enable_testing()
add_subdirectory(Tests)

