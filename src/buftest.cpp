#include "screenbuf.hpp"
#include <iostream>

using namespace std;

int main() {
	Screenbuf buf(8,10);
	buf[0] = "hello";
	buf[1] = "world kd";
	buf_pos buf_pos1 = make_pair(0,0);
	buf.next(buf_pos1);
	buf.next(buf_pos1);
	buf.next(buf_pos1);
	buf.next(buf_pos1);
	buf.next(buf_pos1);
	
	for(int i = 0; i < 10; i++) {
		if(i == 0 || i ==9)
			cout << "--------" << endl;
		for(int j = 0; j < 8; j++) {
			if(buf_pos1.first == i && buf_pos1.second == j)
				cout << '@';
			else
				cout << buf[i][j];
		}
		cout << endl;
	}
	return 0;
}