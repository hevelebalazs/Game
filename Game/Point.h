struct Point{
	float x;
	float y;
	Point operator+(Point otherPoint);
	bool operator==(Point otherPoint);
};