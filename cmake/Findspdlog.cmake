find_package(PkgConfig)

message(PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
set(SPDLOG_DIR ${PROJECT_SOURCE_DIR}/external/spdlog/include/spdlog)
message(SPDLOG_DIR="${SPDLOG_DIR}")
find_file(spdlog_INCLUDE_DIR spdlog.h
        PATH ${CMAKE_LIBRARY_PATH} ${SPDLOG_DIR} /usr/lib /usr/local/lib
        HINTS spdlog
        )

if(NOT spdlog_INCLUDE_DIR AND IS_DIRECTORY "$ENV{DEVCPP}/libs/${SPDLOG_DIR}")
    set(spdlog_INCLUDE_DIR "$ENV{DEVCPP}/libs/${SPDLOG_DIR}")
endif()

message(STATUS "spdlog-Include Dir:    ${spdlog_INCLUDE_DIR}" )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(spdlog DEFAULT_MSG
        spdlog_INCLUDE_DIR
        )

mark_as_advanced(spdlog_INCLUDE_DIR)

set(spdlog_INCLUDE_DIRS ${spdlog_INCLUDE_DIR} )

include_directories(${spdlog_INCLUDE_DIR})