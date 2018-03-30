/**
* @file StepMap.h
* @brief マイクロマウスの迷路のステップマップを扱うクラス
* @author KERI (Github: kerikun11)
* @date 2017.11.05
*/
#pragma once

#include "Maze.h"
#include <complex>

namespace MazeLib {
	/** @def MAZE_STEP_MAX
	*  @brief ステップマップの最大値
	*/
	#define MAZE_STEP_MAX  999
	typedef uint16_t step_t; /**< @brief ステップマップの型 */

	/** @class StepMap
	*  @brief 足立法のためのステップマップを管理するクラス
	*/
	class StepMap{
	public:
		/** @brief コンストラクタ
		*  @param maze 使用する迷路の参照
		*/
		StepMap(Maze& maze) : maze(maze) {
			calcStraightStepTable();
			reset();
		}
		/** @function reset
		*  @brief ステップマップの初期化
		*/
		void reset(const step_t step = MAZE_STEP_MAX){
			for(int8_t y=0; y<MAZE_SIZE; y++)
			for(int8_t x=0; x<MAZE_SIZE; x++)
			setStep(x, y, step); //< ステップをクリア
		}
		/** @function getStep
		*  @param ステップへの参照の取得，書き込み可能
		*  @param v 区画の座標
		*  @return ステップメモリの参照
		*/
		const step_t& getStep(const Vector& v) const { return getStep(v.x, v.y); }
		const step_t& getStep(const int8_t& x, const int8_t& y) const {
			// (x, y) がフィールド内か確認
			if(x<0 || y<0 || x>MAZE_SIZE-1 || y>MAZE_SIZE-1){
				printf("Warning: refered to out of field ------------------------------------------> %2d, %2d\n", x, y);
				static step_t outside;  //< フィールド外のときの戻りメモリ
				outside = MAZE_STEP_MAX; //< フィールド外なら最大ステップとする
				return outside;
			}
			return stepMap[y][x];
		}
		/** @function getStep
		*  @param ステップへの参照の取得，書き込み可能
		*  @param v 区画の座標
		*  @return ステップメモリの参照
		*/
		bool setStep(const Vector& v, const step_t& step) { return setStep(v.x, v.y, step); }
		bool setStep(const int8_t& x, const int8_t& y, const step_t& step) {
			// (x, y) がフィールド内か確認
			if(x<0 || y<0 || x>=MAZE_SIZE || y>=MAZE_SIZE){
				printf("Warning: refered to out of field ------------------------------------------> %2d, %2d\n", x, y);
				return false;
			}
			stepMap[y][x] = step;
			return true;
		}
		/** @function print
		*  @param v ハイライト区画
		*/
		void print(const Vector& v = Vector(-1,-1), const Dir& d = Dir::AbsMax) const {
			printf("\n");
			for(int8_t y=MAZE_SIZE-1; y>=0; y--){
				for(uint8_t x=0; x<MAZE_SIZE; x++)
				printf("+%s" C_RESET, maze.isKnown(x,y,Dir::North) ? (maze.isWall(x,y,Dir::North)?"---":"   ") : C_RED " - ");
				printf("+\n");
				for(uint8_t x=0; x<MAZE_SIZE; x++){
					printf("%s" C_RESET, maze.isKnown(x,y,Dir::West) ? (maze.isWall(x,y,Dir::West)?"|":" ") : C_RED ":");
					if(v==Vector(x, y)){
						printf(" %s%c " C_RESET, C_YELLOW, v==Vector(x,y)?(">^<vX"[d]):' ');
					}else{
						printf("%s%3d" C_RESET, v==Vector(x,y)?C_YELLOW:C_CYAN, stepMap[y][x]);
					}
				}
				printf("%s" C_RESET, maze.isKnown(MAZE_SIZE-1,y,Dir::East) ? (maze.isWall(MAZE_SIZE-1,y,Dir::East)?"|":" ") : C_RED ":");
				printf("\n");
			}
			for(uint8_t x=0; x<MAZE_SIZE; x++)
			printf("+%s" C_RESET, maze.isKnown(x,0,Dir::South) ? (maze.isWall(x,0,Dir::South)?"---":"   ") : C_RED " - ");
			printf("+\n");
		}
		/** @function update
		*  @brief ステップマップの更新
		*  @param dest ステップを0とする区画の配列
		*  @param onlyCanGo true:未知の壁は通貨不可能とする，false:未知の壁はないものとする
		*/
		void update(const std::vector<Vector>& dest, const bool& onlyCanGo = false){
			// 全区画のステップを最大値に設定
			reset();
			// となりの区画のステップが更新されたので更新が必要かもしれない区画のキュー
			std::queue<Vector> q;
			// destに含まれる区画のステップを0とする
			for(const auto& v: dest) {
				setStep(v, 0);
				q.push(v);
			}
			// ステップの更新がなくなるまで更新処理
			while(!q.empty()){
				// 注目する区画を取得
				const Vector& focus = q.front(); q.pop();
				const step_t& focus_step = getStep(focus);
				// 4方向更新がないか調べる
				for(const auto& d: Dir::All()){
				// for(int8_t d=0; d<Dir::AbsMax; d++){
					Vector next = focus;
					for(int i=0; true; i++){
						if(maze.isWall(next, d)) break; //< 壁があったら更新はしない
						if(onlyCanGo && !maze.isKnown(next, d)) break; //< onlyCanGoで未知壁なら更新はしない
						// となりの区画のステップが注目する区画のステップよりも大きければ更新
						next = next.next(d); //< となりの区画のステップを取得
						step_t step = focus_step + straightStepTable[i];
						#if 1
						if(getStep(next) <= step) break; //< これより先，更新されることはない
						setStep(next, step);
						q.push(next); //< 再帰的に更新され得るのでキューにプッシュ
						#else
						if(getStep(next) > step){
						setStep(next, step);
						q.push(next); //< 再帰的に更新され得るのでキューにプッシュ
					}
					#endif
				}
			}
		}
	}
	void calcStraightStepTable(){
		for(int i=0; i<MAZE_SIZE; i++){
			straightStepTable[i] = sqrt((float)i+1) * factor;
		}
	}
public:
	const float factor = 1.0f / (sqrt((float)MAZE_SIZE-1) - sqrt((float)MAZE_SIZE-2));
private:
	Maze& maze; /**< @brief 使用する迷路の参照 */
	step_t stepMap[MAZE_SIZE][MAZE_SIZE]; /**< @brief ステップ数 */
	step_t straightStepTable[MAZE_SIZE];
};
}
