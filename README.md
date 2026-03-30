# ngraphs
A C / C++ library for graphing and displaying indicators in ncurses.

![](screenshot.png)


## Demo

Curently the demo is only prebuilt for x86-64 platforms so if thats you:

Go to [Releases](https://github.com/FoxMoss/libngraphs/releases).

Download the latest `ngraphs-demo-x86-64` file.

Mark as an excutable.
```
chmod +x ngraphs-demo-x86-64
```

Then run:

```
./ngraphs-demo-x86-64
```

## Usage

Recomended intergration is with CMake FetchContent, to add to your project:

```cmake
FetchContent_Declare(
    ngraphs
    URL https://github.com/FoxMoss/libngraphs/archive/refs/tags/v0.0.2.tar.gz
)
FetchContent_MakeAvailable(ngraphs)
```

For time travelers, replace v0.0.2 with whatever the latest release is.

Now you can just link ngraphs like you would a normal library.

```cmake
target_link_libraries(program PRIVATE ngraphs ncurses)
```

In a program ngraphs works by rendering to an ncurses window defined by a user

```c
// ... other setup code

WINDOW *graph_win = newwin(100, 100, 5, 5);
WINDOW *progress_win = newwin(100, 100, 105, 5);

struct ngraph_point_t line_data[5] = {{1,1}, {2,2}, {3,3}, {4,4}, {5,5}};
size_t line_data_len = 5;

// ... in your rendering loop

ngraph_line_graph(graph_win, line_data, line_data_len, false, true);

// 50% progress
ngraph_progress_bar(progress_win, progress, 0.5, 20);
```

For a full example check [demo/main.c](demo/main.c)
