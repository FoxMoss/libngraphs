add_library(ngraph
 main.cpp
)

target_compile_options(ngraph PRIVATE -O3 -Wextra -Wall -Wpedantic -std=c++20)

target_include_directories(ngraph PUBLIC include)

target_link_libraries(ngraph)

add_custom_command(TARGET ngraph POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Add clangd to the base directory")

install(TARGETS ngraph DESTINATION lib)
install(FILES include/ngraph.h DESTINATION include)
