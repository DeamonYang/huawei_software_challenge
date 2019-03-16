#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include "structs.h"

#define MAX_LINE_LENGTH 256
#define MAX_ROAD_LENGTH 0x0fff

using namespace std;

int **G, **D;

void read_data();
void preprocess();
void dispatch();
void output();
vector<int> getRouteFloyd(int, int);

int main(int argc, char *argv[])
{
	// read input filebuf
	read_data();
	preprocess();

	// TODO:process
	// write output file
	output();

	return 0;
}

void read_data() {
	cout << "Begin Reading data!" << endl;

	ifstream fcar("../config/car.txt");
	ifstream froad("../config/road.txt");
	ifstream fcross("../config/cross.txt");

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
	v.push_back(i);
	if (D[i][j] != j) {
		int k = D[i][j];
		vector<int> v1, v2;
		v1 = getRouteFloyd(i, k);
		v2 = getRouteFloyd(k, j);
		v.clear();
		v.insert(v.end(),v1.begin(),v1.end()-1);
		v.insert(v.end(),v2.begin(),v2.end()-1);
	}
	v.push_back(j);
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

void dispatch() {
	// TODO: 调度车辆
}

void output() {
	ofstream fanswer("../config/answer.txt");
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