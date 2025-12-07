# Weiler-Atherton 多边形裁剪程序

这是一个使用 C++ 和 Windows GDI 实现的 Weiler-Atherton 多边形裁剪算法演示程序。

## 功能特性

- ✅ 交互式绘制多边形
- ✅ 绘制矩形裁剪框
- ✅ Weiler-Atherton 多边形裁剪算法
- ✅ 实时可视化裁剪结果

## 编译方法

### 使用 Visual Studio

1. 打开 Visual Studio
2. 创建新的 Win32 项目
3. 将所有 `.cpp` 和 `.h` 文件添加到项目中
4. 编译并运行

### 使用 MinGW/g++

```bash
g++ -o WeilerAtherton.exe main.cpp WeilerAtherton.cpp -lgdi32 -luser32 -mwindows -std=c++11 -O2 -municode
```

或直接运行编译脚本：
```bash
.\build.bat
# 或
.\build.ps1
```

### 使用 MSVC 命令行

```bash
cl /EHsc main.cpp WeilerAtherton.cpp /link user32.lib gdi32.lib
```

## 使用说明

### 操作步骤

1. **绘制多边形**
   - 点击"多边形模式"按钮
   - 在画布上左键点击添加顶点
   - 右键点击或点击"完成多边形"按钮完成当前多边形
   - 可以绘制多个多边形

2. **绘制裁剪框**
   - 点击"裁剪框模式"按钮
   - 按住左键拖动鼠标绘制矩形裁剪框

3. **执行裁剪**
   - 点击"执行裁剪"按钮
   - 裁剪后的多边形将以绿色高亮显示

4. **清空画布**
   - 点击"清空画布"按钮清除所有内容

### 界面说明

- **蓝色线条**: 原始多边形
- **红色矩形**: 裁剪框
- **绿色填充**: 裁剪后的结果多边形
- **红色圆点**: 当前正在绘制的多边形的顶点

## 文件结构

```
├── Point.h              # 点和多边形的基础数据结构
├── WeilerAtherton.h     # Weiler-Atherton 算法头文件
├── WeilerAtherton.cpp   # Weiler-Atherton 算法实现
├── main.cpp             # Windows GDI 图形界面主程序
└── README.md            # 说明文档
```

## 算法说明

### Weiler-Atherton 算法

Weiler-Atherton 算法是一种用于多边形裁剪的算法,特别适合处理凹多边形。算法主要步骤:

1. **构建顶点链表**: 将裁剪多边形和被裁剪多边形构建成环形链表
2. **计算交点**: 找出两个多边形边界的所有交点
3. **插入交点**: 将交点插入到相应的链表中
4. **标记进入/退出点**: 标记交点是进入裁剪区域还是退出裁剪区域
5. **遍历提取**: 从进入点开始遍历,交替在两个多边形链表间切换,提取裁剪后的多边形

## 技术细节

- **编程语言**: C++11
- **图形库**: Windows GDI
- **算法**: Weiler-Atherton 多边形裁剪
- **双缓冲**: 使用双缓冲技术避免闪烁

## 系统要求

- Windows 操作系统 (Windows 7 或更高版本)
- C++ 编译器 (Visual Studio, MinGW, 或 MSVC)

## 注意事项

- 多边形至少需要 3 个顶点才能完成
- 裁剪框必须是矩形
- 算法支持凸多边形和凹多边形的裁剪
- 如果多边形完全在裁剪框外,裁剪结果为空

## 示例

1. 绘制一个五边形
2. 绘制一个裁剪矩形,使其与五边形部分相交
3. 点击"执行裁剪"查看裁剪结果

## 开发者

使用 GitHub Copilot 辅助开发

## 许可证

MIT License
