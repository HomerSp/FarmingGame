#pragma once

#include <atomic>
#include <bitset>
#include <cmath>
#include <cstdint>
#include <set>
#include <unordered_map>
#include <vector>

#include <engine/graphics/color.h>
#include <engine/graphics/colorgradient.h>

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

        T left() const { return x; }
        T top() const { return y; }
        T right() const { return x + width; }
        T bottom() const { return y + height; }

        bool intersects(const Rect<T>& o) const {
            return (x <= o.x + o.width &&
                o.x <= x + width &&
                y <= o.y + o.height &&
                o.y <= y + height);
        }

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

    template<typename T = int32_t>
    struct Quad {
    public:
        Quad(T l = 0, T t = 0, T r = 0, T b = 0)
            : left(l), top(t), right(r), bottom(b)
        {}

        T left, top;
        T right, bottom;
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
        FilledEllipse(int32_t x, int32_t y, int32_t radius, const graphics::ColorGradient& gradient);

        graphics::ColorGradient gradient;
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

    class AtomicF : public std::atomic<float> {
    public:
        AtomicF(float f);

        AtomicF& operator=(float d);
        AtomicF& operator+=(float d);
    };

    static double_t PI();
};
}
