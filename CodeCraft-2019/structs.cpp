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
    Road* road = Roads[nextRoad[from][to]];
    int road_speed = speed > road->speed ? road->speed : speed;
    int index = road->isPositiveDirection(from) ? 0 : road->channel;

    static int time = 0, startedCars = 0;
    if (time < currentTime) {
        time = currentTime;
        // printf("%d\n", startedCars);
        startedCars = 0;
    }

    if (currentTime <= 10 && speed < 6) return false;

    switch (currentTime) {
        case 1: if (startedCars >= 90) return false; break;
        case 2: if (startedCars >= 90) return false; break;
        case 3: if (startedCars >= 80) return false; break;
        case 4: if (startedCars >= 55) return false; break;
        case 5: if (startedCars >= 50) return false; break;
        case 6: if (startedCars >= 40) return false; break;
        case 7: if (startedCars >= 40) return false; break;
        case 8: if (startedCars >= 35) return false; break;
        case 9: if (startedCars >= 30) return false; break;
        case 10: if (startedCars >= 30) return false; break;
        default: if (startedCars >= 20) return false; break;
    }
    int2 tmp = road->getFreeLength(from);
    if (tmp.x != index || tmp.y < road_speed) return false;
    

    // if (startedCars * log(currentTime) > 61) return false;
    // return road->isEmpty(from);
    // return !road->isCrowded(from);
    // if ((planTime + (int)((id-10000)*0.09)) > currentTime) return false;

    startedCars++;
    return true;
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
int2 Road::getFrontStatus(int from_cross_id) {
    int index = isPositiveDirection(from_cross_id) ? 0 : channel;

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

int2 Road::getFreeLength(int from_cross_id) {
    int index = isPositiveDirection(from_cross_id) ? 0 : channel;

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

// 当前方向所有车道均有车即为拥挤
bool Road::isCrowded(int from_cross_id) {
    int index = isPositiveDirection(from_cross_id) ? 0 : channel;

    int channel_not_empty = 0;
    for (int i = index; i < index + channel; i++) {
        for (int j = length - 1; j >= 0; j--) {
            if (roadMap[i][j] != nullptr) {
                channel_not_empty++;
                break;
            }
        }
    }
    return channel_not_empty == channel;
}

bool Road::isEmpty(int from_cross_id) {
    int index = isPositiveDirection(from_cross_id) ? 0 : channel;

    for (int i = index; i < index + channel; i++) {
        for (int j = length - 1; j >= 0; j--) {
            if (roadMap[i][j] != nullptr) return false;
        }
    }
    return true;
}

bool Road::isFirstLaneEmpty(int cross_id) {
    int index = isPositiveDirection(cross_id) ? 0 : channel;
    for (int j = length - 1; j >= 0; j--) {
        if (roadMap[index][j] != nullptr) return false;
    }
    return true;
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