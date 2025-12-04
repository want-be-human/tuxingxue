# 图形学算法实现文档

本文档详细介绍项目中手动实现的五种核心图形学算法，包括算法原理、完整源代码和逐行解释。

---

## 目录

1. [中点画线法 (Midpoint Line Algorithm)](#1-中点画线法)
2. [Bresenham 画线法 (Bresenham Line Algorithm)](#2-bresenham-画线法)
3. [中点画圆法 (Midpoint Circle Algorithm)](#3-中点画圆法)
4. [Bresenham 画圆法 (Bresenham Circle Algorithm)](#4-bresenham-画圆法)
5. [扫描线填充算法 (Scan Line Fill Algorithm)](#5-扫描线填充算法)
6. [栅栏填充算法 (Fence Fill Algorithm)](#6-栅栏填充算法)
7. [三次 B 样条曲线 (Cubic B-Spline Curve)](#7-三次-b-样条曲线)

---

## 1. 中点画线法

### 1.1 算法原理

**中点画线法（Midpoint Line Algorithm）**是一种高效的直线光栅化算法，通过判别式的增量计算避免浮点运算。

#### 核心思想：
- 直线方程：`F(x, y) = ax + by + c = 0`，其中 `a = y1 - y2`，`b = x2 - x1`
- 对于斜率 `0 < k ≤ 1` 的直线，每次 x 递增 1，y 保持不变或递增 1
- 通过判断中点位置决定下一个像素点的选择：
  - 若中点在直线下方，选择上方像素（y++）
  - 若中点在直线上方或在线上，选择当前行像素（y 不变）

#### 判别式：
- 初始判别式：`d0 = 2a + b`
- 当 `d < 0` 时（选择上方像素）：`d' = d + 2(a + b)`
- 当 `d ≥ 0` 时（选择当前行像素）：`d' = d + 2a`

#### 不同斜率的处理：
算法需要处理四种情况以覆盖所有方向的直线：
1. **0 < k ≤ 1**：x 递增，y 可能递增
2. **k > 1**：y 递增，x 可能递增
3. **-1 ≤ k < 0**：x 递增，y 可能递减
4. **k < -1**：y 递减，x 可能递增

### 1.2 完整源代码

```cpp
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
```

### 1.3 代码详解

#### 第一部分：方向处理
```cpp
int dx = x2 - x1;
int dy = y2 - y1;

// 确保从左到右绘制
if (dx < 0) {
    DrawLineMidpoint(hdc, x2, y2, x1, y1, color);
    return;
}
```
- 计算 x 和 y 的增量
- 若 `dx < 0`（从右到左），交换起点和终点，确保统一从左到右绘制
- 简化后续的四种情况处理

#### 第二部分：斜率 0 < k ≤ 1（主循环沿 x 方向）
```cpp
if (dy >= 0 && dy <= dx) {
    int x, y, d0, d1, d2, a, b;
    y = y1;
    a = y1 - y2;          // 直线方程中的a（注意：a为负值）
    b = x2 - x1;          // 直线方程中的b（为正值）
    d0 = 2 * a + b;       // 增量初始值
    d1 = 2 * a;           // d >= 0 时的增量
    d2 = 2 * (a + b);     // d < 0 时的增量
```
- **条件**：`dy >= 0 && dy <= dx`，即斜率在 0 到 1 之间
- **参数计算**：
  - `a = y1 - y2`：直线方程 `F(x,y) = ax + by + c = 0` 中的系数 a
  - `b = x2 - x1`：系数 b（等于 dx）
  - `d0 = 2a + b`：初始判别式，用于判断第一个中点位置
  - `d1 = 2a`：当 `d ≥ 0` 时的增量（选择右侧像素）
  - `d2 = 2(a+b)`：当 `d < 0` 时的增量（选择右上像素）

```cpp
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
```
- **主循环**：x 从 x1 递增到 x2
- **绘制像素**：绘制当前点 `(x, y)`
- **判别**：
  - 若 `d0 < 0`（中点在直线下方）：选择上方像素，`y++`，判别式增加 `d2`
  - 若 `d0 ≥ 0`（中点在直线上方或在线上）：选择当前行像素，y 不变，判别式增加 `d1`

#### 第三部分：斜率 k > 1（主循环沿 y 方向）
```cpp
else if (dy > 0 && dy > dx) {
    int x, y, d0, d1, d2, a, b;
    x = x1;
    a = y1 - y2;
    b = x2 - x1;
    d0 = a + 2 * b;       // 注意：初始判别式公式不同
    d1 = 2 * b;           // d > 0 时的增量
    d2 = 2 * (a + b);     // d <= 0 时的增量
```
- **条件**：`dy > dx`，斜率大于 1
- **角色互换**：此时 y 是主方向，x 是从属方向
- **初始判别式**：`d0 = a + 2b`（与前一种情况的公式对称）

```cpp
    for (y = y1; y <= y2; y++) {
        SetPixelSafe(hdc, x, y, color);
        if (d0 > 0) {       // 注意：判断条件变为 > 0
            x++;
            d0 += d2;
        } else {
            d0 += d1;
        }
    }
}
```
- **主循环**：y 从 y1 递增到 y2
- **判别逻辑**：
  - 若 `d0 > 0`：选择右侧像素，`x++`
  - 若 `d0 ≤ 0`：选择当前列像素，x 不变

#### 第四部分：斜率 -1 ≤ k < 0（向右下）
```cpp
else if (dy < 0 && -dy <= dx) {
    int x, y, d0, d1, d2, a, b;
    y = y1;
    a = y2 - y1;          // 注意：a的计算顺序改变
    b = x2 - x1;
    d0 = 2 * a + b;
    d1 = 2 * a;
    d2 = 2 * (a + b);
```
- **条件**：`dy < 0` 且 `|dy| ≤ dx`
- **参数调整**：`a = y2 - y1`（与第一种情况相反），使 a 为正值

```cpp
    for (x = x1; x <= x2; x++) {
        SetPixelSafe(hdc, x, y, color);
        if (d0 < 0) {
            y--;            // 注意：y递减
            d0 += d2;
        } else {
            d0 += d1;
        }
    }
}
```
- **主循环**：x 递增
- **y 方向**：若需要移动则向下（`y--`）

#### 第五部分：斜率 k < -1（向右下，陡峭）
```cpp
else {
    int x, y, d0, d1, d2, a, b;
    x = x1;
    a = y2 - y1;
    b = x2 - x1;
    d0 = a + 2 * b;
    d1 = 2 * b;
    d2 = 2 * (a + b);
    
    for (y = y1; y >= y2; y--) {    // 注意：y递减
        SetPixelSafe(hdc, x, y, color);
        if (d0 > 0) {
            x++;
            d0 += d2;
        } else {
            d0 += d1;
        }
    }
}
```
- **条件**：`dy < 0` 且 `|dy| > dx`
- **主循环**：y 从 y1 递减到 y2（`y--`）
- **判别逻辑**：与斜率 k > 1 的情况类似，但 y 递减



## 2. Bresenham 画线法

### 2.1 算法原理

**Bresenham 画线法**是最经典的直线光栅化算法，由 Jack E. Bresenham 于 1962 年提出。它完全基于整数运算，效率极高。

#### 核心思想：

- 与中点画线法类似，通过判别式决定下一个像素点
- 判别式的物理意义：测量理想直线与候选像素点之间的距离差
- 完全使用整数加法和位移运算，无需乘法和浮点运算

#### 判别式推导：

对于斜率 `0 < k ≤ 1` 的直线：

- 直线方程：`y = k·x + b`，其中 `k = dy/dx`
- 当 x 递增 1 时，理想 y 值为：`y_ideal = y + k`
- 两个候选像素：
  - **East (E)**：`(x+1, y)` - 保持 y 不变
  - **Northeast (NE)**：`(x+1, y+1)` - y 递增 1
- 判别式：`d = 2·dy - dx`
  - 若 `d < 0`：理想点更接近 E，选择 `(x+1, y)`，增量为 `2·dy`
  - 若 `d ≥ 0`：理想点更接近 NE，选择 `(x+1, y+1)`，增量为 `2·dy - 2·dx`

#### 四种情况处理：

与中点画线法一样，需要分四种情况处理不同斜率：

1. **0 < k ≤ 1**：x 递增，y 可能递增
2. **k > 1**：y 递增，x 可能递增
3. **-1 ≤ k < 0**：x 递增，y 可能递减
4. **k < -1**：y 递减，x 可能递增

### 2.2 完整源代码

```cpp
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
```

### 2.3 代码详解

#### 第一部分：方向处理

```cpp
int dx = x2 - x1;
int dy = y2 - y1;

// 确保从左到右绘制
if (dx < 0) {
    DrawLineBresenham(hdc, x2, y2, x1, y1, color);
    return;
}
```

- 与中点画线法完全相同
- 交换端点确保从左到右绘制
- 简化后续四种情况的处理

#### 第二部分：斜率 0 < k ≤ 1（最基本情况）

```cpp
if (dy >= 0 && dy <= dx) {
    int x, y, d;
    y = y1;
    d = 2 * dy - dx;      // 初始判别式
```

- **条件**：`dy >= 0 && dy <= dx`，斜率在 0 到 1 之间
- **初始化**：
  - `y = y1`：y 的初始值
  - `d = 2·dy - dx`：初始判别式

**判别式推导**：

- 直线的斜率 `k = dy/dx`
- 当 x = x1 时，理想 y 值为 y1
- 当 x = x1 + 1 时，理想 y 值为 `y1 + dy/dx`
- 与两个候选点的距离差：
  - 到 `(x1+1, y1)` 的距离：`dy/dx - 0 = dy/dx`
  - 到 `(x1+1, y1+1)` 的距离：`1 - dy/dx = (dx - dy)/dx`
- 判别式 `d = 2·dy - dx` 的符号决定选择哪个点

```cpp
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
```

- **主循环**：x 从 x1 到 x2
- **判别逻辑**：
  - 若 `d < 0`：选择 East（右侧），y 不变，`d' = d + 2·dy`
  - 若 `d ≥ 0`：选择 NorthEast（右上），`y++`，`d' = d + 2·dy - 2·dx`

**增量公式推导**：

- 选择 E 时：`d_new = 2·dy·(x+2) - dx·(y+k) - ... = d_old + 2·dy`
- 选择 NE 时：`d_new = 2·dy·(x+2) - dx·(y+1+k) - ... = d_old + 2·dy - 2·dx`

#### 第三部分：斜率 k > 1

```cpp
else if (dy > 0 && dy > dx) {
    int x, y, d;
    x = x1;
    d = 2 * dx - dy;      // 主方向变为 y，判别式对称交换
    
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
```

- **主方向交换**：y 为主方向，x 为从属方向
- **初始判别式**：`d = 2·dx - dy`（dx 和 dy 的角色互换）
- **增量**：
  - `d < 0`：选择 North（上方），x 不变，`d += 2·dx`
  - `d ≥ 0`：选择 NorthEast（右上），`x++`，`d += 2·dx - 2·dy`

#### 第四部分：斜率 -1 ≤ k < 0

```cpp
else if (dy < 0 && -dy <= dx) {
    int x, y, d;
    y = y1;
    d = 2 * (-dy) - dx;   // 使用 -dy（绝对值）
    
    for (x = x1; x <= x2; x++) {
        SetPixelSafe(hdc, x, y, color);
        if (d < 0) {
            d += 2 * (-dy);
        } else {
            y--;          // y 递减
            d += 2 * (-dy) - 2 * dx;
        }
    }
}
```

- **dy 取负**：将 `dy` 的符号反转为正值 `-dy`
- **y 递减**：当判别式 `d ≥ 0` 时，y 向下移动（`y--`）
- **增量公式**：使用 `-dy` 替代原公式中的 `dy`

#### 第五部分：斜率 k < -1

```cpp
else {
    int x, y, d;
    x = x1;
    d = 2 * dx - (-dy);   // 主方向为 y，使用 -dy
    
    for (y = y1; y >= y2; y--) {   // y 递减
        SetPixelSafe(hdc, x, y, color);
        if (d < 0) {
            d += 2 * dx;
        } else {
            x++;
            d += 2 * dx - 2 * (-dy);
        }
    }
}
```

- **主方向**：y 递减（从 y1 到 y2，y1 > y2）
- **判别式**：`d = 2·dx - (-dy) = 2·dx + dy`
- **增量**：与斜率 k > 1 类似，但使用 `-dy`

#### Bresenham 与中点画线法的对比

| 特性           | Bresenham 画线法         | 中点画线法                    |
| -------------- | ------------------------ | ----------------------------- |
| **判别式初值** | `d = 2·dy - dx`          | `d = 2a + b`                  |
| **增量公式**   | `2·dy` 和 `2·dy - 2·dx`  | `2a` 和 `2(a+b)`              |
| **参数定义**   | dy, dx                   | a = y1-y2, b = x2-x1          |
| **数学基础**   | 距离判别                 | 中点位置判别                  |
| **效率**       | 略高（更简单的公式）     | 相当                          |
| **结果**       | 完全相同                 | 完全相同                      |

两种算法本质相同，只是推导方式和公式表示不同。

---

## 3. 中点画圆法

### 3.1 算法原理

**中点画圆法**利用圆的**八对称性**和增量决策，只需计算八分之一圆弧（45°），其余部分通过对称得到。

#### 核心思想：
- 圆方程：`F(x, y) = x² + y² - r² = 0`
- 从点 `(0, r)` 开始，沿顺时针方向绘制第一象限的 1/8 圆弧
- 每步有两种选择：
  - **E (东)**：`(x+1, y)` - 水平方向
  - **SE (东南)**：`(x+1, y-1)` - 斜向下
- 中点判别式：`d = F(x+1, y-0.5) = (x+1)² + (y-0.5)² - r²`

#### 判别式更新：
- 初始：`d = 1 - r`
- 若 `d < 0`（中点在圆内）：选择 **E**，`d' = d + 2*x + 3`
- 若 `d ≥ 0`（中点在圆外）：选择 **SE**，`d' = d + 2*(x - y) + 5`

#### 八对称性：
对于圆上一点 `(x, y)`，对称的八个点为：
```
(x, y), (-x, y), (x, -y), (-x, -y)
(y, x), (-y, x), (y, -x), (-y, -x)
```

### 3.2 完整源代码

```cpp
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
```

### 3.3 代码详解

#### 第一部分：初始化
```cpp
int x = 0;              // 起始 x 坐标（从最上方开始）
int y = radius;         // 起始 y 坐标
int d = 1 - radius;     // 初始判别式
```
- 从 `(0, r)` 点开始（圆的最上方）
- `d = 1 - r`：简化后的初始判别式

#### 第二部分：绘制八对称点
```cpp
DrawCirclePoints(hdc, centerX, centerY, x, y, color);
```
- 每计算一个点，立即绘制其对称的八个点

#### 第三部分：主循环（第一象限的1/8圆弧）
```cpp
while (x < y) {         // 只绘制到 45° 处
    if (d < 0) {        // 中点在圆内，选择 E
        d += 2 * x + 3;
    }
    else {              // 中点在圆外，选择 SE
        d += 2 * (x - y) + 5;
        y--;            // y 减 1
    }
    x++;                // x 总是增加
    DrawCirclePoints(hdc, centerX, centerY, x, y, color);
}
```
- `x < y`：只计算 0° 到 45° 的部分
- 判别式更新公式直接套用，无需推导

#### 第四部分：八对称绘制函数
```cpp
void DrawingAlgorithm::DrawCirclePoints(...) {
    // 四个象限的对称点
    SetPixelSafe(hdc, centerX + x, centerY + y, color);  // 第一象限
    SetPixelSafe(hdc, centerX - x, centerY + y, color);  // 第二象限
    SetPixelSafe(hdc, centerX + x, centerY - y, color);  // 第四象限
    SetPixelSafe(hdc, centerX - x, centerY - y, color);  // 第三象限
    
    // x、y 交换后的对称点
    SetPixelSafe(hdc, centerX + y, centerY + x, color);
    SetPixelSafe(hdc, centerX - y, centerY + x, color);
    SetPixelSafe(hdc, centerX + y, centerY - x, color);
    SetPixelSafe(hdc, centerX - y, centerY - x, color);
}
```
- 前四个点：`(x, y)` 在四个象限的对称
- 后四个点：`(y, x)` 在四个象限的对称（x、y 互换）

---

## 4. Bresenham 画圆法

### 4.1 算法原理

**Bresenham 画圆法**是中点画圆法的改进版本，使用更简洁的判别式更新公式。

#### 核心差异：
- 判别式初始值：`d = 3 - 2*r`
- 更新公式更简单：
  - 若 `d < 0`：`d' = d + 4*x + 6`
  - 若 `d ≥ 0`：`d' = d + 4*(x - y) + 10`

#### 优势：
- 公式更简洁
- 计算略快
- 结果与中点法一致

### 4.2 完整源代码

```cpp
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
```

### 4.3 代码详解

#### 与中点法的对比
```cpp
// 中点法
int d = 1 - radius;
while (x < y) {
    if (d < 0) {
        d += 2 * x + 3;
    } else {
        d += 2 * (x - y) + 5;
        y--;
    }
    x++;
}

// Bresenham 法
int d = 3 - 2 * radius;
while (x <= y) {           // 注意：这里是 <=
    if (d < 0) {
        d += 4 * x + 6;
    } else {
        d += 4 * (x - y) + 10;
        y--;
    }
    x++;
}
```

#### 关键差异解析
```cpp
int d = 3 - 2 * radius;    // 初始判别式不同
```
- Bresenham 法的判别式是中点法的两倍，避免了某些边界情况

```cpp
while (x <= y) {           // 循环条件允许 x == y
```
- 包含 45° 处的点，避免遗漏

```cpp
d += 4 * x + 6;            // 增量系数为 4 而非 2
d += 4 * (x - y) + 10;
```
- 公式中系数翻倍，常数项不同
- 数学推导略有差异，但结果一致

---

## 5. 扫描线填充算法

### 5.1 算法原理

**扫描线填充算法（Scan Line Fill）**是最常用的多边形填充方法，通过逐行扫描找出与边界的交点，然后配对填充。

#### 核心思想：
1. 确定多边形的上下边界 `minY` 和 `maxY`
2. 对每条扫描线 `y = minY` 到 `y = maxY`：
   - 计算扫描线与所有边的交点
   - 对交点按 x 坐标排序
   - 交点两两配对，填充中间的像素
3. 绘制多边形边界

#### 交点计算：
对于边 `(x1, y1) → (x2, y2)`，与扫描线 `y` 的交点 x 坐标：
```
x = x1 + (y - y1) * (x2 - x1) / (y2 - y1)
```

#### 配对规则：
- 奇偶规则：第 1-2、3-4、5-6... 个交点之间填充
- 确保交点数为偶数

### 5.2 完整源代码

```cpp
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

            if (y >= p1.y && y < p2.y) {
                if (p2.y != p1.y) {
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
```

### 5.3 代码详解

#### 第一部分：确定扫描范围
```cpp
int minY = points[0].y, maxY = points[0].y;
for (const auto& p : points) {
    if (p.y < minY) minY = p.y;
    if (p.y > maxY) maxY = p.y;
}
```
- 遍历所有顶点，找出最小和最大 y 坐标
- 确定需要扫描的行范围

#### 第二部分：逐行扫描
```cpp
for (int y = minY; y <= maxY; y++) {
    std::vector<int> intersections;  // 存储当前行的交点
```
- 从 `minY` 到 `maxY` 逐行处理
- 为每行创建交点容器

#### 第三部分：计算交点
```cpp
    size_t n = points.size();
    for (size_t i = 0; i < n; i++) {
        Point p1 = points[i];
        Point p2 = points[(i + 1) % n];  // 下一个顶点（首尾相连）
```
- 遍历多边形的每条边
- `(i + 1) % n`：实现循环，最后一个顶点连回第一个

```cpp
        if (p1.y > p2.y) std::swap(p1, p2);  // 确保 p1 在下，p2 在上
```
- 统一边的方向，简化后续判断

```cpp
        if (y >= p1.y && y < p2.y) {  // 扫描线与边相交
```
- `y >= p1.y && y < p2.y`：半开区间，避免顶点重复计数
- 不包括上顶点 `p2.y`，防止共享顶点的两条边都计算交点

```cpp
            if (p2.y != p1.y) {       // 避免水平边（除零）
                int x = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
                intersections.push_back(x);
            }
        }
    }
```
- 线性插值计算交点的 x 坐标
- 公式推导：`(x - p1.x) / (p2.x - p1.x) = (y - p1.y) / (p2.y - p1.y)`

#### 第四部分：排序与填充
```cpp
    std::sort(intersections.begin(), intersections.end());  // 按 x 坐标排序
```
- 排序确保交点从左到右排列

```cpp
    for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
        for (int x = intersections[i]; x <= intersections[i + 1]; x++) {
            SetPixelSafe(hdc, x, y, color);
        }
    }
```
- 奇偶配对：`(0, 1)`, `(2, 3)`, `(4, 5)` ...
- 填充每对交点之间的所有像素

#### 第五部分：绘制边界
```cpp
    size_t n = points.size();
    for (size_t i = 0; i < n; i++) {
        DrawLineBresenham(hdc, points[i].x, points[i].y,
            points[(i + 1) % n].x, points[(i + 1) % n].y, RGB(0, 0, 0));
    }
```
- 用黑色线条绘制多边形边界
- 使用 Bresenham 算法确保精确

---

## 6. 栅栏填充算法

### 6.1 算法原理

**栅栏填充算法（Fence Fill / Parity Fill）**使用**射线法**判断点是否在多边形内部，然后进行**隔行扫描**填充。

#### 核心思想：
1. 确定多边形的包围盒 `[minX, maxX] × [minY, maxY]`
2. 对包围盒内的每个点：
   - 使用射线法判断点是否在多边形内部
   - 若在内部则填充该点
3. 隔行扫描：每隔一行扫描，然后填充相邻行（栅栏效果）

#### 射线法（Ray Casting）：
从点 `(x, y)` 沿水平向右发射射线，统计与多边形边界的交点数：
- **奇数个交点**：点在多边形内部
- **偶数个交点**：点在多边形外部

#### 交点判断条件：
对于边 `(p1, p2)`，射线与边相交当且仅当：
```
(p1.y <= y < p2.y) 或 (p2.y <= y < p1.y)
且 x < p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y)
```

### 6.2 完整源代码

```cpp
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
```

### 6.3 代码详解

#### 第一部分：确定包围盒
```cpp
int minX = points[0].x, maxX = points[0].x;
int minY = points[0].y, maxY = points[0].y;
for (const auto& p : points) {
    if (p.x < minX) minX = p.x;
    if (p.x > maxX) maxX = p.x;
    if (p.y < minY) minY = p.y;
    if (p.y > maxY) maxY = p.y;
}
```
- 遍历所有顶点，找出 x 和 y 的最小最大值
- 确定多边形的轴对齐包围盒（AABB）

#### 第二部分：射线法判断点是否在内部
```cpp
auto isInside = [&](int x, int y) -> bool {
    int crossings = 0;  // 交点计数器
```
- 使用 Lambda 表达式封装判断逻辑
- `[&]`：捕获外部变量 `points`

```cpp
    size_t n = points.size();
    for (size_t i = 0; i < n; i++) {
        Point p1 = points[i];
        Point p2 = points[(i + 1) % n];
```
- 遍历多边形的每条边

```cpp
        if ((p1.y <= y && p2.y > y) || (p1.y > y && p2.y <= y)) {
```
- 判断射线是否与边相交
- 条件 1：`p1.y <= y && p2.y > y`（边从下往上穿过射线）
- 条件 2：`p1.y > y && p2.y <= y`（边从上往下穿过射线）
- 使用 `<=` 和 `<` 的组合避免重复计数

```cpp
            double vt = (double)(y - p1.y) / (p2.y - p1.y);
            if (x < p1.x + vt * (p2.x - p1.x)) {
                crossings++;
            }
```
- `vt`：参数化坐标，表示交点在边上的位置（0 到 1）
- 计算交点的 x 坐标：`x_intersection = p1.x + vt * (p2.x - p1.x)`
- 若点的 x 坐标小于交点 x 坐标，说明射线与边相交

```cpp
        }
    }
    return (crossings % 2) == 1;  // 奇数个交点在内部
};
```
- 奇偶规则：奇数个交点表示点在多边形内部

#### 第三部分：隔行扫描填充
```cpp
for (int y = minY; y <= maxY; y += 2) {  // 每次跳过一行
    for (int x = minX; x <= maxX; x++) {  // 遍历当前行的所有列
        if (isInside(x, y)) {
            SetPixelSafe(hdc, x, y, color);        // 填充当前点
            if (y + 1 <= maxY) {
                SetPixelSafe(hdc, x, y + 1, color);  // 填充下一行的对应点
            }
        }
    }
}
```
- `y += 2`：每次跳过一行（栅栏效果）
- 对于每个检测到在内部的点：
  - 填充当前行 `y`
  - 同时填充下一行 `y + 1`
- 效果：只检测奇数行，但同时填充奇数行和偶数行

#### 第四部分：绘制边界
```cpp
size_t n = points.size();
for (size_t i = 0; i < n; i++) {
    DrawLineBresenham(hdc, points[i].x, points[i].y,
        points[(i + 1) % n].x, points[(i + 1) % n].y, RGB(0, 0, 0));
}
```
- 与扫描线填充相同，用黑色线条绘制边界


---

## 7. 三次 B 样条曲线

### 7.1 算法原理

**三次 B 样条曲线（Cubic B-Spline Curve）**是一种光滑的参数曲线，通过控制点定义曲线形状，但曲线不一定经过控制点。

#### 核心特性：
- **局部控制**：移动一个控制点只影响局部曲线
- **光滑性**：二阶连续可导（C²连续）
- **凸包性质**：曲线在控制点的凸包内

#### 基函数（Basis Functions）：
对于 4 个控制点 `P0, P1, P2, P3` 和参数 `t ∈ [0, 1]`，三次均匀 B 样条的基函数为：

```
B0(t) = (-t³ + 3t² - 3t + 1) / 6
B1(t) = (3t³ - 6t² + 4) / 6
B2(t) = (-3t³ + 3t² + 3t + 1) / 6
B3(t) = t³ / 6
```

#### 曲线点计算：
```
P(t) = B0(t)·P0 + B1(t)·P1 + B2(t)·P2 + B3(t)·P3
```

#### 分段绘制：
- 每 4 个连续控制点生成一段曲线
- n 个控制点可生成 n-3 段曲线
- 例如：6 个控制点生成 3 段曲线
  - 段1：P0, P1, P2, P3
  - 段2：P1, P2, P3, P4
  - 段3：P2, P3, P4, P5

### 7.2 完整源代码

```cpp
// 使用4个控制点和参数t计算B样条曲线上的点
Point BSpline::CalculateCurvePoint(const Point& p0, const Point& p1, 
                                    const Point& p2, const Point& p3, double t) {
    // 三次均匀B样条基函数
    double f1 = (-t*t*t + 3*t*t - 3*t + 1) / 6.0;
    double f2 = (3*t*t*t - 6*t*t + 4) / 6.0;
    double f3 = (-3*t*t*t + 3*t*t + 3*t + 1) / 6.0;
    double f4 = (t*t*t) / 6.0;
    
    double x = f1 * p0.x + f2 * p1.x + f3 * p2.x + f4 * p3.x;
    double y = f1 * p0.y + f2 * p1.y + f3 * p2.y + f4 * p3.y;
    
    return Point((int)(x + 0.5), (int)(y + 0.5));
}

// 绘制完整的B样条曲线
void BSpline::Draw(HDC hdc) {
    if (controlPoints.size() < 4) return;

    // 绘制平滑的B样条曲线(红色)
    HPEN hPenCurve = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPenCurve);

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
            
            // 记录所有由控制点计算得到的曲线点用于标记
            curvePoints.push_back(p);
            
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

    // 绘制所有曲线点的标记(绿色圆圈)
    HPEN hPenGreen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
    hOldPen = (HPEN)SelectObject(hdc, hPenGreen);
    HBRUSH hBrushGreen = CreateSolidBrush(RGB(0, 255, 0));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrushGreen);
    for (const auto& p : curvePoints) {
        Ellipse(hdc, p.x - 2, p.y - 2, p.x + 2, p.y + 2);
    }
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrushGreen);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPenGreen);
}
```

### 7.3 代码详解

#### 第一部分：基函数计算
```cpp
Point BSpline::CalculateCurvePoint(const Point& p0, const Point& p1, 
                                    const Point& p2, const Point& p3, double t) {
```
- 输入：4 个控制点和参数 `t ∈ [0, 1]`
- 输出：曲线上对应的点

```cpp
    // 三次均匀B样条基函数
    double f1 = (-t*t*t + 3*t*t - 3*t + 1) / 6.0;
    double f2 = (3*t*t*t - 6*t*t + 4) / 6.0;
    double f3 = (-3*t*t*t + 3*t*t + 3*t + 1) / 6.0;
    double f4 = (t*t*t) / 6.0;
```
- 计算四个基函数的值
- **f1 = B0(t)**：对应控制点 P0 的权重
  - `t=0` 时：`f1 = 1/6`
  - `t=1` 时：`f1 = 0`
- **f2 = B1(t)**：对应控制点 P1 的权重
  - `t=0` 时：`f2 = 4/6 = 2/3`（最大权重）
  - `t=1` 时：`f2 = 1/6`
- **f3 = B2(t)**：对应控制点 P2 的权重
  - `t=0` 时：`f3 = 1/6`
  - `t=1` 时：`f3 = 2/3`
- **f4 = B3(t)**：对应控制点 P3 的权重
  - `t=0` 时：`f4 = 0`
  - `t=1` 时：`f4 = 1/6`

#### 基函数权重可视化（t = 0.5 时）：
```
f1 = 1/6 ≈ 0.167
f2 = 4/6 ≈ 0.667
f3 = 4/6 ≈ 0.667
f4 = 1/6 ≈ 0.167
总和 = 2.0（注意：不归一化到1，这是B样条的特性）
```

```cpp
    double x = f1 * p0.x + f2 * p1.x + f3 * p2.x + f4 * p3.x;
    double y = f1 * p0.y + f2 * p1.y + f3 * p2.y + f4 * p3.y;
```
- 加权求和：每个控制点乘以对应的基函数值
- 曲线点是控制点的**加权平均**

```cpp
    return Point((int)(x + 0.5), (int)(y + 0.5));
}
```
- 四舍五入转换为整数坐标
- `+ 0.5`：确保正确的舍入方向

#### 第二部分：分段绘制曲线
```cpp
for (size_t i = 0; i + 3 < controlPoints.size(); i++) {
```
- 每次取 4 个连续控制点：`[i, i+1, i+2, i+3]`
- 循环次数 = `控制点数 - 3`
- 例如：6 个控制点会执行 3 次循环

```cpp
    for (int j = 0; j <= segments; j++) {
        double t = (double)j / segments;
```
- 将每段曲线细分为 `segments` 个小段（此处为 20）
- `t` 从 0 到 1 均匀取值：`0, 0.05, 0.1, ..., 0.95, 1.0`

```cpp
        Point p = CalculateCurvePoint(controlPoints[i], controlPoints[i + 1], 
                                     controlPoints[i + 2], controlPoints[i + 3], t);
```
- 调用基函数计算当前参数 `t` 对应的曲线点

```cpp
        curvePoints.push_back(p);  // 记录所有计算出的点
```
- 保存所有曲线点，用于后续绘制绿色标记

```cpp
        if (firstPoint) {
            MoveToEx(hdc, p.x, p.y, NULL);  // 移动到起点
            firstPoint = false;
        } else {
            LineTo(hdc, p.x, p.y);  // 绘制线段到当前点
        }
    }
}
```
- 第一个点：移动画笔到起点
- 后续点：从上一个点连线到当前点
- 形成连续的曲线

#### 第三部分：绘制曲线点标记
```cpp
for (const auto& p : curvePoints) {
    Ellipse(hdc, p.x - 2, p.y - 2, p.x + 2, p.y + 2);
}
```
- 用绿色小圆标记每个计算出的曲线点
- 圆的半径为 2 像素
- 直观展示曲线的离散化程度


