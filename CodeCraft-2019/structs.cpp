#include "structs.h"

using namespace std;


int currentTime = 0;
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

void Car::start() {
    answer.startTime = currentTime;
}
void Car::finish() {
    
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
    // roadMap = new queue<Car*>[channel * (1 + isDuplex)];
    roadMap = vector<queue<Car*>>(channel * (1 + isDuplex));
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
    Cross::roadId[0] = roadId1;
    Cross::roadId[1] = roadId2;
    Cross::roadId[2] = roadId3;
    Cross::roadId[3] = roadId4;

    // 对每条道路而言，另外三条道路进入这条道路的先后顺序表
    static const int table[4][3] = {
        2, 3, 1,
        3, 0, 2,
        0, 3, 1,
        1, 2, 0
    };
    for (int i = 0; i < 4; i++) {
        if (roadId[i] == -1) continue;
        Road* road_to = getRoad(roadId[i]);
        if ((road_to->to == id) && (road_to->isDuplex == 0)) continue;
        for (int j = 0; j < 3; j++) {
            if (roadId[table[i][j]] == -1) continue;
            Road* road_from = getRoad(roadId[table[i][j]]);
            if (road_from->to == id) {
                for (int k = 0; k < road_from->channel; k++) {
                    channelsToRoad[i].push_back(&road_from->roadMap.at(k));
                }
            }
            else if((road_from->from == id) && (road_from->isDuplex == 1)) {
                for (int k = road_from->channel; k < 2*road_from->channel; k++) {
                    channelsToRoad[i].push_back(&road_from->roadMap.at(k));
                }
            }
        }
    }
}

Road* Cross::getRoad(int roadId) {
    return Roads[roadId];
}
