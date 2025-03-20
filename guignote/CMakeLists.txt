cmake_minimum_required(VERSION 3.16)

project(guignote VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Actualizamos la búsqueda para incluir WebSockets.
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Multimedia WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Multimedia WebSockets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        menuwindow.h
        menuwindow.cpp
        menuwindow.ui  # <-- ¡Asegura que CMAKE_AUTOUIC lo procese!
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(guignote
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        resource.qrc
        loginwindow.h loginwindow.cpp
        registerwindow.h registerwindow.cpp
        recoverpasswordwindow.h recoverpasswordwindow.cpp
        menuwindow.h menuwindow.cpp
        imagebutton.h imagebutton.cpp
        icon.h
        icon.cpp
        icons/friends.png icons/hide_password.png icons/house-door.png icons/padlock.png icons/settings.png
        icons/divider.png
        friendswindow.h
        settingswindow.h settingswindow.cpp
        settingswindow.ui
        friendswindow.h friendswindow.cpp
        inventorywindow.h inventorywindow.cpp
        myprofilewindow.h myprofilewindow.cpp
        icons/profile.png
        userprofilewindow.h userprofilewindow.cpp
        loadingwindow.h loadingwindow.cpp
        friendsmessagewindow.h friendsmessagewindow.cpp
    )
else()
    if(ANDROID)
        add_library(guignote SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(guignote
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Enlazamos las bibliotecas, incluyendo WebSockets.
target_link_libraries(guignote PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Multimedia Qt${QT_VERSION_MAJOR}::WebSockets)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.guignote)
endif()
set_target_properties(guignote PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS guignote
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(guignote)
endif()
