#pragma once

#include <bitset>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace engine {
class Types {
public:
    typedef typename std::unordered_map<int, std::unordered_map<int, int>> Map2D;

    template<typename T = int>
    struct Rect {
    public:
        Rect(T x = 0, T y = 0, T w = 0, T h = 0)
            : x(x), y(y), width(w), height(h)
        {}

        T x, y;
        T width, height;
    };

    template<typename T = int, typename R = T>
    struct Point {
    public:
        Point(R x = 0, R y = 0)
            : x(x), y(y)
        {}

        T x, y;
    };

    template<typename T = int>
    struct Dimension {
    public:
        Dimension(T w = 0, T h = 0)
            : width(w), height(h)
        {}

        T width, height;
    };

    struct Cells {
    public:
        Cells(uint32_t cols = 0, uint32_t rows = 0);

        uint32_t cols, rows;
    };

    struct Color {
    public:
        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

        uint8_t r, g, b, a;
    };

    template<typename T = int>
    struct Quad {
    public:
        Quad(T x1 = 0, T y1 = 0, T x2 = 0, T y2 = 0)
            : x1(x1), y1(y1), x2(x2), y2(y2)
        {}

        T x1, y1;
        T x2, y2;
    };

    struct Pair {
    public:
        Pair(int first = 0, int second = 0);

        int first;
        int second;
    };

    struct Ellipse {
    public:
        Ellipse(int32_t x, int32_t y, int32_t radius);

        int32_t x, y;
        int32_t radius;
    };

    struct FilledEllipse : public Ellipse {
    public:
        FilledEllipse(int32_t x, int32_t y, int32_t radius, Types::Color c);

        Types::Color color;
    };

    struct Overlay {
    public:
        Overlay(uint32_t w, uint32_t h, Types::Color bg);

        void addEllipse(const Types::FilledEllipse &ellipse);

        uint32_t width, height;
        Types::Color background;
        std::vector<Types::FilledEllipse> ellipses;
    };

    struct TextAlign {
        typedef enum {
            Left,
            Right,
            Top,
            Bottom,
            CentreV,
            CentreH,
            Last,
        } Type;

        TextAlign(std::initializer_list<Type> types = {});

        bool is(Type t) const;

        std::bitset<Type::Last> bits;
    };
};
}
