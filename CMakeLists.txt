cmake_minimum_required(VERSION 3.24)
project(xspeedtest VERSION 0.1.0 LANGUAGES CXX)

find_package(ECM 6.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Widgets Qml Quick Test)
find_package(KF6WindowSystem REQUIRED)

set(XSPEEDTEST_HELPER_INSTALL_DIR ${KDE_INSTALL_LIBEXECDIR}/xspeedtest)
set(XSPEEDTEST_PLASMOID_INSTALL_DIR ${KDE_INSTALL_DATADIR}/xspeedtest/plasmoid)

configure_file(helper/xspeedtest-helper ${CMAKE_BINARY_DIR}/bin/xspeedtest-helper COPYONLY)
add_custom_target(plasmoid-dev ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/plasmoid ${CMAKE_BINARY_DIR}/bin/plasmoid)

add_subdirectory(src)
add_subdirectory(app)
add_subdirectory(tests)

install(PROGRAMS helper/xspeedtest-helper DESTINATION ${XSPEEDTEST_HELPER_INSTALL_DIR})
install(DIRECTORY plasmoid/ DESTINATION ${XSPEEDTEST_PLASMOID_INSTALL_DIR})
install(FILES data/org.xspeedtest.app.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES plasmoid/contents/icons/xspeedtest.svg DESTINATION ${KDE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
