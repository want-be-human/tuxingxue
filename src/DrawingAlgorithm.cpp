#include "DrawingAlgorithm.h"

// ============ 公共接口实现 ============

void DrawingAlgorithm::DrawLine(HDC hdc, int x1, int y1, int x2, int y2, LineAlgorithm algorithm, COLORREF color) {
    switch (algorithm) {
    case LineAlgorithm::GDI: {
        HPEN hPen = CreatePen(PS_SOLID, 1, color);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x2, y2);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
        break;
    }
    case LineAlgorithm::Midpoint:
        DrawLineMidpoint(hdc, x1, y1, x2, y2, color);
        break;
    case LineAlgorithm::Bresenham:
        DrawLineBresenham(hdc, x1, y1, x2, y2, color);
        break;
    }
}

void DrawingAlgorithm::DrawCircle(HDC hdc, int centerX, int centerY, int radius, CircleAlgorithm algorithm, COLORREF color) {
    switch (algorithm) {
    case CircleAlgorithm::GDI: {
        HPEN hPen = CreatePen(PS_SOLID, 1, color);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        Ellipse(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius);
        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);
        break;
    }
    case CircleAlgorithm::Midpoint:
        DrawCircleMidpoint(hdc, centerX, centerY, radius, color);
        break;
    case CircleAlgorithm::Bresenham:
        DrawCircleBresenham(hdc, centerX, centerY, radius, color);
        break;
    }
}

void DrawingAlgorithm::FillPolygon(HDC hdc, const std::vector<Point>& points, FillAlgorithm algorithm, COLORREF color) {
    if (points.size() < 3) return;

    switch (algorithm) {
    case FillAlgorithm::ScanLine:
        FillPolygonScanLine(hdc, points, color);
        break;
    case FillAlgorithm::Fence:
        FillPolygonFence(hdc, points, color);
        break;
    }
}

// ============ 私有辅助函数实现 ============

void DrawingAlgorithm::SetPixelSafe(HDC hdc, int x, int y, COLORREF color) {
    SetPixel(hdc, x, y, color);
}

void DrawingAlgorithm::DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
    // 处理不同方向的直线
    int dx = x2 - x1;
    int dy = y2 - y1;
    
    // 确保从左到右绘制
    if (dx < 0) {
        DrawLineMidpoint(hdc, x2, y2, x1, y1, color);
        return;
    }
    
    // 斜率 0 < k <= 1 的情况
    if (dy >= 0 && dy <= dx) {
        int x, y, d0, d1, d2, a, b;
        y = y1;
        a = y1 - y2;          // 直线方程中的a
        b = x2 - x1;          // 直线方程中的b
        d0 = 2 * a + b;       // 增量初始值
        d1 = 2 * a;           // d >= 0 时的增量
        d2 = 2 * (a + b);     // d < 0 时的增量
        
        for (x = x1; x <= x2; x++) {
            SetPixelSafe(hdc, x, y, color);
            if (d0 < 0) {
                y++;
                d0 += d2;
            } else {
                d0 += d1;
            }
        }
    }
    // 斜率 k > 1 的情况
    else if (dy > 0 && dy > dx) {
        int x, y, d0, d1, d2, a, b;
        x = x1;
        a = y1 - y2;
        b = x2 - x1;
        d0 = a + 2 * b;
        d1 = 2 * b;
        d2 = 2 * (a + b);
        
        for (y = y1; y <= y2; y++) {
            SetPixelSafe(hdc, x, y, color);
            if (d0 > 0) {
                x++;
                d0 += d2;
            } else {
                d0 += d1;
            }
        }
    }
    // 斜率 -1 <= k < 0 的情况
    else if (dy < 0 && -dy <= dx) {
        int x, y, d0, d1, d2, a, b;
        y = y1;
        a = y2 - y1;
        b = x2 - x1;
        d0 = 2 * a + b;
        d1 = 2 * a;
        d2 = 2 * (a + b);
        
        for (x = x1; x <= x2; x++) {
            SetPixelSafe(hdc, x, y, color);
            if (d0 < 0) {
                y--;
                d0 += d2;
            } else {
                d0 += d1;
            }
        }
    }
    // 斜率 k < -1 的情况
    else {
        int x, y, d0, d1, d2, a, b;
        x = x1;
        a = y2 - y1;
        b = x2 - x1;
        d0 = a + 2 * b;
        d1 = 2 * b;
        d2 = 2 * (a + b);
        
        for (y = y1; y >= y2; y--) {
            SetPixelSafe(hdc, x, y, color);
            if (d0 > 0) {
                x++;
                d0 += d2;
            } else {
                d0 += d1;
            }
        }
    }
}

void DrawingAlgorithm::DrawLineBresenham(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
    // 处理不同方向的直线
    int dx = x2 - x1;
    int dy = y2 - y1;
    
    // 确保从左到右绘制
    if (dx < 0) {
        DrawLineBresenham(hdc, x2, y2, x1, y1, color);
        return;
    }
    
    // 斜率 0 < k <= 1 的情况
    if (dy >= 0 && dy <= dx) {
        int x, y, d;
        y = y1;
        d = 2 * dy - dx;      // 增量d的初始值
        
        for (x = x1; x <= x2; x++) {
            SetPixelSafe(hdc, x, y, color);
            if (d < 0) {
                d += 2 * dy;
            } else {
                y++;
                d += 2 * dy - 2 * dx;
            }
        }
    }
    // 斜率 k > 1 的情况
    else if (dy > 0 && dy > dx) {
        int x, y, d;
        x = x1;
        d = 2 * dx - dy;
        
        for (y = y1; y <= y2; y++) {
            SetPixelSafe(hdc, x, y, color);
            if (d < 0) {
                d += 2 * dx;
            } else {
                x++;
                d += 2 * dx - 2 * dy;
            }
        }
    }
    // 斜率 -1 <= k < 0 的情况
    else if (dy < 0 && -dy <= dx) {
        int x, y, d;
        y = y1;
        d = 2 * (-dy) - dx;
        
        for (x = x1; x <= x2; x++) {
            SetPixelSafe(hdc, x, y, color);
            if (d < 0) {
                d += 2 * (-dy);
            } else {
                y--;
                d += 2 * (-dy) - 2 * dx;
            }
        }
    }
    // 斜率 k < -1 的情况
    else {
        int x, y, d;
        x = x1;
        d = 2 * dx - (-dy);
        
        for (y = y1; y >= y2; y--) {
            SetPixelSafe(hdc, x, y, color);
            if (d < 0) {
                d += 2 * dx;
            } else {
                x++;
                d += 2 * dx - 2 * (-dy);
            }
        }
    }
}

void DrawingAlgorithm::DrawCircleMidpoint(HDC hdc, int centerX, int centerY, int radius, COLORREF color) {
    int x = 0;
    int y = radius;
    int d = 1 - radius;

    DrawCirclePoints(hdc, centerX, centerY, x, y, color);

    while (x < y) {
        if (d < 0) {
            d += 2 * x + 3;
        }
        else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
        DrawCirclePoints(hdc, centerX, centerY, x, y, color);
    }
}

void DrawingAlgorithm::DrawCircleBresenham(HDC hdc, int centerX, int centerY, int radius, COLORREF color) {
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    DrawCirclePoints(hdc, centerX, centerY, x, y, color);

    while (x <= y) {
        if (d < 0) {
            d += 4 * x + 6;
        }
        else {
            d += 4 * (x - y) + 10;
            y--;
        }
        x++;
        DrawCirclePoints(hdc, centerX, centerY, x, y, color);
    }
}

void DrawingAlgorithm::DrawCirclePoints(HDC hdc, int centerX, int centerY, int x, int y, COLORREF color) {
    SetPixelSafe(hdc, centerX + x, centerY + y, color);
    SetPixelSafe(hdc, centerX - x, centerY + y, color);
    SetPixelSafe(hdc, centerX + x, centerY - y, color);
    SetPixelSafe(hdc, centerX - x, centerY - y, color);
    SetPixelSafe(hdc, centerX + y, centerY + x, color);
    SetPixelSafe(hdc, centerX - y, centerY + x, color);
    SetPixelSafe(hdc, centerX + y, centerY - x, color);
    SetPixelSafe(hdc, centerX - y, centerY - x, color);
}

void DrawingAlgorithm::FillPolygonScanLine(HDC hdc, const std::vector<Point>& points, COLORREF color) {
    if (points.size() < 3) return;

    // 找到多边形的上下边界
    int minY = points[0].y, maxY = points[0].y;
    for (const auto& p : points) {
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }

    // 对每条扫描线
    for (int y = minY; y <= maxY; y++) {
        std::vector<int> intersections;

        // 找到与扫描线的所有交点
        size_t n = points.size();
        for (size_t i = 0; i < n; i++) {
            Point p1 = points[i];
            Point p2 = points[(i + 1) % n];

            if (p1.y > p2.y) std::swap(p1, p2);

            if (y >= p1.y && y < p2.y) {//顶点重复
                if (p2.y != p1.y) {//水平边
                    int x = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
                    intersections.push_back(x);
                }
            }
        }

        // 对交点排序
        std::sort(intersections.begin(), intersections.end());

        // 填充交点之间的像素
        for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
            for (int x = intersections[i]; x <= intersections[i + 1]; x++) {
                SetPixelSafe(hdc, x, y, color);
            }
        }
    }

    // 绘制边界
    size_t n = points.size();
    for (size_t i = 0; i < n; i++) {
        DrawLineBresenham(hdc, points[i].x, points[i].y,
            points[(i + 1) % n].x, points[(i + 1) % n].y, RGB(0, 0, 0));
    }
}

void DrawingAlgorithm::FillPolygonFence(HDC hdc, const std::vector<Point>& points, COLORREF color) {
    if (points.size() < 3) return;

    // 找到多边形的边界
    int minX = points[0].x, maxX = points[0].x;
    int minY = points[0].y, maxY = points[0].y;
    for (const auto& p : points) {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }

    // 点在多边形内部的判断(射线法)
    auto isInside = [&](int x, int y) -> bool {
        int crossings = 0;
        size_t n = points.size();
        for (size_t i = 0; i < n; i++) {
            Point p1 = points[i];
            Point p2 = points[(i + 1) % n];

            if ((p1.y <= y && p2.y > y) || (p1.y > y && p2.y <= y)) {
                double vt = (double)(y - p1.y) / (p2.y - p1.y);
                if (x < p1.x + vt * (p2.x - p1.x)) {
                    crossings++;
                }
            }
        }
        return (crossings % 2) == 1;
    };

    // 使用栅栏填充(隔行扫描)
    for (int y = minY; y <= maxY; y += 2) {
        for (int x = minX; x <= maxX; x++) {
            if (isInside(x, y)) {
                SetPixelSafe(hdc, x, y, color);
                if (y + 1 <= maxY) {
                    SetPixelSafe(hdc, x, y + 1, color);
                }
            }
        }
    }

    // 绘制边界
    size_t n = points.size();
    for (size_t i = 0; i < n; i++) {
        DrawLineBresenham(hdc, points[i].x, points[i].y,
            points[(i + 1) % n].x, points[(i + 1) % n].y, RGB(0, 0, 0));
    }
}

// ==================== 实验二：裁剪算法实现 ====================

// Cohen-Sutherland 直线裁剪算法
// 核心思想：使用4位二进制编码表示点相对于裁剪窗口的位置
// 通过逻辑运算快速判断线段是否完全可见、完全不可见或需要裁剪
bool DrawingAlgorithm::ClipLine_CohenSutherland(const Rect& clipRect, Point& p1, Point& p2) {
    int code1 = ComputeOutCode(clipRect, p1);
    int code2 = ComputeOutCode(clipRect, p2);
    
    while (true) {
        if ((code1 | code2) == 0) {
            // 两点都在窗口内，完全可见
            return true;
        }
        else if ((code1 & code2) != 0) {
            // 两点在窗口同一侧外，完全不可见
            return false;
        }
        else {
            // 需要裁剪
            int codeOut = code1 ? code1 : code2;
            Point p;
            
            // 根据编码计算与窗口边界的交点
            if (codeOut & TOP) {
                // 与上边界相交
                p.x = p1.x + (p2.x - p1.x) * (clipRect.top - p1.y) / (p2.y - p1.y);
                p.y = clipRect.top;
            }
            else if (codeOut & BOTTOM) {
                // 与下边界相交
                p.x = p1.x + (p2.x - p1.x) * (clipRect.bottom - p1.y) / (p2.y - p1.y);
                p.y = clipRect.bottom;
            }
            else if (codeOut & RIGHT) {
                // 与右边界相交
                p.y = p1.y + (p2.y - p1.y) * (clipRect.right - p1.x) / (p2.x - p1.x);
                p.x = clipRect.right;
            }
            else if (codeOut & LEFT) {
                // 与左边界相交
                p.y = p1.y + (p2.y - p1.y) * (clipRect.left - p1.x) / (p2.x - p1.x);
                p.x = clipRect.left;
            }
            
            // 用交点替换窗口外的点，并重新计算编码
            if (codeOut == code1) {
                p1 = p;
                code1 = ComputeOutCode(clipRect, p1);
            }
            else {
                p2 = p;
                code2 = ComputeOutCode(clipRect, p2);
            }
        }
    }
}

// 中点分割直线裁剪算法
// 核心思想：递归地对线段求中点，判断中点位置，逐步缩小线段直到完全在窗口内或确定不可见
bool DrawingAlgorithm::ClipLine_MidpointSubdivision(const Rect& clipRect, Point& p1, Point& p2) {
    return ClipLine_MidpointRecursive(clipRect, p1, p2, 0);
}

// 中点分割递归辅助函数
bool DrawingAlgorithm::ClipLine_MidpointRecursive(const Rect& clipRect, Point& p1, Point& p2, int depth) {
    // 防止递归过深（最多20层，足够精确）
    const int MAX_DEPTH = 20;
    
    bool p1Inside = IsInside(clipRect, p1);
    bool p2Inside = IsInside(clipRect, p2);
    
    if (p1Inside && p2Inside) {
        // 两端点都在窗口内，接受
        return true;
    }
    
    // 计算中点
    Point mid((p1.x + p2.x) / 2, (p1.y + p2.y) / 2);
    
    // 如果中点与端点重合（距离很小）或达到最大深度，停止递归
    if (depth >= MAX_DEPTH || (abs(p1.x - p2.x) <= 1 && abs(p1.y - p2.y) <= 1)) {
        // 检查是否至少有一个点在内部
        if (p1Inside || p2Inside) {
            // 调整端点到窗口边界
            if (!p1Inside) {
                p1.x = std::max(clipRect.left, std::min(clipRect.right, p1.x));
                p1.y = std::max(clipRect.top, std::min(clipRect.bottom, p1.y));
            }
            if (!p2Inside) {
                p2.x = std::max(clipRect.left, std::min(clipRect.right, p2.x));
                p2.y = std::max(clipRect.top, std::min(clipRect.bottom, p2.y));
            }
            return true;
        }
        return false;
    }
    
    bool midInside = IsInside(clipRect, mid);
    
    if (p1Inside) {
        // p1在内，尝试延伸到p2方向
        Point temp = p2;
        if (ClipLine_MidpointRecursive(clipRect, mid, temp, depth + 1)) {
            p2 = temp;
            return true;
        }
        p2 = mid;
        return true;
    }
    else if (p2Inside) {
        // p2在内，尝试延伸到p1方向
        Point temp = p1;
        if (ClipLine_MidpointRecursive(clipRect, temp, mid, depth + 1)) {
            p1 = temp;
            return true;
        }
        p1 = mid;
        return true;
    }
    else if (midInside) {
        // 中点在内，两端都在外，分别向两端查找边界
        Point temp1 = p1, temp2 = p2;
        bool found1 = ClipLine_MidpointRecursive(clipRect, temp1, mid, depth + 1);
        bool found2 = ClipLine_MidpointRecursive(clipRect, mid, temp2, depth + 1);
        
        if (found1 && found2) {
            p1 = temp1;
            p2 = temp2;
            return true;
        }
    }
    else {
        // 中点和两端都在外，分别测试两段
        Point temp1 = mid;
        if (ClipLine_MidpointRecursive(clipRect, p1, temp1, depth + 1)) {
            p2 = temp1;
            return true;
        }
        Point temp2 = mid;
        if (ClipLine_MidpointRecursive(clipRect, temp2, p2, depth + 1)) {
            p1 = temp2;
            return true;
        }
    }
    
    return false;
}

// Sutherland-Hodgman 多边形裁剪算法
// 核心思想：依次用裁剪窗口的每条边对多边形进行裁剪
// 每次裁剪都会产生一个新的多边形，最终得到完全在窗口内的部分
bool DrawingAlgorithm::ClipPolygon_SutherlandHodgman(const Rect& clipRect, 
                                                      const std::vector<Point>& inVerts, 
                                                      std::vector<Point>& outVerts) {
    if (inVerts.size() < 3) return false;
    
    std::vector<Point> input = inVerts;
    std::vector<Point> output;
    
    // 依次用四条边裁剪：左、右、下、上
    for (int edge = 0; edge < 4; edge++) {
        if (input.empty()) break;
        
        output.clear();
        ClipPolygonEdge(input, output, edge, clipRect);
        input = output;
    }
    
    outVerts = output;
    return !outVerts.empty() && outVerts.size() >= 3;
}

// Weiler-Atherton 多边形裁剪算法
// 核心思想：构建多边形和裁剪窗口的交点表，沿交点遍历形成裁剪后的多边形
// 适用于凹多边形和复杂多边形的裁剪，可以返回多个裁剪结果
std::vector<std::vector<Point>> DrawingAlgorithm::ClipPolygon_WeilerAtherton(const Rect& clipRect, 
                                                                               const std::vector<Point>& inVerts) {
    if (inVerts.size() < 3) return {};
    
    // 将矩形裁剪窗口转换为多边形顶点序列
    std::vector<Point> clipPoly;
    clipPoly.push_back(Point(clipRect.left, clipRect.top));
    clipPoly.push_back(Point(clipRect.right, clipRect.top));
    clipPoly.push_back(Point(clipRect.right, clipRect.bottom));
    clipPoly.push_back(Point(clipRect.left, clipRect.bottom));
    
    // 调用 Weiler-Atherton 算法，返回所有裁剪结果
    return WeilerAtherton::clip(inVerts, clipPoly);
}

// ==================== 裁剪算法辅助函数实现 ====================

// 计算点的Cohen-Sutherland区域编码
int DrawingAlgorithm::ComputeOutCode(const Rect& rect, const Point& p) {
    int code = INSIDE;
    
    if (p.x < rect.left)
        code |= LEFT;
    else if (p.x > rect.right)
        code |= RIGHT;
        
    if (p.y < rect.top)
        code |= TOP;
    else if (p.y > rect.bottom)
        code |= BOTTOM;
        
    return code;
}

// 判断点是否在矩形内
bool DrawingAlgorithm::IsInside(const Rect& rect, const Point& p) {
    return p.x >= rect.left && p.x <= rect.right && 
           p.y >= rect.top && p.y <= rect.bottom;
}

// Sutherland-Hodgman 单边裁剪
void DrawingAlgorithm::ClipPolygonEdge(const std::vector<Point>& input, 
                                        std::vector<Point>& output, 
                                        int edge, const Rect& rect) {
    if (input.empty()) return;
    
    Point prevVertex = input.back();
    
    for (const Point& currVertex : input) {
        bool prevInside = false;
        bool currInside = false;
        
        // 判断两个顶点是否在当前裁剪边的内侧
        switch (edge) {
        case 0: // 左边界
            prevInside = (prevVertex.x >= rect.left);
            currInside = (currVertex.x >= rect.left);
            break;
        case 1: // 右边界
            prevInside = (prevVertex.x <= rect.right);
            currInside = (currVertex.x <= rect.right);
            break;
        case 2: // 下边界
            prevInside = (prevVertex.y <= rect.bottom);
            currInside = (currVertex.y <= rect.bottom);
            break;
        case 3: // 上边界
            prevInside = (prevVertex.y >= rect.top);
            currInside = (currVertex.y >= rect.top);
            break;
        }
        
        if (currInside) {
            if (!prevInside) {
                // 从外到内，添加交点
                output.push_back(ComputeIntersection(prevVertex, currVertex, edge, rect));
            }
            // 添加当前点
            output.push_back(currVertex);
        }
        else if (prevInside) {
            // 从内到外，只添加交点
            output.push_back(ComputeIntersection(prevVertex, currVertex, edge, rect));
        }
        
        prevVertex = currVertex;
    }
}

// 计算线段与裁剪边的交点
Point DrawingAlgorithm::ComputeIntersection(const Point& p1, const Point& p2, 
                                             int edge, const Rect& rect) {
    Point intersection;
    double t;
    
    switch (edge) {
    case 0: // 左边界
        if (p2.x != p1.x) {
            t = (double)(rect.left - p1.x) / (p2.x - p1.x);
            intersection.x = rect.left;
            intersection.y = p1.y + (int)(t * (p2.y - p1.y));
        }
        else {
            intersection = p1;
        }
        break;
        
    case 1: // 右边界
        if (p2.x != p1.x) {
            t = (double)(rect.right - p1.x) / (p2.x - p1.x);
            intersection.x = rect.right;
            intersection.y = p1.y + (int)(t * (p2.y - p1.y));
        }
        else {
            intersection = p1;
        }
        break;
        
    case 2: // 下边界
        if (p2.y != p1.y) {
            t = (double)(rect.bottom - p1.y) / (p2.y - p1.y);
            intersection.x = p1.x + (int)(t * (p2.x - p1.x));
            intersection.y = rect.bottom;
        }
        else {
            intersection = p1;
        }
        break;
        
    case 3: // 上边界
        if (p2.y != p1.y) {
            t = (double)(rect.top - p1.y) / (p2.y - p1.y);
            intersection.x = p1.x + (int)(t * (p2.x - p1.x));
            intersection.y = rect.top;
        }
        else {
            intersection = p1;
        }
        break;
    }
    
    return intersection;
}
