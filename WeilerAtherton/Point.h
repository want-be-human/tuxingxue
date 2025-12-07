#ifndef POINT_H
#define POINT_H

#include <vector>
#include <cmath>

// 二维点结构
struct Point {
    double x, y;
    
    Point() : x(0), y(0) {}
    Point(double _x, double _y) : x(_x), y(_y) {}
    
    bool operator==(const Point& other) const {
        const double EPSILON = 1e-9;
        return std::abs(x - other.x) < EPSILON && std::abs(y - other.y) < EPSILON;
    }
    
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

// 多边形类型定义 (避免与 Windows GDI 的 Polygon 函数冲突)
typedef std::vector<Point> PolygonShape;

#endif // POINT_H
