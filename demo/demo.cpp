// unused but left here as a reference with equivelent code
#include "ngraphs.h"
#include <math.h>
#include <ncurses.h>

#include <chrono>
#include <clocale>
#include <cstdlib>
#include <thread>
#include <vector>


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

  std::vector<ngraph_point_t> line_data;
  float value = 0;
  float i = 0;
  for (i = 0; i < 200; i += 1) {
    line_data.push_back({i, value});
    value += (float)(rand() % 41 - 20) / 100;;
  }

  std::vector<ngraph_point_t> new_line_data;

  init_color(COLOR_BLACK, 0, 0, 0);
  init_color(COLOR_YELLOW, 925, 375, 0);
  init_color(COLOR_RED, 976, 242, 0);
  init_pair(2, COLOR_YELLOW, COLOR_BLACK);
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(3, COLOR_RED, COLOR_BLACK);

  float progress = 0;

  do {
    window_max_x = getmaxx(stdscr);
    window_max_y = getmaxy(stdscr);
    wresize(graph_win, (window_max_y / 8) * 7 - 5, window_max_x - 20);
    wresize(progress_win, (window_max_y / 8), window_max_x - 20);
    refresh();
    box(graph_win, 0, 0);
    ngraph_line_graph(graph_win, line_data.data(), line_data.size(), false, true);

    progress += 0.1;

    if (progress > 1) {
      progress = 0.0f;

      new_line_data.clear();
      new_line_data.insert(new_line_data.begin(), line_data.begin() + 1,
                           line_data.end());

      new_line_data.push_back({i, value});
      value += (float)(rand() % 41 - 20) / 100;
      i += 1;
      line_data = new_line_data;
    }

    ngraph_progress_bar(progress_win, progress, 1.0, 20);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
  } while (true);

  delwin(graph_win);
  endwin();

  return 0;
}
