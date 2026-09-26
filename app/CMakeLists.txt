include(ECMQmlModule)

add_executable(xspeedtest-app main.cpp)
set_target_properties(xspeedtest-app PROPERTIES OUTPUT_NAME xspeedtest)
ecm_add_qml_module(xspeedtest-app URI "org.xspeedtest.app" VERSION 1.0)
ecm_target_qml_sources(xspeedtest-app SOURCES Main.qml)
target_link_libraries(xspeedtest-app PRIVATE Qt6::Widgets Qt6::Quick Qt6::Qml)
add_dependencies(xspeedtest-app xspeedtest)

install(TARGETS xspeedtest-app ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
