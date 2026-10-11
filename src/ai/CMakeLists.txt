add_library(zoey_ai STATIC
    modelconfig.h
    modelservice.cpp
    modelservice.h
    aiclient.cpp
    aiclient.h
)

target_include_directories(zoey_ai PUBLIC
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(zoey_ai PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
)
