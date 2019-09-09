#include "Maze.h"

using namespace MazeLib;

const std::vector<std::string> mazeData_MM2018HX = {
    "636aaaaaaaa36aaaaa2aaaaaaaaaaaa3", "55562aaaaa3dcaaaaa8aaaaaaaaaaa35",
    "555556222356aaaaaaaaa36aaaaa3615", "5c15540001556aaaaaaa3556aaa21555",
    "56955400015556aaaaa355556a215555", "5c35540001555562223555555f555555",
    "56015c8889415540001554888a955555", "5555caaaaa95554000155d6aaaa95555",
    "555caaaaaabd554000154a8a3eaa8815", "55caa363636295c8889556aa8aaaa355",
    "5576a954141569eaaaa1556aaaaa2155", "5555635555dd56aaaaa9d55622235555",
    "554141555cab556aaaaa355400015555", "5555c955caab5556aaa3555400015555",
    "555caa9caaab55556a35555400015555", "5556aaaaaaaa15555f55555c88895555",
    "5c956aaaaaaa9555ca81554aaaaa95d5", "563556aaaaabf49caab555caaa2aa835",
    "415555622236216aaaa95de2a3ca3755", "555555400014014aaaaa9ea835635555",
    "55555540001c895776aaaaa355415555", "555415400016375d4162223555415555",
    "555555c8889555c29d40001555c95555", "555dc8a2aaa954a96340001555e29555",
    "5556aaa8aaaa956b554000155568a955", "555caaaaaaa2355f55c8889555ca2a95",
    "55563776237555cb5caaaaa955620ab5", "5555541401555cabca2aaaaa94154ab5",
    "55d5555c8955caaaaa96aaaaa955cab5", "483555caaa9caaaaaaa97eaaaa956221",
    "43c948aaaaaaaaaaaa3e8aaaaaa95dd5", "dcaa8aaaaaaaaaaaaa8aaaaaaaaa8aa9",
};

const std::vector<std::string> mazeData_MM2016CX = {
    "a6666663ba627a63", "c666663c01a43c39", "a2623b879847c399",
    "9c25c05b85e23999", "9a43a5b85e219999", "9c385b85e25d9999",
    "9e05b85e25a39999", "9a5b85ba1a599999", "99b85b84587c5999",
    "9c05b85a20666599", "c3db85a5d9bbbb99", "b87847c639800059",
    "85e466665c5dddb9", "8666666666666645", "c666666666666663",
    "e666666666666665",
};

int main(void) {
  std::cout << "Maze File Generator" << std::endl;
#if 0
  /* parameter */
  const auto maze_data = mazeData_MM2017HX_pre;
  const int maze_size = 16;
  const std::string output_filename = "output.maze";
  const Positions goals = {
      //   Position(7, 7), Position(8, 7), Position(7, 8), Position(8, 8),
      //   Position(9, 9), Position(10, 9), Position(9, 10), Position(10, 10),
      //   Position(5, 6), Position(6, 6), Position(5, 7), Position(6, 7),
      //   Position(12, 12), Position(13, 12), Position(12, 13), Position(13,
      //   13),
      Position(4, 4),
      Position(5, 4),
      Position(4, 5),
      Position(5, 5),
  };
  /* process */
  Maze sample;
  std::ofstream of(output_filename);
  sample.parse(maze_data, maze_size);
  sample.setGoals(goals);
  sample.print(of, maze_size);
  sample.print(std::cout, maze_size);
#endif

#if 0
  const std::string mazedata_dir =
      "/home/kerikun11/Dropbox/Projects/MicroMouse/Github/micromouseonline/"
      "mazefiles/classic/";
  for (int y = 2015; y >= 2014; --y) {
    const std::string filename = "japan" + std::to_string(y) + "ef.txt";
    Maze maze = Maze((mazedata_dir + filename).c_str());
    maze.print(std::cout, maze.getMaxX() + 1);
    std::ofstream of("16MM" + std::to_string(y) + "CX.maze");
    maze.print(of, maze.getMaxX() + 1);
  }
#endif

#if 1
  /* Preparation */
  const std::string mazedata_dir =
      "/home/kerikun11/Dropbox/Projects/MicroMouse/Github/micromouseonline/"
      "mazefiles/classic/";
  const std::string filename = "alljapan-033-2012-exp-fin.txt";
  Maze maze = Maze((mazedata_dir + filename).c_str());
  maze.print(std::cout, maze.getMaxX() + 1);
  std::ofstream of(filename);
  maze.print(of, maze.getMaxX() + 1);
#endif
  return 0;
}
