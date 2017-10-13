#include "GridPosition.h"

GridPosition GridPosition::operator+(GridPosition otherPosition) {
	return { row + otherPosition.row, col + otherPosition.col };
}

bool GridPosition::operator==(GridPosition otherPosition) {
	return col == otherPosition.col && row == otherPosition.row;
}