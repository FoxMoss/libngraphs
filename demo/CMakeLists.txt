add_executable(ngraph-demo
 main.c
)

target_compile_options(ngraph-demo PRIVATE -O3 -Wextra -Wall -Wpedantic -std=c23)

target_link_libraries(ngraph-demo ngraphs ncurses)

add_custom_command(TARGET ngraph-demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Add clangd to the base directory")
