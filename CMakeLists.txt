cmake_minimum_required(VERSION 3.20)

project(
    zuu_json
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

option(ENABLE_BENCHMARK "Enable benchmark support" OFF)

# ============================================================
# clang-format tool
# ============================================================

find_program(CLANG_FORMAT_EXE NAMES clang-format)

if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/include/*.hpp
        ${CMAKE_SOURCE_DIR}/internal/*.hpp
        ${CMAKE_SOURCE_DIR}/tests/*.cpp
    )

    add_custom_target(
        format
        COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${ALL_SOURCE_FILES}
        COMMENT "Formatting source files with clang-format"
    )

    add_custom_target(
        format-check
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror -style=file ${ALL_SOURCE_FILES}
        COMMENT "Checking source formatting"
    )
endif()

# ============================================================
# check IPO support
# ============================================================

include(CheckIPOSupported)

check_ipo_supported(
    RESULT IPO_SUPPORTED
    OUTPUT IPO_ERROR
)

# ============================================================
# library set
# ============================================================

set(LIBRARY_OUTPUT_NAME cpp_json_lib)

add_library(${LIBRARY_OUTPUT_NAME}
	${CMAKE_SOURCE_DIR}/src/models/array_view.cpp
	${CMAKE_SOURCE_DIR}/src/models/json.cpp
	${CMAKE_SOURCE_DIR}/src/models/member_view.cpp
	${CMAKE_SOURCE_DIR}/src/models/object_view.cpp
	${CMAKE_SOURCE_DIR}/src/models/storage.cpp
	${CMAKE_SOURCE_DIR}/src/models/value.cpp
	${CMAKE_SOURCE_DIR}/src/parser/parser.cpp
	${CMAKE_SOURCE_DIR}/src/serializer/serializer.cpp
)

target_include_directories(
    ${LIBRARY_OUTPUT_NAME}
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/internal
)

# ============================================================
# main program set
# ============================================================

add_executable(${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/src/main.cpp
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/internal
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        ${LIBRARY_OUTPUT_NAME}
)
# ============================================================
# example program set
# ============================================================

file(GLOB EXAMPLE_SOURCES CONFIGURE_DEPENDS
	${CMAKE_SOURCE_DIR}/example/*.cpp
)

foreach(SOURCE ${EXAMPLE_SOURCES})
	get_filename_component(
		TARGET_NAME 
		${SOURCE} 
		NAME_WE
	)

	add_executable(${TARGET_NAME}
		${SOURCE}
	)

	target_include_directories(${TARGET_NAME}
		PRIVATE
			${CMAKE_SOURCE_DIR}/include
			${CMAKE_SOURCE_DIR}/internal
	)

	target_link_libraries(${TARGET_NAME}
		PRIVATE
			${LIBRARY_OUTPUT_NAME}
	)
endforeach()

# ============================================================
# benchmark program set
# ============================================================

if(ENABLE_BENCHMARK)

    add_subdirectory(third_party/google-benchmark)
    add_executable(${PROJECT_NAME}_benchmark
		${CMAKE_SOURCE_DIR}/tests/bm_dom.cpp
		${CMAKE_SOURCE_DIR}/tests/bm_error_path.cpp
		${CMAKE_SOURCE_DIR}/tests/bm_number.cpp
		${CMAKE_SOURCE_DIR}/tests/bm_parser.cpp
		${CMAKE_SOURCE_DIR}/tests/bm_pipeline.cpp
		${CMAKE_SOURCE_DIR}/tests/bm_serializer.cpp
		${CMAKE_SOURCE_DIR}/tests/bm_storage.cpp
		${CMAKE_SOURCE_DIR}/tests/bm_strings.cpp
		${CMAKE_SOURCE_DIR}/tests/bm_tokenizer.cpp
        ${CMAKE_SOURCE_DIR}/tests/bm_main.cpp
    )

    target_include_directories(
        ${PROJECT_NAME}_benchmark
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/internal
            ${CMAKE_SOURCE_DIR}/tests
    )

    target_link_libraries(
        ${PROJECT_NAME}_benchmark
        PRIVATE
            ${LIBRARY_OUTPUT_NAME}
            benchmark::benchmark
    )

endif()

# ============================================================
# clang-tidy tool
# ============================================================

option(ENABLE_CLANG_TIDY "Enable clang-tidy checking" OFF)

if(ENABLE_CLANG_TIDY)
	find_program(CLANG_TIDY_EXE NAMES clang-tidy)

	if(CLANG_TIDY_EXE)
		set_target_properties(
			${LIBRARY_OUTPUT_NAME}
			PROPERTIES
			CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-p=${CMAKE_BINARY_DIR}"
		)

		set_target_properties(
			${PROJECT_NAME}
			PROPERTIES
			CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-p=${CMAKE_BINARY_DIR}"
		)

		if(DEFINED ALL_SOURCE_FILES)
            add_custom_target(
                tidy
                COMMAND ${CLANG_TIDY_EXE} -p=${CMAKE_BINARY_DIR} --fix ${ALL_SOURCE_FILES}
                COMMENT "Fixing source files with clang-tidy"
            )

            add_custom_target(
                tidy-check
                COMMAND ${CLANG_TIDY_EXE} -p=${CMAKE_BINARY_DIR} --warnings-as-errors=* ${ALL_SOURCE_FILES}
                COMMENT "Checking source files with clang-tidy"
            )
        endif()
	endif()
endif()

# ============================================================
# release optimization set
# ============================================================

function(enable_release_optimizations TARGET_NAME)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(
            ${TARGET_NAME}
            PRIVATE
                $<$<CONFIG:Release>:-O3>
                $<$<CONFIG:Release>:-march=native>
                $<$<CONFIG:Release>:-mtune=native>
                $<$<CONFIG:Release>:-DNDEBUG>
                $<$<CONFIG:Release>:-ffunction-sections>
                $<$<CONFIG:Release>:-fdata-sections>
                $<$<CONFIG:Release>:-mavx2>
        )

        if(WIN32)
            target_link_options(
                ${TARGET_NAME}
                PRIVATE
                    $<$<CONFIG:Release>:LINKER:/OPT:NOREF>
                    $<$<CONFIG:Release>:LINKER:/OPT:ICF>
            )
        else()
            target_link_options(
                ${TARGET_NAME}
                PRIVATE
                    $<$<CONFIG:Release>:-Wl>
                    $<$<CONFIG:Release>:--gc-sections>
            )
        endif()

    elseif(MSVC)
        target_compile_options(
            ${TARGET_NAME}
            PRIVATE
                $<$<CONFIG:Release>:/O2>
                $<$<CONFIG:Release>:/GL>
                $<$<CONFIG:Release>:/DNDEBUG>
                $<$<CONFIG:Release>:/arch:AVX2>
        )

        target_link_options(
            ${TARGET_NAME}
            PRIVATE
                $<$<CONFIG:Release>:/LTCG>
        )
    endif()
endfunction()

# ============================================================
# apply optimization on defined target
# ============================================================

enable_release_optimizations(${LIBRARY_OUTPUT_NAME})
enable_release_optimizations(${PROJECT_NAME})

if(ENABLE_BENCHMARK AND TARGET ${PROJECT_NAME}_benchmark)
    enable_release_optimizations(${PROJECT_NAME}_benchmark)
    enable_release_optimizations(benchmark)
endif()

if(IPO_SUPPORTED)
    set_property(TARGET ${LIBRARY_OUTPUT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

    if(ENABLE_BENCHMARK AND TARGET ${PROJECT_NAME}_benchmark)
        set_property(TARGET ${PROJECT_NAME}_benchmark PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
        set_property(TARGET benchmark PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    endif()
endif()

# ============================================================
# short message
# ============================================================

message(STATUS "Project Name     : ${PROJECT_NAME}")
message(STATUS "Project Version  : ${PROJECT_VERSION}")
message(STATUS "C++ Standard     : ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type       : ${CMAKE_BUILD_TYPE}")

if(IPO_SUPPORTED)
    message(STATUS "IPO              : ${IPO_SUPPORTED}")
else()
    message(STATUS "IPO              : ${IPO_SUPPORTED} (${IPO_ERROR})")
endif()