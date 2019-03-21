#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include "structs.h"

#define MAX_LINE_LENGTH 256
#define MAX_ROAD_LENGTH 0x0fff

using namespace std;

int currentTime = 0;
unsigned finished_cars = 0;
list<Car*> CarsNotReady, CarsReady;
// 用map来存放 方便后续查找
map<int, Car*> Cars;
map<int, Road*> Roads;
map<int, Cross*> Crosses;

int **G, **D, **nextRoad;

bool compCarPlantime(const Car* car1, const Car* car2);
bool compCarSpeed(const Car* car1, const Car* car2);
bool compCarId(const Car* car1, const Car* car2);
void read_data(string carPath, string roadPath, string crossPath);
void preprocess();
void process();
void dispatchCarsOnRoad();
void dispatchCarsInGarage();
void output(string answerPath);
void floyd();
vector<int> getRouteFloyd(int, int);
int getNextRoadFloyd(int, int);
bool dispatchFirstCar(Cross*, Road*, Lane*);
void dispatchFollowingCars(Lane*);
void fake_process();
void refreshCarsReady();

int main(int argc, char *argv[])
{
	if (argc < 5) {
		std::cout << "please input args: carPath, roadPath, crossPath, answerPath" << std::endl;
		exit(1);
	}
	
	std::string carPath(argv[1]);
	std::string roadPath(argv[2]);
	std::string crossPath(argv[3]);
	std::string answerPath(argv[4]);
	
	std::cout << "carPath is " << carPath << std::endl;
	std::cout << "roadPath is " << roadPath << std::endl;
	std::cout << "crossPath is " << crossPath << std::endl;
	std::cout << "answerPath is " << answerPath << std::endl;
	
	#ifdef DEBUG
	clock_t start, end;
	start = clock();
	#endif

	// read input filebuf
	read_data(carPath, roadPath, crossPath);
	preprocess();

	// process
	// fake_process();
	process();
	// write output file
	output(answerPath);

	// print information
	#ifdef DEBUG
	end = clock();
	int total_car_time = 0;
	for (auto it = Cars.begin(); it != Cars.end(); it++) {
		total_car_time += (it->second->answer.stopTime - it->second->answer.startTime);
	}
	cout << "系统调度时间: " << currentTime - 1 << endl;
	cout << "所有车辆总调度时间: " << total_car_time << endl;
	cout << "程序运行时间: " << (end - start)/1000 << endl;
	#endif

	return 0;
}

void read_data(string carPath, string roadPath, string crossPath) {
	cout << "Begin Reading data!" << endl;

	ifstream fcar(carPath.c_str());
	ifstream froad(roadPath.c_str());
	ifstream fcross(crossPath.c_str());

	// 读取并解析数据: Cars
	char buffer[MAX_LINE_LENGTH];
	while (fcar.getline(buffer, MAX_LINE_LENGTH)) {
		if (buffer[0] == '#') continue;
		int int_start = 0, len = strlen(buffer);
		vector<int> value;
		for (int i = 0; i < len; i++) {
			// 每个数字前面的字符都为'('或' '
			if (buffer[i] == '(' || buffer[i] == ' ') {
				int_start = i + 1;
			}
			// 每个数字后面的字符都为'）'或','
			// 并将该字符替换为'\0'以截断字符串，从而将char[]转换为int，存入队列中
			else if (buffer[i] == ')' || buffer[i] == ',') {
				buffer[i] = '\0';
				value.push_back(atoi(buffer + int_start));
			}
		}
		// 初始化对象
		Cars[value.at(0)] = new Car(value.at(0), value.at(1), value.at(2), value.at(3), value.at(4));
		// CarsReady.push_back(Cars[value.at(0)]);
		CarsNotReady.push_back(Cars[value.at(0)]);
		// if (value.at(4) > 10) exit(1);
	}
	fcar.close();
	// if (Cars.size() > 20000) exit(1);

	// 读取并解析数据: Roads
	while (froad.getline(buffer, MAX_LINE_LENGTH)) {
		if (buffer[0] == '#') continue;
		int int_start = 0, len = strlen(buffer);
		vector<int> value;
		for (int i = 0; i < len; i++) {
			// 每个数字前面的字符都为'('或' '
			if (buffer[i] == '(' || buffer[i] == ' ') {
				int_start = i + 1;
			}
			// 每个数字后面的字符都为'）'或','
			// 并将该字符替换为'\0'以截断字符串，从而将char[]转换为int，存入队列中
			else if (buffer[i] == ')' || buffer[i] == ',') {
				buffer[i] = '\0';
				value.push_back(atoi(buffer + int_start));
			}
		}
		// 初始化对象
		Roads[value.at(0)] = new Road(value.at(0), value.at(1), value.at(2), value.at(3), value.at(4), value.at(5), value.at(6));
	}
	froad.close();
	// if (Roads.size() > 200) exit(1);

	// 读取并解析数据: Crosses
	while (fcross.getline(buffer, MAX_LINE_LENGTH)) {
		if (buffer[0] == '#') continue;
		int int_start = 0, len = strlen(buffer);
		vector<int> value;
		for (int i = 0; i < len; i++) {
			// 每个数字前面的字符都为'('或' '
			if (buffer[i] == '(' || buffer[i] == ' ') {
				int_start = i + 1;
			}
			// 每个数字后面的字符都为'）'或','
			// 并将该字符替换为'\0'以截断字符串，从而将char[]转换为int，存入队列中
			else if (buffer[i] == ')' || buffer[i] == ',') {
				buffer[i] = '\0';
				value.push_back(atoi(buffer + int_start));
			}
		}
		// 初始化对象
		Crosses[value.at(0)] = new Cross(value.at(0), value.at(1), value.at(2), value.at(3), value.at(4));
	}
	fcross.close();
	// if (Crosses.size() > 200) exit(1);
}

void preprocess() {
	int num_crosses = Crosses.size();
	// 分配空间
	// 这里假定cross的id是从1开始顺下去的
	G = new int*[num_crosses + 1];
	D = new int*[num_crosses + 1];
	nextRoad = new int*[num_crosses + 1];
	for (int i = 1; i <= num_crosses; i++) {
		G[i] = new int[num_crosses + 1];
		D[i] = new int[num_crosses + 1];
		nextRoad[i] = new int[num_crosses + 1];
	}
}

void process() {
	// 对所有车辆按照准备出发时间进行排序
	// CarsNotReady.sort(compCarPlantime);
	// 开始进行系统调度
	for (currentTime = 1; finished_cars < Cars.size(); currentTime++) {
		// 更新CarsReady
		refreshCarsReady();
		// 更新路径矩阵
		floyd();

		// 对道路上的车进行调度
		dispatchCarsOnRoad();
		// 启动一些尚未出发的车
		dispatchCarsInGarage();
	}
}

void dispatchCarsOnRoad() {
	for (auto it = Crosses.begin(); it != Crosses.end(); it++) {
		Cross* cross = it->second;
		int dispatched_lanes = 0;
		for (int i = 0; dispatched_lanes < cross->total_channels; ++i %= 4) {
			Road* road = cross->getRoad(i);
			if (road == nullptr) continue;  // 道路不存在
			if (road->to == cross->id && road->isDuplex == 0) continue;  // 单行道，无法进入

			// 对要经过cross进入road的lane按照交通规则给定的顺序依次访问
			for (auto it1 = cross->channelsToRoad[i].begin(); it1 != cross->channelsToRoad[i].end(); it1++) {
				// 如果进入该路口的所有车道调度完毕，则该路口调度完毕
				if (dispatched_lanes == cross->total_channels) break;

				Lane* lane = *it1;
				// 如果这条车道已经结束调度，则继续
				if (lane->dispatchedTimes == currentTime) {
					continue;
				}
				
				if (dispatchFirstCar(cross, road, lane)) {
					dispatched_lanes++;
					lane->dispatchedTimes++;
				}				
			}
		}
	}
}

void dispatchCarsInGarage() {
	// 模拟fake_process里面的算法
	if (CarsReady.size() == 0) return;
	for (auto it = CarsReady.begin(); it != CarsReady.end(); ) {
		Car* car = *it;
		if ((car->planTime + (int)((car->id-10000)*0.09)) <= currentTime) {
			if (car->start()) {
				it = CarsReady.erase(it);
			}
			else {
				static int times = 0;
				cout << "Start car failed! (crowded)" << ++times << endl;
				it++;
			}
		}
		else {
			it++;
		}
	}
}

void output(string answerPath) {
	ofstream fanswer(answerPath.c_str());
	fanswer << "#(carId,StartTime,RoadId...)" << endl;
	for (auto it = Cars.begin(); it != Cars.end(); it++) {
		Car* car = it->second;
		fanswer << "(" << car->id << ", ";
		fanswer << car->answer.startTime;
		for (auto it2 = car->answer.route.begin(); it2 != car->answer.route.end(); it2++) {
			fanswer << ", " << *it2;
		}
		fanswer << ")" << endl;
	}
}

vector<int> getRouteFloyd(int from, int to) {
	int i = from, j = to;
	vector<int> v;
	if (D[i][j] != j) {
		int k = D[i][j];
		vector<int> v1, v2;
		v1 = getRouteFloyd(i, k);
		v2 = getRouteFloyd(k, j);
		v.insert(v.end(),v1.begin(),v1.end());
		v.insert(v.end(),v2.begin(),v2.end());
	}
	else {
		Cross* cross_from = Crosses[from];
		for (int i = 0; i < 4; i++) {
			Road* road = cross_from->getRoad(i);
			if (road == nullptr) {
				continue;
			}
			else if (road->from == from && road->to == to) {
				v.push_back(road->id);
				break;
			}
			else if (road->to == from && road->isDuplex == 1 && road->from == to) {
				v.push_back(road->id);
				break;
			}
			else if (i == 3) {
				cout << "ERROR!" << endl;
			}
		}
	}
	return v;
}

int getNextRoadFloyd(int from, int to) {
	int i = from, j = to;
	// if (i == 1 && j == 24) {
	// 	int a = 0;
	// }
	if (i == j) {
		return -1;  // 到达终点
	}
	else if (D[i][j] != j) {
		int k = D[i][j];
		return getNextRoadFloyd(i, k);
	}
	else {
		Cross* cross_from = Crosses[from];
		for (int i = 0; i < 4; i++) {
			Road* road = cross_from->getRoad(i);
			if (road == nullptr) {
				continue;
			}
			else if (road->from == from && road->to == to) {
				return road->id;
			}
			else if (road->to == from && road->isDuplex == 1 && road->from == to) {
				return road->id;
			}
		}
	}
	assert(0);
	return -3;  // ERROR
}

void refreshCarsReady() {
	if (CarsNotReady.size() == 0) return;
	for (auto it = CarsNotReady.begin(); it != CarsNotReady.end(); ) {
		if (true) {  //(*it)->planTime <= currentTime) {
			CarsReady.push_back(*it);
			it = CarsNotReady.erase(it);
		}
		else {
			it++;
		}
	}
	// 对所有已准备好出发的车辆按照最大速度进行排序
	// CarsReady.sort(compCarSpeed);
	
	// 对所有已准备好出发的车辆按照id排序
	// CarsReady.sort(compCarId);
}

void floyd() {
	int num_crosses = Crosses.size();
	// 生成邻接矩阵
	for (int i = 1; i <= num_crosses; i++) {
		for (int j = 1; j <= num_crosses; j++) {
			if (i != j) {
				G[i][j] = MAX_ROAD_LENGTH;
			}
			else {
				G[i][j] = 0;
			}
		}
	}

	for (auto it = Roads.begin(); it != Roads.end(); it++) {
		Road* road = it->second;
		if (road->isCrowded(Crosses[road->from])) {
			G[road->from][road->to] = MAX_ROAD_LENGTH;
		}
		else {
			G[road->from][road->to] = road->length;
		}

		if (road->isDuplex == 0 || road->isCrowded(Crosses[road->to])) {
			G[road->to][road->from] = MAX_ROAD_LENGTH;
		}
		else {
			G[road->to][road->from] = road->length;
		}
	}

	// Floyd算法
	for (int i = 1; i <= num_crosses; i++) {
		for (int j = 1; j <= num_crosses; j++) {
			D[i][j] = j;
		}
	}

	for (int k = 1; k <= num_crosses; k++) {
		for (int i = 1; i <= num_crosses; i++) {
			for (int j = 1; j <= num_crosses; j++) {
				if (i != j && i != k && j != k) {
					if (G[i][j] > G[i][k] + G[k][j]) {
						G[i][j] = G[i][k] + G[k][j];
						D[i][j] = k;
					}
				}
			}
		}
	}

	// 生成nextRoad矩阵
	for (int i = 1; i <= num_crosses; i++) {
		for (int j = 1; j <= num_crosses; j++) {
			nextRoad[i][j] = -1;
		}
	}
	for (int i = 1; i <= num_crosses; i++) {
		for (int j = 1; j <= num_crosses; j++) {
			if (nextRoad[i][j] == -1){
				nextRoad[i][j] = getNextRoadFloyd(i, j);
			}
		}
	}
}

// return:
// true:  调度完成
// false: 调度未完成
bool dispatchFirstCar(Cross* cross, Road* road, Lane* lane) {
	// 如果车道为空，则这条车道视作已完成调度
	if (lane->carsOnLane.size() == 0) {
		return true;
	}
	// 取车道里最靠前的一辆车为研究对象
	Car* car = lane->carsOnLane.front();

	// 这辆车是从上一条路跑来的，相当于原先车道是空的
	if (car->answer.stopTime == currentTime) {
		return true;
	}

	// 讨论这辆车能否到达或是通过前面的路口
	car->status.nextRoadID = nextRoad[cross->id][car->to];
	if (car->status.nextRoadID == car->status.roadID) {
		// 不能掉头，换一条路
		Road* road;
		for (int i = 0; i < 4; i++) {
			if ((road = cross->getRoad(i)) != nullptr && road->id != car->status.nextRoadID) {
				car->status.nextRoadID = road->id;
				break;
			}
		}
	}
	assert(car->status.nextRoadID != car->status.roadID);
	
	// 如果没有下一条路，即前方路口是终点
	if (car->status.nextRoadID == -1) {
		Road* currentRoad = Roads[car->status.roadID];
		int speed = currentRoad->speed > car->speed ? car->speed : currentRoad->speed;
		car->status.location += speed;
		if (car->status.location > currentRoad->length) {
			car->finish();
			return false;
		}
		else {
			dispatchFollowingCars(lane);
			return true;
		}
	}
	// 如果当前道路不是规划的下一条道路
	if (car->status.nextRoadID != road->id) {
		return false;
	}
	// 当前道路是一条正常的路
	int2 tmp = car->reachCross(cross);
	switch (tmp.x) {
		case -2:  // 车辆无法到达下一个路口，向前行驶一定距离
			car->status.location += road->speed > car->speed ? car->speed : road->speed; 
			car->answer.stopTime = currentTime;
			dispatchFollowingCars(lane);
			return true;
		case -1:  // 车辆被阻塞在路口，位置更新至路口处
			car->status.location = Roads[car->status.roadID]->length;
			car->answer.stopTime = currentTime;
			dispatchFollowingCars(lane);
			return true;
		default:  // 车辆穿过路口
			car->goThrough(tmp);
			return false;
	}
}

void dispatchFollowingCars(Lane* lane) {
	Road* currentRoad = nullptr;
	Car *car, *front_car = nullptr;
	for (auto it = lane->carsOnLane.begin(); it != lane->carsOnLane.end(); it++) {
		if (it == lane->carsOnLane.begin()) {
			front_car = *it;
			currentRoad = Roads[front_car->status.roadID];
			continue;  // 不再调控第一辆车
		}
		car = *it;
		if (car->answer.stopTime == currentTime) {  // 这辆车以及以后的车是从别的路跑来的
			return;
		}
		int speed = currentRoad->speed > car->speed ? car->speed : currentRoad->speed;
		car->status.location += speed;
		if (car->status.location >= front_car->status.location) {
			car->status.location = front_car->status.location - 1;
		}
		car->answer.stopTime++;
		// assert(car->status.location > 0);
		front_car = car;
	}
}

bool compCarPlantime(const Car* car1, const Car* car2) {
	return car1->planTime < car2->planTime;
}

bool compCarSpeed(const Car* car1, const Car* car2) {
	return car1->speed > car2->speed;
}

bool compCarId(const Car* car1, const Car* car2) {
	return car1->id > car2->id;
}

void fake_process() {
	floyd();
	for (auto it = Cars.begin(); it != Cars.end(); it++) {
		Car* car = it->second;
		car->answer.startTime = car->planTime + (int)((car->id-10000)*0.09);
		car->answer.route = getRouteFloyd(car->from, car->to);
	}
}