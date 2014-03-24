#pragma once

#include <iostream>
#include <queue>
#include <vector>
#include <exception>

class EmptyQueueException : public std::exception {
	virtual const char* what() const throw()
	{
		return "Empty Queue Exception";
	}
};

class SelectionRule {

private:
	std::vector<char> active;
	std::vector<char> *segment;

protected:
	int N;

public:
	virtual int next(void) = 0;
	virtual void add(int u, int height, int excess) = 0;
	virtual bool empty(void) = 0;
	virtual void gap(int h) = 0;

	void activate(int u) { active[u] = 1; }
	void deactivate(int u) { active[u] = 0; }
	bool isActive(int u) { return active[u]; }
	void setSegment(std::vector<char> *s) { segment = s; }
	bool inSegment(int u) { return (*segment)[u]; }

	SelectionRule(int N) : N(N), active(N) {}

	virtual ~SelectionRule() {}
};

class HighestLevelRule : virtual public SelectionRule {

private:
	int highest;
	std::vector<std::queue<int> > hq;

	void updateHighest(void);

public:
	virtual int next(void);
	virtual void add(int u, int height, int excess);
	virtual bool empty(void);
	virtual void gap(int h);

	HighestLevelRule(int N) : SelectionRule(N), highest(0), hq(N) {}
};

class FIFORule : virtual public SelectionRule {

private:
	std::queue<int> q;

public:
	virtual int next(void);
	virtual void add(int u, int height, int excess);
	virtual bool empty(void);
	virtual void gap(int h);

	FIFORule(int N) : SelectionRule(N) {}
};

