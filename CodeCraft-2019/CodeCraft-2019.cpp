#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <map>
#include "structs.h"

#define MAX_LINE_LENGTH 256

using namespace std;

void read_data();
void preprocess();

int main(int argc, char *argv[])
{
	// read input filebuf
	read_data();
	preprocess();

	// data sample usage
	cout << Cars[10000]->getCrossFrom()->id << endl;

	// TODO:process
	// TODO:write output file

	return 0;
}

void read_data() {
	cout << "Begin Reading data!" << endl;

	ifstream fcar("../config/car.txt", ios::in);
	ifstream froad("../config/road.txt", ios::in);
	ifstream fcross("../config/cross.txt", ios::in);

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

void preprocess() {
	// TODO: 生成邻接矩阵等数据预处理
}