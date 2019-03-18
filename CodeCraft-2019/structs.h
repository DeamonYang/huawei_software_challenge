#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#include <vector>
#include <queue>
#include <algorithm>
#include <map>
#include <list>

using namespace std;

typedef struct {int x, y;} int2;

// Car和Road里的声明调用了Cross, 因此必须前置声明Cross
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
        int channelNum, location, isWaitingDispatch;
    } status;
    // 车辆出发时间及行驶路径
    struct {
        int startTime;
        vector<int> route;
    } answer;
    
    Car(int id, int from, int to, int speed, int planTime);
    Cross* getCrossFrom();
    Cross* getCrossTo();
    void start();
    void finish();
    int2 reachCross(Cross* cross);
    void goThrough(int2 channel_length);

};

struct Road {
	int id, length, speed, channel, from, to, isDuplex;
    // 储存路上汽车情况，每个队列表示一个车道
    // 队列顺序: 先正方向后反方向，每个方向都按车道号由小向大排
    vector<queue<Car*>> roadMap;

	Road(int id, int length, int speed, int channel, int from, int to, int isDuplex);
    Cross* getCrossFrom();
    Cross* getCrossTo();
    void dispatch();
    // 获取进入当前道路时，当前道路内的可行驶距离
    int2 getFreeLength(int fromCrossId);
};

struct Cross {
	int id, roadId[4];
    // 对每一条路，按进入这条路的先后顺序列出另外几条路的车道，用队列指针表示车道
    vector<queue<Car*>*> channelsToRoad[4];

	Cross(int id, int roadId1, int roadId2, int roadId3, int roadId4);
    Road* getRoad(int num);
    bool dispatch_finished();
};

// 系统调度时间
extern int currentTime;
// 储存数据的全局变量
extern map<int, Car*> Cars;
extern map<int, Road*> Roads;
extern map<int, Cross*> Crosses;

#endif