cmake_minimum_required(VERSION 3.16)

project(webui-wire LANGUAGES CXX)

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    set(LIBKIND "d")
else()
    set(LIBKIND "r")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
        execute_process(
            COMMAND ${GIT_EXECUTABLE} clone ${dep} ${webui-wire_SOURCE_DIR}/deps/${dir}
            COMMAND_ECHO STDOUT
        )
    endif()
endfunction()

function(sub_target lib dir build_target build_type)
    add_custom_target(${lib}
        ALL ${CMAKE_COMMAND} -S . -B ./build
        COMMAND ${CMAKE_COMMAND} --build ./build --target ${build_target} --config ${build_type}
        WORKING_DIRECTORY ${webui-wire_SOURCE_DIR}/deps/${dir}
    )
    add_custom_target(${lib}-${LIBKIND}
        ALL ${CMAKE_COMMAND} -E copy ${webui-wire_SOURCE_DIR}/deps/${dir}/build/${build_type}/${lib}.lib ${webui-wire_SOURCE_DIR}/deps
        COMMAND ${CMAKE_COMMAND} -E rename ${webui-wire_SOURCE_DIR}/deps/${lib}.lib ${webui-wire_SOURCE_DIR}/deps/${lib}-${LIBKIND}.lib
    )
    add_dependencies(${lib}-${LIBKIND} ${lib})
endfunction()

function(link_target target lib)
    target_link_libraries(${target} ${webui-wire_SOURCE_DIR}/deps/${lib}-${LIBKIND}.lib)
endfunction()

git_dep(webui https://github.com/hdijkema/webui.git)
git_dep(asio https://github.com/chriskohlhoff/asio.git)
git_dep(Crow https://github.com/CrowCpp/Crow.git)
git_dep(upa https://github.com/upa-url/upa.git)
git_dep(json https://github.com/nlohmann/json.git)
git_dep(nativefiledialog https://github.com/hdijkema/nativefiledialog.git)

include_directories(.)
include_directories(deps/webui/include)
include_directories(deps/nativefiledialog/src/include)
include_directories(deps/json/include deps/json/include/nlohmann)
include_directories(deps/Crow/include)
include_directories(deps/asio/asio/include)
include_directories(deps/upa/include/ deps/upa/include/upa)

if(TARGET clean)
    set(upa_target clean)
    set(other_target clean)
else()
    set(upa_target upa_url)
    set(other_target ALL_BUILD)
endif()

sub_target(nativefiledialog-1 nativefiledialog ${other_target} ${CMAKE_BUILD_TYPE})
sub_target(webui-2-static webui ${other_target} Release) #Debug)
sub_target(upa_url upa ${upa_target} ${CMAKE_BUILD_TYPE})

add_library(libwebui-wire SHARED
    webui_wire.h
    webui_wire_defs.h
    i_webui_wire.h
    i_webui_wire.cpp

    misc.h misc.cpp
    variant_t.h

    application_t.h application_t.cpp
    object_t.h object_t.cpp
    eventqueue_t.h eventqueue_t.cpp

    utf8_strings.h utf8_strings.cpp

    readlineinthread.h readlineinthread.cpp
    timer_t.h timer_t.cpp
    event_t.h event_t.cpp
    fileinfo_t.h fileinfo_t.cpp

    webwirehandler.h webwirehandler.cpp
    webwireprofile.h webwireprofile.cpp
    webuiwindow.h webuiwindow.cpp
    default_css.h default_css.cpp
    execjs.h execjs.cpp
    webwirestandarddialogs.h webwirestandarddialogs.cpp

    httpserver_t.h httpserver_t.cpp
    mimetypes_t.h mimetypes_t.cpp

    ${webui-wire_SOURCE_DIR}/deps/upa/src/url.cpp
)

add_dependencies(libwebui-wire webui-2-static-${LIBKIND} nativefiledialog-1-${LIBKIND} upa_url-${LIBKIND})

target_compile_definitions(libwebui-wire PRIVATE LIBWEBUI_WIRE_BUILDING)

link_target(libwebui-wire webui-2-static)
link_target(libwebui-wire nativefiledialog-1)
link_target(libwebui-wire upa_url)

add_executable(webui-wire main.cpp)
target_link_libraries(webui-wire libwebui-wire.lib)

add_dependencies(webui-wire libwebui-wire)

include(GNUInstallDirs)
install(TARGETS libwebui-wire
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
