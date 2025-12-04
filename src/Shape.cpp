#include "Shape.h"
#include <cmath>

// ============ Line 类实现 ============
Line::Line(LineAlgorithm algo)
    : hasStart(false), complete(false), algorithm(algo), start(0, 0), end(0, 0), previewEnd(0, 0) {}

void Line::Draw(HDC hdc) {
    if (complete) {
        // 根据算法选择不同颜色
        COLORREF color = RGB(0, 0, 0);  // GDI - 黑色
        if (algorithm == LineAlgorithm::Midpoint) {
            color = RGB(255, 0, 0);  // 中点法 - 红色
        } else if (algorithm == LineAlgorithm::Bresenham) {
            color = RGB(0, 0, 255);  // Bresenham - 蓝色
        }
        
        // 如果被选中，使用更粗的线条
        if (isSelected) {
            HPEN hPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 255)); // 紫红色高亮
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
            MoveToEx(hdc, start.x, start.y, NULL);
            LineTo(hdc, end.x, end.y);
            SelectObject(hdc, hOldPen);
            DeleteObject(hPen);
        }
        
        DrawingAlgorithm::DrawLine(hdc, start.x, start.y, end.x, end.y, algorithm, color);
    }
}

void Line::DrawPreview(HDC hdc) {
    // 只有在已经有起点且预览点已设置时才绘制预览虚线
    if (hasStart && !complete && (previewEnd.x != 0 || previewEnd.y != 0)) {
        HPEN hPen = CreatePen(PS_DOT, 1, RGB(128, 128, 128));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        MoveToEx(hdc, start.x, start.y, NULL);
        LineTo(hdc, previewEnd.x, previewEnd.y);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
}

bool Line::IsComplete() const {
    return complete;
}

void Line::AddPoint(const Point& p) {
    if (!hasStart) {
        start = p;
        hasStart = true;
        // 第一次点击时，不设置预览点，避免出现从原点到起点的虚线
        previewEnd = start;
    }
    else if (!complete) {
        end = p;
        complete = true;
    }
}

void Line::SetPreviewPoint(const Point& p) {
    if (hasStart && !complete) {
        previewEnd = p;
    }
}

void Line::SetAlgorithm(LineAlgorithm algo) {
    algorithm = algo;
}

// ============ Circle 类实现 ============
Circle::Circle(CircleAlgorithm algo)
    : radius(0), hasCenter(false), complete(false), algorithm(algo), center(0, 0), previewPoint(0, 0) {}

int Circle::CalculateRadius(const Point& p1, const Point& p2) {
    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;
    return (int)sqrt(dx * dx + dy * dy);
}

void Circle::Draw(HDC hdc) {
    if (complete && radius > 0) {
        // 根据算法选择不同颜色
        COLORREF color = RGB(0, 0, 0);  // GDI - 黑色
        if (algorithm == CircleAlgorithm::Midpoint) {
            color = RGB(255, 0, 0);  // 中点法 - 红色
        } else if (algorithm == CircleAlgorithm::Bresenham) {
            color = RGB(0, 0, 255);  // Bresenham - 蓝色
        }
        
        // 如果被选中，绘制高亮边框
        if (isSelected) {
            HPEN hPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 255));
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
            HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
            Ellipse(hdc, center.x - radius, center.y - radius, 
                         center.x + radius, center.y + radius);
            SelectObject(hdc, hOldPen);
            SelectObject(hdc, hOldBrush);
            DeleteObject(hPen);
        }
        
        DrawingAlgorithm::DrawCircle(hdc, center.x, center.y, radius, algorithm, color);
    }
}

void Circle::DrawPreview(HDC hdc) {
    if (hasCenter && !complete && (previewPoint.x != center.x || previewPoint.y != center.y)) {
        int r = CalculateRadius(center, previewPoint);
        if (r > 0) {
            HPEN hPen = CreatePen(PS_DOT, 1, RGB(128, 128, 128));
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
            HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
            Ellipse(hdc, center.x - r, center.y - r, center.x + r, center.y + r);
            SelectObject(hdc, hOldPen);
            SelectObject(hdc, hOldBrush);
            DeleteObject(hPen);
        }
    }
}

bool Circle::IsComplete() const {
    return complete;
}

void Circle::AddPoint(const Point& p) {
    if (!hasCenter) {
        center = p;
        hasCenter = true;
        // 第一次点击时，预览点设为中心点，避免出现半径为0的圆
        previewPoint = center;
    }
    else if (!complete) {
        radius = CalculateRadius(center, p);
        complete = true;
    }
}

void Circle::SetPreviewPoint(const Point& p) {
    if (hasCenter && !complete) {
        previewPoint = p;
    }
}

void Circle::SetAlgorithm(CircleAlgorithm algo) {
    algorithm = algo;
}

Point Circle::GetCenter() const {
    return center;
}

int Circle::GetRadius() const {
    return radius;
}

// ============ Rectangle 类实现 ============
Rectangle::Rectangle() : hasFirstPoint(false), complete(false), topLeft(0, 0), bottomRight(0, 0), previewPoint(0, 0) {}

void Rectangle::Draw(HDC hdc) {
    if (complete) {
        int penWidth = isSelected ? 3 : 1;
        COLORREF penColor = isSelected ? RGB(255, 0, 255) : RGB(0, 0, 0);
        HPEN hPen = CreatePen(PS_SOLID, penWidth, penColor);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        ::Rectangle(hdc, topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);
    }
}

void Rectangle::DrawPreview(HDC hdc) {
    if (hasFirstPoint && !complete && (previewPoint.x != topLeft.x || previewPoint.y != topLeft.y)) {
        HPEN hPen = CreatePen(PS_DOT, 1, RGB(128, 128, 128));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        ::Rectangle(hdc, topLeft.x, topLeft.y, previewPoint.x, previewPoint.y);
        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);
    }
}

bool Rectangle::IsComplete() const {
    return complete;
}

void Rectangle::AddPoint(const Point& p) {
    if (!hasFirstPoint) {
        topLeft = p;
        hasFirstPoint = true;
        // 第一次点击时，预览点设为起点，避免出现从原点到起点的矩形
        previewPoint = topLeft;
    }
    else if (!complete) {
        bottomRight = p;
        complete = true;
    }
}

void Rectangle::SetPreviewPoint(const Point& p) {
    if (hasFirstPoint && !complete) {
        previewPoint = p;
    }
}

Point Rectangle::GetTopLeft() const {
    return topLeft;
}

Point Rectangle::GetBottomRight() const {
    return bottomRight;
}

// ============ Polyline 类实现 ============
Polyline::Polyline() : closed(false) {}

void Polyline::Draw(HDC hdc) {
    if (points.size() < 2) return;

    int penWidth = isSelected ? 3 : 1;
    COLORREF penColor = isSelected ? RGB(255, 0, 255) : RGB(0, 0, 0);
    HPEN hPen = CreatePen(PS_SOLID, penWidth, penColor);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    for (size_t i = 0; i < points.size() - 1; i++) {
        MoveToEx(hdc, points[i].x, points[i].y, NULL);
        LineTo(hdc, points[i + 1].x, points[i + 1].y);
    }

    // 如果封闭,连接最后一个点和第一个点
    if (closed && points.size() > 2) {
        MoveToEx(hdc, points.back().x, points.back().y, NULL);
        LineTo(hdc, points[0].x, points[0].y);
    }

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

void Polyline::DrawPreview(HDC hdc) {
    // 多段线没有预览
}

bool Polyline::IsComplete() const {
    return closed;
}

void Polyline::AddPoint(const Point& p) {
    points.push_back(p);
}

void Polyline::SetPreviewPoint(const Point& p) {
    // 多段线不需要预览点
}

void Polyline::Close() {
    if (points.size() >= 3) {
        closed = true;
    }
}

const std::vector<Point>& Polyline::GetPoints() const {
    return points;
}

size_t Polyline::GetPointCount() const {
    return points.size();
}

// ============ BSpline 类实现 ============
BSpline::BSpline(int minPts) : complete(false), minPoints(minPts) {}

// 使用4个控制点和参数t计算B样条曲线上的点
// t的范围是[0,1],对应一段曲线段
Point BSpline::CalculateCurvePoint(const Point& p0, const Point& p1, const Point& p2, const Point& p3, double t) {
    // 三次均匀B样条基函数
    double f1 = (-t*t*t + 3*t*t - 3*t + 1) / 6.0;
    double f2 = (3*t*t*t - 6*t*t + 4) / 6.0;
    double f3 = (-3*t*t*t + 3*t*t + 3*t + 1) / 6.0;
    double f4 = (t*t*t) / 6.0;
    
    double x = f1 * p0.x + f2 * p1.x + f3 * p2.x + f4 * p3.x;
    double y = f1 * p0.y + f2 * p1.y + f3 * p2.y + f4 * p3.y;
    
    return Point((int)(x + 0.5), (int)(y + 0.5));
}

void BSpline::Draw(HDC hdc) {
    if (controlPoints.size() < minPoints) return;

    // 绘制控制多边形(虚线,灰色)
    HPEN hPenDot = CreatePen(PS_DOT, 1, RGB(200, 200, 200));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPenDot);
    for (size_t i = 0; i < controlPoints.size() - 1; i++) {
        MoveToEx(hdc, controlPoints[i].x, controlPoints[i].y, NULL);
        LineTo(hdc, controlPoints[i + 1].x, controlPoints[i + 1].y);
    }
    SelectObject(hdc, hOldPen);
    DeleteObject(hPenDot);

    // 绘制控制点(小黑圆)
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    for (const auto& p : controlPoints) {
        Ellipse(hdc, p.x - 3, p.y - 3, p.x + 3, p.y + 3);
    }
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);

    // 如果控制点少于4个,不绘制曲线
    if (controlPoints.size() < 4) return;

    // 绘制平滑的B样条曲线(红色)
    HPEN hPenCurve = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
    hOldPen = (HPEN)SelectObject(hdc, hPenCurve);

    // 每4个连续的控制点生成一段曲线
    const int segments = 20;  // 每段曲线的细分数
    bool firstPoint = true;
    
    curvePoints.clear();
    
    for (size_t i = 0; i + 3 < controlPoints.size(); i++) {
        // 对每一段进行细分
        for (int j = 0; j <= segments; j++) {
            double t = (double)j / segments;
            Point p = CalculateCurvePoint(controlPoints[i], controlPoints[i + 1], 
                                         controlPoints[i + 2], controlPoints[i + 3], t);
            
            // 记录曲线点用于标记
            if (j == segments) { 
                curvePoints.push_back(p);
            }
            
            if (firstPoint) {
                MoveToEx(hdc, p.x, p.y, NULL);
                firstPoint = false;
            } else {
                LineTo(hdc, p.x, p.y);
            }
        }
    }

    SelectObject(hdc, hOldPen);
    DeleteObject(hPenCurve);

    // 绘制曲线标记点(绿色圆圈)
    HPEN hPenGreen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
    hOldPen = (HPEN)SelectObject(hdc, hPenGreen);
    HBRUSH hBrushGreen = CreateSolidBrush(RGB(0, 255, 0));
    hOldBrush = (HBRUSH)SelectObject(hdc, hBrushGreen);
    for (const auto& p : curvePoints) {
        Ellipse(hdc, p.x - 4, p.y - 4, p.x + 4, p.y + 4);
    }
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrushGreen);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPenGreen);
}

void BSpline::DrawPreview(HDC hdc) {
    // 绘制已有的控制点
    if (controlPoints.size() > 0) {
        // 绘制控制点(小黑圆)
        HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        for (const auto& p : controlPoints) {
            Ellipse(hdc, p.x - 3, p.y - 3, p.x + 3, p.y + 3);
        }
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
        
        // 绘制控制多边形(虚线)
        if (controlPoints.size() > 1) {
            HPEN hPenDot = CreatePen(PS_DOT, 1, RGB(200, 200, 200));
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPenDot);
            for (size_t i = 0; i < controlPoints.size() - 1; i++) {
                MoveToEx(hdc, controlPoints[i].x, controlPoints[i].y, NULL);
                LineTo(hdc, controlPoints[i + 1].x, controlPoints[i + 1].y);
            }
            SelectObject(hdc, hOldPen);
            DeleteObject(hPenDot);
        }
    }

    // 如果有足够的点(>=4),绘制部分曲线
    if (controlPoints.size() >= 4) {
        // 绘制平滑的预览曲线
        HPEN hPenCurve = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPenCurve);

        const int segments = 20;
        bool firstPoint = true;
        std::vector<Point> previewCurvePoints;
        
        for (size_t i = 0; i + 3 < controlPoints.size(); i++) {
            for (int j = 0; j <= segments; j++) {
                double t = (double)j / segments;
                Point p = CalculateCurvePoint(controlPoints[i], controlPoints[i + 1], 
                                             controlPoints[i + 2], controlPoints[i + 3], t);
                
                if (j == segments / 2) {
                    previewCurvePoints.push_back(p);
                }
                
                if (firstPoint) {
                    MoveToEx(hdc, p.x, p.y, NULL);
                    firstPoint = false;
                } else {
                    LineTo(hdc, p.x, p.y);
                }
            }
        }

        SelectObject(hdc, hOldPen);
        DeleteObject(hPenCurve);

        // 绘制标记点(绿色)
        if (previewCurvePoints.size() > 0) {
            HPEN hPenGreen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
            hOldPen = (HPEN)SelectObject(hdc, hPenGreen);
            HBRUSH hBrushGreen = CreateSolidBrush(RGB(0, 255, 0));
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrushGreen);
            for (const auto& p : previewCurvePoints) {
                Ellipse(hdc, p.x - 4, p.y - 4, p.x + 4, p.y + 4);
            }
            SelectObject(hdc, hOldBrush);
            DeleteObject(hBrushGreen);
            SelectObject(hdc, hOldPen);
            DeleteObject(hPenGreen);
        }
    }
}

bool BSpline::IsComplete() const {
    return complete;
}

void BSpline::AddPoint(const Point& p) {
    controlPoints.push_back(p);
}

void BSpline::SetPreviewPoint(const Point& p) {
    // B样条不需要预览点
}

void BSpline::Finish() {
    complete = true;
}

size_t BSpline::GetPointCount() const {
    return controlPoints.size();
}

// ============ FilledRegion 类实现 ============
FilledRegion::FilledRegion(const std::vector<Point>& pts, FillAlgorithm algo, COLORREF color)
    : points(pts), algorithm(algo), complete(true), fillColor(color) {}

void FilledRegion::Draw(HDC hdc) {
    if (points.size() >= 3) {
        DrawingAlgorithm::FillPolygon(hdc, points, algorithm, fillColor);
    }
}

void FilledRegion::DrawPreview(HDC hdc) {}

bool FilledRegion::IsComplete() const {
    return complete;
}

void FilledRegion::AddPoint(const Point& p) {}

void FilledRegion::SetPreviewPoint(const Point& p) {}
// Shape 变换接口和多边形类的实现
// 这个文件包含实验二新增的变换功能实现

#include "Shape.h"
#include <algorithm>
#include <cmath>

// ==================== Line 类变换实现 ====================

void Line::Translate(int dx, int dy) {
    start = start.Translate(dx, dy);
    end = end.Translate(dx, dy);
}

void Line::Scale(double sx, double sy, const Point& center) {
    start = start.Scale(sx, sy, center);
    end = end.Scale(sx, sy, center);
}

void Line::Rotate(double angleRad, const Point& center) {
    start = start.Rotate(angleRad, center);
    end = end.Rotate(angleRad, center);
}

Point Line::GetCenter() const {
    return Point((start.x + end.x) / 2, (start.y + end.y) / 2);
}

bool Line::HitTest(const Point& p, int tolerance) const {
    if (!complete) return false;
    
    // 点到线段的距离判断
    int dx = end.x - start.x;
    int dy = end.y - start.y;
    
    if (dx == 0 && dy == 0) {
        // 起点终点重合
        return p.DistanceTo(start) <= tolerance;
    }
    
    // 计算点到直线的距离
    double lineLengthSquared = dx * dx + dy * dy;
    double t = ((p.x - start.x) * dx + (p.y - start.y) * dy) / lineLengthSquared;
    
    // 限制t在[0,1]范围内（线段而非直线）
    t = std::max(0.0, std::min(1.0, t));
    
    Point closest(start.x + (int)(t * dx), start.y + (int)(t * dy));
    return p.DistanceTo(closest) <= tolerance;
}

// ==================== Circle 类变换实现 ====================

void Circle::Translate(int dx, int dy) {
    center = center.Translate(dx, dy);
}

void Circle::Scale(double sx, double sy, const Point& scaleCenter) {
    center = center.Scale(sx, sy, scaleCenter);
    // 使用平均缩放因子缩放半径
    radius = (int)(radius * (sx + sy) / 2.0);
}

void Circle::Rotate(double angleRad, const Point& rotateCenter) {
    // 旋转圆心，半径不变
    center = center.Rotate(angleRad, rotateCenter);
}

bool Circle::HitTest(const Point& p, int tolerance) const {
    if (!complete) return false;
    
    // 点击圆周附近或圆内部
    double distance = p.DistanceTo(center);
    // 在圆周附近或在圆内部都算选中
    return (abs(distance - radius) <= tolerance) || (distance <= radius);
}

// ==================== Rectangle 类变换实现 ====================

void Rectangle::Translate(int dx, int dy) {
    topLeft = topLeft.Translate(dx, dy);
    bottomRight = bottomRight.Translate(dx, dy);
}

void Rectangle::Scale(double sx, double sy, const Point& center) {
    topLeft = topLeft.Scale(sx, sy, center);
    bottomRight = bottomRight.Scale(sx, sy, center);
}

void Rectangle::Rotate(double angleRad, const Point& center) {
    topLeft = topLeft.Rotate(angleRad, center);
    bottomRight = bottomRight.Rotate(angleRad, center);
}

Point Rectangle::GetCenter() const {
    return Point((topLeft.x + bottomRight.x) / 2, 
                 (topLeft.y + bottomRight.y) / 2);
}

bool Rectangle::HitTest(const Point& p, int tolerance) const {
    if (!complete) return false;
    
    int left = std::min(topLeft.x, bottomRight.x);
    int right = std::max(topLeft.x, bottomRight.x);
    int top = std::min(topLeft.y, bottomRight.y);
    int bottom = std::max(topLeft.y, bottomRight.y);
    
    // 检查是否在矩形边界附近
    bool nearLeft = abs(p.x - left) <= tolerance && p.y >= top - tolerance && p.y <= bottom + tolerance;
    bool nearRight = abs(p.x - right) <= tolerance && p.y >= top - tolerance && p.y <= bottom + tolerance;
    bool nearTop = abs(p.y - top) <= tolerance && p.x >= left - tolerance && p.x <= right + tolerance;
    bool nearBottom = abs(p.y - bottom) <= tolerance && p.x >= left - tolerance && p.x <= right + tolerance;
    
    // 检查是否在矩形内部
    bool inside = (p.x >= left && p.x <= right && p.y >= top && p.y <= bottom);
    
    return nearLeft || nearRight || nearTop || nearBottom || inside;
}

// ==================== Polyline 类变换实现 ====================

void Polyline::Translate(int dx, int dy) {
    for (auto& p : points) {
        p = p.Translate(dx, dy);
    }
}

void Polyline::Scale(double sx, double sy, const Point& center) {
    for (auto& p : points) {
        p = p.Scale(sx, sy, center);
    }
}

void Polyline::Rotate(double angleRad, const Point& center) {
    for (auto& p : points) {
        p = p.Rotate(angleRad, center);
    }
}

Point Polyline::GetCenter() const {
    if (points.empty()) return Point();
    
    int sumX = 0, sumY = 0;
    for (const auto& p : points) {
        sumX += p.x;
        sumY += p.y;
    }
    return Point(sumX / points.size(), sumY / points.size());
}

bool Polyline::HitTest(const Point& p, int tolerance) const {
    if (points.size() < 2) return false;
    
    // 检查是否靠近任何线段
    for (size_t i = 0; i < points.size() - 1; i++) {
        Line tempLine;
        tempLine.SetEndpoints(points[i], points[i + 1]);
        if (tempLine.HitTest(p, tolerance)) {
            return true;
        }
    }
    
    // 如果是闭合的，检查最后一条边
    if (closed && points.size() >= 2) {
        Line tempLine;
        tempLine.SetEndpoints(points.back(), points.front());
        if (tempLine.HitTest(p, tolerance)) {
            return true;
        }
    }
    
    // 如果多段线已闭合，使用射线法检查是否在内部
    if (closed && points.size() >= 3) {
        bool inside = false;
        for (size_t i = 0, j = points.size() - 1; i < points.size(); j = i++) {
            if (((points[i].y > p.y) != (points[j].y > p.y)) &&
                (p.x < (points[j].x - points[i].x) * (p.y - points[i].y) / 
                       (points[j].y - points[i].y) + points[i].x)) {
                inside = !inside;
            }
        }
        if (inside) return true;
    }
    
    return false;
}

// ==================== Polygon 类实现 ====================

Polygon::Polygon() : complete(false), previewPoint(0, 0) {}

void Polygon::Draw(HDC hdc) {
    if (vertices.size() < 2) return;
    
    // 绘制多边形边
    HPEN hPen = CreatePen(PS_SOLID, 2, isSelected ? RGB(255, 0, 0) : RGB(0, 0, 0));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    
    MoveToEx(hdc, vertices[0].x, vertices[0].y, NULL);
    for (size_t i = 1; i < vertices.size(); i++) {
        LineTo(hdc, vertices[i].x, vertices[i].y);
    }
    
    // 如果已完成，闭合多边形
    if (complete && vertices.size() >= 3) {
        LineTo(hdc, vertices[0].x, vertices[0].y);
    }
    
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    
    // 绘制顶点标记
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 255, 0));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    for (const auto& v : vertices) {
        Ellipse(hdc, v.x - 3, v.y - 3, v.x + 3, v.y + 3);
    }
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
}

void Polygon::DrawPreview(HDC hdc) {
    if (vertices.empty() || complete) return;
    
    // 绘制从最后一个顶点到鼠标位置的预览线
    HPEN hPen = CreatePen(PS_DOT, 1, RGB(128, 128, 128));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    
    MoveToEx(hdc, vertices.back().x, vertices.back().y, NULL);
    LineTo(hdc, previewPoint.x, previewPoint.y);
    
    // 如果有多个点，显示闭合预览
    if (vertices.size() >= 2) {
        LineTo(hdc, vertices[0].x, vertices[0].y);
    }
    
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

bool Polygon::IsComplete() const {
    return complete;
}

void Polygon::AddPoint(const Point& p) {
    if (complete) return;
    vertices.push_back(p);
}

void Polygon::SetPreviewPoint(const Point& p) {
    if (!complete) {
        previewPoint = p;
    }
}

void Polygon::Close() {
    if (vertices.size() >= 3) {
        complete = true;
    }
}

void Polygon::Translate(int dx, int dy) {
    for (auto& v : vertices) {
        v = v.Translate(dx, dy);
    }
}

void Polygon::Scale(double sx, double sy, const Point& center) {
    for (auto& v : vertices) {
        v = v.Scale(sx, sy, center);
    }
}

void Polygon::Rotate(double angleRad, const Point& center) {
    for (auto& v : vertices) {
        v = v.Rotate(angleRad, center);
    }
}

Point Polygon::GetCenter() const {
    if (vertices.empty()) return Point();
    
    int sumX = 0, sumY = 0;
    for (const auto& v : vertices) {
        sumX += v.x;
        sumY += v.y;
    }
    return Point(sumX / vertices.size(), sumY / vertices.size());
}

bool Polygon::HitTest(const Point& p, int tolerance) const {
    if (vertices.size() < 2) return false;
    
    // 首先检查是否靠近任何边
    for (size_t i = 0; i < vertices.size(); i++) {
        size_t next = (i + 1) % vertices.size();
        if (!complete && next == 0) break;  // 未闭合时不检查最后一条边
        
        // 检查点到线段的距离
        const Point& p1 = vertices[i];
        const Point& p2 = vertices[next];
        
        // 计算点到线段的距离
        int dx = p2.x - p1.x;
        int dy = p2.y - p1.y;
        double lineLengthSquared = dx * dx + dy * dy;
        
        if (lineLengthSquared < 1) continue; // 线段太短，跳过
        
        double t = ((p.x - p1.x) * dx + (p.y - p1.y) * dy) / (double)lineLengthSquared;
        t = std::max(0.0, std::min(1.0, t));
        
        int closestX = p1.x + (int)(t * dx);
        int closestY = p1.y + (int)(t * dy);
        
        int distX = p.x - closestX;
        int distY = p.y - closestY;
        double distance = sqrt(distX * distX + distY * distY);
        
        if (distance <= tolerance) {
            return true;
        }
    }
    
    // 如果多边形已闭合，也检查是否在内部
    if (complete && vertices.size() >= 3) {
        bool inside = false;
        for (size_t i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++) {
            if (((vertices[i].y > p.y) != (vertices[j].y > p.y)) &&
                (p.x < (vertices[j].x - vertices[i].x) * (p.y - vertices[i].y) / 
                       (vertices[j].y - vertices[i].y) + vertices[i].x)) {
                inside = !inside;
            }
        }
        if (inside) return true;
    }
    
    return false;
}

// ==================== BSpline 类变换实现 ====================

void BSpline::Translate(int dx, int dy) {
    for (auto& p : controlPoints) {
        p = p.Translate(dx, dy);
    }
    for (auto& p : curvePoints) {
        p = p.Translate(dx, dy);
    }
}

void BSpline::Scale(double sx, double sy, const Point& center) {
    for (auto& p : controlPoints) {
        p = p.Scale(sx, sy, center);
    }
    for (auto& p : curvePoints) {
        p = p.Scale(sx, sy, center);
    }
}

void BSpline::Rotate(double angleRad, const Point& center) {
    for (auto& p : controlPoints) {
        p = p.Rotate(angleRad, center);
    }
    for (auto& p : curvePoints) {
        p = p.Rotate(angleRad, center);
    }
}

Point BSpline::GetCenter() const {
    if (controlPoints.empty()) return Point();
    
    int sumX = 0, sumY = 0;
    for (const auto& p : controlPoints) {
        sumX += p.x;
        sumY += p.y;
    }
    return Point(sumX / controlPoints.size(), sumY / controlPoints.size());
}

bool BSpline::HitTest(const Point& p, int tolerance) const {
    // 如果还没有足够的控制点，只检查控制点
    if (controlPoints.size() < 4) {
        for (const auto& cp : controlPoints) {
            if (p.DistanceTo(cp) <= tolerance + 5) { // 增大容差
                return true;
            }
        }
        return false;
    }
    
    // 检查是否靠近任何控制点
    for (const auto& cp : controlPoints) {
        if (p.DistanceTo(cp) <= tolerance + 3) {
            return true;
        }
    }
    
    // 检查是否靠近曲线上的点
    for (const auto& curvePt : curvePoints) {
        if (p.DistanceTo(curvePt) <= tolerance + 2) {
            return true;
        }
    }
    
    // 如果曲线点较少，检查是否靠近控制多边形
    if (curvePoints.size() < controlPoints.size() * 5 && controlPoints.size() >= 2) {
        for (size_t i = 0; i < controlPoints.size() - 1; i++) {
            const Point& p1 = controlPoints[i];
            const Point& p2 = controlPoints[i + 1];
            
            int dx = p2.x - p1.x;
            int dy = p2.y - p1.y;
            double lineLengthSquared = dx * dx + dy * dy;
            
            if (lineLengthSquared < 1) continue;
            
            double t = ((p.x - p1.x) * dx + (p.y - p1.y) * dy) / (double)lineLengthSquared;
            t = std::max(0.0, std::min(1.0, t));
            
            int closestX = p1.x + (int)(t * dx);
            int closestY = p1.y + (int)(t * dy);
            
            int distX = p.x - closestX;
            int distY = p.y - closestY;
            double distance = sqrt(distX * distX + distY * distY);
            
            if (distance <= tolerance + 5) {
                return true;
            }
        }
    }
    
    return false;
}
