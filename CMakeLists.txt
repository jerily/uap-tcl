cmake_minimum_required(VERSION 3.22.1)
project(uap-tcl VERSION 1.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(WIN32)
    set(CMAKE_SHARED_LIBRARY_PREFIX "lib")
    set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
endif()
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ADDRESS_SANITIZER "Enable address and undefined behavior sanitizers" OFF)
set(UAP_CORE_DIR "" CACHE PATH "Regenerate tables and test data from this uap-core directory")
find_package(TclStub 8.6 REQUIRED)
include(CTest)

set(UAP_GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(UAP_TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
if(UAP_CORE_DIR)
    if(NOT TCL_TCLSH OR NOT EXISTS "${UAP_CORE_DIR}/regexes.yaml")
        message(FATAL_ERROR "Regeneration requires TCL_TCLSH and UAP_CORE_DIR/regexes.yaml")
    endif()
    set(UAP_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
    set(UAP_TEST_DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/test-data")
    file(MAKE_DIRECTORY "${UAP_GENERATED_DIR}")
    set(generated_files)
    foreach(kind UserAgent OS Device)
        if(kind STREQUAL "UserAgent")
            set(section user_agent_parsers)
        elseif(kind STREQUAL "OS")
            set(section os_parsers)
        else()
            set(section device_parsers)
        endif()
        set(output "${UAP_GENERATED_DIR}/tcluap${kind}_re.h")
        add_custom_command(OUTPUT "${output}"
            COMMAND ${CMAKE_COMMAND} "-DTCL_TCLSH=${TCL_TCLSH}"
                "-DSCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/tools/re_normalize.tcl"
                "-DSECTION=${section}" "-DINPUT=${UAP_CORE_DIR}/regexes.yaml" "-DOUTPUT=${output}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateData.cmake"
            DEPENDS "${UAP_CORE_DIR}/regexes.yaml" tools/re_normalize.tcl cmake/GenerateData.cmake
            VERBATIM)
        list(APPEND generated_files "${output}")
    endforeach()
    if(BUILD_TESTING)
        foreach(group tests test_resources)
            file(MAKE_DIRECTORY "${UAP_TEST_DATA_DIR}/${group}")
            file(GLOB yaml_files CONFIGURE_DEPENDS "${UAP_CORE_DIR}/${group}/*.yaml")
            foreach(input ${yaml_files})
                get_filename_component(name "${input}" NAME_WE)
                set(output "${UAP_TEST_DATA_DIR}/${group}/${name}.data")
                add_custom_command(OUTPUT "${output}"
                    COMMAND ${CMAKE_COMMAND} "-DTCL_TCLSH=${TCL_TCLSH}"
                        "-DSCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/tools/test_normalize.tcl"
                        "-DINPUT=${input}" "-DOUTPUT=${output}"
                        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateData.cmake"
                    DEPENDS "${input}" tools/test_normalize.tcl cmake/GenerateData.cmake
                    VERBATIM)
                list(APPEND generated_files "${output}")
            endforeach()
        endforeach()
    endif()
    add_custom_target(uap_data DEPENDS ${generated_files})
endif()

add_library(tcluap src/common.c src/library.c src/tcluapUserAgent.c
    src/tcluapOS.c src/tcluapDevice.c src/tcluapCache.c)
set_target_properties(tcluap PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(tcluap PRIVATE "${UAP_GENERATED_DIR}" ${TCL_INCLUDE_PATH})
target_compile_definitions(tcluap PRIVATE TCL_THREADS VERSION=${PROJECT_VERSION})
if(TARGET uap_data)
    add_dependencies(tcluap uap_data)
endif()
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(tcluap PRIVATE -Wall -Wextra -Wpedantic)
endif()
if(ADDRESS_SANITIZER)
    target_compile_options(tcluap PRIVATE -g -fsanitize=undefined,address)
    target_link_options(tcluap PRIVATE -fsanitize=undefined,address)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(tcluap PRIVATE TCL_MEM_DEBUG=1)
endif()

if(BUILD_SHARED_LIBS)
    target_compile_definitions(tcluap PRIVATE USE_TCL_STUBS)
    target_link_libraries(tcluap PRIVATE ${TCL_STUB_LIBRARY})
    if(TCL_TCLSH)
        add_custom_target(shell COMMAND ${CMAKE_COMMAND} -E env
            "TCLLIBPATH={${CMAKE_CURRENT_BINARY_DIR}}" ${TCL_TCLSH}
            USES_TERMINAL DEPENDS tcluap)
    endif()
    configure_file(pkgIndex.tcl.in pkgIndex.tcl @ONLY)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl" DESTINATION lib/tcluap${PROJECT_VERSION})
else()
    # The embedding application links Tcl and registers Tcluap_Init.
    target_compile_definitions(tcluap PRIVATE STATIC_BUILD)
endif()
install(TARGETS tcluap
    LIBRARY DESTINATION lib/tcluap${PROJECT_VERSION}
    ARCHIVE DESTINATION lib/tcluap${PROJECT_VERSION}
    RUNTIME DESTINATION lib/tcluap${PROJECT_VERSION})

if(BUILD_TESTING)
    if(BUILD_SHARED_LIBS)
        if(NOT TCL_TCLSH)
            message(FATAL_ERROR "Tests require TCL_TCLSH; disable with BUILD_TESTING=OFF")
        endif()
        set(test_shell "${TCL_TCLSH}")
        set(test_package_dir "${CMAKE_CURRENT_BINARY_DIR}")
        if(UNIX AND CMAKE_NM)
            add_test(NAME TclStubs COMMAND ${CMAKE_COMMAND}
                -DNM=${CMAKE_NM} -DMODULE=$<TARGET_FILE:tcluap>
                -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/stubs.cmake)
        endif()
    else()
        string(REGEX MATCH "^[0-9]+\\.[0-9]+" tcl_version "${TCL_VERSION}")
        find_library(TCL_LIBRARY NAMES tcl${tcl_version} tcl
            HINTS ${TCL_LIBRARY_DIR} "${TCL_INCLUDE_PATH}/../lib" REQUIRED)
        add_executable(test_embed tests/embed.c)
        target_include_directories(test_embed PRIVATE ${TCL_INCLUDE_PATH})
        target_link_libraries(test_embed PRIVATE tcluap ${TCL_LIBRARY})
        set(test_shell "$<TARGET_FILE:test_embed>")
        set(test_package_dir "${CMAKE_CURRENT_BINARY_DIR}/static-tests")
        file(MAKE_DIRECTORY "${test_package_dir}")
        file(WRITE "${test_package_dir}/pkgIndex.tcl"
            "package ifneeded uap ${PROJECT_VERSION} [list load {} Tcluap]\n")
    endif()
    add_test(NAME AllUnitTests COMMAND ${test_shell} ${CMAKE_CURRENT_SOURCE_DIR}/tests/all.tcl)
    set_tests_properties(AllUnitTests PROPERTIES ENVIRONMENT
        "TCLLIBPATH={${test_package_dir}};UAP_TEST_DATA_DIR=${UAP_TEST_DATA_DIR}")
endif()
