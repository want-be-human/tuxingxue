#pragma once
#include <cmath>

// 基础点类
class Point {
public:
    int x;
    int y;

    Point() : x(0), y(0) {}
    Point(int _x, int _y) : x(_x), y(_y) {}

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
    
    // 几何变换辅助函数
    // 平移
    Point Translate(int dx, int dy) const {
        return Point(x + dx, y + dy);
    }
    
    // 缩放（相对于中心点）
    Point Scale(double sx, double sy, const Point& center) const {
        int newX = center.x + (int)((x - center.x) * sx);
        int newY = center.y + (int)((y - center.y) * sy);
        return Point(newX, newY);
    }
    
    // 旋转（相对于中心点，角度为弧度）
    Point Rotate(double angleRad, const Point& center) const {
        double s = sin(angleRad);
        double c = cos(angleRad);
        int dx = x - center.x;
        int dy = y - center.y;
        int newX = center.x + (int)(dx * c - dy * s);
        int newY = center.y + (int)(dx * s + dy * c);
        return Point(newX, newY);
    }
    
    // 计算两点间距离
    double DistanceTo(const Point& other) const {
        int dx = x - other.x;
        int dy = y - other.y;
        return sqrt(dx * dx + dy * dy);
    }
};

// 矩形类（用于裁剪窗口）
class Rect {
public:
    int left, top, right, bottom;
    
    Rect() : left(0), top(0), right(0), bottom(0) {}
    Rect(int l, int t, int r, int b) : left(l), top(t), right(r), bottom(b) {}
    Rect(const Point& p1, const Point& p2) {
        left = (p1.x < p2.x) ? p1.x : p2.x;
        right = (p1.x > p2.x) ? p1.x : p2.x;
        top = (p1.y < p2.y) ? p1.y : p2.y;
        bottom = (p1.y > p2.y) ? p1.y : p2.y;
    }
    
    int Width() const { return right - left; }
    int Height() const { return bottom - top; }
    Point Center() const { return Point((left + right) / 2, (top + bottom) / 2); }
    
    bool Contains(const Point& p) const {
        return p.x >= left && p.x <= right && p.y >= top && p.y <= bottom;
    }
};
