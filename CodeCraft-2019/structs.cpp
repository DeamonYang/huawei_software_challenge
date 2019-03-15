#include "structs.h"

using namespace std;

// 用map来存放 方便后续查找
map<int, Car*> Cars;
map<int, Road*> Roads;
map<int, Cross*> Crosses;

// Car
Car::Car(int id, int from, int to, int speed, int planTime) {
    Car::id = id;
    Car::from = from;
    Car::to = to;
    Car::speed = speed;
    Car::planTime = planTime;
}

Cross* Car::getCrossFrom() {
    return Crosses[from];
}
Cross* Car::getCrossTo() {
    return Crosses[to];
}

// Road
Road::Road(int id, int length, int speed, int channel, int from, int to, int isDuplex) {
    Road::id = id;
    Road::length = length;
    Road::speed = speed;
    Road::channel = channel;
    Road::from = from;
    Road::to = to;
    Road::isDuplex = isDuplex;
}

Cross* Road::getCrossFrom() {
    return Crosses[from];
}
Cross* Road::getCrossTo() {
    return Crosses[to];
}

// Cross
Cross::Cross(int id, int roadId1, int roadId2, int roadId3, int roadId4) {
    Cross::id = id;
    Cross::roadId1 = roadId1;
    Cross::roadId2 = roadId2;
    Cross::roadId3 = roadId3;
    Cross::roadId4 = roadId4;
}

Road* Cross::getRoad1() {
    return Roads[roadId1];
}

Road* Cross::getRoad2() {
    return Roads[roadId2];
}

Road* Cross::getRoad3() {
    return Roads[roadId3];
}

Road* Cross::getRoad4() {
    return Roads[roadId4];
}
