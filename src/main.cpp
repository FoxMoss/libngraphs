#include <math.h>
#include <ncurses.h>
#include <stdio.h>
#include <time.h>

#include <algorithm>
#include <chrono>
#include <clocale>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#define MARGIN_SIZE 10

float x_to_screen_space(float value, float min, float max, float range) {
  auto value_range = max - min;
  return std::round(((value - min) / value_range * (range - MARGIN_SIZE))) +
         MARGIN_SIZE;
}
float y_to_screen_space(float value, float min, float max, float range) {
  auto value_range = max - min;
  return std::round(((value_range - (value - min)) / value_range *
                     (range - MARGIN_SIZE / 2)));
}

void ngraph_draw_vertical_ticks(WINDOW* win, float min_y, float max_y,
                                float min_x, float max_x, float window_max_y,
                                float window_max_x, bool vertical,
                                bool horizontal) {
  if (vertical) {
    auto y_ticks_divider = std::max(std::roundf(window_max_y / 2), 1.0f);
    float line_increments =
        std::pow(10, std::ceil(std::log10(max_y - min_y))) / y_ticks_divider;
    auto base_line = std::round(min_y / line_increments) * line_increments;

    for (int y = 0; y < y_ticks_divider * 10; y++) {
      auto real_tick = base_line + line_increments * y;
      auto screen_y = y_to_screen_space(real_tick, min_y, max_y, window_max_y);
      for (size_t x = 0; x < window_max_x; x++) {
        mvwaddstr(win, screen_y, x, "-");
      }
      mvwprintw(win, screen_y, 0, "%0.2f", real_tick);
    }
  }
  if (horizontal) {
    auto x_ticks_divider = std::max(std::roundf(window_max_x / 20), 1.0f);
    float line_increments =
        std::pow(10, std::ceil(std::log10(max_x - min_x))) / x_ticks_divider;
    auto base_line = std::round(min_x / line_increments) * line_increments;

    for (size_t x = 0; x < x_ticks_divider * 10; x++) {
      auto real_tick = base_line + line_increments * x;

      auto screen_x =
          std::floor(x_to_screen_space(real_tick, min_x, max_x, window_max_x));
      for (size_t y = 0; y < window_max_y; y++) {
        mvwaddstr(win, y, screen_x, "|");
      }
      mvwprintw(win, window_max_y - 1, screen_x - 3, "%0.2f", real_tick);
    }
  }
}

void ngraph_draw_sextants(WINDOW* win, bool* drawn_map, int window_max_y,
                          int window_max_x) {
  for (size_t y = 0; y < window_max_y; y += 3) {
    for (size_t x = 0; x < window_max_x; x += 2) {
      auto first_first = drawn_map[x + y * window_max_x];
      auto second_first = drawn_map[x + 1 + y * window_max_x];
      auto first_second = drawn_map[x + (y + 1) * window_max_x];
      auto second_second = drawn_map[x + 1 + (y + 1) * window_max_x];
      auto first_third = drawn_map[x + (y + 2) * window_max_x];
      auto second_third = drawn_map[x + 1 + (y + 2) * window_max_x];

      std::string blocks[] = {
          ".", "🬀", "🬁", "🬂", "🬃", "🬄", "🬅", "🬆", "🬇", "🬈", "🬉", "🬊", "🬋",
          "🬌", "🬍", "🬎", "🬏", "🬐", "🬑", "🬒", "🬓", "▌", "🬔", "🬕", "🬖", "🬗",
          "🬘", "🬙", "🬚", "🬛", "🬜", "🬝", "🬞", "🬟", "🬠", "🬡", "🬢", "🬣", "🬤",
          "🬥", "🬦", "🬧", "▐", "🬨", "🬩", "🬪", "🬫", "🬬", "🬭", "🬮", "🬯", "🬰",
          "🬱", "🬲", "🬳", "🬴", "🬵", "🬶", "🬷", "🬸", "🬹", "🬺", "🬻", "█", " "};
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
        mvwaddstr(win, y / 3, x / 2, blocks[index].c_str());
      }
    }
  }
}

void ngraph_line_graph(WINDOW* win,
                       std::vector<std::pair<float, float>>& line_data,
                       bool show_zero_x, bool show_zero_y) {
  werase(win);
  if (line_data.empty()) {
    return;
  }

  float min_x = std::min_element(
                    line_data.begin(), line_data.end(),
                    [](std::pair<float, float>& a, std::pair<float, float>& b) {
                      return a.first < b.first;
                    })
                    ->first;
  float max_x = std::max_element(
                    line_data.begin(), line_data.end(),
                    [](std::pair<float, float>& a, std::pair<float, float>& b) {
                      return a.first < b.first;
                    })
                    ->first;

  float min_y = std::min_element(
                    line_data.begin(), line_data.end(),
                    [](std::pair<float, float>& a, std::pair<float, float>& b) {
                      return a.second < b.second;
                    })
                    ->second;
  float max_y = std::max_element(
                    line_data.begin(), line_data.end(),
                    [](std::pair<float, float>& a, std::pair<float, float>& b) {
                      return a.second < b.second;
                    })
                    ->second;

  if (show_zero_x) {
    min_x = std::min(min_x, 0.0f);
    max_x = std::max(max_x, 0.0f);
  }
  if (show_zero_y) {
    min_y = std::min(min_y, 0.0f);
    max_y = std::max(max_y, 0.0f);
  }

  int window_max_x = getmaxx(win) * 2;
  int window_max_y = getmaxy(win) * 3;

  size_t map_length = window_max_x * window_max_y;
  bool drawn_map[map_length];
  memset(drawn_map, 0, map_length);

  float x_last = 0;
  float y_last = 0;

  wattron(win, COLOR_PAIR(1));
  for (auto point_iter = line_data.begin();
       point_iter != line_data.end() && point_iter + 1 != line_data.end();
       point_iter++) {
    float x_screenspace = x_to_screen_space(point_iter->first, min_x, max_x,
                                            (float)window_max_x / 2) *
                          2;
    float y_screenspace = y_to_screen_space(point_iter->second, min_y, max_y,
                                            (float)window_max_y / 3) *
                          3;

    float next_x_screenspace =
        x_to_screen_space((point_iter + 1)->first, min_x, max_x,
                          (float)window_max_x / 2) *
        2;
    float next_y_screenspace =
        y_to_screen_space((point_iter + 1)->second, min_y, max_y,
                          (float)window_max_y / 3) *
        3;

    auto x_diff = (next_x_screenspace - x_screenspace);
    auto y_diff = (next_y_screenspace - y_screenspace);

    auto x_cursor = x_screenspace;
    auto y_cursor = y_screenspace;

    do {
      double x_prog = 1 - ((next_x_screenspace - x_cursor) / x_diff);
      double y_prog = 1 - ((next_y_screenspace - y_cursor) / y_diff);
      if (x_diff == 0.0) {
        x_prog = 1.0;
      }
      if (y_diff == 0.0) {
        y_prog = 1.0;
      }

      if (x_prog < y_prog) {
        x_cursor += x_diff / fabs(x_diff);
      } else {
        y_cursor += y_diff / fabs(y_diff);
      }
      if (x_prog >= 1.0 && y_prog >= 1.0) {
        break;
      }

      auto my_pos = std::clamp((int)std::round(x_cursor), 0, window_max_x - 1) +
                    (int)std::round(y_cursor * window_max_x);
      if (my_pos <= map_length && map_length > 0) {
        drawn_map[my_pos] = true;
      }
    } while (true);
  }

  ngraph_draw_vertical_ticks(win, min_y, max_y, min_x, max_x,
                             ((float)window_max_y) / 3,
                             ((float)window_max_x) / 2, true, true);

  wattron(win, COLOR_PAIR(2));
  ngraph_draw_sextants(win, drawn_map, window_max_y, window_max_x);
  wattroff(win, COLOR_PAIR(2));

  wrefresh(win);
}

void ngraph_draw_loading_bar(WINDOW* win, float value, float max,
                             uint16_t segment_count) {
  werase(win);

  auto progress = std::min(value / max, 1.0f);

  int window_max_x = getmaxx(win) * 2;
  int window_max_y = getmaxy(win) * 3;

  size_t map_length = window_max_x * window_max_y;
  bool shaded_map[map_length];
  memset(shaded_map, 0, map_length);

  float segment_size = (((float)window_max_x) / (segment_count + 1) * 2);
  float half_segment_size = segment_size / 2;

  for (int32_t i = 0; i < segment_count; i++) {
    for (int32_t x = i * half_segment_size;
         x < i * half_segment_size + segment_size; x++) {
      for (int32_t y = 0; y < window_max_y; y++) {
        float bottom_y = (x - i * half_segment_size) / (segment_size / 2) - 0.7;
        float shaded_bottom_y =
            std::pow((value * ((float)segment_count / 4)) - ((float)i / 4) + 1, 3);
        float top_y = (x - i * half_segment_size) / (segment_size / 2);

        if (bottom_y < 1 - ((float)y / window_max_y) &&
            1 - ((float)y / window_max_y) < top_y) {
          auto my_pos = (int)std::round(x) + (int)std::round(y * window_max_x);
          if (my_pos <= map_length && map_length > 0) {
            if (shaded_bottom_y > 1 - ((float)y / window_max_y)) {
              shaded_map[my_pos] = true;
            }
          }
        }
      }
    }
  }
  //
  // wattron(win, COLOR_PAIR(2));
  // ngraph_draw_sextants(win, drawn_map, window_max_y, window_max_x);
  // wattroff(win, COLOR_PAIR(2));
  //
  wattron(win, COLOR_PAIR(3));
  ngraph_draw_sextants(win, shaded_map, window_max_y, window_max_x);
  wattroff(win, COLOR_PAIR(3));

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

  auto graph_win = newwin(window_max_y / 2 - 5, window_max_x - 20, 5, 10);
  auto progress_win =
      newwin(window_max_y / 8, window_max_x - 20, window_max_y / 8 * 7, 10);

  std::vector<std::pair<float, float>> line_data;
  float value = 0;
  float i = 0;
  for (i = 0; i < 200; i += 1) {
    line_data.push_back({i, value});
    value += (float)(rand() % 5 - 2) / 100;
  }

  std::vector<std::pair<float, float>> new_line_data;

  init_color(COLOR_BLACK, 0, 0, 0);
  init_color(COLOR_YELLOW, 925, 375, 0);
  init_color(COLOR_RED, 976, 242, 0);
  init_pair(2, COLOR_YELLOW, COLOR_BLACK);
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(3, COLOR_RED, COLOR_BLACK);

  float progress = 0;

  int ch;
  do {
    window_max_x = getmaxx(stdscr);
    window_max_y = getmaxy(stdscr);
    wresize(graph_win, (window_max_y / 8) * 7 - 5, window_max_x - 20);
    wresize(progress_win, (window_max_y / 8), window_max_x - 20);
    refresh();
    box(graph_win, 0, 0);
    ngraph_line_graph(graph_win, line_data, false, true);

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

    ngraph_draw_loading_bar(progress_win, progress, 1.0, 20);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
  } while (true);

  delwin(graph_win);
  endwin();

  return 0;
}
