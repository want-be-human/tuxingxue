#pragma once
#include <windows.h>
#include <vector>
#include <memory>
#include "Shape.h"
#include "Point.h"
#include "DrawingAlgorithm.h"

// 绘制模式枚举
enum class DrawMode {
    None,
    Line,
    LineMidpoint,
    LineBresenham,
    Circle,
    CircleMidpoint,
    CircleBresenham,
    Rectangle,
    Polyline,
    BSpline,
    // 实验二新增
    Polygon,           // 任意多边形
    Translate,         // 平移模式
    Scale,             // 缩放模式
    Rotate,            // 旋转模式
    SetClipWindow      // 设置裁剪窗口模式
};

// 画布类 - 管理所有图形和绘制操作
class Canvas {
private:
    std::vector<std::shared_ptr<Shape>> shapes;      // 已完成的图形
    std::shared_ptr<Shape> currentShape;              // 当前正在绘制的图形
    DrawMode currentMode;                             // 当前绘制模式
    Point previewPoint;                               // 鼠标预览点
    bool isDrawing;                                   // 是否正在绘制
    int selectedShapeIndex;                           // 选中的图形索引(-1表示未选中)
    bool isSelectMode;                                // 是否处于选择模式
    FillAlgorithm pendingFillAlgorithm;              // 待执行的填充算法
    
    // 实验二新增
    Rect clipRect;                                    // 裁剪窗口
    bool hasClipRect;                                 // 是否设置了裁剪窗口
    Point transformAnchor;                            // 变换锚点（用于旋转/缩放中心）
    bool hasTransformAnchor;                          // 是否设置了变换锚点
    Point dragStart;                                  // 拖拽起点（用于平移）
    bool isDragging;                                  // 是否正在拖拽
    
    void CreateNewShape();
    
public:
    Canvas();
    void SetDrawMode(DrawMode mode);
    DrawMode GetDrawMode() const;
    void OnMouseLeftDown(int x, int y);
    void OnMouseRightDown(int x, int y);
    void OnMouseMove(int x, int y);
    void Draw(HDC hdc);
    void Clear();
    void FillLastClosedShape(FillAlgorithm algorithm);
    void FillRegion(const std::vector<Point>& points, FillAlgorithm algorithm);
    void StartSelectModeForFill(FillAlgorithm algorithm);
    void SelectShapeAtPoint(int x, int y);
    void FillSelectedShape();
    
    // ==================== 实验二：图形选择功能 ====================
    void SelectShapeAt(const Point& p);
    int GetSelectedShapeIndex() const;
    std::shared_ptr<Shape> GetSelectedShape();
    void ClearSelection();
    
    // ==================== 实验二：图形变换功能 ====================
    void TranslateSelectedShape(int dx, int dy);
    void ScaleSelectedShape(double sx, double sy, const Point& center);
    void RotateSelectedShape(double angleRad, const Point& center);
    
    // ==================== 实验二：裁剪功能 ====================
    void SetClipRect(const Rect& rect);
    void ClearClipRect();
    bool HasClipRect() const;
    Rect GetClipRect() const;
    void ClipLines(LineClipAlgorithm algorithm);
    void ClipPolygons(PolygonClipAlgorithm algorithm);
    void DrawClipRect(HDC hdc);
    
private:
    // 辅助函数：将圆转换为多边形点集
    std::vector<Point> GetCirclePoints(std::shared_ptr<Circle> circle);
    // 辅助函数：将矩形转换为多边形点集
    std::vector<Point> GetRectanglePoints(std::shared_ptr<class Rectangle> rect);
};
