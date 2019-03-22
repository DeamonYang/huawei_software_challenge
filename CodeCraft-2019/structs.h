#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#define DEBUG

#include <vector>
#include <queue>
#include <algorithm>
#include <map>
#include <list>

#ifdef DEBUG
#include <assert.h>
#include <time.h>
#endif

using namespace std;

typedef struct {int x, y;} int2;

// Car和Road里的声明调用了Cross, 因此必须前置声明Cross
struct Car;
struct Lane;
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
    void goThrough(int2 channel_length);

};

struct Lane{
    #ifdef DEBUG
    int road_id, channel_index;  // 调试用
    #endif
    int dispatchedTimes = 0;
    list<Car*> carsOnLane;
};

struct Road {
	int id, length, speed, channel, from, to, isDuplex;
    // 储存路上汽车情况，每个队列表示一个车道
    // 队列顺序: 先正方向后反方向，每个方向都按车道号由小向大排
    vector<Lane> roadMap;

	Road(int id, int length, int speed, int channel, int from, int to, int isDuplex);
    // 获取进入当前道路时，当前道路内的可行驶距离
    int2 getFreeLength(int fromCrossId);
    bool isCrowded(Cross* cross);
};

struct Cross {
	int id, roadId[4];
    // 能够进入该路口的车道数量
    int total_channels = 0;
    // 对每一条路，按进入这条路的先后顺序列出另外几条路的车道，用队列指针表示车道
    vector<Lane*> channelsToRoad[4];

	Cross(int id, int roadId1, int roadId2, int roadId3, int roadId4);
    Road* getRoad(int num);
};

// 系统调度时间
extern int currentTime;
extern unsigned finished_cars;
extern list<Car*> CarsNotReady, CarsReady;
// 储存数据的全局变量
extern map<int, Car*> Cars;
extern map<int, Road*> Roads;
extern map<int, Cross*> Crosses;

extern int **G, **D, **nextRoad;

#endif