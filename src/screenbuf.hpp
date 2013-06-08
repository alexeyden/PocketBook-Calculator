#ifndef SCREENBUF_HPP
#define SCREENBUF_HPP
#include <vector>
#include <string>
#include <utility>

using std::vector;
using std::string;
using std::pair;

typedef pair<unsigned short, unsigned short> buf_pos;

class Screenbuf {
public:
	Screenbuf(unsigned width, unsigned height) {
		m_buffer = new vector<string>(height);
		c_width = width;
		c_height = height;
	}
	
	string& operator[](unsigned row) {
		return m_buffer->at(row);
	}

	bool next(buf_pos& pos) {
		if((pos.second + 1) >= (*m_buffer)[pos.first].size()) {
			if(pos.first+1  >= (*m_buffer).size())
				return false;
			else {
				pos.first++;
				pos.second = 0;
			}
		}
		else
			pos.second++;
		
		return true;
	}
	
	bool prev(buf_pos& pos) {
	}
	
	bool nextLine(buf_pos& pos) {
	}
	
	bool prevLine(buf_pos& pos) {
		
	}
	
	~Screenbuf() {
		delete m_buffer;
	}
	
private:
	bool index_valid(size_t index) {
		if(index < c_width*c_height)
			return true;
		return false;
	}
	
	unsigned c_width;
	unsigned c_height;
	vector<string>* m_buffer;
};


#endif