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
list<Car*> CarsNotReady, CarsReady, CarsRunning;
// 用map来存放 方便后续查找
map<int, Car*> Cars;
map<int, Road*> Roads;
map<int, Cross*> Crosses;
map<int, int> crossId2Num;

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
void dispatchFollowingCars(Cross*, Road*, int);
void fake_process();
void refreshCarsReady();
bool conflict(Cross*, Road*, Car* car);
bool dispatch(Cross*, Road*);
bool dispatch_car(Cross*, Road*, Car* car);

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
	
	// #ifdef DEBUG
	clock_t start, end;
	start = clock();
	// #endif

	// read input filebuf
	read_data(carPath, roadPath, crossPath);
	preprocess();

	// process
	// fake_process();
	process();
	// write output file
	output(answerPath);

	// print information
	// #ifdef DEBUG
	end = clock();
	int total_car_time = 0;
	for (auto it = Cars.begin(); it != Cars.end(); it++) {
		total_car_time += (it->second->answer.stopTime - it->second->planTime);
	}
	cout << "系统调度时间: " << currentTime - 1 << endl;
	cout << "所有车辆总调度时间: " << total_car_time << endl;
	cout << "程序运行时间: " << (end - start)/1000 << endl;
	// #endif

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
		CarsNotReady.push_back(Cars[value.at(0)]);
	}
	fcar.close();

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

	// 读取并解析数据: Crosses
	int num = 1;
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
		Crosses[num] = new Cross(num, value.at(1), value.at(2), value.at(3), value.at(4));
		crossId2Num[value.at(0)] = num;
		num++;
	}
	fcross.close();
}

void preprocess() {
	// 适配不连续的cross_id
	for (auto it = Roads.begin(); it != Roads.end(); it++) {
		Road* road = it->second;
		road->from = crossId2Num[road->from];
		road->to = crossId2Num[road->to];
	}
	for (auto it = Cars.begin(); it != Cars.end(); it++) {
		Car* car = it->second;
		car->from = crossId2Num[car->from];
		car->to = crossId2Num[car->to];
	}
	for (auto it = Crosses.begin(); it != Crosses.end(); it++) {
		// 初始化 total_roads
		for (int i = 0; i < 4; i++) {
			Road* road = it->second->getRoad(i);
			if (road == nullptr) continue;
			if (road->from == it->second->id && road->isDuplex == 0) continue;
			it->second->total_roads++;
		}
	}

	int num_crosses = Crosses.size();
	// 分配空间
	G = new int*[num_crosses + 1];
	D = new int*[num_crosses + 1];
	nextRoad = new int*[num_crosses + 1];
	for (int i = 1; i <= num_crosses; i++) {
		G[i] = new int[num_crosses + 1];
		D[i] = new int[num_crosses + 1];
		nextRoad[i] = new int[num_crosses + 1];
	}
	floyd();
}

void process() {
	// 对所有车辆按照准备出发时间进行排序
	// CarsNotReady.sort(compCarPlantime);
	// 开始进行系统调度
	for (currentTime = 0; finished_cars < Cars.size(); currentTime++) {
		#ifdef DEBUG
		static int last_running = 0, last_finished = 0;
		cout << "currentTime: " << currentTime << "\t deltaCarsRunning: " << (int)CarsRunning.size() - last_running << "\t deltaCarsFinished: " << finished_cars - last_finished << endl;
		last_running = CarsRunning.size();
		last_finished = finished_cars;
		vector<int2> status;
		for (auto it = CarsRunning.begin(); it != CarsRunning.end(); it++) {
			status.push_back({(*it)->status.roadID, (*it)->status.location});
		}
		#endif
		// 更新CarsReady
		refreshCarsReady();
		// 更新路径矩阵
		// floyd();

		// 对道路上的车进行调度
		dispatchCarsOnRoad();
		// 启动一些尚未出发的车
		dispatchCarsInGarage();

		#ifdef DEBUG
		int i = 0, lock_flag = 0;
		if (!status.empty() && status.size() == CarsRunning.size()) {
			lock_flag = 1;
			for (auto it = CarsRunning.begin(); it != CarsRunning.end(); it++, i++) {
				if (status[i].x != (*it)->status.roadID || status[i].y != (*it)->status.location) {
					lock_flag = 0;
					break;
				}
			}
		}
		if (lock_flag) {
			cout << "死锁!!!" << endl;
			exit(1);
		}
		#endif
	}
}

void dispatchCarsOnRoad() {
	// 第一步: 遍历各道路，确定所有车辆的行驶状态
	for (auto it = Roads.begin(); it != Roads.end(); it++) {
		Road* road = it->second;
		Car*** roadMap = road->roadMap;

		for (int j = road->length-1; j >= 0; j--) {
			for (int i = 0; i < road->channel * (1 + road->isDuplex); i++) {
				Car* car = roadMap[i][j];
				if (car == nullptr) continue;

				int speed = road->speed > car->speed ? car->speed : road->speed;
				for (int k = j + 1; ; k++) {
					// 该车道前方没有车辆
					if (k == road->length) {
						// a) 可以出路口
						if (j + speed >= road->length) {
							car->status.isWaiting = true;
							road->carsWaitingForDispatched[i < road->channel ? 0 : 1].push_back(car);
							break;
						}
						// b) 不能出路口
						else {
							car->status.isWaiting = false;
							car->status.location += speed;
							roadMap[i][j] = nullptr;
							assert(roadMap[i][j+speed] == nullptr);
							roadMap[i][j+speed] = car;
							break;
						}

					}
					// 该车道前方有车辆
					if (roadMap[i][k] != nullptr) {
						// c) 被等待状态车辆所阻挡
						if (roadMap[i][k]->status.isWaiting && (k - j <= speed)) {
							car->status.isWaiting = true;
							road->carsWaitingForDispatched[i < road->channel ? 0 : 1].push_back(car);
							break;
						}
						// d) 被终止状态车辆所阻挡或等待状态车辆距离太远，不会造成阻挡
						else {
							int s = k - j - 1 < speed ? k - j - 1 : speed;  // 可行驶的距离
							car->status.isWaiting = false;
							car->status.location += s;
							roadMap[i][j] = nullptr;
							assert(roadMap[i][j+s] == nullptr);
							roadMap[i][j+s] = car;
							break;
						}
					}
				}
			}
		}
	}

	// 第二步: 遍历各路口，调度等待状态的车辆
	// 注意: 此处要求Cross的id从1开始依次递增
	static const int num_crosses = Crosses.size();
	for (int i = 1, dispatched_crosses = 0; dispatched_crosses < num_crosses; i %= num_crosses, i++) {
		Cross* cross = Crosses[i];
		if (cross->dispatched_times == currentTime) continue;  // 这里性能有提升空间
		Road* road;
		for (int j = 0; j < 4; j++) {
			if (cross->rankDist[j] == -1 || (road = cross->getRoad(cross->rankDist[j])) == nullptr) continue;  // 道路不存在
			if (road->from == cross->id && road->isDuplex == 0) continue;  // 单行道，无法进入路口

			// 如果进入该路口的所有道路调度完毕，则该路口调度完毕
			if (cross->dispatched_roads == cross->total_roads) {
				cross->dispatched_times++;
				cross->dispatched_roads = 0;
				dispatched_crosses++;
				break;
			}
			
			// 道路上没有需要调度的车, 则该道路视作调度完成
			bool road_direction = !road->isPositiveDirection(cross->id);
			if (road->carsWaitingForDispatched[road_direction ? 0 : 1].empty()) {
				if (road->dispatched_times[road_direction ? 0 : 1] < currentTime) {
					road->dispatched_times[road_direction ? 0 : 1] = currentTime;
					cross->dispatched_roads++;
				}
				continue;
			}

			if (dispatch(cross, road)) {
				road->dispatched_times[road_direction ? 0 : 1] = currentTime;
				cross->dispatched_roads++;
			}
		}
	}
}


bool dispatch(Cross* cross, Road* road) {
	bool road_direction = !road->isPositiveDirection(cross->id);
	if (road->carsWaitingForDispatched[road_direction ? 0 : 1].empty()) return true;
	Car* car = road->carsWaitingForDispatched[road_direction ? 0 : 1].front();

	// 先看能否行驶到路口
	int speed = road->speed > car->speed ? car->speed : road->speed;
	if (car->status.location + speed <= road->length) {
		// 到不了路口，继续向前行驶
		car->status.isWaiting = false;
		car->status.location += speed;
		road->roadMap[car->status.channelNum][car->status.location - 1 - speed] = nullptr;
		assert(road->roadMap[car->status.channelNum][car->status.location - 1] == nullptr);
		road->roadMap[car->status.channelNum][car->status.location - 1] = car;
		road->carsWaitingForDispatched[road_direction ? 0 : 1].pop_front();
		dispatchFollowingCars(cross, road, car->status.channelNum);
		return dispatch(cross, road);
	}

	car->status.nextRoadID = nextRoad[cross->id][car->to];
	if (conflict(cross, road, car)) 
		return false;

	if (nextRoad[cross->id][car->to] == -1) {
		car->finish();
		road->carsWaitingForDispatched[road_direction ? 0 : 1].pop_front();
		return dispatch(cross, road);
	}
	else {
		int2 tmp = car->reachCross(cross);
		assert(tmp.x != -2);
		switch (tmp.x) {
			case -1:  // 车辆被阻塞在路口，位置更新至路口处
				car->status.isWaiting = false;
				road->carsWaitingForDispatched[road_direction ? 0 : 1].pop_front();
				// car->answer.stopTime++;
				road->roadMap[car->status.channelNum][car->status.location - 1] = nullptr;
				assert(road->roadMap[car->status.channelNum][road->length - 1] == nullptr);
				road->roadMap[car->status.channelNum][road->length - 1] = car;
				car->status.location = road->length;
				dispatchFollowingCars(cross, road, car->status.channelNum);
				return dispatch(cross, road);
			case -3:  // 车辆理论上能通过路口，但是前方有处于等待状态的车辆阻挡，等待下次调度
				return false;
			default:  // 车辆穿过路口
				assert(tmp.y > 0);
				car->status.isWaiting = false;
				road->carsWaitingForDispatched[road_direction ? 0 : 1].pop_front();
				road->roadMap[car->status.channelNum][car->status.location - 1] = nullptr;
				assert(Roads[car->status.nextRoadID]->roadMap[tmp.x][tmp.y - 1] == nullptr);
				Roads[car->status.nextRoadID]->roadMap[tmp.x][tmp.y - 1] = car;
				car->status.roadID = car->status.nextRoadID;
				car->status.channelNum = tmp.x;
				car->status.location = tmp.y;
				car->answer.route.push_back(car->status.roadID);
				return dispatch(cross, road);
		}
	}
}

bool conflict(Cross* cross, Road* road, Car* car) {
	if (nextRoad[cross->id][car->to] == -1) return false;  // 前往终点则不会冲突 
	int front_road_id = cross->getFrontRoad(road);
	int left_road_id = cross->getLeftRoad(road);
	int right_road_id = cross->getRightRoad(road);
	if (car->status.nextRoadID == front_road_id) {
		return false;
	}
	if (car->status.nextRoadID == left_road_id) {
		if (right_road_id != -1) {
			Road* right_road = Roads[right_road_id];
			bool right_road_direction = !right_road->isPositiveDirection(cross->id);
			if (!right_road->carsWaitingForDispatched[right_road_direction ? 0 : 1].empty()) {
				if (right_road->carsWaitingForDispatched[right_road_direction ? 0 : 1].front()->status.nextRoadID == left_road_id) {
					return true;
				}
			}
		}
		return false;
	}
	if (car->status.nextRoadID == right_road_id) {
		if (left_road_id != -1) {
			Road* left_road = Roads[left_road_id];
			bool left_road_direction = !left_road->isPositiveDirection(cross->id);
			if (!left_road->carsWaitingForDispatched[left_road_direction ? 0 : 1].empty()) {
				if (left_road->carsWaitingForDispatched[left_road_direction ? 0 : 1].front()->status.nextRoadID == right_road_id) {
					return true;
				}
			}
		}
		if (front_road_id != -1) {
			Road* front_road = Roads[front_road_id];
			bool front_road_direction = !front_road->isPositiveDirection(cross->id);
			if (!front_road->carsWaitingForDispatched[front_road_direction ? 0 : 1].empty()) {
				if (front_road->carsWaitingForDispatched[front_road_direction ? 0 : 1].front()->status.nextRoadID == right_road_id) {
					return true;
				}
			}
		}
		return false;
	}
	assert(0);
	return true;
}

void dispatchCarsInGarage() {
	if (CarsReady.size() == 0) return;
	for (auto it = CarsReady.begin(); it != CarsReady.end(); ) {
		Car* car = *it;
		if (car->start()) {
			it = CarsReady.erase(it);
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
	// assert(0);
	return -3;  // ERROR
}

void refreshCarsReady() {
	if (CarsNotReady.size() == 0) return;
	for (auto it = CarsNotReady.begin(); it != CarsNotReady.end(); ) {
		if ((*it)->planTime <= currentTime) {
			CarsReady.push_back(*it);
			it = CarsNotReady.erase(it);
		}
		else {
			it++;
		}
	}
	// 对所有已准备好出发的车辆按照最大速度进行排序
	CarsReady.sort(compCarSpeed);
	
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
		G[road->from][road->to] = road->length;
		G[road->to][road->from] = road->isDuplex == 1 ? road->length : MAX_ROAD_LENGTH;

	// 先不考虑加权
	// 	long double num = 0;
	// 	for (int i = 0; i < road->channel; i++) {
	// 		if (!road->roadMap[i].carsOnLane.empty()) {
	// 			Lane* lane = &road->roadMap[i];
	// 			for (auto it = lane->carsOnLane.begin(); it != lane->carsOnLane.end(); it++) {
	// 				Car* car = *it;
	// 				num += car->status.location*car->status.location;
	// 				// num += pow(car->status.location, 1.2);
	// 			}
	// 		}
	// 	}
	// 	assert(num >= 0);
	// 	road->length_weight[0] = num + 1;
	// 	if (road->isDuplex) {
	// 		for (int i = road->channel, num = 0; i < 2 * road->channel; i++) {
	// 			if (!road->roadMap[i].carsOnLane.empty()) {
	// 				Lane* lane = &road->roadMap[i];
	// 				for (auto it = lane->carsOnLane.begin(); it != lane->carsOnLane.end(); it++) {
	// 					Car* car = *it;
	// 					num += car->status.location*car->status.location;
	// 					// num += pow(car->status.location, 1.2);
	// 				}
	// 			}
	// 		}
	// 		assert(num >= 0);
	// 		road->length_weight[1] = num + 1;
	// 	}


		// G[road->from][road->to] = road->length * road->length_weight[0];
		// G[road->to][road->from] = road->isDuplex == 1 ? road->length * road->length_weight[1] : MAX_ROAD_LENGTH;
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

void dispatchFollowingCars(Cross* cross, Road* currentRoad, int channelNum) {
	bool road_direction = !currentRoad->isPositiveDirection(cross->id);
	Car *car, *front_car = nullptr;
	Car*** roadMap = currentRoad->roadMap;
	bool flag = false;
	for (int j = currentRoad->length - 1; j >= 0; j--) {
		if (roadMap[channelNum][j] == nullptr) continue;
		if (!flag) {
			front_car = roadMap[channelNum][j];
			flag = true;
			continue;
		}
		assert(front_car->status.isWaiting == false);
		car = roadMap[channelNum][j];
		if (car->status.isWaiting == false) continue;

		int speed = currentRoad->speed > car->speed ? car->speed : currentRoad->speed;
		if (car->status.location + speed >= front_car->status.location) {
			car->status.location = front_car->status.location - 1;
		}
		else {
			car->status.location += speed;
		}
		car->status.isWaiting = false;
		list<Car*>* l = &currentRoad->carsWaitingForDispatched[road_direction ? 0 : 1];
		for (auto it = l->begin(); it != l->end(); it++) {
			if ((*it)->id == car->id) {
				l->erase(it);
				break;
			}
		}
		currentRoad->roadMap[channelNum][j] = nullptr;
		assert(currentRoad->roadMap[channelNum][car->status.location - 1] == nullptr);
		currentRoad->roadMap[channelNum][car->status.location - 1] = car;
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
		car->answer.startTime = car->planTime + (int)((car->id-10000)*0.05);
		car->answer.route = getRouteFloyd(car->from, car->to);
	}
}