

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
  #include <io.h>
  #include <windows.h>
  #define IS_TTY() _isatty(_fileno(stdout))
#else
  #include <unistd.h>
  #define IS_TTY() isatty(STDOUT_FILENO)
#endif

static const std::vector<std::string> LOGO = {
  ".............................................#...............#..",
  ".............................................#...............#..",
  "...........................................###.......#.......###",
  ".............................................#......###......#..",
  ".....#.........#........#######.....#####..###...##.....##...###",
  "....###.......###.......#..........#.....#...#...##.....##...#..",
  "....#.#......##.#.......#.........##.....##..##.............##..",
  "....#..#.....#...#......#.........#.......#...#.............#...",
  "...##..##...#....#......#.........#.......#...#.............#...",
  "...#....#...#....##.....#####.....#.......#...#....#####....#...",
  "...#.....###......#.....#.........#.......#...#...#.....#...#...",
  "...#..............#.....#.........#.......#....#..#.....#..#....",
  "...#..............#.....#.........#.......#....#..#.....#..#....",
  "..##..............##....#.........##.....##....#.#.......#.#....",
  "###...##......##...###..#..........#.....#.....###.......###....",
  "..#...##.###..##...#....#######.....#####.......#.........#.....",
  "###.......#........###..........................................",
  "..#................#............................................",
  "..#................#............................................",
};

// Text shown above and below the logo (edit freely)
static const char* TITLE = "WELCOME TO MEOW CLI";
static const char* DESCRIPTION =
  "SmartFile can manage your files and process student assignments from your "
  "terminal. Organize, validate, test and analyze with simple commands.";

// 256-color palette
static const int BLUE = 75, PURPLE = 177, SHADOW = 240;

// Which color a lit pixel gets, by column: M | E O | W
static int colorForColumn(int x) {
  if (x < 23) return PURPLE;   // M (cat face)
  if (x < 44) return BLUE;     // E, O
  return PURPLE;               // W (upside-down cat face)
}

// Word-wrap plain ASCII text to at most `width` columns.
static std::vector<std::string> wrap(const std::string& text, size_t width) {
  std::vector<std::string> lines;
  std::string line, word;
  std::istringstream in(text);
  while (in >> word) {
    if (!line.empty() && line.size() + 1 + word.size() > width) {
      lines.push_back(line);
      line.clear();
    }
    if (!line.empty()) line += ' ';
    line += word;
  }
  if (!line.empty()) lines.push_back(line);
  return lines;
}

static void render(bool color, bool shadow, bool wide) {
  int h = (int)LOGO.size(), w = (int)LOGO[0].size();
  int H = h + (shadow ? 1 : 0), W = w + (shadow ? 1 : 0);
  if (!wide && H % 2) ++H;  // half-blocks consume two pixel rows per text line

  // lit[y][x] = color code of a lit pixel (-1 = none); sh[y][x] = shadow here
  std::vector<std::vector<int>>  lit(H, std::vector<int>(W, -1));
  std::vector<std::vector<bool>> sh(H, std::vector<bool>(W, false));
  for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x)
      if (LOGO[y][x] == '#') {
        lit[y][x] = colorForColumn(x);
        if (shadow) sh[y + 1][x + 1] = true;  // same shape, shifted down-right
      }

  auto fg    = [&](int c) { if (color) std::cout << "\x1b[38;5;" << c << "m"; };
  auto reset = [&]()      { if (color) std::cout << "\x1b[0m"; };

  if (wide) {  // 1 pixel = 2 columns: ██ for the logo, ░░ for the shadow
    for (int y = 0; y < H; ++y) {
      for (int x = 0; x < W; ++x) {
        if (lit[y][x] >= 0)  { fg(lit[y][x]); std::cout << "██"; reset(); }
        else if (sh[y][x])   { fg(SHADOW);    std::cout << "░░"; reset(); }
        else                   std::cout << "  ";
      }
      std::cout << "\n";
    }
    return;
  }

  // compact: 1 pixel = 1 column, two pixel rows per text line (▀ ▄ █)
  for (int y = 0; y < H; y += 2) {
    for (int x = 0; x < W; ++x) {
      int t = lit[y][x], b = lit[y + 1][x];
      if (t < 0 && b < 0) {  // nothing lit in this cell: shadow or blank
        if (sh[y][x] || sh[y + 1][x]) { fg(SHADOW); std::cout << "░"; reset(); }
        else std::cout << ' ';
        continue;
      }
      if (!color) { std::cout << (t >= 0 && b >= 0 ? "█" : t >= 0 ? "▀" : "▄"); continue; }
      if (t == b)      std::cout << "\x1b[38;5;" << t << "m█";
      else if (b < 0)  std::cout << "\x1b[38;5;" << t << "m▀";
      else if (t < 0)  std::cout << "\x1b[38;5;" << b << "m▄";
      else             std::cout << "\x1b[38;5;" << t << ";48;5;" << b << "m▀";
      reset();
    }
    std::cout << "\n";
  }
}

int main(int argc, char** argv) {
  bool color = IS_TTY(), shadow = true, wide = false;
  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    if (a == "--no-color")   color = false;
    else if (a == "--color")     color = true;
    else if (a == "--shadow")    shadow = true;
    else if (a == "--no-shadow") shadow = false;
    else if (a == "--wide")   wide = true;
  }

#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD mode = 0;
  if (GetConsoleMode(h, &mode)) SetConsoleMode(h, mode | 0x0004);  // enable ANSI colors
#endif

  // width of the rendered logo in terminal columns, so the text lines up with it
  int cols = (int)LOGO[0].size() + (shadow ? 1 : 0);
  if (wide) cols *= 2;

  std::cout << "\n";
  if (color) std::cout << "\x1b[1;97m";           // bold white
  std::cout << TITLE;
  if (color) std::cout << "\x1b[0m";
  std::cout << "\n\n";

  render(color, shadow, wide);

  std::cout << "\n";
  if (color) std::cout << "\x1b[38;5;250m";       // light gray
  for (const std::string& line : wrap(DESCRIPTION, cols)) std::cout << line << "\n";
  if (color) std::cout << "\x1b[0m";
  std::cout << "\n";
  return 0;
}
