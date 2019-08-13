#ifndef MAZESIMULATOR_H
#define MAZESIMULATOR_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QEventLoop>
#include <QTimer>

#include "Maze.h"
#include "RobotBase.h"

using namespace MazeLib;

/**
 * @brief The MazeSimulator class
 */
class MazeSimulator : public MazeLib::RobotBase
{
public:
    MazeSimulator(QGraphicsScene* scene): RobotBase(maze),scene(scene){}

    void clear(){
        /* set Background Color */
        scene->clear();
        scene->setBackgroundBrush(Qt::black);
        /* Print Line Number */
        int w = wall_unit_px;
        int s = MazeLib::MAZE_SIZE;
        for(int i=0; i<MazeLib::MAZE_SIZE; ++i) {
            scene->addText(QString::number(i))->setPos((i+0.25)*w, s*w);
            scene->addText(QString::number(i))->setPos(-w, (s-i-1)*w);
        }
        /* Print Cell Line */
        //        QPen pen;
        //        pen.setColor(Qt::gray);
        //        pen.setStyle(Qt::DotLine);
        //        for(int i=0; i<MazeLib::MAZE_SIZE + 1; ++i) {
        //            scene->addLine(cell2posX(i), cell2posY(0), cell2posX(i), cell2posY(s), pen);
        //            scene->addLine(cell2posX(0), cell2posY(i), cell2posX(s), cell2posY(i), pen);
        //        }
    }
    void drawMaze(const MazeLib::Maze& maze) {
        /* Print Walls */
        for(int x=0; x<MazeLib::MAZE_SIZE + 1; ++x)
            for(int y=0; y<MazeLib::MAZE_SIZE + 1; ++y) {
                for(const auto d: {MazeLib::Dir::West, MazeLib::Dir::South}){
                    /* skip when it's out of the field */
                    if(x==MazeLib::MAZE_SIZE && d == MazeLib::Dir::South)
                        continue;
                    if(y==MazeLib::MAZE_SIZE && d == MazeLib::Dir::West)
                        continue;
                    /* Draw Wall */
                    QPen pen(Qt::red);
                    if(maze.isKnown(x, y, d)){
                        if(maze.isWall(x, y, d))
                            pen.setStyle(Qt::SolidLine), pen.setWidth(pillar_px);
                        else
                            pen.setStyle(Qt::DotLine), pen.setColor(Qt::gray);
                    } else {
                        if(maze.isWall(x, y, d))
                            pen.setStyle(Qt::DashDotLine);
                        else
                            pen.setStyle(Qt::DotLine);
                    }
                    addWall(scene, MazeLib::VecDir{MazeLib::Vector(x,y), d}, pen);
                }
            }
    }
    void drawVecDir(const VecDir& vd) {
        /* Draw Machine */
        const auto v = vd.first;
        const auto d = vd.second;
//        scene->addRect(cell2posX(v.x), cell2posY(v.y+1), wall_px, wall_px, QPen(Qt::yellow), QBrush(Qt::yellow));
        QPolygon pol;
        pol  << QPoint(0, wall_unit_px/4)  << QPoint(0, -wall_unit_px/4) << QPoint(wall_unit_px/4, 0);
        pol.translate(-QPoint(wall_unit_px/2, 0));
        QMatrix mat;
        mat.rotate(-45*d);
        pol = mat.map(pol);
        pol.translate(QPoint(cell2posX(v.x)+wall_unit_px/2, cell2posY(v.y)-wall_unit_px/2));
        scene->addPolygon(pol, QPen(Qt::yellow), QBrush(Qt::yellow));
    }
    Maze& getMazeTarget() { return maze_target; }
    void setMazeTarget(const Maze& maze) { maze_target = maze; }
    bool endFastRunBackingToStartRun() {
        /* エラー処理 */
        if (getShortestDirs().empty()) {
            logw << "ShortestDirs are empty!" << std::endl;
            return false;
        }
        /* real を最短後の位置に移す */
        Vector v = maze.getStart();
        for (const auto d : getShortestDirs())
            v = v.next(d);
        real = VecDir(v, getShortestDirs().back());
        /* 基底関数を呼ぶ */
        return RobotBase::endFastRunBackingToStartRun();
    }

protected:
    Maze maze;
    Maze maze_target;
    VecDir fake_offset;
    VecDir real;

    virtual void findWall(bool &left, bool &front, bool &right,
                          bool &back) override {
        left = maze_target.isWall(real.first, real.second + Dir::Left);
        front = maze_target.isWall(real.first, real.second + Dir::Front);
        right = maze_target.isWall(real.first, real.second + Dir::Right);
        back = maze_target.isWall(real.first, real.second + Dir::Back);
#if 0
        /* 前1区画先の壁を読める場合 */
        if (!front)
            updateWall(curVec.next(curDir), curDir,
                       maze_target.isWall(real.first.next(real.second), real.second));
#endif
    }
    virtual void calcNextDirsPreCallback() override {
        start = std::chrono::system_clock::now();
    }
    virtual void calcNextDirsPostCallback(SearchAlgorithm::State prevState
                                          __attribute__((unused)),
                                          SearchAlgorithm::State newState
                                          __attribute__((unused))) override {
        end = std::chrono::system_clock::now();
        usec = std::chrono::duration_cast<std::chrono::microseconds>(end - start)
                .count();
        max_usec = std::max(max_usec, usec);
        if (newState == prevState)
            return;
        /* State Change has occurred */
        if (prevState == SearchAlgorithm::IDENTIFYING_POSITION) {
            min_id_wall = std::min(
                        min_id_wall, getSearchAlgorithm().getIdMaze().getWallLogs().size());
            max_id_wall = std::max(
                        max_id_wall, getSearchAlgorithm().getIdMaze().getWallLogs().size());
        }
        if (newState == SearchAlgorithm::IDENTIFYING_POSITION) {
        }
        if (newState == SearchAlgorithm::SEARCHING_ADDITIONALLY) {
        }
        if (newState == SearchAlgorithm::BACKING_TO_START) {
        }
        if (newState == SearchAlgorithm::REACHED_START) {
        }
    }
    virtual void discrepancyWithKnownWall() override {
        if (getState() != SearchAlgorithm::IDENTIFYING_POSITION) {
            printInfo();
            std::cout
                    << "There was a discrepancy with known information! CurVecDir:\t"
                    << VecDir{getCurVec(), getCurDir()} << std::endl;
        }
    }
    virtual void crashed() {
        std::cerr << "The robot crashed into the wall! fake_offset:\t"
                  << fake_offset << "\treal:\t" << real << std::endl;
    }
    virtual void queueAction(const Action action) override {
        clear();
        drawMaze(maze);
        drawVecDir(real);
        msleep(10);
        cost += getTimeCost(action);
        step++;
        switch (action) {
        case RobotBase::START_STEP:
            real.first = Vector(0, 1);
            real.second = Dir::North;
            f++;
            break;
        case RobotBase::START_INIT:
            break;
        case RobotBase::ST_HALF_STOP:
            break;
        case RobotBase::TURN_L:
            real.second = real.second + Dir::Left;
            if (!maze_target.canGo(real.first, real.second))
                crashed();
            real.first = real.first.next(real.second);
            l++;
            break;
        case RobotBase::TURN_R:
            real.second = real.second + Dir::Right;
            if (!maze_target.canGo(real.first, real.second))
                crashed();
            real.first = real.first.next(real.second);
            r++;
            break;
        case RobotBase::ROTATE_180:
            real.second = real.second + Dir::Back;
            if (!maze_target.canGo(real.first, real.second))
                crashed();
            real.first = real.first.next(real.second);
            b++;
            break;
        case RobotBase::ST_FULL:
            if (!maze_target.canGo(real.first, real.second))
                crashed();
            real.first = real.first.next(real.second);
            f++;
            break;
        case RobotBase::ST_HALF:
            break;
        }
    }
    virtual float getTimeCost(const Action action) {
        const float velocity = 240.0f;
        const float segment = 90.0f;
        switch (action) {
        case RobotBase::START_STEP:
            return 1.0f;
        case RobotBase::START_INIT:
            return 3.0f;
        case RobotBase::ST_HALF_STOP:
            return segment / 2 / velocity;
        case RobotBase::TURN_L:
            return 71 / velocity;
        case RobotBase::TURN_R:
            return 71 / velocity;
        case RobotBase::ROTATE_180:
            return 2.0f;
        case RobotBase::ST_FULL:
            return segment / velocity;
        case RobotBase::ST_HALF:
            return segment / 2 / velocity;
        }
        return 0;
    }

private:
    int step = 0, f = 0, l = 0, r = 0, b = 0; /**< 探索の評価のためのカウンタ */
    float cost = 0;
    int max_usec = 0;
    int usec = 0;
    size_t max_id_wall = 0;
    size_t min_id_wall = MAZE_SIZE * MAZE_SIZE * 4;
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;

private:
    QGraphicsScene* scene;
    int wall_unit_px = 28;
    int pillar_px = 2;
    int wall_px = wall_unit_px-pillar_px;

    QGraphicsItem* addWall(QGraphicsScene* scene, MazeLib::VecDir vd, const QPen& pen) {
        int x = vd.first.x;
        int y = vd.first.y;
        MazeLib::Dir d = vd.second;
        switch (d) {
        case MazeLib::Dir::East:
            return scene->addLine(cell2posX(x+1), cell2posY(y)-pillar_px/2, cell2posX(x+1), cell2posY(y)-pillar_px/2-wall_px, pen);
        case MazeLib::Dir::North:
            return scene->addLine(cell2posX(x)+pillar_px/2, cell2posY(y+1), cell2posX(x)+pillar_px/2+wall_px, cell2posY(y+1), pen);
        case MazeLib::Dir::West:
            return scene->addLine(cell2posX(x), cell2posY(y)-pillar_px/2, cell2posX(x), cell2posY(y)-pillar_px/2-wall_px, pen);
        case MazeLib::Dir::South:
            return scene->addLine(cell2posX(x)+pillar_px/2, cell2posY(y), cell2posX(x)+pillar_px/2+wall_px, cell2posY(y), pen);
        default:
            break;
        }
        return nullptr;
    }
    int cell2posX(int x) const {
        int w = wall_unit_px;
        return x*w;
    }
    int cell2posY(int y) const {
        int w = wall_unit_px;
        int s = MazeLib::MAZE_SIZE;
        return (s-y)*w;
    }

    void msleep(int ms) {
        QEventLoop loop;
        QTimer::singleShot(ms, &loop, SLOT(quit()));
        loop.exec();
    }
};

#endif // MAZESIMULATOR_H
