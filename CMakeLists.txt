cmake_minimum_required(VERSION 3.19)
project(FileExplorer LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(FileExplorer
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    node.h node.cpp
    notepad.ui
    notepad.h notepad.cpp
    originfile.h originfile.cpp
    directory.h directory.cpp
    file.h file.cpp
    historynode.h historynode.cpp
    navigationhistory.h navigationhistory.cpp




)

target_link_libraries(FileExplorer
    PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)

install(TARGETS FileExplorer
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET FileExplorer
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
