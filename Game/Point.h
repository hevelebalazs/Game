struct Point{
	static float distanceSquare(Point point1, Point point2);
	float x;
	float y;
	Point operator+(Point otherPoint);
	bool operator==(Point otherPoint);
};