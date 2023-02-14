project(Tests VERSION 0.1)

set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

find_package(Catch2 REQUIRED)

include(CTest)
include(Catch)

juce_add_console_app(Tests PRODUCT_NAME "Unit Test Runner")


file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.h" "*.cpp")


target_sources(Tests PRIVATE ${SOURCE_LIST})

target_compile_definitions(Tests PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0)

target_link_libraries(Tests PRIVATE
        Catch2::Catch2WithMain
        juce_recommended_config_flags
        juce_recommended_lto_flags
        juce_recommended_warning_flags
        juce_core
        shared_modules)

catch_discover_tests(Tests)
