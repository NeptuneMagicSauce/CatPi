cmake_minimum_required(VERSION 3.25)
project(CatPi)
FILE(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp)
FILE(GLOB_RECURSE resources CONFIGURE_DEPENDS data/*.qrc)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_LINKER_TYPE "LLD")
# compile-commands are needed by clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON) # must be before add_executable() # fails with clang19 (and modules?)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
add_executable(${PROJECT_NAME} ${sources} ${resources})
set(QTLIBLIST Widgets) # Core Gui
find_package(Qt6 REQUIRED ${QTLIBLIST})
foreach(qtlib ${QTLIBLIST})
  target_link_libraries(${PROJECT_NAME} Qt6::${qtlib})
endforeach()
set(QTPLUGINDIR plugins/platforms)

# hardcode, works on my machine
target_link_libraries(${PROJECT_NAME} ${LIBPINCTRLDIR}/libpinctrl.a)
