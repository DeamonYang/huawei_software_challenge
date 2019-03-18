#include "structs.h"

using namespace std;

// Car
Car::Car(int id, int from, int to, int speed, int planTime) {
    Car::id = id;
    Car::from = from;
    Car::to = to;
    Car::speed = speed;
    Car::planTime = planTime;
    Car::status.roadID = -2;
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

// return: 
// -1: 无法通过路口
// 0: 被阻塞在路口
// default: 下条道路行驶距离
int2 Car::reachCross(Cross* cross) {
    int2 tmp = Roads[status.nextRoadID]->getFreeLength(cross->id);
    if (status.roadID == -2) {
        return {-1, tmp.y};
    }
    else {
        int R1 = Roads[status.roadID]->speed;
        int R2 = Roads[status.nextRoadID]->speed;
        int V = speed;
        int V1 = R1 > V ? V : R1;
        int V2 = R1 > V ? V : R2;
        int S1 = Roads[status.roadID]->length - status.location;
        int S2 = tmp.y;
        if (S1 >= V1) return {-1, -1};
        // if (S2 >= V2) S2 = V2;  // 似乎与下一条规则重复
        if (S2 >= (V2 - S1)) S2 = V2 - S1;
        if (S1 >= V2) return {-1, 0};
        return {tmp.x, S2};
    }
}

void Car::goThrough(int2 channel_length) {
    // 要到达目的地的车离开
    if (to == id) {
        finish();
        return;
    }
    // 正在驶向这个路口的车
    status.roadID = status.nextRoadID;
    status.channelNum = channel_length.x;
    status.location = channel_length.y;
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

int2 Road::getFreeLength(int fromCrossId) {
    int index;
    if (fromCrossId == from) {
        index = 0;
    }
    else if (fromCrossId == to && isDuplex == 1) {
        index = channel;
    }
    else {
        // cout << "ERROR" << endl;
    }

    int freeLength = 0;
    for (int i = index; i < index + channel; i++) {
        int location = roadMap[i].back()->status.location;
        if (location != 1) {
            freeLength = location - 1;
            return {i - index, freeLength};
        }
    }
    return {-1, freeLength};  // -1表示所有车道均拥堵
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
        Road* road_to = getRoad(i);
        if ((road_to->to == id) && (road_to->isDuplex == 0)) continue;
        for (int j = 0; j < 3; j++) {
            if (roadId[table[i][j]] == -1) continue;
            Road* road_from = getRoad(table[i][j]);
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

Road* Cross::getRoad(int num) {
    if (roadId[num] == -1) {
        return nullptr;
    }
    else {
        return Roads[roadId[num]];
    }
}

bool Cross::dispatch_finished() {
    return true;
}
