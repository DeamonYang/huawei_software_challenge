#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>

#define MAX_LINE_LENGTH 256

using namespace std;

class Car {
private:
	int id, from, to, speed, planTime;

public:
	Car(int id, int from, int to, int speed, int planTime) {
		this->id = id;
		this->from = from;
		this->to = to;
		this->speed = speed;
		this->planTime = planTime;
	}
};

class Road {
private:
	int id, length, speed, channel, from, to, isDuplex;

public:
	Road(int id, int length, int speed, int channel, int from, int to, int isDuplex) {
		this->id = id;
		this->length = length;
		this->speed = speed;
		this->channel = channel;
		this->from = from;
		this->to = to;
		this->isDuplex = isDuplex;
	}
};

class Cross {
private:
	int id, roadId1, roadId2, roadId3, roadId4;

public:
	Cross(int id, int roadId1, int roadId2, int roadId3, int roadId4) {
		this->id = id;
		this->roadId1 = roadId1;
		this->roadId2 = roadId2;
		this->roadId3 = roadId3;
		this->roadId4 = roadId4;
	}
};

class Answer {
private:
	int carId, StartTime;
	queue<int> RoadId;
};

int main(int argc, char *argv[])
{
	cout << "Begin" << endl;

	// TODO:read input filebuf
	ifstream fcar("../config/car.txt", ios::in);
	ifstream froad("../config/road.txt", ios::in);
	ifstream fcross("../config/cross.txt", ios::in);

	// 用vector来存放 其实也可以考虑用一个大数组单次分配内存
	vector<Car*> Cars;
	vector<Road*> Roads;
	vector<Cross*> Crosses;

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
		Cars.push_back(new Car(value.at(0), value.at(1), value.at(2), value.at(3), value.at(4)));
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
		Roads.push_back(new Road(value.at(0), value.at(1), value.at(2), value.at(3), value.at(4), value.at(5), value.at(6)));
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
		Crosses.push_back(new Cross(value.at(0), value.at(1), value.at(2), value.at(3), value.at(4)));
	}

	// TODO:process
	// TODO:write output file

	return 0;
}