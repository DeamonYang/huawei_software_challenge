#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#define DEBUG

#include <vector>
#include <queue>
#include <algorithm>
#include <map>
#include <list>
#include <cmath>

// #ifdef DEBUG
#include <assert.h>
#include <time.h>
// #endif

using namespace std;

typedef struct {int x, y;} int2;

// Car和Road里的声明调用了Cross, 因此必须前置声明Cross
struct Car;
// struct Lane;
struct Road;
struct Cross;

struct Car {
	int id, from, to, speed, planTime;
    // 车辆状态
    struct {
        // 对roadId 与 nextRoadId:
        // -1: 终点
        // -2: 起点
        int roadID, nextRoadID;
        // location: 1...length
        int channelNum, location;
        bool isWaiting = false;
    } status;
    // 车辆出发时间及行驶路径
    struct {
        int startTime, stopTime;
        vector<int> route;
    } answer;
    
    Car(int id, int from, int to, int speed, int planTime);
    bool ready();
    bool start();
    void finish();
    int2 reachCross(Cross* cross);

};

// struct Lane{
//     // #ifdef DEBUG
//     // int road_id, channel_index;  // 调试用
//     // #endif
//     // int dispatchedTimes = 0;
//     list<Car*> carsOnLane;
//     // vector<Car*> carsOnLane;
// };

struct Road {
	int id, length, speed, channel, from, to, isDuplex;
    // 储存路上汽车情况，每个队列表示一个车道
    // 队列顺序: 先正方向后反方向，每个方向都按车道号由小向大排
    // vector<vector<Car*>> roadMap;
    Car ***roadMap;

    list<Car*> carsWaitingForDispatched[2];

    int dispatched_times[2] = {-1, -1};

    // 权重，用来衡量拥挤程度
    // double length_weight[2] = {1, 1};

	Road(int id, int length, int speed, int channel, int from, int to, int isDuplex);
    // 获取进入当前道路时，当前道路内的可行驶距离
    int2 getFrontStatus(int fromCrossId);
    int2 getFreeLength(int fromCrossId);
    bool isCrowded(int cross_id);
    bool isEmpty(int cross_id);
    Car* getFirstWaitingCar(int cross_id);
    bool isPositiveDirection(int from_cross_id);
};

struct Cross {
	int id, roadId[4];
    // 对道路按id排序后存储
    int rankDist[4] = {-1, -1, -1, -1};
    // 能够进入该路口的道路数量
    int dispatched_roads = 0, total_roads = 0;
    // 该路口被调度的次数，用来判断当前时间片该路口是否完成调度
    int dispatched_times = -1;

	Cross(int id, int roadId1, int roadId2, int roadId3, int roadId4);
    Road* getRoad(int num);
    int getFrontRoad(Road* road);
    int getLeftRoad(Road* road);
    int getRightRoad(Road* road);
};

// 系统调度时间
extern int currentTime;
extern unsigned finished_cars;
extern list<Car*> CarsNotReady, CarsReady, CarsRunning;
// 储存数据的全局变量
extern map<int, Car*> Cars;
extern map<int, Road*> Roads;
extern map<int, Cross*> Crosses;

extern int **G, **D, **nextRoad;

#endif