#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "MainWindow.h"
#include "Canvas.h"

#pragma comment(lib, "comctl32.lib")

// 全局变量定义
HINSTANCE g_hInst;
HWND g_hMainWnd;
Canvas g_canvas;

// 创建菜单
HMENU CreateMainMenu() {
    HMENU hMenu = CreateMenu();
    HMENU hFileMenu = CreatePopupMenu();
    HMENU hLineMenu = CreatePopupMenu();
    HMENU hCircleMenu = CreatePopupMenu();
    HMENU hShapeMenu = CreatePopupMenu();
    HMENU hFillMenu = CreatePopupMenu();
    
    // 实验二新增菜单
    HMENU hTransformMenu = CreatePopupMenu();
    HMENU hClipMenu = CreatePopupMenu();
    HMENU hLineClipMenu = CreatePopupMenu();
    HMENU hPolyClipMenu = CreatePopupMenu();
    
    // 文件菜单
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_CLEAR, L"清空画布");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_EXIT, L"退出");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"文件");
    
    // 直线菜单
    AppendMenuW(hLineMenu, MF_STRING, ID_LINE_GDI, L"直线 - GDI");
    AppendMenuW(hLineMenu, MF_STRING, ID_LINE_MIDPOINT, L"直线 - 中点法");
    AppendMenuW(hLineMenu, MF_STRING, ID_LINE_BRESENHAM, L"直线 - Bresenham算法");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hLineMenu, L"直线");
    
    // 圆菜单
    AppendMenuW(hCircleMenu, MF_STRING, ID_CIRCLE_GDI, L"圆 - GDI");
    AppendMenuW(hCircleMenu, MF_STRING, ID_CIRCLE_MIDPOINT, L"圆 - 中点法");
    AppendMenuW(hCircleMenu, MF_STRING, ID_CIRCLE_BRESENHAM, L"圆 - Bresenham算法");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hCircleMenu, L"圆");
    
    // 图形菜单 (添加多边形)
    AppendMenuW(hShapeMenu, MF_STRING, ID_RECTANGLE, L"矩形");
    AppendMenuW(hShapeMenu, MF_STRING, ID_POLYLINE, L"多段线");
    AppendMenuW(hShapeMenu, MF_STRING, ID_BSPLINE, L"B样条曲线");
    AppendMenuW(hShapeMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hShapeMenu, MF_STRING, ID_POLYGON, L"任意多边形");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hShapeMenu, L"图形");
    
    // 填充菜单
    AppendMenuW(hFillMenu, MF_STRING, ID_FILL_SCANLINE, L"扫描线填充");
    AppendMenuW(hFillMenu, MF_STRING, ID_FILL_FENCE, L"栅栏填充");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFillMenu, L"填充");
    
    // 变换菜单
    AppendMenuW(hTransformMenu, MF_STRING, ID_TRANSLATE, L"平移");
    AppendMenuW(hTransformMenu, MF_STRING, ID_SCALE, L"缩放");
    AppendMenuW(hTransformMenu, MF_STRING, ID_ROTATE, L"旋转");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hTransformMenu, L"变换");
    
    // 裁剪菜单
    AppendMenuW(hClipMenu, MF_STRING, ID_SET_CLIP_WINDOW, L"设置裁剪窗口");
    AppendMenuW(hClipMenu, MF_STRING, ID_CLEAR_CLIP, L"清除裁剪窗口");
    AppendMenuW(hClipMenu, MF_SEPARATOR, 0, NULL);
    
    // 直线裁剪子菜单
    AppendMenuW(hLineClipMenu, MF_STRING, ID_CLIP_LINE_COHEN, L"Cohen-Sutherland算法");
    AppendMenuW(hLineClipMenu, MF_STRING, ID_CLIP_LINE_MIDPT, L"中点分割算法");
    AppendMenuW(hClipMenu, MF_POPUP, (UINT_PTR)hLineClipMenu, L"直线裁剪");
    
    // 多边形裁剪子菜单
    AppendMenuW(hPolyClipMenu, MF_STRING, ID_CLIP_POLY_SH, L"Sutherland-Hodgman算法");
    AppendMenuW(hPolyClipMenu, MF_STRING, ID_CLIP_POLY_WA, L"Weiler-Atherton算法");
    AppendMenuW(hClipMenu, MF_POPUP, (UINT_PTR)hPolyClipMenu, L"多边形裁剪");
    
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hClipMenu, L"裁剪");
    
    return hMenu;
}

// 处理命令
void HandleCommand(WPARAM wParam) {
    switch (LOWORD(wParam)) {
    case ID_FILE_CLEAR:
        g_canvas.Clear();
        InvalidateRect(g_hMainWnd, NULL, FALSE);
        break;
        
    case ID_FILE_EXIT:
        PostQuitMessage(0);
        break;
        
    case ID_LINE_GDI:
        g_canvas.SetDrawMode(DrawMode::Line);
        break;
        
    case ID_LINE_MIDPOINT:
        g_canvas.SetDrawMode(DrawMode::LineMidpoint);
        break;
        
    case ID_LINE_BRESENHAM:
        g_canvas.SetDrawMode(DrawMode::LineBresenham);
        break;
        
    case ID_CIRCLE_GDI:
        g_canvas.SetDrawMode(DrawMode::Circle);
        break;
        
    case ID_CIRCLE_MIDPOINT:
        g_canvas.SetDrawMode(DrawMode::CircleMidpoint);
        break;
        
    case ID_CIRCLE_BRESENHAM:
        g_canvas.SetDrawMode(DrawMode::CircleBresenham);
        break;
        
    case ID_RECTANGLE:
        g_canvas.SetDrawMode(DrawMode::Rectangle);
        break;
        
    case ID_POLYLINE:
        g_canvas.SetDrawMode(DrawMode::Polyline);
        break;
        
    case ID_BSPLINE:
        g_canvas.SetDrawMode(DrawMode::BSpline);
        break;
        
    case ID_FILL_SCANLINE:
        g_canvas.StartSelectModeForFill(FillAlgorithm::ScanLine);
        MessageBox(g_hMainWnd, L"请点击要填充的封闭图形", L"选择填充", MB_OK | MB_ICONINFORMATION);
        break;
        
    case ID_FILL_FENCE:
        g_canvas.StartSelectModeForFill(FillAlgorithm::Fence);
        MessageBox(g_hMainWnd, L"请点击要填充的封闭图形", L"选择填充", MB_OK | MB_ICONINFORMATION);
        break;
    
    // ==================== 实验二命令处理 ====================
    
    // 多边形绘制
    case ID_POLYGON:
        g_canvas.SetDrawMode(DrawMode::Polygon);
        MessageBox(g_hMainWnd, L"左键点击绘制顶点，右键完成多边形", L"绘制多边形", MB_OK | MB_ICONINFORMATION);
        break;
    
    // 平移
    case ID_TRANSLATE:
        g_canvas.SetDrawMode(DrawMode::Translate);
        MessageBox(g_hMainWnd, L"第一次点击选中图形，第二次点击设定平移后的位置", L"平移", MB_OK | MB_ICONINFORMATION);
        break;
    
    // 缩放
    case ID_SCALE:
        g_canvas.SetDrawMode(DrawMode::Scale);
        MessageBox(g_hMainWnd, L"第一次点击选中图形（以图形中心为缩放中心），第二次点击设定缩放比例", L"缩放", MB_OK | MB_ICONINFORMATION);
        break;
    
    // 旋转
    case ID_ROTATE:
        g_canvas.SetDrawMode(DrawMode::Rotate);
        MessageBox(g_hMainWnd, L"第一次点击选中图形并设定旋转中心，第二次点击设定旋转角度", L"旋转", MB_OK | MB_ICONINFORMATION);
        break;
    
    // 设置裁剪窗口
    case ID_SET_CLIP_WINDOW:
        g_canvas.SetDrawMode(DrawMode::SetClipWindow);
        MessageBox(g_hMainWnd, L"拖动鼠标绘制裁剪窗口矩形", L"设置裁剪窗口", MB_OK | MB_ICONINFORMATION);
        break;
    
    // 清除裁剪窗口
    case ID_CLEAR_CLIP:
        g_canvas.ClearClipRect();
        InvalidateRect(g_hMainWnd, NULL, FALSE);
        MessageBox(g_hMainWnd, L"裁剪窗口已清除", L"清除裁剪窗口", MB_OK | MB_ICONINFORMATION);
        break;
    
    // Cohen-Sutherland直线裁剪
    case ID_CLIP_LINE_COHEN:
        if (g_canvas.HasClipRect()) {
            g_canvas.ClipLines(LineClipAlgorithm::CohenSutherland);
            InvalidateRect(g_hMainWnd, NULL, FALSE);
            MessageBox(g_hMainWnd, L"已使用Cohen-Sutherland算法裁剪所有直线", L"直线裁剪", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBox(g_hMainWnd, L"请先设置裁剪窗口！", L"错误", MB_OK | MB_ICONWARNING);
        }
        break;
    
    // 中点分割直线裁剪
    case ID_CLIP_LINE_MIDPT:
        if (g_canvas.HasClipRect()) {
            g_canvas.ClipLines(LineClipAlgorithm::MidpointSubdivision);
            InvalidateRect(g_hMainWnd, NULL, FALSE);
            MessageBox(g_hMainWnd, L"已使用中点分割算法裁剪所有直线", L"直线裁剪", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBox(g_hMainWnd, L"请先设置裁剪窗口！", L"错误", MB_OK | MB_ICONWARNING);
        }
        break;
    
    // Sutherland-Hodgman多边形裁剪
    case ID_CLIP_POLY_SH:
        if (g_canvas.HasClipRect()) {
            g_canvas.ClipPolygons(PolygonClipAlgorithm::SutherlandHodgman);
            InvalidateRect(g_hMainWnd, NULL, FALSE);
            MessageBox(g_hMainWnd, L"已使用Sutherland-Hodgman算法裁剪所有多边形", L"多边形裁剪", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBox(g_hMainWnd, L"请先设置裁剪窗口！", L"错误", MB_OK | MB_ICONWARNING);
        }
        break;
    
    // Weiler-Atherton多边形裁剪
    case ID_CLIP_POLY_WA:
        if (g_canvas.HasClipRect()) {
            g_canvas.ClipPolygons(PolygonClipAlgorithm::WeilerAtherton);
            InvalidateRect(g_hMainWnd, NULL, FALSE);
            MessageBox(g_hMainWnd, L"已使用Weiler-Atherton算法裁剪所有多边形", L"多边形裁剪", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBox(g_hMainWnd, L"请先设置裁剪窗口！", L"错误", MB_OK | MB_ICONWARNING);
        }
        break;
    }
}

// 窗口过程
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND:
        HandleCommand(wParam);
        break;
        
    case WM_LBUTTONDOWN: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        g_canvas.OnMouseLeftDown(x, y);
        InvalidateRect(hwnd, NULL, FALSE);
        break;
    }
    
    case WM_RBUTTONDOWN: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        g_canvas.OnMouseRightDown(x, y);
        InvalidateRect(hwnd, NULL, FALSE);
        break;
    }
    
    case WM_MOUSEMOVE: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        g_canvas.OnMouseMove(x, y);
        InvalidateRect(hwnd, NULL, FALSE);
        break;
    }

    case WM_ERASEBKGND:
        // 我们在WM_PAINT里用双缓冲完整重绘背景，因此禁止系统擦除，减少闪烁
        return 1;
    
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        
        // 创建内存DC进行双缓冲
        RECT rect;
        GetClientRect(hwnd, &rect);
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
        HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
        
        // 填充背景
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdcMem, &rect, hBrush);
        DeleteObject(hBrush);
        
        // 绘制图形
        g_canvas.Draw(hdcMem);
        
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

// 注册窗口类
BOOL RegisterMainWindow() {
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = g_hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    // 由我们自己在双缓冲中清屏，避免系统背景擦除导致的闪烁
    wc.hbrBackground = NULL;
    wc.lpszClassName = L"GraphicsDrawingApp";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    return RegisterClassEx(&wc);
}

// 创建主窗口
HWND CreateMainWindow() {
    return CreateWindowEx(
        0,
        L"GraphicsDrawingApp",
        L"2023112605乔鹏博",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1024, 768,
        NULL, CreateMainMenu(), g_hInst, NULL
    );
}

// WinMain入口点
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_hInst = hInstance;
    
    // 初始化通用控件
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);
    
    // 注册窗口类
    if (!RegisterMainWindow()) {
        MessageBox(NULL, L"窗口类注册失败！", L"错误", MB_ICONERROR);
        return 0;
    }
    
    // 创建主窗口
    g_hMainWnd = CreateMainWindow();
    if (!g_hMainWnd) {
        MessageBox(NULL, L"窗口创建失败！", L"错误", MB_ICONERROR);
        return 0;
    }
    
    ShowWindow(g_hMainWnd, nCmdShow);
    UpdateWindow(g_hMainWnd);
    
    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
