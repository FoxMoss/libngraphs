#include <time.h>
#include <locale.h>
#include <ncurses.h>
#include <sched.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ngraphs.h"

int main() {
  setlocale(LC_ALL, "en_US.UTF-8");
  initscr();
  cbreak();
  curs_set(0);

  keypad(stdscr, TRUE);

  start_color();

  int window_max_x = getmaxx(stdscr);
  int window_max_y = getmaxy(stdscr);

  auto graph_win = newwin(window_max_y / 2 - 5, window_max_x - 20, 5, 10);
  auto progress_win =
      newwin(window_max_y / 8, window_max_x - 20, window_max_y / 8 * 7, 10);

  size_t line_data_len = 200;
  struct ngraph_point_t* line_data = (struct ngraph_point_t*)malloc(
      line_data_len * sizeof(struct ngraph_point_t));
  struct ngraph_point_t* back_buffer= (struct ngraph_point_t*)malloc(
      line_data_len * sizeof(struct ngraph_point_t));
  float value = 0;
  size_t i = 0;
  for (; i < line_data_len; i += 1) {
    line_data[i] = (struct ngraph_point_t){.x = (float)i, .y = value};

    value += (float)(rand() % 41 - 20) / 100;
  }

  init_color(COLOR_BLACK, 0, 0, 0);
  init_color(COLOR_YELLOW, 925, 375, 0);
  init_color(COLOR_RED, 976, 242, 0);
  init_pair(2, COLOR_YELLOW, COLOR_BLACK);
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(3, COLOR_RED, COLOR_BLACK);

  float progress = 0;
  long start = clock();
  const long goal = CLOCKS_PER_SEC*1;


  do {
    sched_yield();

    window_max_x = getmaxx(stdscr);
    window_max_y = getmaxy(stdscr);
    wresize(graph_win, (window_max_y / 8) * 7 - 5, window_max_x - 20);
    wresize(progress_win, (window_max_y / 8), window_max_x - 20);
    mvwin(progress_win, window_max_y / 8 * 7, 10);

    refresh();
    box(graph_win, 0, 0);
    ngraph_line_graph(graph_win, line_data, line_data_len, false, true);

    progress = (float)(clock()-start)/goal;

    if (progress > 1) {
      progress = 0.0f;
      start = clock();

      memcpy(back_buffer, line_data + 1,
             (line_data_len - 1) * sizeof(struct ngraph_point_t));
      memcpy(line_data, back_buffer,
             line_data_len * sizeof(struct ngraph_point_t));
      line_data[line_data_len - 1] =
          (struct ngraph_point_t){.x = (float)i, .y = value};
      value += (float)(rand() % 41 - 20) / 100;
      i += 1;
    }

    ngraph_progress_bar(progress_win, progress, 1.0, 20);
  } while (true);

  delwin(graph_win);
  endwin();

  return 0;
}
