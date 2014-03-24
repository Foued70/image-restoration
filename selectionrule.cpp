#include "selectionrule.hpp"

using namespace std;

void HighestLevelRule::gap(int h) {
	highest = h - 1;
	updateHighest();
}

void HighestLevelRule::updateHighest(void) {
	int s = highest;
	highest = -1;

	for (int i = s; i >= 0; --i) {
		if (hq[i].size() > 0) {
			highest = i;
			break;
		}
	}
}

bool HighestLevelRule::empty(void) {
	return highest < 0;
}

int HighestLevelRule::next(void) {
	if (empty()) throw EmptyQueueException();

	int u = hq[highest].front();
	hq[highest].pop();
	deactivate(u);

	updateHighest();

	return u;
}

void HighestLevelRule::add(int u, int height, int excess) {
	if (isActive(u)) return;
	if (!inSegment(u)) return;
	if (height >= N) return;
	if (excess == 0) return;

	hq[height].push(u);
	if (height > highest) highest = height;
}

void FIFORule::gap(int h) {
}

int FIFORule::next(void) {
	int u;

	if (!q.empty()) {
		u = q.front();
		q.pop();
		deactivate(u);
	}
	else {
		throw EmptyQueueException();
	}

	return u;
}

void FIFORule::add(int u, int height, int excess) {
	if (isActive(u)) return;
	//if (!inSegment(u)) return;
	if (height >= N) return;
	if (excess == 0) return;

	activate(u);
	q.push(u);
}

bool FIFORule::empty(void) {
	return q.empty();
}

