#include <math.h>
#include <ncurses.h>
#include <stdio.h>
#include <time.h>

#include <algorithm>
#include <clocale>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

float x_to_screen_space(float value, float min, float max, float range) {
  return std::round(((value + min) / max * range));
}
float y_to_screen_space(float value, float min, float max, float range) {
  return std::round(((max - (value + min)) / max * range));
}

void ngraph_line_graph(WINDOW* win,
                       std::vector<std::pair<float, float>>& line_data) {
  werase(win);
  if (line_data.empty()) {
    return;
  }
  float min_y = -std::min(std::min_element(line_data.begin(), line_data.end(),
                                           [](std::pair<float, float>& a,
                                              std::pair<float, float>& b) {
                                             return a.second < b.second;
                                           })
                              ->second,
                          0.0f);
  float max_y = std::max_element(
                    line_data.begin(), line_data.end(),
                    [](std::pair<float, float>& a, std::pair<float, float>& b) {
                      return a.second < b.second;
                    })
                    ->second +
                min_y;

  float min_x = -std::min_element(line_data.begin(), line_data.end(),
                                  [](std::pair<float, float>& a,
                                     std::pair<float, float>& b) {
                                    return a.first < b.first;
                                  })
                     ->first;
  float max_x = std::max_element(
                    line_data.begin(), line_data.end(),
                    [](std::pair<float, float>& a, std::pair<float, float>& b) {
                      return a.first < b.first;
                    })
                    ->first +
                min_x;

  int window_max_x = getmaxx(win) * 2;
  int window_max_y = getmaxy(win) * 3;

  size_t map_length = window_max_x * window_max_y;
  bool drawn_map[map_length];
  memset(drawn_map, 0, map_length);

  float x_last = 0;
  float y_last = 0;

  for (auto point_iter = line_data.begin();
       point_iter != line_data.end() && point_iter + 1 != line_data.end();
       point_iter++) {
    float x_screenspace =
        x_to_screen_space(point_iter->first, min_x, max_x, window_max_x);
    float y_screenspace =
        y_to_screen_space(point_iter->second, min_y, max_y, window_max_y);

    float next_x_screenspace =
        x_to_screen_space((point_iter + 1)->first, min_x, max_x, window_max_x);
    float next_y_screenspace =
        y_to_screen_space((point_iter + 1)->second, min_y, max_y, window_max_y);

    auto x_diff = (next_x_screenspace - x_screenspace);
    auto y_diff = (next_y_screenspace - y_screenspace);

    auto x_cursor = x_screenspace;
    auto y_cursor = y_screenspace;

    do {
      if (std::fabs(x_cursor - x_screenspace) <
          std::fabs(next_x_screenspace - x_screenspace)) {
        x_cursor += x_diff / fabs(x_diff);
      } else if (std::fabs(y_cursor - y_screenspace) <
                 std::fabs(next_y_screenspace - y_screenspace)) {
        y_cursor += y_diff / fabs(y_diff);
      } else {
        break;
      }

      auto my_pos = std::clamp((int)std::round(x_cursor), 0, window_max_x - 1) +
                    (int)std::round(y_cursor * window_max_x);
      if (my_pos <= map_length && map_length > 0) {
        drawn_map[my_pos] = true;
      }
    } while (true);
  }

  wattron(win, COLOR_PAIR(1));
  float line_increments =
      std::pow(10, std::floor(std::log10(max_y + min_y))) / 2;
  auto base_line = std::round(-min_y / line_increments) * line_increments;

  for (size_t y = 0; y < 20; y++) {
    auto real_tick = base_line + line_increments * y;
    auto screen_y = y_to_screen_space(real_tick, min_y, max_y, window_max_y);
    for (size_t x = 0; x < window_max_x; x += 2) {
      mvwaddstr(win, screen_y / 3, x / 2, "-");
    }
    mvwprintw(win, screen_y / 3, 0, "%0.2f", real_tick);
  }

  wattron(win, COLOR_PAIR(2));
  for (size_t y = 0; y < window_max_y; y += 3) {
    for (size_t x = 0; x < window_max_x; x += 2) {
      auto first_first = drawn_map[x + y * window_max_x];
      auto second_first = drawn_map[x + 1 + y * window_max_x];
      auto first_second = drawn_map[x + (y + 1) * window_max_x];
      auto second_second = drawn_map[x + 1 + (y + 1) * window_max_x];
      auto first_third = drawn_map[x + (y + 2) * window_max_x];
      auto second_third = drawn_map[x + 1 + (y + 2) * window_max_x];

      char* blocks[] = {".", "🬀", "🬁", "🬂", "🬃", "🬄", "🬅", "🬆", "🬇", "🬈", "🬉",
                        "🬊", "🬋", "🬌", "🬍", "🬎", "🬏", "🬐", "🬑", "🬒", "🬓", "▌",
                        "🬔", "🬕", "🬖", "🬗", "🬘", "🬙", "🬚", "🬛", "🬜", "🬝", "🬞",
                        "🬟", "🬠", "🬡", "🬢", "🬣", "🬤", "🬥", "🬦", "🬧", "▐", "🬨",
                        "🬩", "🬪", "🬫", "🬬", "🬭", "🬮", "🬯", "🬰", "🬱", "🬲", "🬳",
                        "🬴", "🬵", "🬶", "🬷", "🬸", "🬹", "🬺", "🬻", "█", " "};
      uint8_t index = 0;
      if (first_first) {
        index |= 0b00000001;
      }
      if (second_first) {
        index |= 0b00000010;
      }
      if (first_second) {
        index |= 0b00000100;
      }
      if (second_second) {
        index |= 0b00001000;
      }
      if (first_third) {
        index |= 0b00010000;
      }
      if (second_third) {
        index |= 0b00100000;
      }

      if (index <= 64 && index > 0) {
        mvwaddstr(win, y / 3, x / 2, blocks[index]);
      }
    }
  }
  wattroff(win, COLOR_PAIR(2));
  wrefresh(win);
}

int main(int argc, char* argv[]) {
  setlocale(LC_ALL, "en_US.UTF-8");
  initscr();
  cbreak();
  curs_set(0);

  keypad(stdscr, TRUE);

  start_color();

  int window_max_x = getmaxx(stdscr);
  int window_max_y = getmaxy(stdscr);

  auto graph_win = newwin(window_max_y - 10, window_max_x - 20, 5, 10);

  std::vector<std::pair<float, float>> line_data;
  float value = 0;
  uint32_t i = 0;
  for (i = 0; i < 2000; i++) {
    line_data.push_back({i, value});
    value += (float)(rand() % 5 - 2) / 100;
  }

  std::vector<std::pair<float, float>> new_line_data;

  init_color(COLOR_BLACK, 0, 0, 0);
  init_color(COLOR_YELLOW, 925, 375, 0);
  init_color(COLOR_RED, 628, 0, 0);
  init_pair(2, COLOR_YELLOW, COLOR_BLACK);
  init_pair(1, COLOR_RED, COLOR_BLACK);

  int ch;
  do {
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    window_max_x = getmaxx(stdscr);
    window_max_y = getmaxy(stdscr);
    wresize(graph_win, window_max_y - 10, window_max_x - 20);
    refresh();
    box(graph_win, 0, 0);
    ngraph_line_graph(graph_win, line_data);

    new_line_data.clear();
    new_line_data.insert(new_line_data.begin(), line_data.begin() + 1,
                         line_data.end());
    new_line_data.push_back({i, value});
    i++;
    value += (float)(rand() % 5 - 2) / 100;

    line_data = new_line_data;

    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    int64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 +
                       (end.tv_nsec - start.tv_nsec) / 1000;

    struct timespec req;
    req.tv_nsec = std::min(41666 - delta_us, (int64_t)0);
    req.tv_sec = 0;
    clock_nanosleep(CLOCK_MONOTONIC_RAW, 0, &req, NULL);
  } while (true);

  delwin(graph_win);
  endwin();

  return 0;
}
