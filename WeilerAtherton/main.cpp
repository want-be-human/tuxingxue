#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <vector>
#include <string>
#include "WeilerAtherton.h"

// 全局变量
HINSTANCE hInst;
HWND hwnd;
std::vector<PolygonShape> polygons;           // 所有多边形
PolygonShape currentPolygon;                  // 当前正在绘制的多边形
PolygonShape clipRect;                        // 裁剪矩形
std::vector<PolygonShape> clippedPolygons;    // 裁剪后的多边形

enum DrawMode {
    MODE_POLYGON,      // 绘制多边形模式
    MODE_CLIP_RECT,    // 绘制裁剪矩形模式
    MODE_NONE          // 无操作模式
};

DrawMode currentMode = MODE_POLYGON;
bool isDrawingRect = false;
Point rectStart;

// 函数声明
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DrawScene(HDC hdc);
void DrawPolygonShape(HDC hdc, const PolygonShape& poly, COLORREF color, int width = 1);
void DrawPoint(HDC hdc, const Point& p, COLORREF color, int size = 3);

// 主函数
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;
    
    // 注册窗口类
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"WeilerAthertonApp";
    
    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, L"窗口注册失败!", L"错误", MB_ICONERROR);
        return 0;
    }
    
    // 创建窗口
    hwnd = CreateWindowEx(
        0,
        L"WeilerAthertonApp",
        L"Weiler-Atherton 多边形裁剪",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1000, 700,
        NULL, NULL, hInstance, NULL
    );
    
    if (!hwnd) {
        MessageBox(NULL, L"窗口创建失败!", L"错误", MB_ICONERROR);
        return 0;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}

// 窗口过程
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static POINT lastPoint = {-1, -1};
    
    switch (msg) {
        case WM_CREATE: {
            // 创建按钮
            CreateWindow(L"BUTTON", L"多边形模式",
                        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        10, 10, 120, 30, hwnd, (HMENU)1, hInst, NULL);
            
            CreateWindow(L"BUTTON", L"裁剪框模式",
                        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        140, 10, 120, 30, hwnd, (HMENU)2, hInst, NULL);
            
            CreateWindow(L"BUTTON", L"执行裁剪",
                        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        270, 10, 120, 30, hwnd, (HMENU)3, hInst, NULL);
            
            CreateWindow(L"BUTTON", L"清空画布",
                        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        400, 10, 120, 30, hwnd, (HMENU)4, hInst, NULL);
            
            CreateWindow(L"BUTTON", L"完成多边形",
                        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                        530, 10, 120, 30, hwnd, (HMENU)5, hInst, NULL);
            
            break;
        }
        
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case 1: // 多边形模式
                    currentMode = MODE_POLYGON;
                    MessageBox(hwnd, L"已切换到多边形绘制模式\n左键点击添加顶点，右键或点击'完成多边形'按钮完成当前多边形",
                             L"模式切换", MB_OK | MB_ICONINFORMATION);
                    break;
                    
                case 2: // 裁剪框模式
                    currentMode = MODE_CLIP_RECT;
                    MessageBox(hwnd, L"已切换到裁剪框绘制模式\n按住左键拖动绘制矩形",
                             L"模式切换", MB_OK | MB_ICONINFORMATION);
                    break;
                    
                case 3: // 执行裁剪
                    if (polygons.empty()) {
                        MessageBox(hwnd, L"请先绘制多边形!", L"提示", MB_OK | MB_ICONWARNING);
                    } else if (clipRect.size() != 4) {
                        MessageBox(hwnd, L"请先绘制裁剪矩形!", L"提示", MB_OK | MB_ICONWARNING);
                    } else {
                        clippedPolygons.clear();
                        for (const auto& poly : polygons) {
                            auto result = WeilerAtherton::clip(poly, clipRect);
                            clippedPolygons.insert(clippedPolygons.end(), result.begin(), result.end());
                        }
                        InvalidateRect(hwnd, NULL, TRUE);
                        
                        if (clippedPolygons.empty()) {
                            MessageBox(hwnd, L"裁剪后没有可见多边形", L"裁剪结果", MB_OK | MB_ICONINFORMATION);
                        } else {
                            wchar_t msg[100];
                            wsprintf(msg, L"裁剪成功！生成 %d 个多边形", clippedPolygons.size());
                            MessageBox(hwnd, msg, L"裁剪结果", MB_OK | MB_ICONINFORMATION);
                        }
                    }
                    break;
                    
                case 4: // 清空画布
                    polygons.clear();
                    currentPolygon.clear();
                    clipRect.clear();
                    clippedPolygons.clear();
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                    
                case 5: // 完成多边形
                    if (currentPolygon.size() >= 3) {
                        polygons.push_back(currentPolygon);
                        currentPolygon.clear();
                        InvalidateRect(hwnd, NULL, TRUE);
                    } else if (!currentPolygon.empty()) {
                        MessageBox(hwnd, L"多边形至少需要3个顶点!", L"提示", MB_OK | MB_ICONWARNING);
                    }
                    break;
            }
            break;
        }
        
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            if (y < 50) break; // 避免点击按钮区域
            
            if (currentMode == MODE_POLYGON) {
                // 添加多边形顶点
                currentPolygon.push_back(Point(x, y));
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (currentMode == MODE_CLIP_RECT) {
                // 开始绘制矩形
                isDrawingRect = true;
                rectStart = Point(x, y);
                clipRect.clear();
            }
            break;
        }
        
        case WM_MOUSEMOVE: {
            if (isDrawingRect && (wParam & MK_LBUTTON)) {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                
                // 更新矩形
                clipRect.clear();
                clipRect.push_back(rectStart);
                clipRect.push_back(Point(x, rectStart.y));
                clipRect.push_back(Point(x, y));
                clipRect.push_back(Point(rectStart.x, y));
                
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        
        case WM_LBUTTONUP: {
            if (isDrawingRect) {
                isDrawingRect = false;
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                
                // 完成矩形
                clipRect.clear();
                clipRect.push_back(rectStart);
                clipRect.push_back(Point(x, rectStart.y));
                clipRect.push_back(Point(x, y));
                clipRect.push_back(Point(rectStart.x, y));
                
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        
        case WM_RBUTTONDOWN: {
            // 右键完成当前多边形
            if (currentMode == MODE_POLYGON && currentPolygon.size() >= 3) {
                polygons.push_back(currentPolygon);
                currentPolygon.clear();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // 创建双缓冲
            RECT rect;
            GetClientRect(hwnd, &rect);
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
            
            // 填充背景
            FillRect(hdcMem, &rect, (HBRUSH)(COLOR_WINDOW + 1));
            
            DrawScene(hdcMem);
            
            // 复制到屏幕
            BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcMem, 0, 0, SRCCOPY);
            
            // 清理
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            
            EndPaint(hwnd, &ps);
            break;
        }
        
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    
    return 0;
}

// 绘制场景
void DrawScene(HDC hdc) {
    // 绘制已完成的多边形
    for (const auto& poly : polygons) {
        DrawPolygonShape(hdc, poly, RGB(0, 0, 255), 2);
    }
    
    // 绘制当前正在绘制的多边形
    if (!currentPolygon.empty()) {
        DrawPolygonShape(hdc, currentPolygon, RGB(100, 100, 255), 1);
        for (const auto& p : currentPolygon) {
            DrawPoint(hdc, p, RGB(255, 0, 0), 4);
        }
    }
    
    // 绘制裁剪矩形
    if (clipRect.size() == 4) {
        DrawPolygonShape(hdc, clipRect, RGB(255, 0, 0), 2);
    }
    
    // 绘制裁剪后的多边形
    if (!clippedPolygons.empty()) {
        for (const auto& poly : clippedPolygons) {
            DrawPolygonShape(hdc, poly, RGB(0, 200, 0), 3);
            
            // 填充裁剪后的多边形
            if (poly.size() >= 3) {
                POINT* points = new POINT[poly.size()];
                for (size_t i = 0; i < poly.size(); i++) {
                    points[i].x = (LONG)poly[i].x;
                    points[i].y = (LONG)poly[i].y;
                }
                
                HBRUSH hBrush = CreateSolidBrush(RGB(200, 255, 200));
                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
                HPEN hOldPen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));
                
                ::Polygon(hdc, points, (int)poly.size());  // 使用全局作用域的 Polygon 函数
                
                SelectObject(hdc, hOldPen);
                SelectObject(hdc, hOldBrush);
                DeleteObject(hBrush);
                delete[] points;
            }
        }
    }
    
    // 绘制说明文字
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));
    
    wchar_t info[200];
    wsprintf(info, L"当前模式: %s | 多边形数: %d | 裁剪结果: %d",
            currentMode == MODE_POLYGON ? L"绘制多边形" : L"绘制裁剪框",
            polygons.size(),
            clippedPolygons.size());
    TextOut(hdc, 10, 600, info, wcslen(info));
}

// 绘制多边形
void DrawPolygonShape(HDC hdc, const PolygonShape& poly, COLORREF color, int width) {
    if (poly.size() < 2) return;
    
    HPEN hPen = CreatePen(PS_SOLID, width, color);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    
    MoveToEx(hdc, (int)poly[0].x, (int)poly[0].y, NULL);
    for (size_t i = 1; i < poly.size(); i++) {
        LineTo(hdc, (int)poly[i].x, (int)poly[i].y);
    }
    LineTo(hdc, (int)poly[0].x, (int)poly[0].y);
    
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

// 绘制点
void DrawPoint(HDC hdc, const Point& p, COLORREF color, int size) {
    HBRUSH hBrush = CreateSolidBrush(color);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    
    Ellipse(hdc, (int)p.x - size, (int)p.y - size,
            (int)p.x + size, (int)p.y + size);
    
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
}
