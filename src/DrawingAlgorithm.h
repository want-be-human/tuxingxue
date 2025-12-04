#pragma once
#include <windows.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include "Point.h"

// 绘制算法枚举
enum class LineAlgorithm {
    GDI,          // 使用GDI直接绘制
    Midpoint,     // 中点法
    Bresenham     // Bresenham算法
};

enum class CircleAlgorithm {
    GDI,          // 使用GDI直接绘制
    Midpoint,     // 中点法
    Bresenham     // Bresenham算法
};

enum class FillAlgorithm {
    ScanLine,     // 扫描线法
    Fence         // 栅栏填充法
};

// 裁剪算法枚举
enum class LineClipAlgorithm {
    CohenSutherland,    // Cohen-Sutherland算法
    MidpointSubdivision // 中点分割算法
};

enum class PolygonClipAlgorithm {
    SutherlandHodgman,  // Sutherland-Hodgman算法
    WeilerAtherton      // Weiler-Atherton算法
};

// 绘制算法类
class DrawingAlgorithm {
public:
    // 直线绘制算法
    static void DrawLine(HDC hdc, int x1, int y1, int x2, int y2, LineAlgorithm algorithm, COLORREF color = RGB(0, 0, 0));
    
    // 圆绘制算法
    static void DrawCircle(HDC hdc, int centerX, int centerY, int radius, CircleAlgorithm algorithm, COLORREF color = RGB(0, 0, 0));
    
    // 填充算法
    static void FillPolygon(HDC hdc, const std::vector<Point>& points, FillAlgorithm algorithm, COLORREF color = RGB(100, 100, 255));

    // ==================== 实验二：裁剪算法 ====================
    
    // Cohen-Sutherland 直线裁剪算法
    // 返回值：true表示线段（部分）可见，false表示完全不可见
    // p1, p2会被修改为裁剪后的端点
    static bool ClipLine_CohenSutherland(const Rect& clipRect, Point& p1, Point& p2);
    
    // 中点分割直线裁剪算法
    // 返回值：true表示线段（部分）可见，false表示完全不可见
    // p1, p2会被修改为裁剪后的端点
    static bool ClipLine_MidpointSubdivision(const Rect& clipRect, Point& p1, Point& p2);
    
    // Sutherland-Hodgman 多边形裁剪算法（适用于凸多边形裁剪窗口）
    // 返回值：true表示有可见部分，false表示完全不可见
    // outVerts会存储裁剪后的多边形顶点
    static bool ClipPolygon_SutherlandHodgman(const Rect& clipRect, const std::vector<Point>& inVerts, std::vector<Point>& outVerts);
    
    // Weiler-Atherton 多边形裁剪算法（适用于凹多边形）
    // 返回值：true表示有可见部分，false表示完全不可见
    // outVerts会存储裁剪后的多边形顶点
    static bool ClipPolygon_WeilerAtherton(const Rect& clipRect, const std::vector<Point>& inVerts, std::vector<Point>& outVerts);

private:
    // 画点的辅助函数
    static void SetPixelSafe(HDC hdc, int x, int y, COLORREF color);
    
    // 中点法绘制直线
    static void DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color);
    
    // Bresenham算法绘制直线
    static void DrawLineBresenham(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color);
    
    // 中点法绘制圆
    static void DrawCircleMidpoint(HDC hdc, int centerX, int centerY, int radius, COLORREF color);
    
    // Bresenham算法绘制圆
    static void DrawCircleBresenham(HDC hdc, int centerX, int centerY, int radius, COLORREF color);
    
    // 绘制圆的八个对称点
    static void DrawCirclePoints(HDC hdc, int centerX, int centerY, int x, int y, COLORREF color);
    
    // 扫描线填充算法
    static void FillPolygonScanLine(HDC hdc, const std::vector<Point>& points, COLORREF color);
    
    // 栅栏填充算法
    static void FillPolygonFence(HDC hdc, const std::vector<Point>& points, COLORREF color);
    
    // ==================== 裁剪算法辅助函数 ====================
    
    // Cohen-Sutherland 区域编码
    static const int INSIDE = 0; // 0000
    static const int LEFT = 1;   // 0001
    static const int RIGHT = 2;  // 0010
    static const int BOTTOM = 4; // 0100
    static const int TOP = 8;    // 1000
    
    // 计算点的区域编码
    static int ComputeOutCode(const Rect& rect, const Point& p);
    
    // 中点分割算法的递归辅助函数
    static bool ClipLine_MidpointRecursive(const Rect& clipRect, Point& p1, Point& p2, int depth);
    
    // 判断点是否在裁剪窗口内
    static bool IsInside(const Rect& rect, const Point& p);
    
    // Sutherland-Hodgman 裁剪单条边
    static void ClipPolygonEdge(const std::vector<Point>& input, std::vector<Point>& output, 
                                int edge, const Rect& rect);
    
    // 计算两线段交点
    static Point ComputeIntersection(const Point& p1, const Point& p2, int edge, const Rect& rect);
};

