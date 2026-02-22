include(ECMAddTests)

# Common test sources (core + backends needed for parsing tests)
set(TEST_COMMON_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/commandrunner.cpp
    ${CMAKE_SOURCE_DIR}/src/core/packagemodel.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/toolchecker.cpp
    ${CMAKE_SOURCE_DIR}/src/backends/pacmanbackend.cpp
    ${CMAKE_SOURCE_DIR}/src/backends/aurbackend.cpp
    ${CMAKE_SOURCE_DIR}/src/backends/flatpakbackend.cpp
    ${CMAKE_SOURCE_DIR}/src/backends/firmwarebackend.cpp
    ${CMAKE_SOURCE_DIR}/src/backends/updatemanager.cpp
)

set(TEST_COMMON_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src  # For generated kcfg header
)

set(TEST_COMMON_LINK_LIBRARIES
    Qt6::Test
    Qt6::Core
    Qt6::Quick
    Qt6::Qml
    KF6::CoreAddons
    KF6::I18n
    KF6::ConfigCore
    KF6::ConfigGui
)

# CommandRunner test
add_executable(tst_commandrunner tst_commandrunner.cpp ${TEST_COMMON_SOURCES})
target_include_directories(tst_commandrunner PRIVATE ${TEST_COMMON_INCLUDE_DIRS})
target_link_libraries(tst_commandrunner PRIVATE ${TEST_COMMON_LINK_LIBRARIES})
add_test(NAME tst_commandrunner COMMAND tst_commandrunner)

# Pacman backend test
add_executable(tst_pacmanbackend tst_pacmanbackend.cpp ${TEST_COMMON_SOURCES})
target_include_directories(tst_pacmanbackend PRIVATE ${TEST_COMMON_INCLUDE_DIRS})
target_link_libraries(tst_pacmanbackend PRIVATE ${TEST_COMMON_LINK_LIBRARIES})
add_test(NAME tst_pacmanbackend COMMAND tst_pacmanbackend)

# Flatpak backend test
add_executable(tst_flatpakbackend tst_flatpakbackend.cpp ${TEST_COMMON_SOURCES})
target_include_directories(tst_flatpakbackend PRIVATE ${TEST_COMMON_INCLUDE_DIRS})
target_link_libraries(tst_flatpakbackend PRIVATE ${TEST_COMMON_LINK_LIBRARIES})
add_test(NAME tst_flatpakbackend COMMAND tst_flatpakbackend)
