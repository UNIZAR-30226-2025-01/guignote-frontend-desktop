cmake_minimum_required(VERSION 3.16)
project(guignote VERSION 0.1 LANGUAGES CXX)

option(BUILD_TESTING "Build tests" ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Multimedia WebSockets Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Multimedia WebSockets Test)

# ----------- 1.  FUENTES NORMALES -----------------
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp        mainwindow.h  mainwindow.ui
    menuwindow.cpp        menuwindow.h  menuwindow.ui
    loginwindow.cpp       loginwindow.h
    registerwindow.cpp    registerwindow.h
    recoverpasswordwindow.cpp  recoverpasswordwindow.h
    imagebutton.cpp       imagebutton.h
    icon.cpp              icon.h
    friendswindow.cpp     friendswindow.h
    settingswindow.cpp    settingswindow.h  settingswindow.ui
    inventorywindow.cpp   inventorywindow.h
    myprofilewindow.cpp   myprofilewindow.h
    userprofilewindow.cpp userprofilewindow.h
    loadingwindow.cpp     loadingwindow.h
    friendsmessagewindow.cpp friendsmessagewindow.h
    gamemessagewindow.cpp gamemessagewindow.h
    rankingwindow.cpp     rankingwindow.h
    orientacion.h
    ventanasalirpartida.h ventanasalirpartida.cpp
    botonaccion.h botonaccion.cpp
    estadopartida.h estadopartida.cpp
    carta.cpp carta.h
    mano.cpp mano.h
    resource.qrc
)

qt_add_executable(guignote MANUAL_FINALIZATION ${PROJECT_SOURCES}
    rejoinwindow.h rejoinwindow.cpp
    customgameswindow.h customgameswindow.cpp crearcustomgame.h crearcustomgame.cpp

)

target_link_libraries(guignote PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::WebSockets
)
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(guignote)
endif()

# ----------- 2.  EJECUTABLE DE TESTS ---------------
if(BUILD_TESTING)
    enable_testing()

    # quitamos main.cpp de la lista de fuentes para los tests
    list(REMOVE_ITEM PROJECT_SOURCES main.cpp)

    add_executable(tests_guignote
        tests/main_tests.cpp
        tests/test_mainwindow.cpp
        tests/test_registerwindow.cpp
        tests/test_loginwindow.cpp
        ${PROJECT_SOURCES}
        tests/test_mainwindow.h
        tests/test_registerwindow.h
        tests/test_loginwindow.h
        tests/test_menuwindow.h
        tests/test_menuwindow.cpp
        tests/test_userprofilewindow.h
        tests/test_userprofilewindow.cpp
        tests/test_settingswindow.h
        tests/test_settingswindow.cpp
        tests/test_rankingwindow.h
        tests/test_rankingwindow.cpp
        tests/test_myprofilewindow.h
        tests/test_myprofilewindow.cpp
        tests/test_inventorywindow.h
        tests/test_inventorywindow.cpp
        tests/test_friendswindow.h
        tests/test_friendswindow.cpp
        rejoinwindow.h
        rejoinwindow.cpp
        customgameswindow.h customgameswindow.cpp crearcustomgame.h crearcustomgame.cpp





        # ← añade TODA la lógica a probar
    )
    set_source_files_properties(tests/main_tests.cpp PROPERTIES SKIP_AUTOMOC ON)

    target_include_directories(tests_guignote PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    target_link_libraries(tests_guignote PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test      # imprescindible
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Multimedia
        Qt${QT_VERSION_MAJOR}::WebSockets
    )

    add_test(NAME guignote_tests COMMAND tests_guignote)
endif()
