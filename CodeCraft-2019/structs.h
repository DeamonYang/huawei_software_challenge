#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#include <map>
#include <queue>

using namespace std;

// Car和Road里的声明调用了Cross, 因此必须前置声明Cross
struct Cross;

struct Car {
	int id, from, to, speed, planTime;
    
    Car(int id, int from, int to, int speed, int planTime);
    Cross* getCrossFrom();
    Cross* getCrossTo();
};

struct Road {
	int id, length, speed, channel, from, to, isDuplex;

	Road(int id, int length, int speed, int channel, int from, int to, int isDuplex);
    Cross* getCrossFrom();
    Cross* getCrossTo();
};

struct Cross {
	int id, roadId1, roadId2, roadId3, roadId4;

	Cross(int id, int roadId1, int roadId2, int roadId3, int roadId4);
    Road* getRoad1();
    Road* getRoad2();
    Road* getRoad3();
    Road* getRoad4();
};

struct Answer {
	int carId, StartTime;
	queue<int> RoadId;
};

// 储存数据的全局变量
extern map<int, Car*> Cars;
extern map<int, Road*> Roads;
extern map<int, Cross*> Crosses;

#endif