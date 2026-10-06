add_library(zoey_core STATIC
    appcontroller.cpp
    appcontroller.h
)

target_include_directories(zoey_core PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(zoey_core PUBLIC Qt${QT_VERSION_MAJOR}::Core)
