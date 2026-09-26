set(CORE_SRC ${CMAKE_SOURCE_DIR}/src)

function(xspeedtest_test name)
    add_executable(${name} ${name}.cpp ${ARGN})
    target_include_directories(${name} PRIVATE ${CORE_SRC})
    target_link_libraries(${name} PRIVATE Qt6::Core Qt6::Test)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

xspeedtest_test(tst_history ${CORE_SRC}/historymodel.cpp)

xspeedtest_test(tst_settings ${CORE_SRC}/speedsettings.cpp)

xspeedtest_test(tst_plasmoidinstaller ${CORE_SRC}/plasmoidinstaller.cpp ${CORE_SRC}/paths.cpp)
target_compile_definitions(tst_plasmoidinstaller PRIVATE XSPEEDTEST_PLASMOID_INSTALL_PATH="/nonexistent/installed-plasmoid" XSPEEDTEST_MODULE_INSTALL_PATH="/nonexistent/module" XSPEEDTEST_HELPER_INSTALL_PATH="/nonexistent/helper")

xspeedtest_test(tst_runner ${CORE_SRC}/speedtestrunner.cpp ${CORE_SRC}/historymodel.cpp ${CORE_SRC}/paths.cpp)
target_compile_definitions(tst_runner PRIVATE XSPEEDTEST_HELPER_INSTALL_PATH="/nonexistent/installed-helper" XSPEEDTEST_PLASMOID_INSTALL_PATH="/nonexistent" XSPEEDTEST_MODULE_INSTALL_PATH="/nonexistent" STUB_HELPER="${CMAKE_CURRENT_SOURCE_DIR}/stub-helper.sh")

add_executable(tst_qmlload tst_qmlload.cpp)
target_link_libraries(tst_qmlload PRIVATE Qt6::Test Qt6::Quick Qt6::Qml)
target_compile_definitions(tst_qmlload PRIVATE QML_IMPORT_DIR="${CMAKE_BINARY_DIR}/bin")
add_dependencies(tst_qmlload xspeedtest)
add_test(NAME tst_qmlload COMMAND tst_qmlload)

add_executable(tst_visualizers tst_visualizers.cpp)
target_link_libraries(tst_visualizers PRIVATE Qt6::Test Qt6::Qml)
target_compile_definitions(tst_visualizers PRIVATE VISUALIZERS_JS="${CMAKE_SOURCE_DIR}/src/qml/Visualizers.js")
add_test(NAME tst_visualizers COMMAND tst_visualizers)
