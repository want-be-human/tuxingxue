#ifndef WEILER_ATHERTON_H
#define WEILER_ATHERTON_H

#include "Point.h"
#include <vector>
#include <memory>

// 顶点类型枚举
enum class VertexType {
    NORMAL,        // 普通顶点
    INTERSECTION   // 交点
};

// 顶点节点
struct Vertex {
    Point point;
    VertexType type;
    bool isEntry;           // 交点是否为进入点
    Vertex* next;           // 同一多边形的下一个顶点
    Vertex* neighbor;       // 对应另一个多边形的交点
    bool visited;           // 是否已访问
    double alpha;           // 交点在边上的参数位置 [0,1]
    
    Vertex(const Point& p, VertexType t = VertexType::NORMAL)
        : point(p), type(t), isEntry(false), next(nullptr), 
          neighbor(nullptr), visited(false), alpha(0.0) {}
};

class WeilerAtherton {
public:
    // 执行 Weiler-Atherton 裁剪算法
    // subject: 被裁剪的多边形
    // clipPoly: 裁剪窗口多边形
    // 返回：裁剪后的多边形列表（可能有多个）
    static std::vector<std::vector<Point>> clip(const std::vector<Point>& subject, const std::vector<Point>& clipPoly);
    
private:
    // 计算两条线段的交点
    static bool lineIntersection(const Point& p1, const Point& p2,
                                  const Point& p3, const Point& p4,
                                  Point& intersection, double& alpha, double& beta);
    
    // 判断点是否在多边形内部
    static bool isPointInPolygon(const Point& p, const std::vector<Point>& poly);
    
    // 构建顶点链表
    static Vertex* buildVertexList(const std::vector<Point>& poly);
    
    // 插入交点
    static void insertIntersections(Vertex* subjectHead, Vertex* clipHead,
                                    const std::vector<Point>& subject, const std::vector<Point>& clip);
    
    // 标记进入/退出点
    static void markEntryExit(Vertex* subjectHead, const std::vector<Point>& clipPoly);
    
    // 遍历并提取裁剪后的多边形
    static std::vector<std::vector<Point>> tracePolygons(Vertex* subjectHead);
    
    // 释放顶点链表
    static void freeVertexList(Vertex* head);
};

#endif // WEILER_ATHERTON_H
