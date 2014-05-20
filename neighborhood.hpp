#pragma once

#include <vector>

class Coord {
public:
	int x;
	int y;
	int w;

	Coord(int x, int y, int w) : x(x), y(y), w(w) {}
};


class Neighborhood {
private:
	std::vector<Coord> v;

public:
	void add(int x, int y, int w) {
		v.push_back(Coord(x, y, w));
	}

	std::vector<Coord>::const_iterator begin() { return v.begin(); }
	std::vector<Coord>::const_iterator end() { return v.end(); }
	size_t size() { return v.size(); }

	typedef std::vector<Coord>::const_iterator const_iterator;
};

