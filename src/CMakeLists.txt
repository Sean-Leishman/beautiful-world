set(LIBRARY_NAME raytracer_lib)

file(GLOB SOURCES "*.cpp")

add_library(${LIBRARY_NAME} ${SOURCES})
message(STATUS "SOURCES=${SOURCES}")
target_include_directories(${LIBRARY_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include")

target_compile_features(raytracer_lib PUBLIC cxx_std_20)
target_link_libraries(${LIBRARY_NAME} PUBLIC OpenMP::OpenMP_CXX)

source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})
