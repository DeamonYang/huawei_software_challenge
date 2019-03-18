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
unsigned int finished_cars = 0;
queue<Car*> starting_cars;
// 用map来存放 方便后续查找
map<int, Car*> Cars;
map<int, Road*> Roads;
map<int, Cross*> Crosses;

int **G, **D;

void read_data(string carPath, string roadPath, string crossPath);
void preprocess();
void process();
void output(string answerPath);
vector<int> getRouteFloyd(int, int);

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
	
	// read input filebuf
	read_data(carPath, roadPath, crossPath);
	preprocess();

	// process
	// process();
	// write output file
	output(answerPath);

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
		int int_start, len = strlen(buffer);
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
	}

	// 读取并解析数据: Roads
	while (froad.getline(buffer, MAX_LINE_LENGTH)) {
		if (buffer[0] == '#') continue;
		int int_start, len = strlen(buffer);
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

	// 读取并解析数据: Crosses
	while (fcross.getline(buffer, MAX_LINE_LENGTH)) {
		if (buffer[0] == '#') continue;
		int int_start, len = strlen(buffer);
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

void preprocess() {
	int num_crosses = Crosses.size();
	// 分配空间
	// int G[num_crosses + 1][num_crosses + 1], D[num_crosses + 1][num_crosses + 1];
	G = new int*[num_crosses + 1];
	D = new int*[num_crosses + 1];
	for (int i = 1; i <= num_crosses; i++) {
		G[i] = new int[num_crosses + 1];
		D[i] = new int[num_crosses + 1];
	}

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
		int from = road->from;
		int to = road->to;
		G[from][to] = road->length;
		G[to][from] = road->isDuplex ? road->length : MAX_ROAD_LENGTH;
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

	// 规划路线
	for (auto it = Cars.begin(); it != Cars.end(); it++) {
		Car* car = it->second;
		car->answer.route = getRouteFloyd(car->from, car->to);
	}
}

void process() {
	// TODO: 调度车辆
	for (currentTime = 1; finished_cars < Cars.size(); currentTime++) {
		for (auto it = Crosses.begin(); it != Crosses.end(); it++) {
			Cross* cross = it->second;
			for (int i = 0; cross->dispatch_finished() == false; ++i %= 4) {
				Road* road = cross->getRoad(i);
				if (road == nullptr) {  // 道路不存在
					continue;
				}
				else if (road->to == cross->id && road->isDuplex == 0) {  // 单行道，无法进入
					continue;
				}

				for (auto channel = cross->channelsToRoad[i].begin(); channel != cross->channelsToRoad[i].end(); channel++) {
				    Car* car = (*channel)->front();
					int2 tmp = car->reachCross(cross);
					switch (tmp.y) {
						case -1: 
							break;
						case 0:
							car->status.location = Roads[car->status.roadID]->length;
						default:
							car->goThrough(tmp);
					}
				}
				
				// 要开始行驶的车进入

			}
		}
	}
}

void output(string answerPath) {
	ofstream fanswer(answerPath.c_str());
	fanswer << "#(carId,StartTime,RoadId...)" << endl;
	for (auto it = Cars.begin(); it != Cars.end(); it++) {
		Car* car = it->second;
		fanswer << "(" << car->id << ", ";
		// fanswer << car->answer.startTime;
		fanswer << car->planTime + (car->id-10000)*10;
		// fanswer << car->planTime*car->planTime*car->planTime*car->planTime;
		for (auto it2 = car->answer.route.begin(); it2 != car->answer.route.end(); it2++) {
			fanswer << ", " << *it2;
		}
		fanswer << ")" << endl;
	}
}