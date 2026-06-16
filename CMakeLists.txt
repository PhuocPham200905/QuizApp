cmake_minimum_required(VERSION 3.20)
project(QuizApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(CURL REQUIRED)

add_executable(QuizApp WIN32
    main.cpp
    GuiApp.cpp
)

target_link_libraries(QuizApp PRIVATE
    comctl32
    nlohmann_json::nlohmann_json
    CURL::libcurl
)

if(WIN32)
    add_custom_command(TARGET QuizApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:QuizApp>
            $<TARGET_FILE_DIR:QuizApp>
        COMMAND_EXPAND_LISTS
    )
endif()
