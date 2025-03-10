cmake_minimum_required(VERSION 3.16)

project(guignote VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)


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
        myprofilewindow.h myprofilewindow.cpp
        icons/profile.png





    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET guignote APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(guignote SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(guignote
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(guignote PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
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
