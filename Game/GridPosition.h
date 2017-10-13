#pragma once
struct GridPosition {
	int row;
	int col;

	GridPosition operator+(GridPosition otherPosition);
	bool operator==(GridPosition otherPosition);
};