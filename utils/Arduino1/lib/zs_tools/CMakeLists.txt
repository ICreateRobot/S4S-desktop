cmake_minimum_required(VERSION 3.10)

set(PROJ_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE SRC_FILES "${PROJ_ROOT}/src/*.c")

# 排除目录
set(EXCLUDE_PATTERNS 
    "${PROJ_ROOT}/src/component/cc/"   # 注意末尾斜杠表示目录
)

foreach(PATTERN ${EXCLUDE_PATTERNS})
    string(REGEX REPLACE "([.+*?^$()\\[\\]{}|])" "\\\\\\1" PATTERN_ESC "${PATTERN}")
    list(FILTER SRC_FILES EXCLUDE REGEX "^${PATTERN_ESC}")
endforeach()

add_subdirectory("src/component/cc")

add_library(zs_tools ${SRC_FILES})
target_link_libraries(zs_tools cc)
