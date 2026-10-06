add_executable(tst_appcontroller tst_appcontroller.cpp)
target_link_libraries(tst_appcontroller PRIVATE zoey_core Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME appcontroller COMMAND tst_appcontroller)

add_executable(tst_mainwindow
    tst_mainwindow.cpp
    ${PROJECT_SOURCE_DIR}/src/ui/mainwindow.cpp
    ${PROJECT_SOURCE_DIR}/src/ui/mainwindow.h
)
target_link_libraries(tst_mainwindow PRIVATE
    zoey_core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME mainwindow COMMAND tst_mainwindow)
set_tests_properties(mainwindow PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
