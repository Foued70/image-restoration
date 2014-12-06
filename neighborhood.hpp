#pragma once

#include <iostream>
#include <vector>
#include <cmath>

class Coord {
public:
	int x;
	int y;
	int w;
	mutable double dt;

	Coord() : x(0), y(0), w(0), dt(0.0) {}
	Coord(int x, int y, int w) : x(x), y(y), w(w), dt(0.0) {}

	double angle() {
		return atan2(y, x);
	}
};

class CoordCompare {
public:
	bool operator()(const Coord& lhs, const Coord& rhs) {
		double p1 = copysign(1.0 - lhs.x/(fabs(lhs.x) + fabs(lhs.y)), lhs.y);
		double p2 = copysign(1.0 - rhs.x/(fabs(rhs.x) + fabs(rhs.y)), rhs.y);

		return p1 < p2;
	}
};

class Neighborhood {
private:
	std::set<Coord, CoordCompare> v;

public:
	void add(int x, int y, int w) {
		v.insert(Coord(x, y, w));
	}

	void add8(int x, int y, int w) {
		v.insert(Coord( x, y, w));
		v.insert(Coord(-x, y, w));
		v.insert(Coord( x,-y, w));
		v.insert(Coord(-x,-y, w));
		v.insert(Coord( y, x, w));
		v.insert(Coord(-y, x, w));
		v.insert(Coord( y,-x, w));
		v.insert(Coord(-y,-x, w));
	}

	std::set<Coord, CoordCompare>::iterator begin() { return v.begin(); }
	std::set<Coord, CoordCompare>::iterator end() { return v.end(); }
	std::set<Coord, CoordCompare>::reverse_iterator rbegin() { return v.rbegin(); }
	std::set<Coord, CoordCompare>::reverse_iterator rend() { return v.rend(); }
	size_t size() { return v.size(); }

	typedef std::set<Coord, CoordCompare>::iterator iterator;

	void setupAngles() {
		for (Neighborhood::iterator it = this->begin();
				it != this->end();
				++it) {
			Coord prev;
			Coord next;

			if (it == this->begin()) {
				prev = *(this->rbegin());
			} else {
				prev = *(--it);
				it++;
			}

			if (++it == this->end()) {
				next = *(this->begin());
			} else {
				next = *it;
			}
			it--;

			it->dt = next.angle() - prev.angle();
			while (it->dt > 2.0 * M_PI)
				it->dt -= 2.0 * M_PI;
			while (it->dt < 0.0)
				it->dt += 2.0 * M_PI;
			it->dt /= 2.0;
		}
	}
};

