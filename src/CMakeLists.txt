add_library(ngraphs
 main.cpp
)

target_compile_options(ngraphs PRIVATE -O3 -Wextra -Wall -Wpedantic -std=c++20)

target_include_directories(ngraphs PUBLIC include)

add_custom_command(TARGET ngraphs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Add clangd to the base directory")

install(TARGETS ngraphs DESTINATION lib)
install(FILES include/ngraphs.h DESTINATION include)
