add_subdirectory(ai)
add_subdirectory(core)

add_executable(Zoey
    main.cpp
    ui/mainwindow.cpp
    ui/mainwindow.h
)

target_include_directories(Zoey PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Zoey PRIVATE
    zoey_core
    zoey_ai
    Qt${QT_VERSION_MAJOR}::Widgets
)

install(TARGETS Zoey RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
