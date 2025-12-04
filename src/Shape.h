#pragma once
#include <windows.h>
#include <vector>
#include <memory>
#include "Point.h"
#include "DrawingAlgorithm.h"

// 图形基类
class Shape {
public:
    virtual ~Shape() {}
    virtual void Draw(HDC hdc) = 0;
    virtual bool IsComplete() const = 0;
    virtual void AddPoint(const Point& p) = 0;
    virtual void SetPreviewPoint(const Point& p) = 0;
    virtual void DrawPreview(HDC hdc) = 0;
    
    // ==================== 实验二：几何变换接口 ====================
    // 平移
    virtual void Translate(int dx, int dy) = 0;
    // 缩放（相对于中心点）
    virtual void Scale(double sx, double sy, const Point& center) = 0;
    // 旋转（相对于中心点，角度为弧度）
    virtual void Rotate(double angleRad, const Point& center) = 0;
    // 获取图形中心点（用于变换）
    virtual Point GetCenter() const = 0;
    // 命中测试（用于选择图形）
    virtual bool HitTest(const Point& p, int tolerance = 5) const = 0;
    // 设置选中状态
    virtual void SetSelected(bool selected) { isSelected = selected; }
    virtual bool IsSelected() const { return isSelected; }
    
protected:
    bool isSelected = false;
};

// 直线类
class Line : public Shape {
private:
    Point start;
    Point end;
    bool hasStart;
    bool complete;
    LineAlgorithm algorithm;
    Point previewEnd;
    
public:
    Line(LineAlgorithm algo = LineAlgorithm::GDI);
    void Draw(HDC hdc) override;
    void DrawPreview(HDC hdc) override;
    bool IsComplete() const override;
    void AddPoint(const Point& p) override;
    void SetPreviewPoint(const Point& p) override;
    void SetAlgorithm(LineAlgorithm algo);
    
    // 实验二：变换接口实现
    void Translate(int dx, int dy) override;
    void Scale(double sx, double sy, const Point& center) override;
    void Rotate(double angleRad, const Point& center) override;
    Point GetCenter() const override;
    bool HitTest(const Point& p, int tolerance = 5) const override;
    
    // 获取端点（用于裁剪）
    Point GetStart() const { return start; }
    Point GetEnd() const { return end; }
    void SetEndpoints(const Point& p1, const Point& p2) { start = p1; end = p2; }
};

// 圆类
class Circle : public Shape {
private:
    Point center;
    int radius;
    bool hasCenter;
    bool complete;
    CircleAlgorithm algorithm;
    Point previewPoint;
    
    int CalculateRadius(const Point& p1, const Point& p2);
    
public:
    Circle(CircleAlgorithm algo = CircleAlgorithm::GDI);
    void Draw(HDC hdc) override;
    void DrawPreview(HDC hdc) override;
    bool IsComplete() const override;
    void AddPoint(const Point& p) override;
    void SetPreviewPoint(const Point& p) override;
    void SetAlgorithm(CircleAlgorithm algo);
    
    // 实验二：变换接口
    void Translate(int dx, int dy) override;
    void Scale(double sx, double sy, const Point& center) override;
    void Rotate(double angleRad, const Point& center) override;
    Point GetCenter() const override;
    bool HitTest(const Point& p, int tolerance = 5) const override;
    
    // 获取圆的参数用于填充
    int GetRadius() const;
};

// 矩形类
class Rectangle : public Shape {
private:
    Point topLeft;
    Point bottomRight;
    bool hasFirstPoint;
    bool complete;
    Point previewPoint;
    
public:
    Rectangle();
    void Draw(HDC hdc) override;
    void DrawPreview(HDC hdc) override;
    bool IsComplete() const override;
    void AddPoint(const Point& p) override;
    void SetPreviewPoint(const Point& p) override;
    
    // 实验二：变换接口
    void Translate(int dx, int dy) override;
    void Scale(double sx, double sy, const Point& center) override;
    void Rotate(double angleRad, const Point& center) override;
    Point GetCenter() const override;
    bool HitTest(const Point& p, int tolerance = 5) const override;
    
    // 获取矩形的顶点用于填充
    Point GetTopLeft() const;
    Point GetBottomRight() const;
};

// 多段线类
class Polyline : public Shape {
private:
    std::vector<Point> points;
    bool closed;
    
public:
    Polyline();
    void Draw(HDC hdc) override;
    void DrawPreview(HDC hdc) override;
    bool IsComplete() const override;
    void AddPoint(const Point& p) override;
    void SetPreviewPoint(const Point& p) override;
    void Close();
    
    // 实验二：变换接口
    void Translate(int dx, int dy) override;
    void Scale(double sx, double sy, const Point& center) override;
    void Rotate(double angleRad, const Point& center) override;
    Point GetCenter() const override;
    bool HitTest(const Point& p, int tolerance = 5) const override;
    
    const std::vector<Point>& GetPoints() const;
    size_t GetPointCount() const;
};

// ==================== 实验二：任意多边形类 ====================
// 通过鼠标点击添加顶点，右键或双击结束并自动闭合
class Polygon : public Shape {
private:
    std::vector<Point> vertices;   // 顶点
    bool complete;
    Point previewPoint;
    
public:
    Polygon();
    void Draw(HDC hdc) override;
    void DrawPreview(HDC hdc) override;
    bool IsComplete() const override;
    void AddPoint(const Point& p) override;
    void SetPreviewPoint(const Point& p) override;
    void Close();  // 结束输入并闭合多边形
    
    // 实验二：变换接口
    void Translate(int dx, int dy) override;
    void Scale(double sx, double sy, const Point& center) override;
    void Rotate(double angleRad, const Point& center) override;
    Point GetCenter() const override;
    bool HitTest(const Point& p, int tolerance = 5) const override;
    
    // 获取顶点（用于裁剪）
    const std::vector<Point>& GetVertices() const { return vertices; }
    void SetVertices(const std::vector<Point>& verts) { vertices = verts; }
    size_t GetVertexCount() const { return vertices.size(); }
};

// B样条曲线类
class BSpline : public Shape {
private:
    std::vector<Point> controlPoints;     // 控制点
    std::vector<Point> curvePoints;       // 曲线上的标记点
    bool complete;
    int minPoints;
    
    // 使用4个控制点和参数t计算曲线上的点(三次均匀B样条)
    Point CalculateCurvePoint(const Point& p0, const Point& p1, const Point& p2, const Point& p3, double t);
    
public:
    BSpline(int minPts = 4);
    void Draw(HDC hdc) override;
    void DrawPreview(HDC hdc) override;
    bool IsComplete() const override;
    void AddPoint(const Point& p) override;
    void SetPreviewPoint(const Point& p) override;
    void Finish();
    
    // 实验二：变换接口
    void Translate(int dx, int dy) override;
    void Scale(double sx, double sy, const Point& center) override;
    void Rotate(double angleRad, const Point& center) override;
    Point GetCenter() const override;
    bool HitTest(const Point& p, int tolerance = 5) const override;
    
    size_t GetPointCount() const;
};

// 填充区域类(用于封闭图形的填充)
class FilledRegion : public Shape {
private:
    std::vector<Point> points;
    FillAlgorithm algorithm;
    bool complete;
    COLORREF fillColor;
    
public:
    FilledRegion(const std::vector<Point>& pts, FillAlgorithm algo, COLORREF color = RGB(100, 100, 255));
    void Draw(HDC hdc) override;
    void DrawPreview(HDC hdc) override;
    bool IsComplete() const override;
    void AddPoint(const Point& p) override;
    void SetPreviewPoint(const Point& p) override;
    
    // 实验二：变换接口（填充区域不需要变换）
    void Translate(int dx, int dy) override {}
    void Scale(double sx, double sy, const Point& center) override {}
    void Rotate(double angleRad, const Point& center) override {}
    Point GetCenter() const override { return points.empty() ? Point() : points[0]; }
    bool HitTest(const Point& p, int tolerance = 5) const override { return false; }
};
