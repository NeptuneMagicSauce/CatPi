cmake_minimum_required(VERSION 3.25)
project(CatPi)
FILE(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp)
FILE(GLOB_RECURSE resources CONFIGURE_DEPENDS data/*.qrc)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_LINKER_TYPE "LLD")
# compile-commands are needed by clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_AUTOMOC ON) # not needed, must be before add_executable()
set(CMAKE_AUTORCC ON)
# set(CMAKE_AUTOUIC ON)) # not needed
add_compile_options(-Wall -Wextra) # must be before add_executable()
add_executable(${PROJECT_NAME} ${sources} ${resources})
set(QTLIBLIST Widgets) # Core Gui
find_package(Qt6 REQUIRED ${QTLIBLIST})
foreach(qtlib ${QTLIBLIST})
  target_link_libraries(${PROJECT_NAME} Qt6::${qtlib})
endforeach()
# set(QTPLUGINDIR plugins/platforms) // really ?

# lib pinctrl
target_link_libraries(${PROJECT_NAME} ${LIBPINCTRLDIR}/libpinctrl.a)

# lib hx711
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${LIBHX711}/include")
target_link_libraries(${PROJECT_NAME} "-llgpio")
target_link_libraries(${PROJECT_NAME} "${LIBHX711}/build/static/libhx711.a")

# needed for stacktrace
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  target_link_libraries(${PROJECT_NAME} "-lstdc++exp")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
  target_link_libraries(${PROJECT_NAME} "-no-pie")
endif()
