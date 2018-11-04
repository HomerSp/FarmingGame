#pragma once

#include <cstdint>
#include <unordered_map>

namespace engine {
class Types {
public:
    typedef typename std::unordered_map<int, std::unordered_map<int, int>> Map2D;

    struct Rect {
    public:
        Rect(int x = 0, int y = 0, int w = 0, int h = 0);

        int x, y;
        int width, height;
    };

    struct RectF {
    public:
        RectF(float x = 0, float y = 0, float w = 0, float h = 0);

        float x, y;
        float width, height;
    };

    struct Point {
    public:
        Point(int x = 0, int y = 0);

        int x, y;
    };

    struct PointF {
    public:
        PointF(float x = 0, float y = 0);

        float x, y;
    };

    struct Dimension {
    public:
        Dimension(int w = 0, int h = 0);

        int width;
        int height;
    };

    struct Cells {
    public:
        Cells(int cols = 0, int rows = 0);

        int cols;
        int rows;
    };

    struct Color {
    public:
        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

        uint8_t r, g, b, a;
    };

    struct Quad {
    public:
        Quad(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0);

        int x1, y1;
        int x2, y2;
    };

    struct QuadF {
    public:
        QuadF(float x1 = 0, float y1 = 0, float x2 = 0, float y2 = 0);

        float x1, y1;
        float x2, y2;
    };

    struct Pair {
    public:
        Pair(int first = 0, int second = 0);

        int first;
        int second;
    };
};
}
