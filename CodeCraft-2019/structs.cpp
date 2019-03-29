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

bool Car::ready() {
    if ((planTime + (int)((id-10000)*0.09)) > currentTime) return false;
    else return true;
}

bool Car::start() {
    // 决定是否发车
    if (currentTime < planTime) return false;
    if (!ready()) return false;

    int road_id = nextRoad[from][to];
    Road* road = Roads[road_id];
    int2 tmp = road->getFreeLength(from);
    // 路口拥堵的话，无法发车
    if (tmp.x == -1) return false;

    // 发车
    answer.startTime = currentTime;
    answer.route.push_back(road_id);
    status.roadID = road_id;
    status.channelNum = tmp.x;
    int max_speed = road->speed > speed ? speed : road->speed;
    status.location = tmp.y > max_speed ? max_speed : tmp.y;
    assert(road->roadMap[status.channelNum][status.location - 1] == nullptr);
    road->roadMap[status.channelNum][status.location - 1] = this;
    CarsRunning.push_back(this);
    return true;
}

void Car::finish() {
    answer.stopTime = currentTime;
    finished_cars++;
    Roads[status.roadID]->roadMap[status.channelNum][status.location - 1] = nullptr;
    for (auto it = CarsRunning.begin(); it != CarsRunning.end(); it++) {
        if ((*it)->id == id) {
            CarsRunning.erase(it);
            break;
        }
    }
}

// return: 
// {-3, 0}: 等待下次调度
// {-2, 0}: 无法通过路口
// {-1, 0}: 被阻塞在路口
// {index, length}: {下条车道编号， 下条道路可行驶距离}
// 注意：这个函数假定从这辆车到路口处没有其他车辆的阻碍，即只能对车道上第一辆车进行分析
int2 Car::reachCross(Cross* cross) {
    Road* next_road = Roads[status.nextRoadID];
    int2 tmp = next_road->getFrontStatus(cross->id);
    if (tmp.x == -1) return {-1, 0};  // 前方道路拥堵
    Road* road = Roads[status.roadID];
    // int R1 = road->speed;
    int R2 = next_road->speed;
    // int V1 = R1 > speed ? speed : R1;
    int V2 = R2 > speed ? speed : R2;
    int S1 = road->length - status.location;
    int S2 = V2 - S1 > 0 ? V2 - S1 : 0;
    // if (S1 > V1) return {-2, 0};
    if (S1 >= V2 || S2 == 0) return {-1, 0};
    if (tmp.x < -1000) {
        if (tmp.y < S2) return {-3, 0};
        else tmp.x += 10000;
    }
    return {tmp.x, S2 < tmp.y ? S2 : tmp.y};
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

    roadMap = new Car**[channel * (1 + isDuplex)];
    for (int i = 0; i < channel * (1 + isDuplex); i++) {
        roadMap[i] = new Car*[length];
    }
    for (int i = 0; i < channel * (1 + isDuplex); i++) {
        for (int j = 0; j < length; j++) {
            roadMap[i][j] = nullptr;
        }
    }
}

// return:
// {-1, 0}: 被阻塞在路口
// {index, length}: {下条车道编号， 下条道路可行驶距离}
// {-10000+index, length}: 下条车道前方车辆处于等待状态
int2 Road::getFrontStatus(int fromCrossId) {
    int index = -1;
    if (fromCrossId == from) {
        index = 0;
    }
    else if (fromCrossId == to && isDuplex == 1) {
        index = channel;
    }

    int i, j;
    for (i = index; i < index + channel; i++) {
        if (roadMap[i][0] != nullptr && roadMap[i][0]->status.isWaiting == false) {
            continue;
        }
        for (j = 0; j < length; j++) {
            if (roadMap[i][j] != nullptr) {
                if (roadMap[i][j]->status.isWaiting) {
                    return {-10000 + i, j};
                }
                else {
                    return {i, j};
                }
            }
        }
        return {i, length};
    }
    return {-1, 0};
}

int2 Road::getFreeLength(int fromCrossId) {
    int index = -1;
    if (fromCrossId == from) {
        index = 0;
    }
    else if (fromCrossId == to && isDuplex == 1) {
        index = channel;
    }

    int i, j;
    for (i = index; i < index + channel; i++) {
        if (roadMap[i][0] != nullptr) continue;
        for (j = 1; j < length; j++) {
            if (roadMap[i][j] != nullptr) {
                return {i, j};
            }
        }
        return {i, length};
    }
    return {-1, 0};
}

bool Road::isPositiveDirection(int from_cross_id) {
    if (from_cross_id == from) return true;
    if (from_cross_id == to) return false;
    assert(0);
}

// Cross
Cross::Cross(int id, int roadId1, int roadId2, int roadId3, int roadId4) {
    Cross::id = id;
    Cross::roadId[0] = roadId1;
    Cross::roadId[1] = roadId2;
    Cross::roadId[2] = roadId3;
    Cross::roadId[3] = roadId4;

    int roadIdRanked[4];
    for (int i = 0; i < 4; i++) {
        roadIdRanked[i] = roadId[i];
    }
    sort(roadIdRanked, roadIdRanked + 4);
    for (int i = 0; i < 4; i++) {
        int tmp = (int)(find(roadIdRanked, roadIdRanked + 4, roadId[i]) - roadIdRanked);
        rankDist[tmp] = i;
    }

    // 初始化 total_roads
    for (int i = 0; i < 4; i++) {
        Road* road = getRoad(i);
        if (road == nullptr) continue;
        if (road->from == id && road->isDuplex == 0) continue;
        total_roads++;
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

int Cross::getFrontRoad(Road* road) {
    for (int i = 0; ; ++i %= 4) {
        if (roadId[i] == road->id) {
            i += 2; i %= 4;
            return roadId[i];
        }
    }
}

int Cross::getLeftRoad(Road* road) {
    for (int i = 0; ; ++i %= 4) {
        if (roadId[i] == road->id) {
            i += 1; i %= 4;
            return roadId[i];
        }
    }
}

int Cross::getRightRoad(Road* road) {
    for (int i = 0; ; ++i %= 4) {
        if (roadId[i] == road->id) {
            i += 3; i %= 4;
            return roadId[i];
        }
    }
}