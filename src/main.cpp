#include <math.h>
#include <ncurses.h>
#include <stdio.h>
#include <time.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

#include "ngraphs.h"

#define MARGIN_SIZE 10

float x_to_screen_space(float value, float min, float max, float range) {
  auto value_range = max - min;
  return std::round(((value - min) / value_range * (range - MARGIN_SIZE))) +
         MARGIN_SIZE;
}
float y_to_screen_space(float value, float min, float max, float range) {
  auto value_range = max - min;
  return std::round(((value_range - (value - min)) / value_range *
                     (range - (float)MARGIN_SIZE / 2)));
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
  for (int y = 0; y < window_max_y; y += 3) {
    for (int x = 0; x < window_max_x; x += 2) {
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

void ngraph_line_graph(WINDOW* win, ngraph_point_t* line_data,
                       size_t line_data_len, bool show_zero_x,
                       bool show_zero_y) {
  ngraph_line_graph_max(win, line_data, line_data_len, show_zero_x, show_zero_y,
                        false, 0, false, 0);
}
void ngraph_line_graph_max(WINDOW* win, ngraph_point_t* line_data,
                           size_t line_data_len, bool show_zero_x,
                           bool show_zero_y, bool show_max_x, float graph_max_x,
                           bool show_max_y, float graph_max_y) {
  werase(win);
  if (line_data_len == 0) {
    return;
  }

  float min_x = std::min_element(line_data, line_data + line_data_len,
                                 [](ngraph_point_t& a, ngraph_point_t& b) {
                                   return a.x < b.x;
                                 })
                    ->x;
  float max_x = std::max_element(line_data, line_data + line_data_len,
                                 [](ngraph_point_t& a, ngraph_point_t& b) {
                                   return a.x < b.x;
                                 })
                    ->x;

  float min_y = std::min_element(line_data, line_data + line_data_len,
                                 [](ngraph_point_t& a, ngraph_point_t& b) {
                                   return a.y < b.y;
                                 })
                    ->y;
  float max_y = std::max_element(line_data, line_data + line_data_len,
                                 [](ngraph_point_t& a, ngraph_point_t& b) {
                                   return a.y < b.y;
                                 })
                    ->y;

  if (show_zero_x) {
    min_x = std::min(min_x, 0.0f);
    max_x = std::max(max_x, 0.0f);
  }
  if (show_zero_y) {
    min_y = std::min(min_y, 0.0f);
    max_y = std::max(max_y, 0.0f);
  }
  if (show_max_x) {
    min_x = std::min(min_x, graph_max_x);
    max_x = std::max(max_x, graph_max_x);
  }
  if (show_max_y) {
    min_y = std::min(min_y, graph_max_y);
    max_y = std::max(max_y, graph_max_y);
  }

  int window_max_x = getmaxx(win) * 2;
  int window_max_y = getmaxy(win) * 3;

  size_t map_length = window_max_x * window_max_y;
  std::vector<uint8_t> drawn_map(map_length, 0);

  wattron(win, COLOR_PAIR(1));
  for (auto point_iter = line_data; point_iter != line_data + line_data_len &&
                                    point_iter + 1 != line_data + line_data_len;
       point_iter++) {
    float x_screenspace = x_to_screen_space(point_iter->x, min_x, max_x,
                                            (float)window_max_x / 2) *
                          2;
    float y_screenspace = y_to_screen_space(point_iter->y, min_y, max_y,
                                            (float)window_max_y / 3) *
                          3;

    float next_x_screenspace =
        x_to_screen_space((point_iter + 1)->x, min_x, max_x,
                          (float)window_max_x / 2) *
        2;
    float next_y_screenspace =
        y_to_screen_space((point_iter + 1)->y, min_y, max_y,
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

      size_t my_pos =
          std::clamp((int)std::round(x_cursor), 0, window_max_x - 1) +
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
  ngraph_draw_sextants(win, (bool*)drawn_map.data(), window_max_y,
                       window_max_x);
  wattroff(win, COLOR_PAIR(2));

  wrefresh(win);
}

void ngraph_progress_bar(WINDOW* win, float value, float max,
                         uint16_t segment_count) {
  werase(win);

  auto progress = std::min(value / max, 1.0f);
  progress = std::pow(progress, 2);

  int window_max_x = getmaxx(win) * 2;
  int window_max_y = getmaxy(win) * 3;

  size_t map_length = window_max_x * window_max_y;
  std::vector<uint8_t> shaded_map(map_length, 0);

  float segment_size = (((float)window_max_x) / (segment_count + 1) * 2);
  float half_segment_size = segment_size / 2;

  for (int32_t i = 0; i < segment_count; i++) {
    for (int32_t x = i * half_segment_size;
         x < i * half_segment_size + segment_size; x++) {
      for (int32_t y = 0; y < window_max_y; y++) {
        float bottom_y = (x - i * half_segment_size) / (segment_size / 2) - 0.7;
        float shaded_bottom_y = std::pow(
            (progress * ((float)segment_count / 4)) - ((float)i / 4) + 1, 3);
        float top_y = (x - i * half_segment_size) / (segment_size / 2);

        if (bottom_y < 1 - ((float)y / window_max_y) &&
            1 - ((float)y / window_max_y) < top_y) {
          size_t my_pos =
              (int)std::round(x) + (int)std::round(y * window_max_x);
          if (my_pos <= map_length && map_length > 0) {
            if (shaded_bottom_y > 1 - ((float)y / window_max_y)) {
              shaded_map[my_pos] = true;
            }
          }
        }
      }
    }
  }
  wattron(win, COLOR_PAIR(3));
  ngraph_draw_sextants(win, (bool*)shaded_map.data(), window_max_y,
                       window_max_x);
  wattroff(win, COLOR_PAIR(3));

  wrefresh(win);
}

