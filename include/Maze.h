/**
 * @file Maze.h
 * @brief マイクロマウスの迷路クラスを定義
 * @author KERI (Github: kerikun11)
 * @url https://kerikeri.top/
 * @date 2017.10.30
 */
#pragma once

#include <array>
#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#include <iomanip> /*< for std::setw() */

namespace MazeLib {
/**
 *  @brief 迷路の1辺の区画数の定数．2の累乗でなければならない
 */
static constexpr int MAZE_SIZE = 32;
static constexpr int MAZE_SIZE_BIT = std::log2(MAZE_SIZE);

/**
 *  @brief 迷路のカラー表示切替
 */
#if 1
#define C_RED "\x1b[31m"
#define C_GREEN "\x1b[32m"
#define C_YELLOW "\x1b[33m"
#define C_BLUE "\x1b[34m"
#define C_MAGENTA "\x1b[35m"
#define C_CYAN "\x1b[36m"
#define C_RESET "\x1b[0m"
#else
#define C_RED ""
#define C_GREEN ""
#define C_YELLOW ""
#define C_BLUE ""
#define C_MAGENTA ""
#define C_CYAN ""
#define C_RESET ""
#endif
#define ESC_UP(n) "\x1b[" #n "A" //< カーソルの移動；マクロなので定数のみ

/**
 * @brief ログ
 */
#ifndef loge
#define loge (std::cerr << "[E][" << __FILE__ << ":" << __LINE__ << "] ")
#endif
#ifndef logw
#define logw (std::cerr << "[W][" << __FILE__ << ":" << __LINE__ << "] ")
#endif
#ifndef logi
#define logi (std::cout << "[I][" << __FILE__ << ":" << __LINE__ << "] ")
#endif

/**
 * @brief 迷路上の方向を定義
 * 実体は 8bit の整数
 */
struct Dir {
public:
  /**
   *  @brief 絶対方向の列挙型
   */
  enum AbsoluteDir : int8_t {
    East,
    NorthEast,
    North,
    NorthWest,
    West,
    SouthWest,
    South,
    SouthEast,
  };
  /**
   *  @brief 相対方向の列挙型
   */
  enum RelativeDir : int8_t {
    Front,
    Left45,
    Left,
    Left135,
    Back,
    Right135,
    Right,
    Right45,
  };
  static constexpr int8_t Max = 8;
  /**
   *  @param d Absolute Direction
   */
  Dir(const enum AbsoluteDir d = East) : d(d) {}
  Dir(const int8_t d) : d(d & 7) {} /**< @brief 定義範囲内に直す */
  /** @brief 整数へのキャスト */
  operator int8_t() const { return d; }
  const Dir &operator=(const Dir &obj) { return d = obj.d, *this; }
  char toChar() const { return ">'^`<,v.x"[d]; }
  bool isAlong() const { return (d & 1) == 0; }
  bool isDiag() const { return (d & 1) == 1; }
  /**
   *  @brief 方向配列を生成する静的関数
   */
  static const std::array<Dir, 4> &ENWS() {
    static const std::array<Dir, 4> all = {East, North, West, South};
    return all;
  }
  static const std::array<Dir, 4> &Diag4() {
    static const std::array<Dir, 4> all = {NorthEast, NorthWest, SouthWest,
                                           SouthEast};
    return all;
  }
  /**
   * @brief 表示
   */
  friend std::ostream &operator<<(std::ostream &os, const Dir d) {
    return os << d.toChar();
  }

private:
  int8_t d; /**< @brief 方向の実体 */
};
static_assert(sizeof(Dir) == 1, "sizeof(Dir) Error"); /**< Size Check */
/**
 *  @brief Dir構造体の動的配列
 */
using Dirs = std::vector<Dir>;

/**
 * @brief 迷路上の座標を定義．左下の区画が (0,0) の (x,y) 平面
 * 実体は 16bit の整数
 */
union Vector {
public:
  struct {
    int8_t x; /**< @brief 迷路の区画座標 */
    int8_t y; /**< @brief 迷路の区画座標 */
  };
  uint16_t all; /**< @brief まとめて扱うとき用 */
  Vector(int8_t x, int8_t y) : x(x), y(y) {}
  Vector() : all(0) {}
  /**
   * @brief 演算子のオーバーロード
   */
  const Vector operator+(const Vector &obj) const {
    return Vector(x + obj.x, y + obj.y);
  }
  const Vector operator-(const Vector &obj) const {
    return Vector(x - obj.x, y - obj.y);
  }
  bool operator==(const Vector &obj) const { return all == obj.all; }
  bool operator!=(const Vector &obj) const { return all != obj.all; }
  /** @function next
   *  @brief 自分の引数方向に隣接した区画のVectorを返す
   *  @param 隣接方向
   *  @return 隣接座標
   */
  const Vector next(const Dir d) const {
    switch (d) {
    case Dir::East:
      return Vector(x + 1, y);
    case Dir::North:
      return Vector(x, y + 1);
    case Dir::West:
      return Vector(x - 1, y);
    case Dir::South:
      return Vector(x, y - 1);
    }
    assert(1); /*< invalid direction */
    return *this;
  }
  /**
   * @brief フィールド外かどうかを判定する関数
   * @return true フィールド外
   * @return false フィールド内
   */
  bool isOutsideofField() const {
    // return x < 0 || x >= MAZE_SIZE || y < 0 || y >= MAZE_SIZE;
    /* 高速化; MAZE_SIZE が2の累乗であることを使用 */
    return ((x | y) & (0x100 - MAZE_SIZE));
  }
  /**
   * @brief 座標を回転変換する
   * @param d 回転角度
   * @return const Vector
   */
  const Vector rotate(const Dir d) const;
  const Vector rotate(const Dir d, const Vector &center) const {
    return center + (*this - center).rotate(d);
  }
  /**
   * @brief 表示
   */
  friend std::ostream &operator<<(std::ostream &os, const Vector v);
};
static_assert(sizeof(Vector) == 2, "sizeof(Vector) Error"); /**< Size Check */
/**
 * @brief Vector構造体の動的配列
 */
using Vectors = std::vector<Vector>;

/**
 * @brief VecDir
 */
using VecDir = std::pair<Vector, Dir>;
/**
 * @brief Vector と Dir を同時に表示
 */
std::ostream &operator<<(std::ostream &os, const VecDir &obj);

/**
 * @brief 区画ベースではなく，壁ベースの管理ID
 */
union __attribute__((__packed__)) WallIndex {
  /**
   * @brief 壁をuniqueな通し番号として表現したときの総数
   */
  static constexpr int SIZE = MAZE_SIZE * MAZE_SIZE * 2;
  /**
   * @brief インデックスの実体
   */
  struct {
    int8_t x : 7; /**< @brief x coordinate of the cell */
    int8_t y : 7; /**< @brief y coordinate of the cell */
    uint8_t
        z : 1; /**< @brief position assignment in the cell; 0:East,1:North */
  };

public:
  WallIndex(const int8_t x, const int8_t y, const uint8_t z)
      : x(x), y(y), z(z) {}
  WallIndex(const int8_t x, const int8_t y, const Dir d) : x(x), y(y) {
    uniquify(d);
  }
  WallIndex(const Vector v, const Dir d) : x(v.x), y(v.y) { uniquify(d); }
  WallIndex() {}
  /**
   * @brief 迷路中の壁をuniqueな通し番号として表現したID
   *
   * @return uint16_t ID
   */
  operator uint16_t() const {
    return (z << (2 * MAZE_SIZE_BIT)) | (y << MAZE_SIZE_BIT) | x;
  }
  void uniquify(const Dir d) {
    z = (d >> 1) & 1; /*< East,West => 0, North,South => 1 */
    if (d == Dir::West)
      x--;
    if (d == Dir::South)
      y--;
  }
  const Dir getDir() const { return z == 0 ? Dir::East : Dir::North; }
  const Vector getVector() const { return Vector(x, y); }
  friend std::ostream &operator<<(std::ostream &os, const WallIndex i) {
    return os << "( " << std::setw(2) << (int)i.x << ", " << std::setw(2)
              << (int)i.y << ", " << i.getDir().toChar() << ")";
  }
  bool isInsideOfFiled() const {
    // return (x >= 0 && y >= 0 && x < MAZE_SIZE && y < MAZE_SIZE);
    /* 高速化; MAZE_SIZE が2の累乗であることを使用 */
    return !((x | y) & (0x100 - MAZE_SIZE));
  }
  const WallIndex next(const Dir d) const {
    switch (d) {
    case Dir::East:
      return WallIndex(x + 1, y, z);
    case Dir::NorthEast:
      return WallIndex(x + (1 - z), y + z, 1 - z);
    case Dir::North:
      return WallIndex(x, y + 1, z);
    case Dir::NorthWest:
      return WallIndex(x - z, y + z, 1 - z);
    case Dir::West:
      return WallIndex(x - 1, y, z);
    case Dir::SouthWest:
      return WallIndex(x - z, y - (1 - z), 1 - z);
    case Dir::South:
      return WallIndex(x, y - 1, z);
    case Dir::SouthEast:
      return WallIndex(x + (1 - z), y - (1 - z), 1 - z);
    default:
      assert(1); /*< invalid direction */
      return WallIndex(x, y, z);
    }
  }
  const std::array<Dir, 6> getNextDir6() const {
    const auto d = getDir();
    return {d + Dir::Front,   d + Dir::Back,    d + Dir::Left45,
            d + Dir::Right45, d + Dir::Left135, d + Dir::Right135};
  }
  const std::array<Dir, 3> getNextDir3(const Dir d) const {
    return {d + Dir::Front, d + Dir::Left45, d + Dir::Right45};
  }
};
static_assert(sizeof(WallIndex) == 2,
              "sizeof(WallIndex) Error"); /**< Size Check */
using WallIndexes = std::vector<WallIndex>;

/**
 * @brief 区画位置，方向，壁の有無を保持する構造体
 * 実体は 16bit の整数
 */
union __attribute__((__packed__)) WallLog {
  struct __attribute__((__packed__)) {
    int x : 6;          /**< @brief 区画のx座標 */
    int y : 6;          /**< @brief 区画のx座標 */
    unsigned int d : 3; /**< @brief 方向 */
    unsigned int b : 1; /**< @brief 壁の有無 */
  };
  unsigned int all : 16; /**< @brief 全フラグ参照用 */
  WallLog() {}
  WallLog(const Vector v, const Dir d, const bool b)
      : x(v.x), y(v.y), d(d), b(b) {}
  WallLog(const int8_t x, const int8_t y, const Dir d, const bool b)
      : x(x), y(y), d(d), b(b) {}
  operator Vector() const { return Vector(x, y); }
  operator Dir() const { return d; }
  friend std::ostream &operator<<(std::ostream &os, const WallLog &obj);
};
static_assert(sizeof(WallLog) == 2, "WallLog Size Error"); /**< Size Check */
/**
 * @brief WallLog構造体の動的配列
 */
using WallLogs = std::vector<WallLog>;

/**
 * @brief 迷路の壁情報を管理するクラス
 * 実体は，壁情報とスタート位置とゴール位置s
 */
class Maze {
public:
  Maze(const Vector start = Vector(0, 0)) : start(start) { reset(); }
  Maze(const Vectors &goals, const Vector start = Vector(0, 0))
      : goals(goals), start(start) {
    reset();
  }
  /**
   *  @brief ファイル名から迷路をパースするコンストラクタ
   *  @param filename ファイル名
   */
  Maze(const char *filename) { parse(filename); }
  /**
   *  @brief 配列から迷路を読み込むコンストラクタ
   *  @param data 各区画16進表記の文字列配列
   *  例：{"abaf", "1234", "abab", "aaff"}
   */
  Maze(const char data[MAZE_SIZE + 1][MAZE_SIZE + 1],
       const std::array<Dir, 4> bit_to_dir_map = {Dir::East, Dir::North,
                                                  Dir::West, Dir::South});
  /** @function reset
   *  @brief 迷路の初期化．壁を削除し，スタート区画を既知に
   */
  void reset(const bool setStartWall = true);
  /**
   *  @brief 壁の有無を返す
   *  @param v 区画の座標
   *  @param d 壁の方向
   *  @return true: 壁あり，false: 壁なし
   */
  bool isWall(const Vector v, const Dir d) const {
    return isWall(wall, WallIndex(v, d));
  }
  bool isWall(const int8_t x, const int8_t y, const Dir d) const {
    return isWall(wall, WallIndex(x, y, d));
  }
  bool isWall(const WallIndex i) const { return isWall(wall, i); }
  /**
   *  @brief 壁を更新をする
   *  @param v 区画の座標
   *  @param d 壁の方向
   *  @param b 壁の有無 true:壁あり，false:壁なし
   */
  void setWall(const Vector v, const Dir d, const bool b) {
    return setWall(wall, WallIndex(v, d), b);
  }
  void setWall(const int8_t x, const int8_t y, const Dir d, const bool b) {
    return setWall(wall, WallIndex(x, y, d), b);
  }
  void setWall(const WallIndex i, const bool b) { return setWall(wall, i, b); }
  /**
   *  @brief 壁が探索済みかを返す
   *  @param v 区画の座標
   *  @param d 壁の方向
   *  @return true: 探索済み，false: 未探索
   */
  bool isKnown(const Vector v, const Dir d) const {
    return isWall(known, WallIndex(v, d));
  }
  bool isKnown(const int8_t x, const int8_t y, const Dir d) const {
    return isWall(known, WallIndex(x, y, d));
  }
  bool isKnown(const WallIndex i) const { return isWall(known, i); }
  /**
   *  @brief 壁の既知を更新する
   *  @param v 区画の座標
   *  @param d 壁の方向
   *  @param b 壁の未知既知 true:既知，false:未知
   */
  void setKnown(const Vector v, const Dir d, const bool b) {
    return setWall(known, WallIndex(v, d), b);
  }
  void setKnown(const int8_t x, const int8_t y, const Dir d, const bool b) {
    return setWall(known, WallIndex(x, y, d), b);
  }
  void setKnown(const WallIndex i, const bool b) {
    return setWall(known, i, b);
  }
  /**
   *  @brief 通過可能かどうかを返す
   *  @param v 区画の座標
   *  @param d 壁の方向
   *  @return true:既知かつ壁なし，false:それ以外
   */
  bool canGo(const WallIndex i) const { return isKnown(i) && !isWall(i); }
  bool canGo(const Vector v, const Dir d) const {
    return isKnown(v, d) && !isWall(v, d);
  }
  /**
   *  @brief 引数区画の壁の数を返す
   *  @param v 区画の座標
   *  @return 壁の数 0~4
   */
  int8_t wallCount(const Vector v) const;
  /**
   *  @brief 引数区画の未知壁の数を返す
   *  @param v 区画の座標
   *  @return 既知壁の数 0~4
   */
  int8_t unknownCount(const Vector v) const;
  /**
   *  @brief 既知の壁と照らしあわせながら，壁を更新する関数
   *  @param v 区画の座標
   *  @param d 壁の方向
   *  @param b 壁の有無
   *  @return true: 正常に更新された, false: 既知の情報と不一致だった
   */
  bool updateWall(const Vector v, const Dir d, const bool b,
                  const bool pushLog = true);
  /**
   * @brief 直前に更新した壁を見探索状態にリセットする
   * @param num リセットする壁の数
   */
  void resetLastWall(const int num);
  /**
   *  @brief 迷路の表示
   *  @param of output-stream
   */
  void print(std::ostream &os = std::cout) const;
  /**
   *  @brief 迷路の文字列から壁をパースする
   *  @param is input-stream
   */
  bool parse(std::istream &is);
  bool parse(const char *filename) {
    std::ifstream ifs(filename);
    if (ifs.fail())
      return false;
    return parse(ifs);
  }
  /**
   *  @brief パス付の迷路の表示
   *  @param start パスのスタート座標
   *  @param dirs 移動方向の配列
   *  @param of output-stream
   */
  void printPath(std::ostream &os, const Vector start, const Dirs &dirs) const;
  void printPath(const Vector start, const Dirs &dirs) const {
    printPath(std::cout, start, dirs);
  }

  /**
   * @brief Set the Goals object
   */
  void setGoals(const Vectors &goals) { this->goals = goals; }
  /**
   * @brief Set the Start object
   */
  void setStart(const Vector &start) { this->start = start; }
  /**
   * @brief Get the Goals object
   */
  const Vectors &getGoals() const { return goals; }
  /**
   * @brief Get the Start object
   */
  const Vector &getStart() const { return start; }
  /**
   * @brief Get the Wall Logs object
   */
  const WallLogs &getWallLogs() const { return wallLogs; }
  int8_t getMinX() const { return min_x; }
  int8_t getMinY() const { return min_y; }
  int8_t getMaxX() const { return max_x; }
  int8_t getMaxY() const { return max_y; }

private:
  std::bitset<WallIndex::SIZE> wall;  /**< 壁情報 */
  std::bitset<WallIndex::SIZE> known; /**< 既知壁情報 */
  Vectors goals;                      /**< ゴール区画 */
  Vector start;                       /**< スタート区画 */
  WallLogs wallLogs;                  /**< 更新した壁のログ */
  int8_t min_x;
  int8_t min_y;
  int8_t max_x;
  int8_t max_y;

  bool isWall(const std::bitset<WallIndex::SIZE> &wall,
              const WallIndex i) const {
    if (!i.isInsideOfFiled() || (i.z == 0 && i.x == MAZE_SIZE - 1) ||
        (i.z == 1 && i.y == MAZE_SIZE - 1))
      return true;
    return wall[i];
  }
  void setWall(std::bitset<WallIndex::SIZE> &wall, const WallIndex i,
               const bool b) const {
    if (!i.isInsideOfFiled() || (i.z == 0 && i.x == MAZE_SIZE - 1) ||
        (i.z == 1 && i.y == MAZE_SIZE - 1))
      return;
    wall[i] = b;
  }
};
} // namespace MazeLib
