#include "Canvas.h"
#include "Shape.h"
#include "DrawingAlgorithm.h"
#include <cmath>
#include <algorithm>

Canvas::Canvas() : currentMode(DrawMode::None), isDrawing(false), 
                   selectedShapeIndex(-1), isSelectMode(false), 
                   pendingFillAlgorithm(FillAlgorithm::ScanLine),
                   hasClipRect(false), hasTransformAnchor(false), isDragging(false) {}

void Canvas::SetDrawMode(DrawMode mode) {
    currentMode = mode;
    currentShape.reset();
    isDrawing = false;
    
    // 只在切换到绘图模式时清理变换状态
    if (mode != DrawMode::Translate && mode != DrawMode::Scale && 
        mode != DrawMode::Rotate && mode != DrawMode::SetClipWindow) {
        isDragging = false;
        hasTransformAnchor = false;
    }
    
    // 只在切换到普通绘图模式时清除选中状态
    if (mode != DrawMode::Translate && 
        mode != DrawMode::Scale && mode != DrawMode::Rotate &&
        !isSelectMode) {  // 填充选择模式下不清除
        ClearSelection();
    }
}

DrawMode Canvas::GetDrawMode() const {
    return currentMode;
}

void Canvas::OnMouseLeftDown(int x, int y) {
    Point p(x, y);
    
    // 处理平移模式
    if (currentMode == DrawMode::Translate) {
        if (selectedShapeIndex < 0) {
            // 第一次点击：选择图形
            SelectShapeAt(p);
            if (selectedShapeIndex >= 0) {
                // 选中后，记录起点，准备拖动
                dragStart = p;
                isDragging = true;
            }
        } else if (isDragging) {
            // 第二次点击：完成平移
            int dx = p.x - dragStart.x;
            int dy = p.y - dragStart.y;
            TranslateSelectedShape(dx, dy);
            ClearSelection();
            isDragging = false;
        }
        return;
    }
    
    // 处理缩放模式
    if (currentMode == DrawMode::Scale) {
        if (selectedShapeIndex < 0) {
            // 第一次点击：选择图形
            SelectShapeAt(p);
            if (selectedShapeIndex >= 0) {
                // 选中后，默认以图形中心为缩放中心
                auto shape = GetSelectedShape();
                if (shape) {
                    transformAnchor = shape->GetCenter();
                    hasTransformAnchor = true;
                    dragStart = p; // 记录初始点
                }
            }
        } else if (hasTransformAnchor) {
            // 第二次点击：完成缩放
            auto shape = GetSelectedShape();
            if (shape) {
                Point center = transformAnchor;
                double dist1 = dragStart.DistanceTo(center);
                double dist2 = p.DistanceTo(center);
                double scale = (dist1 > 10) ? (dist2 / dist1) : 1.0; // 避免过小的距离
                // 限制缩放范围在 0.1 到 5.0 之间
                scale = std::max(0.1, std::min(5.0, scale));
                ScaleSelectedShape(scale, scale, center);
            }
            ClearSelection();
            hasTransformAnchor = false;
        }
        return;
    }
    
    // 处理旋转模式
    if (currentMode == DrawMode::Rotate) {
        if (selectedShapeIndex < 0) {
            // 第一次点击：选择图形
            SelectShapeAt(p);
            if (selectedShapeIndex >= 0) {
                // 选中后，设置旋转中心为鼠标点击位置
                transformAnchor = p;
                hasTransformAnchor = true;
                dragStart = p; // 记录初始角度参考点
            }
        } else if (hasTransformAnchor) {
            // 第二次点击：完成旋转
            auto shape = GetSelectedShape();
            if (shape) {
                // 计算旋转角度：从初始点到当前点相对于旋转中心的角度变化
                int dx1 = dragStart.x - transformAnchor.x;
                int dy1 = dragStart.y - transformAnchor.y;
                int dx2 = p.x - transformAnchor.x;
                int dy2 = p.y - transformAnchor.y;
                double angle1 = atan2((double)dy1, (double)dx1);
                double angle2 = atan2((double)dy2, (double)dx2);
                double angleRad = angle2 - angle1;
                RotateSelectedShape(angleRad, transformAnchor);
            }
            ClearSelection();
            hasTransformAnchor = false;
        }
        return;
    }
    
    // 处理设置裁剪窗口模式
    if (currentMode == DrawMode::SetClipWindow) {
        if (!hasTransformAnchor) {
            transformAnchor = p;
            hasTransformAnchor = true;
        } else {
            Rect rect;
            rect.left = std::min(transformAnchor.x, p.x);
            rect.right = std::max(transformAnchor.x, p.x);
            rect.top = std::min(transformAnchor.y, p.y);
            rect.bottom = std::max(transformAnchor.y, p.y);
            SetClipRect(rect);
            hasTransformAnchor = false;
            currentMode = DrawMode::None;
        }
        return;
    }
    
    // 如果处于填充选择模式
    if (isSelectMode) {
        SelectShapeAtPoint(x, y);
        FillSelectedShape();
        isSelectMode = false;
        // 不重置selectedShapeIndex，让填充后的图形保持可选
        return;
    }
    
    if (currentMode == DrawMode::None) return;

    // 处理普通图形绘制
    // 如果没有当前图形,创建新图形
    if (!currentShape) {
        CreateNewShape();
    }

    if (currentShape) {
        currentShape->AddPoint(p);
        isDrawing = true;

        // 检查图形是否完成
        if (currentShape->IsComplete()) {
            shapes.push_back(currentShape);
            currentShape.reset();
            isDrawing = false;
        }
    }
}

void Canvas::OnMouseRightDown(int x, int y) {
    if (!currentShape) return;

    // 多段线:右键完成绘制
    if (auto polyline = std::dynamic_pointer_cast<class Polyline>(currentShape)) {
        if (polyline->GetPointCount() >= 2) {
            polyline->Close();
            shapes.push_back(currentShape);
            currentShape.reset();
            isDrawing = false;
        }
    }
    // B样条:右键完成绘制
    else if (auto bspline = std::dynamic_pointer_cast<BSpline>(currentShape)) {
        if (bspline->GetPointCount() >= 4) {  // 至少4个控制点才能绘制平滑曲线
            bspline->Finish();
            shapes.push_back(currentShape);
            currentShape.reset();
            isDrawing = false;
        }
    }
    // 实验二：多边形 - 右键完成绘制并闭合
    else if (auto polygon = std::dynamic_pointer_cast<class Polygon>(currentShape)) {
        if (polygon->GetVertexCount() >= 3) {  // 至少3个顶点才能形成多边形
            polygon->Close();
            shapes.push_back(currentShape);
            currentShape.reset();
            isDrawing = false;
        }
    }
}

void Canvas::OnMouseMove(int x, int y) {
    previewPoint = Point(x, y);

    if (currentShape && isDrawing) {
        currentShape->SetPreviewPoint(previewPoint);
    }
}

void Canvas::Draw(HDC hdc) {
    // 绘制已完成的图形
    for (const auto& shape : shapes) {
        shape->Draw(hdc);
    }

    // 绘制当前正在绘制的图形
    if (currentShape) {
        currentShape->Draw(hdc);
        currentShape->DrawPreview(hdc);
    }
    
    // 实验二：绘制裁剪窗口
    if (hasClipRect) {
        DrawClipRect(hdc);
    }
    
    // 绘制裁剪窗口预览
    if (currentMode == DrawMode::SetClipWindow && hasTransformAnchor) {
        HPEN hPen = CreatePen(PS_DOT, 1, RGB(255, 0, 255));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        
        int left = std::min(transformAnchor.x, previewPoint.x);
        int right = std::max(transformAnchor.x, previewPoint.x);
        int top = std::min(transformAnchor.y, previewPoint.y);
        int bottom = std::max(transformAnchor.y, previewPoint.y);
        
        ::Rectangle(hdc, left, top, right, bottom);
        
        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);
    }
    
    // 绘制变换预览提示
    if (currentMode == DrawMode::Translate && isDragging && selectedShapeIndex >= 0) {
        // 绘制从起点到当前位置的箭头指示
        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 128, 255));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        
        // 绘制平移向量
        MoveToEx(hdc, dragStart.x, dragStart.y, NULL);
        LineTo(hdc, previewPoint.x, previewPoint.y);
        
        // 绘制箭头
        double angle = atan2((double)(previewPoint.y - dragStart.y), 
                            (double)(previewPoint.x - dragStart.x));
        int arrowLen = 10;
        double arrowAngle = 3.14159 / 6; // 30度
        
        int ax1 = previewPoint.x - (int)(arrowLen * cos(angle - arrowAngle));
        int ay1 = previewPoint.y - (int)(arrowLen * sin(angle - arrowAngle));
        int ax2 = previewPoint.x - (int)(arrowLen * cos(angle + arrowAngle));
        int ay2 = previewPoint.y - (int)(arrowLen * sin(angle + arrowAngle));
        
        LineTo(hdc, ax1, ay1);
        MoveToEx(hdc, previewPoint.x, previewPoint.y, NULL);
        LineTo(hdc, ax2, ay2);
        
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
        
        // 显示平移距离文本
        int dx = previewPoint.x - dragStart.x;
        int dy = previewPoint.y - dragStart.y;
        wchar_t buffer[100];
        swprintf(buffer, 100, L"ΔX=%d, ΔY=%d", dx, dy);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0, 128, 255));
        TextOut(hdc, (dragStart.x + previewPoint.x) / 2, 
                     (dragStart.y + previewPoint.y) / 2 - 20, buffer, wcslen(buffer));
    }
    
    if (currentMode == DrawMode::Scale && hasTransformAnchor && selectedShapeIndex >= 0) {
        // 绘制缩放中心点和参考线
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 128, 0));
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        Ellipse(hdc, transformAnchor.x - 5, transformAnchor.y - 5, 
                     transformAnchor.x + 5, transformAnchor.y + 5);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
        
        // 绘制从中心到初始点和当前点的线
        HPEN hPen = CreatePen(PS_DOT, 1, RGB(255, 128, 0));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        MoveToEx(hdc, transformAnchor.x, transformAnchor.y, NULL);
        LineTo(hdc, dragStart.x, dragStart.y);
        MoveToEx(hdc, transformAnchor.x, transformAnchor.y, NULL);
        LineTo(hdc, previewPoint.x, previewPoint.y);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
        
        // 显示缩放比例
        double dist1 = dragStart.DistanceTo(transformAnchor);
        double dist2 = previewPoint.DistanceTo(transformAnchor);
        double scale = (dist1 > 10) ? (dist2 / dist1) : 1.0;
        scale = std::max(0.1, std::min(5.0, scale));
        wchar_t buffer[50];
        swprintf(buffer, 50, L"缩放: %.2fx", scale);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 128, 0));
        TextOut(hdc, transformAnchor.x + 10, transformAnchor.y + 10, buffer, wcslen(buffer));
    }
    
    if (currentMode == DrawMode::Rotate && hasTransformAnchor && selectedShapeIndex >= 0) {
        // 绘制旋转中心点
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 128));
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        Ellipse(hdc, transformAnchor.x - 5, transformAnchor.y - 5, 
                     transformAnchor.x + 5, transformAnchor.y + 5);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
        
        // 绘制旋转角度指示
        HPEN hPen = CreatePen(PS_DOT, 1, RGB(255, 0, 128));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        MoveToEx(hdc, transformAnchor.x, transformAnchor.y, NULL);
        LineTo(hdc, dragStart.x, dragStart.y);
        MoveToEx(hdc, transformAnchor.x, transformAnchor.y, NULL);
        LineTo(hdc, previewPoint.x, previewPoint.y);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
        
        // 显示旋转角度
        int dx1 = dragStart.x - transformAnchor.x;
        int dy1 = dragStart.y - transformAnchor.y;
        int dx2 = previewPoint.x - transformAnchor.x;
        int dy2 = previewPoint.y - transformAnchor.y;
        double angle1 = atan2((double)dy1, (double)dx1);
        double angle2 = atan2((double)dy2, (double)dx2);
        double angleDeg = (angle2 - angle1) * 180.0 / 3.14159265359;
        wchar_t buffer[50];
        swprintf(buffer, 50, L"旋转: %.1f°", angleDeg);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 0, 128));
        TextOut(hdc, transformAnchor.x + 10, transformAnchor.y + 10, buffer, wcslen(buffer));
    }
}

void Canvas::Clear() {
    shapes.clear();
    currentShape.reset();
    isDrawing = false;
}

void Canvas::FillLastClosedShape(FillAlgorithm algorithm) {
    // 查找最后一个可填充的图形(多段线、圆或矩形)
    // 根据算法选择不同颜色
    COLORREF fillColor = (algorithm == FillAlgorithm::ScanLine) ? 
                         RGB(135, 206, 250) :  // 扫描线 - 浅蓝色
                         RGB(255, 182, 193);   // 栅栏填充 - 浅粉色
    
    for (auto it = shapes.rbegin(); it != shapes.rend(); ++it) {
        // 检查是否是多段线
        if (auto polyline = std::dynamic_pointer_cast<class Polyline>(*it)) {
            if (polyline->IsComplete() && polyline->GetPointCount() >= 3) {
                auto filled = std::make_shared<FilledRegion>(
                    polyline->GetPoints(), algorithm, fillColor);
                shapes.push_back(filled);
                return;
            }
        }
        // 检查是否是圆
        else if (auto circle = std::dynamic_pointer_cast<Circle>(*it)) {
            if (circle->IsComplete()) {
                // 获取圆的信息并转换为多边形点集
                std::vector<Point> circlePoints = GetCirclePoints(circle);
                if (circlePoints.size() >= 3) {
                    auto filled = std::make_shared<FilledRegion>(
                        circlePoints, algorithm, fillColor);
                    shapes.push_back(filled);
                    return;
                }
            }
        }
        // 检查是否是矩形
        else if (auto rect = std::dynamic_pointer_cast<class Rectangle>(*it)) {
            if (rect->IsComplete()) {
                // 获取矩形的四个顶点
                std::vector<Point> rectPoints = GetRectanglePoints(rect);
                if (rectPoints.size() == 4) {
                    auto filled = std::make_shared<FilledRegion>(
                        rectPoints, algorithm, fillColor);
                    shapes.push_back(filled);
                    return;
                }
            }
        }
        // 检查是否是任意多边形
        else if (auto polygon = std::dynamic_pointer_cast<class Polygon>(*it)) {
            if (polygon->IsComplete() && polygon->GetVertexCount() >= 3) {
                auto filled = std::make_shared<FilledRegion>(
                    polygon->GetVertices(), algorithm, fillColor);
                shapes.push_back(filled);
                return;
            }
        }
    }
}

void Canvas::FillRegion(const std::vector<Point>& points, FillAlgorithm algorithm) {
    if (points.size() >= 3) {
        auto filled = std::make_shared<FilledRegion>(points, algorithm, RGB(100, 150, 255));
        shapes.push_back(filled);
    }
}

void Canvas::StartSelectModeForFill(FillAlgorithm algorithm) {
    isSelectMode = true;
    pendingFillAlgorithm = algorithm;
    selectedShapeIndex = -1;
}

void Canvas::SelectShapeAtPoint(int x, int y) {
    // 从后往前遍历(选择最上层的图形)
    for (int i = (int)shapes.size() - 1; i >= 0; i--) {
        // 检查多段线
        if (auto polyline = std::dynamic_pointer_cast<class Polyline>(shapes[i])) {
            if (polyline->IsComplete() && polyline->GetPointCount() >= 3) {
                // 简单的点在多边形内判断
                const auto& pts = polyline->GetPoints();
                bool inside = false;
                for (size_t j = 0, k = pts.size() - 1; j < pts.size(); k = j++) {
                    if (((pts[j].y > y) != (pts[k].y > y)) &&
                        (x < (pts[k].x - pts[j].x) * (y - pts[j].y) / (pts[k].y - pts[j].y) + pts[j].x)) {
                        inside = !inside;
                    }
                }
                if (inside) {
                    selectedShapeIndex = i;
                    return;
                }
            }
        }
        // 检查圆
        else if (auto circle = std::dynamic_pointer_cast<Circle>(shapes[i])) {
            if (circle->IsComplete()) {
                Point center = circle->GetCenter();
                int radius = circle->GetRadius();
                int dx = x - center.x;
                int dy = y - center.y;
                if (dx * dx + dy * dy <= radius * radius) {
                    selectedShapeIndex = i;
                    return;
                }
            }
        }
        // 检查矩形
        else if (auto rect = std::dynamic_pointer_cast<class Rectangle>(shapes[i])) {
            if (rect->IsComplete()) {
                Point tl = rect->GetTopLeft();
                Point br = rect->GetBottomRight();
                int minX = std::min(tl.x, br.x);
                int maxX = std::max(tl.x, br.x);
                int minY = std::min(tl.y, br.y);
                int maxY = std::max(tl.y, br.y);
                if (x >= minX && x <= maxX && y >= minY && y <= maxY) {
                    selectedShapeIndex = i;
                    return;
                }
            }
        }
        // 检查任意多边形
        else if (auto polygon = std::dynamic_pointer_cast<class Polygon>(shapes[i])) {
            if (polygon->IsComplete() && polygon->GetVertexCount() >= 3) {
                // 点在多边形内判断
                const auto& verts = polygon->GetVertices();
                bool inside = false;
                for (size_t j = 0, k = verts.size() - 1; j < verts.size(); k = j++) {
                    if (((verts[j].y > y) != (verts[k].y > y)) &&
                        (x < (verts[k].x - verts[j].x) * (y - verts[j].y) / (verts[k].y - verts[j].y) + verts[j].x)) {
                        inside = !inside;
                    }
                }
                if (inside) {
                    selectedShapeIndex = i;
                    return;
                }
            }
        }
    }
}

void Canvas::FillSelectedShape() {
    if (selectedShapeIndex < 0 || selectedShapeIndex >= (int)shapes.size()) {
        return;
    }
    
    COLORREF fillColor = (pendingFillAlgorithm == FillAlgorithm::ScanLine) ? 
                         RGB(135, 206, 250) :  // 扫描线 - 浅蓝色
                         RGB(255, 182, 193);   // 栅栏填充 - 浅粉色
    
    auto& shape = shapes[selectedShapeIndex];
    
    // 检查是否是多段线
    if (auto polyline = std::dynamic_pointer_cast<class Polyline>(shape)) {
        if (polyline->IsComplete() && polyline->GetPointCount() >= 3) {
            auto filled = std::make_shared<FilledRegion>(
                polyline->GetPoints(), pendingFillAlgorithm, fillColor);
            shapes.push_back(filled);
        }
    }
    // 检查是否是圆
    else if (auto circle = std::dynamic_pointer_cast<Circle>(shape)) {
        if (circle->IsComplete()) {
            std::vector<Point> circlePoints = GetCirclePoints(circle);
            if (circlePoints.size() >= 3) {
                auto filled = std::make_shared<FilledRegion>(
                    circlePoints, pendingFillAlgorithm, fillColor);
                shapes.push_back(filled);
            }
        }
    }
    // 检查是否是矩形
    else if (auto rect = std::dynamic_pointer_cast<class Rectangle>(shape)) {
        if (rect->IsComplete()) {
            std::vector<Point> rectPoints = GetRectanglePoints(rect);
            if (rectPoints.size() == 4) {
                auto filled = std::make_shared<FilledRegion>(
                    rectPoints, pendingFillAlgorithm, fillColor);
                shapes.push_back(filled);
            }
        }
    }
    // 检查是否是任意多边形
    else if (auto polygon = std::dynamic_pointer_cast<class Polygon>(shape)) {
        if (polygon->IsComplete() && polygon->GetVertexCount() >= 3) {
            auto filled = std::make_shared<FilledRegion>(
                polygon->GetVertices(), pendingFillAlgorithm, fillColor);
            shapes.push_back(filled);
        }
    }
}

std::vector<Point> Canvas::GetCirclePoints(std::shared_ptr<Circle> circle) {
    std::vector<Point> points;
    if (!circle) return points;
    
    Point center = circle->GetCenter();
    int radius = circle->GetRadius();
    
    // 将圆近似为多边形(使用60个点)
    const int numPoints = 60;
    for (int i = 0; i < numPoints; i++) {
        double angle = 2.0 * 3.14159265359 * i / numPoints;
        int x = center.x + (int)(radius * cos(angle));
        int y = center.y + (int)(radius * sin(angle));
        points.push_back(Point(x, y));
    }
    
    return points;
}

std::vector<Point> Canvas::GetRectanglePoints(std::shared_ptr<class Rectangle> rect) {
    std::vector<Point> points;
    if (!rect) return points;
    
    Point topLeft = rect->GetTopLeft();
    Point bottomRight = rect->GetBottomRight();
    
    // 矩形的四个顶点(按顺时针或逆时针顺序)
    points.push_back(topLeft);                                          // 左上
    points.push_back(Point(bottomRight.x, topLeft.y));                  // 右上
    points.push_back(bottomRight);                                      // 右下
    points.push_back(Point(topLeft.x, bottomRight.y));                  // 左下
    
    return points;
}

std::vector<Point> Canvas::GetPolylinePoints(std::shared_ptr<class Polyline> polyline) {
    std::vector<Point> points;
    if (!polyline || !polyline->IsComplete()) return points;
    
    // 多段线本身就是点集，直接返回
    return polyline->GetPoints();
}

std::vector<Point> Canvas::GetBSplinePoints(std::shared_ptr<BSpline> bspline) {
    std::vector<Point> points;
    if (!bspline || !bspline->IsComplete()) return points;
    
    // B样条曲线需要将控制点转换为闭合多边形
    // 使用控制点形成的凸包作为近似多边形
    // 这里简单地使用控制点作为多边形顶点
    // 更精确的方法是对曲线进行采样
    const auto& controlPoints = bspline->GetPoints();
    if (controlPoints.size() < 3) return points;
    
    // 将控制点作为多边形顶点返回
    for (const auto& pt : controlPoints) {
        points.push_back(pt);
    }
    
    return points;
}

void Canvas::CreateNewShape() {
    switch (currentMode) {
    case DrawMode::Line:
        currentShape = std::make_shared<Line>(LineAlgorithm::GDI);
        break;
    case DrawMode::LineMidpoint:
        currentShape = std::make_shared<Line>(LineAlgorithm::Midpoint);
        break;
    case DrawMode::LineBresenham:
        currentShape = std::make_shared<Line>(LineAlgorithm::Bresenham);
        break;
    case DrawMode::Circle:
        currentShape = std::make_shared<Circle>(CircleAlgorithm::GDI);
        break;
    case DrawMode::CircleMidpoint:
        currentShape = std::make_shared<Circle>(CircleAlgorithm::Midpoint);
        break;
    case DrawMode::CircleBresenham:
        currentShape = std::make_shared<Circle>(CircleAlgorithm::Bresenham);
        break;
    case DrawMode::Rectangle:
        currentShape = std::make_shared<class Rectangle>();
        break;
    case DrawMode::Polyline:
        currentShape = std::make_shared<class Polyline>();
        break;
    case DrawMode::BSpline:
        currentShape = std::make_shared<BSpline>();
        break;
    // 实验二新增
    case DrawMode::Polygon:
        currentShape = std::make_shared<class Polygon>();
        break;
    default:
        break;
    }
}

// ==================== 实验二：图形选择功能 ====================

void Canvas::SelectShapeAt(const Point& p) {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < (int)shapes.size()) {
        shapes[selectedShapeIndex]->SetSelected(false);
    }
    
    selectedShapeIndex = -1;
    for (int i = shapes.size() - 1; i >= 0; i--) {
        if (shapes[i]->HitTest(p, 5)) {
            selectedShapeIndex = i;
            shapes[i]->SetSelected(true);
            break;
        }
    }
}

int Canvas::GetSelectedShapeIndex() const {
    return selectedShapeIndex;
}

std::shared_ptr<Shape> Canvas::GetSelectedShape() {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < (int)shapes.size()) {
        return shapes[selectedShapeIndex];
    }
    return nullptr;
}

void Canvas::ClearSelection() {
    if (selectedShapeIndex >= 0 && selectedShapeIndex < (int)shapes.size()) {
        shapes[selectedShapeIndex]->SetSelected(false);
    }
    selectedShapeIndex = -1;
}

// ==================== 实验二：图形变换功能 ====================

void Canvas::TranslateSelectedShape(int dx, int dy) {
    auto shape = GetSelectedShape();
    if (shape) {
        shape->Translate(dx, dy);
    }
}

void Canvas::ScaleSelectedShape(double sx, double sy, const Point& center) {
    auto shape = GetSelectedShape();
    if (shape) {
        shape->Scale(sx, sy, center);
    }
}

void Canvas::RotateSelectedShape(double angleRad, const Point& center) {
    auto shape = GetSelectedShape();
    if (shape) {
        shape->Rotate(angleRad, center);
    }
}

// ==================== 实验二：裁剪功能 ====================

void Canvas::SetClipRect(const Rect& rect) {
    clipRect = rect;
    hasClipRect = true;
}

void Canvas::ClearClipRect() {
    hasClipRect = false;
}

bool Canvas::HasClipRect() const {
    return hasClipRect;
}

Rect Canvas::GetClipRect() const {
    return clipRect;
}

void Canvas::ClipLines(LineClipAlgorithm algorithm) {
    if (!hasClipRect) return;
    
    for (auto& shape : shapes) {
        auto line = std::dynamic_pointer_cast<Line>(shape);
        if (line && line->IsComplete()) {
            Point p1 = line->GetStart();
            Point p2 = line->GetEnd();
            
            bool visible = false;
            if (algorithm == LineClipAlgorithm::CohenSutherland) {
                visible = DrawingAlgorithm::ClipLine_CohenSutherland(clipRect, p1, p2);
            }
            else if (algorithm == LineClipAlgorithm::MidpointSubdivision) {
                visible = DrawingAlgorithm::ClipLine_MidpointSubdivision(clipRect, p1, p2);
            }
            
            if (visible) {
                line->SetEndpoints(p1, p2);
            }
        }
    }
}

void Canvas::ClipPolygons(PolygonClipAlgorithm algorithm) {
    if (!hasClipRect) return;
    
    // 用于存储新创建的填充区域
    std::vector<std::shared_ptr<Shape>> clippedShapes;
    
    for (auto& shape : shapes) {
        std::vector<Point> inVerts;
        bool needsClipping = false;
        
        // 处理多边形
        if (auto polygon = std::dynamic_pointer_cast<class Polygon>(shape)) {
            if (polygon->IsComplete()) {
                inVerts = polygon->GetVertices();
                needsClipping = true;
            }
        }
        // 处理圆形
        else if (auto circle = std::dynamic_pointer_cast<Circle>(shape)) {
            if (circle->IsComplete()) {
                inVerts = GetCirclePoints(circle);
                needsClipping = true;
            }
        }
        // 处理矩形
        else if (auto rect = std::dynamic_pointer_cast<class Rectangle>(shape)) {
            if (rect->IsComplete()) {
                inVerts = GetRectanglePoints(rect);
                needsClipping = true;
            }
        }
        // 处理多段线
        else if (auto polyline = std::dynamic_pointer_cast<class Polyline>(shape)) {
            if (polyline->IsComplete()) {
                inVerts = GetPolylinePoints(polyline);
                needsClipping = true;
            }
        }
        // 处理B样条曲线
        else if (auto bspline = std::dynamic_pointer_cast<BSpline>(shape)) {
            if (bspline->IsComplete()) {
                inVerts = GetBSplinePoints(bspline);
                needsClipping = true;
            }
        }
        
        // 执行裁剪
        if (needsClipping && inVerts.size() >= 3) {
            std::vector<Point> outVerts;
            bool visible = false;
            
            if (algorithm == PolygonClipAlgorithm::SutherlandHodgman) {
                visible = DrawingAlgorithm::ClipPolygon_SutherlandHodgman(clipRect, inVerts, outVerts);
                
                // 更新图形顶点
                if (visible && outVerts.size() >= 3) {
                    // 对于多边形，直接更新顶点
                    if (auto polygon = std::dynamic_pointer_cast<class Polygon>(shape)) {
                        polygon->SetVertices(outVerts);
                    }
                    // 对于其他图形类型，创建新的多边形替换原图形
                    else {
                        auto newPolygon = std::make_shared<class Polygon>();
                        newPolygon->SetVertices(outVerts);
                        newPolygon->Close();
                        // 保持选中状态
                        if (shape->IsSelected()) {
                            newPolygon->SetSelected(true);
                        }
                        shape = newPolygon;
                    }
                }
            }
            else if (algorithm == PolygonClipAlgorithm::WeilerAtherton) {
                // Weiler-Atherton 算法：保留框内部分，用绿色填充显示
                // 注意：WeilerAtherton 可能返回多个裁剪结果
                auto results = DrawingAlgorithm::ClipPolygon_WeilerAtherton(clipRect, inVerts);
                
                // 处理所有裁剪结果
                for (const auto& outVerts : results) {
                    if (outVerts.size() >= 3) {
                        // 创建绿色填充的裁剪区域（保留框内部分）
                        COLORREF greenFillColor = RGB(0, 255, 0);  // 绿色填充
                        auto filledRegion = std::make_shared<FilledRegion>(
                            outVerts, FillAlgorithm::ScanLine, greenFillColor);
                        clippedShapes.push_back(filledRegion);
                        
                        // 创建边框轮廓
                        auto newPolygon = std::make_shared<class Polygon>();
                        newPolygon->SetVertices(outVerts);
                        newPolygon->Close();
                        clippedShapes.push_back(newPolygon);
                    }
                }
            }
        }
    }
    
    // 使用 Weiler-Atherton 时，添加新创建的填充区域
    if (algorithm == PolygonClipAlgorithm::WeilerAtherton && !clippedShapes.empty()) {
        shapes.insert(shapes.end(), clippedShapes.begin(), clippedShapes.end());
    }
}

void Canvas::DrawClipRect(HDC hdc) {
    if (!hasClipRect) return;
    
    HPEN hPen = CreatePen(PS_DASH, 2, RGB(255, 0, 255));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    
    ::Rectangle(hdc, clipRect.left, clipRect.top, clipRect.right, clipRect.bottom);
    
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);
}
