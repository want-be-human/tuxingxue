#include "WeilerAtherton.h"
#include <algorithm>
#include <cmath>

const double EPSILON = 1e-9;

// 计算两条线段的交点
bool WeilerAtherton::lineIntersection(const Point& p1, const Point& p2,
                                      const Point& p3, const Point& p4,
                                      Point& intersection, double& alpha, double& beta) {
    double x1 = p1.x, y1 = p1.y;
    double x2 = p2.x, y2 = p2.y;
    double x3 = p3.x, y3 = p3.y;
    double x4 = p4.x, y4 = p4.y;
    
    double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    
    if (std::abs(denom) < EPSILON) {
        return false; // 平行或重合
    }
    
    alpha = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
    beta = ((x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)) / denom;
    
    // 检查交点是否在两条线段上
    if (alpha > EPSILON && alpha < 1.0 - EPSILON && 
        beta > EPSILON && beta < 1.0 - EPSILON) {
        intersection.x = x1 + alpha * (x2 - x1);
        intersection.y = y1 + alpha * (y2 - y1);
        return true;
    }
    
    return false;
}

// 判断点是否在多边形内部 (射线法)
bool WeilerAtherton::isPointInPolygon(const Point& p, const PolygonShape& poly) {
    int n = poly.size();
    if (n < 3) return false;
    
    int count = 0;
    for (int i = 0; i < n; i++) {
        Point p1 = poly[i];
        Point p2 = poly[(i + 1) % n];
        
        if ((p1.y <= p.y && p2.y > p.y) || (p1.y > p.y && p2.y <= p.y)) {
            double x_intersect = p1.x + (p.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
            if (p.x < x_intersect) {
                count++;
            }
        }
    }
    
    return (count % 2) == 1;
}

// 构建顶点链表
Vertex* WeilerAtherton::buildVertexList(const PolygonShape& poly) {
    if (poly.empty()) return nullptr;
    
    Vertex* head = new Vertex(poly[0]);
    Vertex* current = head;
    
    for (size_t i = 1; i < poly.size(); i++) {
        Vertex* v = new Vertex(poly[i]);
        current->next = v;
        current = v;
    }
    current->next = head; // 形成环形链表
    
    return head;
}

// 插入交点
void WeilerAtherton::insertIntersections(Vertex* subjectHead, Vertex* clipHead,
                                         const PolygonShape& subject, const PolygonShape& clip) {
    Vertex* s = subjectHead;
    int sIndex = 0;
    
    do {
        Vertex* sNext = s->next;
        Point p1 = s->point;
        Point p2 = sNext->point;
        
        std::vector<std::pair<Vertex*, double>> intersections;
        
        Vertex* c = clipHead;
        int cIndex = 0;
        
        do {
            Vertex* cNext = c->next;
            Point p3 = c->point;
            Point p4 = cNext->point;
            
            Point inter;
            double alpha, beta;
            
            if (lineIntersection(p1, p2, p3, p4, inter, alpha, beta)) {
                // 在 subject 边上创建交点
                Vertex* sInter = new Vertex(inter, VertexType::INTERSECTION);
                sInter->alpha = alpha;
                
                // 在 clip 边上创建交点
                Vertex* cInter = new Vertex(inter, VertexType::INTERSECTION);
                cInter->alpha = beta;
                
                // 建立邻居关系
                sInter->neighbor = cInter;
                cInter->neighbor = sInter;
                
                intersections.push_back({sInter, alpha});
                
                // 将交点插入 clip 链表
                std::vector<std::pair<Vertex*, double>> clipIntersections;
                Vertex* temp = c->next;
                while (temp != cNext) {
                    if (temp->type == VertexType::INTERSECTION) {
                        clipIntersections.push_back({temp, temp->alpha});
                    }
                    temp = temp->next;
                }
                clipIntersections.push_back({cInter, beta});
                
                // 按 alpha 排序
                std::sort(clipIntersections.begin(), clipIntersections.end(),
                         [](const std::pair<Vertex*, double>& a, const std::pair<Vertex*, double>& b) { 
                             return a.second < b.second; 
                         });
                
                // 重新插入到 clip 链表
                Vertex* cCurrent = c;
                for (const auto& pair : clipIntersections) {
                    pair.first->next = cCurrent->next;
                    cCurrent->next = pair.first;
                    cCurrent = pair.first;
                }
                cCurrent->next = cNext;
            }
            
            c = cNext;
            cIndex++;
        } while (c != clipHead);
        
        // 按 alpha 值排序交点
        std::sort(intersections.begin(), intersections.end(),
                 [](const std::pair<Vertex*, double>& a, const std::pair<Vertex*, double>& b) { 
                     return a.second < b.second; 
                 });
        
        // 将排序后的交点插入 subject 链表
        Vertex* sCurrent = s;
        for (const auto& pair : intersections) {
            pair.first->next = sCurrent->next;
            sCurrent->next = pair.first;
            sCurrent = pair.first;
        }
        
        s = sNext;
        sIndex++;
    } while (s != subjectHead);
}

// 标记进入/退出点
void WeilerAtherton::markEntryExit(Vertex* subjectHead, const PolygonShape& clipPoly) {
    Vertex* v = subjectHead;
    bool inside = isPointInPolygon(v->point, clipPoly);
    
    do {
        if (v->type == VertexType::INTERSECTION) {
            v->isEntry = !inside;
            inside = !inside;
        }
        v = v->next;
    } while (v != subjectHead);
}

// 遍历并提取裁剪后的多边形
std::vector<PolygonShape> WeilerAtherton::tracePolygons(Vertex* subjectHead) {
    std::vector<PolygonShape> result;
    
    Vertex* v = subjectHead;
    do {
        if (v->type == VertexType::INTERSECTION && v->isEntry && !v->visited) {
            PolygonShape poly;
            Vertex* current = v;
            
            do {
                current->visited = true;
                current->neighbor->visited = true;
                poly.push_back(current->point);
                
                if (current->isEntry) {
                    // 沿着 subject 多边形前进
                    current = current->next;
                    while (current->type != VertexType::INTERSECTION) {
                        poly.push_back(current->point);
                        current = current->next;
                    }
                } else {
                    // 沿着 clip 多边形前进
                    current = current->next;
                    while (current->type != VertexType::INTERSECTION) {
                        poly.push_back(current->point);
                        current = current->next;
                    }
                }
                
                // 切换到另一个多边形
                current = current->neighbor;
                
            } while (current != v && !current->visited);
            
            if (poly.size() >= 3) {
                result.push_back(poly);
            }
        }
        v = v->next;
    } while (v != subjectHead);
    
    return result;
}

// 释放顶点链表
void WeilerAtherton::freeVertexList(Vertex* head) {
    if (!head) return;
    
    Vertex* current = head;
    do {
        Vertex* next = current->next;
        delete current;
        current = next;
    } while (current != head);
}

// 执行 Weiler-Atherton 裁剪
std::vector<PolygonShape> WeilerAtherton::clip(const PolygonShape& subject, const PolygonShape& clipPoly) {
    if (subject.size() < 3 || clipPoly.size() < 3) {
        return {};
    }
    
    // 构建顶点链表
    Vertex* subjectHead = buildVertexList(subject);
    Vertex* clipHead = buildVertexList(clipPoly);
    
    // 插入交点
    insertIntersections(subjectHead, clipHead, subject, clipPoly);
    
    // 标记进入/退出点
    markEntryExit(subjectHead, clipPoly);
    
    // 提取裁剪后的多边形
    std::vector<PolygonShape> result = tracePolygons(subjectHead);
    
    // 如果没有交点，检查是否完全包含
    if (result.empty()) {
        bool subjectInClip = true;
        for (const auto& p : subject) {
            if (!isPointInPolygon(p, clipPoly)) {
                subjectInClip = false;
                break;
            }
        }
        if (subjectInClip) {
            result.push_back(subject);
        }
    }
    
    // 释放内存
    freeVertexList(subjectHead);
    freeVertexList(clipHead);
    
    return result;
}
