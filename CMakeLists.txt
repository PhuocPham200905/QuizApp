cmake_minimum_required(VERSION 3.20)
project(QuizApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(QuizApp WIN32
    main.cpp
    GuiApp.cpp
    GuiApp.h
    src/Common.h
    src/models/Models.h
    src/services/FirebaseService.h
    src/data/QuizData.h
    src/ui/ControlIds.h
    src/ui/features/GuiCore.inl
    src/ui/features/AuthScreens.inl
    src/ui/features/DashboardScreens.inl
    src/ui/features/QuestionScreens.inl
    src/ui/features/ExamScreens.inl
    src/ui/features/ResultScreens.inl
    src/ui/features/GuiUtilities.inl
)

source_group("Models" FILES
    src/models/Models.h
)

source_group("Data and Services" FILES
    src/services/FirebaseService.h
    src/data/QuizData.h
)

source_group("UI Features" FILES
    src/ui/features/AuthScreens.inl
    src/ui/features/DashboardScreens.inl
    src/ui/features/QuestionScreens.inl
    src/ui/features/ExamScreens.inl
    src/ui/features/ResultScreens.inl
)

source_group("UI Core" FILES
    src/ui/ControlIds.h
    src/ui/features/GuiCore.inl
    src/ui/features/GuiUtilities.inl
)

if(MSVC)
    target_compile_options(QuizApp PRIVATE /utf-8)
else()
    target_compile_options(QuizApp PRIVATE
        -finput-charset=UTF-8
        -fexec-charset=UTF-8
    )
endif()

target_link_libraries(QuizApp PRIVATE
    comctl32
    comdlg32
    shell32
    ole32
    gdiplus
    nlohmann_json::nlohmann_json
    CURL::libcurl
    ZLIB::ZLIB
)

if(WIN32)
    add_custom_command(TARGET QuizApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:QuizApp>
            $<TARGET_FILE_DIR:QuizApp>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/sample_questions.csv
            $<TARGET_FILE_DIR:QuizApp>/sample_questions.csv
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/assets/hcmute-logo.png
            $<TARGET_FILE_DIR:QuizApp>/hcmute-logo.png
        COMMAND_EXPAND_LISTS
    )
endif()
