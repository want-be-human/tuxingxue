#pragma once
#include <windows.h>

// 全局变量声明
extern HINSTANCE g_hInst;
extern HWND g_hMainWnd;

// ==================== 实验一菜单和工具栏ID ====================
#define ID_FILE_CLEAR       1001
#define ID_FILE_EXIT        1002

#define ID_LINE_GDI         2001
#define ID_LINE_MIDPOINT    2002
#define ID_LINE_BRESENHAM   2003

#define ID_CIRCLE_GDI       3001
#define ID_CIRCLE_MIDPOINT  3002
#define ID_CIRCLE_BRESENHAM 3003

#define ID_RECTANGLE        4001
#define ID_POLYLINE         4002
#define ID_BSPLINE          4003

#define ID_FILL_SCANLINE    5001
#define ID_FILL_FENCE       5002

// ==================== 实验二菜单和工具栏ID ====================

// 图形绘制
#define ID_POLYGON          6001   // 任意多边形

// 图形选择与变换
// #define ID_SELECT           7001   // 选择图形（已集成到变换操作中）
#define ID_TRANSLATE        7002   // 平移
#define ID_SCALE            7003   // 缩放
#define ID_ROTATE           7004   // 旋转

// 裁剪窗口
#define ID_SET_CLIP_WINDOW  8001   // 设置裁剪窗口
#define ID_CLEAR_CLIP       8002   // 清除裁剪窗口

// 直线裁剪
#define ID_CLIP_LINE_COHEN  8101   // Cohen-Sutherland直线裁剪
#define ID_CLIP_LINE_MIDPT  8102   // 中点分割直线裁剪

// 多边形裁剪
#define ID_CLIP_POLY_SH     8201   // Sutherland-Hodgman多边形裁剪
#define ID_CLIP_POLY_WA     8202   // Weiler-Atherton多边形裁剪

// 函数声明
HMENU CreateMainMenu();
void HandleCommand(WPARAM wParam);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL RegisterMainWindow();
HWND CreateMainWindow();
