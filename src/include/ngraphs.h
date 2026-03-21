#ifndef NGRAPHS
#define NGRAPHS
#include <ncurses.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ngraph_point_t {
  float x, y;
};

void ngraph_line_graph(WINDOW* win, struct ngraph_point_t* line_data,
                       size_t line_data_len, bool show_zero_x,
                       bool show_zero_y) ;

void ngraph_progress_bar(WINDOW* win, float value, float max,
                         uint16_t segment_count);

#ifdef __cplusplus
}
#endif
#endif  // !NGRAPHS
