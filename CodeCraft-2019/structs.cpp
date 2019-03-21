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

bool Car::start() {
    int road_id = nextRoad[from][to];
    int2 tmp = Roads[road_id]->getFreeLength(from);
    // 路口拥堵的话，无法发车
    if (tmp.x == -1) return false;

    answer.startTime = currentTime;
    answer.stopTime = currentTime;
    answer.route.push_back(road_id);
    status.roadID = road_id;
    status.channelNum = tmp.x;
    // assert(tmp.y > 0);
    status.location = tmp.y > speed ? speed : tmp.y;
    Roads[status.roadID]->roadMap.at(status.channelNum).carsOnLane.push_back(this);
    return true;
}
void Car::finish() {
    answer.stopTime = currentTime;
    finished_cars++;
    Roads[status.roadID]->roadMap.at(status.channelNum).carsOnLane.pop_front();
}

// return: 
// {-2, 0}: 无法通过路口
// {-1, 0}: 被阻塞在路口
// {index, length}: {下条车道编号， 下条道路可行驶距离}
// 注意：这个函数假定从这辆车到路口处没有其他车辆的阻碍，即只能对车道上第一辆车进行分析
int2 Car::reachCross(Cross* cross) {
    int2 tmp = Roads[status.nextRoadID]->getFreeLength(cross->id);
    if (tmp.x == -1) return {-1, 0};  // 前方道路拥堵
    int R1 = Roads[status.roadID]->speed;
    int R2 = Roads[status.nextRoadID]->speed;
    int V = speed;
    int V1 = R1 > V ? V : R1;
    int V2 = R1 > V ? V : R2;
    int S1 = Roads[status.roadID]->length - status.location;
    int S2 = tmp.y;
    if (S1 >= V1) return {-2, 0};
    // if (S2 >= V2) S2 = V2;  // 似乎与下一条规则重复
    if (S2 >= (V2 - S1)) S2 = V2 - S1;
    if (S1 >= V2) return {-1, 0};
    return {tmp.x, S2};
}

void Car::goThrough(int2 lane_length) {
    answer.stopTime = currentTime;
    Roads[status.roadID]->roadMap[status.channelNum].carsOnLane.pop_front();
    Roads[status.nextRoadID]->roadMap[lane_length.x].carsOnLane.push_back(this);
    status.roadID = status.nextRoadID;
    status.channelNum = lane_length.x;
    // assert(lane_length.y > 0);
    status.location = lane_length.y;
    answer.route.push_back(status.roadID);
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
    roadMap = vector<Lane>(channel * (1 + isDuplex));
    #ifdef DEBUG
    for (unsigned i = 0; i < roadMap.size(); i++) {
        roadMap[i].road_id = id;
        roadMap[i].channel_index = i;
    }
    #endif
}

int2 Road::getFreeLength(int fromCrossId) {
    int index = -1;
    if (fromCrossId == from) {
        index = 0;
    }
    else if (fromCrossId == to && isDuplex == 1) {
        index = channel;
    }
    // assert(index != -1);

    for (int i = index; i < index + channel; i++) {
        // 车道上没车
        list<Car*>* q = &roadMap[i].carsOnLane;
        if (q->size() == 0) {
            return {i, length};
        }
        // 车道上有车
        int location = q->back()->status.location;
        if (location > 1) {
            return {i, location - 1};
        }
    }
    // 所有车道均拥堵
    return {-1, 0};
}

bool Road::isCrowded(Cross* cross) {
    // if (currentTime <= 1) return false;  // 留给fake_floyd用的
    // int index = -1;
    // if (cross->id == from) {
    //     index = 0;
    // }
    // else if (cross->id == to && isDuplex == 1) {
    //     index = channel;
    // }
    // assert(index != -1);

    // for (int i = index; i < index + channel; i++) {
    //     if (roadMap[i].carsOnLane.empty()) return false;
    //     if (roadMap[i].carsOnLane.front()->status.location < length) return false;
    // }
    // return true;
    return false;
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
        0, 1, 3,
        1, 2, 0
    };
    // 初始化 channelsToRoad
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
    // 初始化 total_channels
    for (int i = 0; i < 4; i++) {
        if (roadId[i] == -1) continue;
        Road* road_from = getRoad(i);
        if ((road_from->from == id) && (road_from->isDuplex == 0)) continue;
        total_channels += road_from->channel;
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