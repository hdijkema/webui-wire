cmake_minimum_required(VERSION 3.16)

project(webui-wire LANGUAGES CXX)
enable_language(C)

#set(CMAKE_VERBOSE_MAKEFILE ON)

if(APPLE)
    enable_language(OBJC)
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(LIBKIND "d")
else()
    set(LIBKIND "r")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(LIBEXT lib)
    set(LIBPRE )
else()
    set(LIBEXT a)
    set(LIBPRE lib)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
  add_compile_options(-fPIC)
  set(CFLAGS_ENV "CFLAGS=-fPIC")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  add_compile_options(-fPIC)
  set(CFLAGS_ENV "CFLAGS=-fPIC")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # using Intel C++
  set(CFLAGS_ENV "")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
  set(CFLAGS_ENV "")
endif()


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GIT_EXECUTABLE git)

if(NOT(EXISTS ./deps))
    file(MAKE_DIRECTORY ./deps)
endif()

function(git_dep dir dep)
    if(EXISTS ${webui-wire_SOURCE_DIR}/deps/${dir})
        execute_process(
            COMMAND ${GIT_EXECUTABLE} pull
            COMMAND_ECHO STDOUT
            WORKING_DIRECTORY ${webui-wire_SOURCE_DIR}/deps/${dir}
        )
    else()
        message("Executing git clone...")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} clone ${dep} ${webui-wire_SOURCE_DIR}/deps/${dir}
            COMMAND_ECHO STDOUT
        )
    endif()
endfunction()

function(sub_target lib dir build_target build_type)
    add_custom_target(${lib}
        ALL ${CMAKE_COMMAND} -E env ${CFLAGS_ENV} ${CMAKE_COMMAND} -S . -B ./build/${build_type}
        COMMAND ${CMAKE_COMMAND} -E env "CFLAGS=-fPIC" ${CMAKE_COMMAND} --build ./build/${build_type} --target ${build_target} --config ${build_type}
        WORKING_DIRECTORY ${webui-wire_SOURCE_DIR}/deps/${dir}
    )
    add_custom_target(${lib}-${LIBKIND}
        ALL ${CMAKE_COMMAND} -E copy ${webui-wire_SOURCE_DIR}/deps/${dir}/build/${build_type}/${lib}.${LIBEXT} ${webui-wire_SOURCE_DIR}/deps
        COMMAND ${CMAKE_COMMAND} -E rename ${webui-wire_SOURCE_DIR}/deps/${lib}.${LIBEXT} ${webui-wire_SOURCE_DIR}/deps/${lib}-${LIBKIND}.${LIBEXT}
    )
    add_dependencies(${lib}-${LIBKIND} ${lib})
endfunction()

function(link_target target lib)
    target_link_libraries(${target} ${webui-wire_SOURCE_DIR}/deps/${lib}-${LIBKIND}.${LIBEXT})
endfunction()

git_dep(webui https://github.com/hdijkema/webui.git)
git_dep(CxxUrl https://github.com/chmike/CxxUrl.git)
git_dep(json https://github.com/nlohmann/json.git)
git_dep(nativefiledialog https://github.com/hdijkema/nativefiledialog.git)

include_directories(. include include/base include/utils)
include_directories(deps/webui/include)
include_directories(deps/nativefiledialog/src/include)
include_directories(deps/json/include deps/json/include/nlohmann)
include_directories(deps/cxxurl)

if(TARGET clean)
    set(other_target clean)
else()
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(other_target ALL_BUILD)
    else()
        set(other_target all)
    endif()
endif()


add_subdirectory(deps/nativefiledialog)
add_subdirectory(deps/webui)
add_subdirectory(deps/CxxUrl)

add_library(libwebui-wire STATIC

    # main webui-wire stuff
    include/webui_wire.h include/webui_wire_defs.h

    # interface, possibly for FFI
    include/i_webui_wire.h src/i_webui_wire.cpp

    # webui-wire / webui related
    include/webwirehandler.h src/webwirehandler.cpp
    include/webwireprofile.h src/webwireprofile.cpp
    include/webuiwindow.h src/webuiwindow.cpp
    include/webwirestandarddialogs.h src/webwirestandarddialogs.cpp
    include/readlineinthread.h src/readlineinthread.cpp
    include/execjs.h src/execjs.cpp
    include/mimetypes_t.h src/mimetypes_t.cpp
    include/default_css.h src/default_css.cpp
    include/httpresponse_t.h src/httpresponse_t.cpp

    # Base functionality
    include/base/object_t.h src/base/object_t.cpp
    include/base/eventqueue_t.h src/base/eventqueue_t.cpp
    include/base/event_t.h src/base/event_t.cpp
    include/base/timer_t.h src/base/timer_t.cpp
    include/base/application_t.h src/base/application_t.cpp

    # Utility basic stuff
    include/utils/misc.h src/utils/misc.cpp
    include/utils/utf8_strings.h src/utils/utf8_strings.cpp
    include/utils/utf8_utils.h src/utils/utf8_utils.c
    include/utils/webui_utils.h src/utils/webui_utils.cpp
    include/utils/fileinfo_t.h src/utils/fileinfo_t.cpp
    include/utils/variant_t.h
    src/utils/utf8_utils.cpp
    include/utils/json.h
    src/utils/json.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK REQUIRED gtk+-3.0)

    include_directories(${GTK_INCLUDE_DIRS})
    message("${GTK_INCLUDE_DIRS}")
    link_directories(${GTK_LIBRARY_DIRS})
endif()

target_compile_definitions(libwebui-wire PRIVATE LIBWEBUI_WIRE_BUILDING)

add_executable(webui-wire
    src/main.cpp
)

if(APPLE)
    target_sources(libwebui-wire PRIVATE
        include/utils/apple_utils.h
        src/utils/apple_utils.mm
    )
endif()

target_link_libraries(webui-wire libwebui-wire)

target_link_libraries(libwebui-wire CxxUrl)
target_link_libraries(libwebui-wire webui)
target_link_libraries(libwebui-wire nativefiledialog)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(webui-wire ${GTK_LIBRARIES})
endif()

include(GNUInstallDirs)
install(TARGETS webui-wire
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES webui-wire.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)

