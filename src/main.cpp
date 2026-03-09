#include <ncurses.h>
#include <stdio.h>

#include <algorithm>
#include <clocale>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <unordered_map>
#include <utility>
#include <vector>

int main(int argc, char* argv[]) {
  setlocale(LC_ALL, "en_US.UTF-8");
  initscr();
  cbreak();
  curs_set(0);

  keypad(stdscr, TRUE);

  std::vector<std::pair<float, float>> line_data;

  float value = 0;
  for (uint16_t i = 0; i < 100; i++) {
    line_data.push_back({i, value});
    value += rand() % 10 - 2;
  }

  float max_y = std::max_element(
                    line_data.begin(), line_data.end(),
                    [](std::pair<float, float>& a, std::pair<float, float>& b) {
                      return a.second < b.second;
                    })
                    ->second;

  float max_x = std::max_element(
                    line_data.begin(), line_data.end(),
                    [](std::pair<float, float>& a, std::pair<float, float>& b) {
                      return a.first < b.first;
                    })
                    ->first;

  int ch;
  do {
    clear();
    int window_max_x = getmaxx(stdscr);
    int window_max_y = getmaxy(stdscr) * 2;

    std::unordered_map<size_t, int> drawn_before;

    float x_last = 0;
    float y_last = 0;

    for (auto point_iter = line_data.begin(); point_iter + 1 != line_data.end();
         point_iter++) {
      float x_screenspace =
          std::round((point_iter->first / max_x * window_max_x));
      float y_screenspace =
          std::round((max_y - point_iter->second) / max_y * window_max_y);
      float next_x_screenspace =
          std::round((point_iter + 1)->first / max_x * window_max_x);
      float next_y_screenspace =
          std::round((max_y - (point_iter + 1)->second) / max_y * window_max_y);

      auto biggest_difference =
          std::ceil(std::max(std::fabs(next_x_screenspace - x_screenspace),
                             std::fabs(next_y_screenspace - y_screenspace)));
      auto x_diff = (next_x_screenspace - x_screenspace) / biggest_difference;
      auto y_diff = (next_y_screenspace - y_screenspace) / biggest_difference;
      // auto scale = std::sqrt(std::pow(x_diff, 2) + std::pow(y_diff, 2));
      // x_diff /= scale;
      // y_diff /= scale;

      auto x_cursor = x_screenspace;
      auto y_cursor = y_screenspace;
      float x_potential = 0;
      float y_potential = 0;

      do {
        // if (std::fabs(x_potential) > 0 && std::fabs(y_potential) > 0) {
        if (std::fabs(x_cursor - x_screenspace) <
            std::fabs(next_x_screenspace - x_screenspace)) {
          x_cursor += x_diff / fabs(x_diff);
        } else if (std::fabs(y_cursor - y_screenspace) <
                   std::fabs(next_y_screenspace - y_screenspace)) {
          y_cursor += y_diff / fabs(y_diff);
        }

        char* c;
        double null;
        bool is_up = std::modf(y_cursor / 2, &null) > 0.5;
        bool last_is_up = std::modf(y_last / 2, &null) > 0.5;
        if (drawn_before[(int)round(y_cursor / 2) * window_max_x +
                         (int)round(x_cursor)] != !is_up + 1) {
          c = "█";
        } else if (is_up) {
          c = "▀";
        } else {
          c = "▄";
        }
        drawn_before[(int)round(y_cursor / 2) * window_max_x +
                     (int)round(x_cursor)] = is_up + 1;

        mvaddstr((int)round(y_cursor / 2), (int)round(x_cursor), c);
        x_last = x_cursor;
        y_last = y_cursor;
      } while (std::fabs(x_cursor - x_screenspace) <
                   std::fabs(next_x_screenspace - x_screenspace) ||
               std::fabs(y_cursor - y_screenspace) <
                   std::fabs(next_y_screenspace - y_screenspace));
    }
    refresh();
  } while ((ch = getch()) != 'q');

  // endwin();

  return 0;
}
