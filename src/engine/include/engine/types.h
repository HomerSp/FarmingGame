#pragma once

#include <bitset>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace engine {
class Types {
public:
    typedef typename std::unordered_map<int32_t, std::unordered_map<int32_t, int32_t>> Map2D;

    template<typename T = int32_t>
    struct Rect {
    public:
        Rect(T x = 0, T y = 0, T w = 0, T h = 0)
            : x(x), y(y), width(w), height(h)
        {}

        T x, y;
        T width, height;
    };

    template<typename T = int32_t, typename R = T>
    struct Point {
    public:
        Point(R x = 0, R y = 0) noexcept
            : x(x), y(y)
        {}

        Point operator+(const Point<T, R>& o) const {
            return Point<T>(x + o.x, y + o.y);
        }

        bool operator==(const Point<T, R>& o) const {
            return x == o.x && y == o.y;
        }

        bool operator!=(const Point<T, R>& o) const {
            return ! operator==(o);
        }

        T x, y;
    };

    template<typename T = int32_t>
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

    template<typename T = int32_t>
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
        Pair(int32_t first = 0, int32_t second = 0);

        int32_t first;
        int32_t second;
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

    static constexpr uint32_t hash(const char* str, uint32_t h = 0)
    {
        return !str[h] ? 5381 : (hash(str, h + 1) * 33) ^ str[h];
    }
};
}
