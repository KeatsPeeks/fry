#pragma once

#include "../deps/nuklear/nuklear.h"

namespace app {
    struct Point {
        int x{};
        int y{};
    };

    inline bool operator==(const Point& p1, const Point& p2) { return p1.x == p2.x && p1.y == p2.y; }

    struct Vector {
        int x{};
        int y{};
    };
    inline Vector operator/(Vector v, int d) {
        return {v.x / d, v.y / d};
    }

    inline Point operator+(Point p, Vector v) {
        return {p.x + v.x, p.y + v.y};
    }
    inline Point operator-(Point p, Vector v) {
        return {p.x - v.x, p.y - v.y};
    }
    inline Point operator+(Vector v, Point p) {
        return p + v;
    }

    struct Size {
        int w{};
        int h{};
    };
    inline Size operator/(Size s, int d) {
        return {s.w / d, s.h / d};
    }
    inline Size operator+(Size s, int d) {
        return {s.w + d, s.h + d};
    }
    inline Size operator+(int d, Size s) {
        return s + d;
    }

    struct Rect {
        Point position{};
        Size size{};
    };

    inline struct nk_rect to_nk_rect(Rect rect) {
        return {
            static_cast<float>(rect.position.x),
            static_cast<float>(rect.position.y),
            static_cast<float>(rect.size.w),
            static_cast<float>(rect.size.h)
        };
    }

}  // namespace app

