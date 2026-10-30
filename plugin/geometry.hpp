// SPDX-License-Identifier: MIT
// Part of kwinish (Copyright (c) 2026 lhen).
#pragma once

#include <algorithm>
#include <cmath>
#include <optional>

namespace KwGeometry {
    struct LogicalPoint {
        double x = 0, y = 0;
    };
    struct LogicalBox {
        double x = 0, y = 0, w = 0, h = 0;
    };
    struct PhysicalBox {
        double x = 0, y = 0, w = 0, h = 0;
    };

    // Supported numeric domain (closed bounds). Every function returns
    // std::nullopt, never a wrong box, for NaN/inf, a bound violation or a
    // non-positive length/scale. Rationale, margins and measurements:
    // docs/port-0-57/GEOMETRY.md section 4.
    //  - kMaxLogical: |component| of every raw logical input (D, E, W, F, M,
    //    extBottom, aaMargin) and the limit of D.w/D.h: Wayland positions and
    //    sizes are int32, so 2^31 covers every value the compositor receives.
    //  - kMin/kMaxScale: monitor scale is a float; 1/16..16 is far wider than
    //    any real output and keeps pixel magnitudes <= 2^45.
    //  - kMaxPhysical: |component| of a preview box (kMaxLogical * kMaxScale).
    //  - kMaxBox: |component| of a derived logical box fed to unionDamage; it
    //    also bounds previewBBox's result (checked by static_assert in the test).
    // Coverage was measured to fail only from ~2^50 pixels, where one ulp of the
    // logical coordinate reaches a quarter pixel; the largest pixel EDGE of an
    // accepted box is 2^45 (x+w <= 2*kMaxBox, scaled by kMaxScale), 5 bits below
    // that onset, so no coverage depends on rounding slack of the representation.
    // Edges are the double values x+w / y+h, exactly what the host computes for
    // a CBox; an extent too small to move its own origin in double arithmetic
    // (x+w == x) is degenerate and rejected as well (see detail::resolved).
    // Every LogicalBox returned (bbox, union, preview) is itself valid input to
    // unionDamage (detail::boxOk); a result outside that box domain is nullopt.
    inline constexpr double kMaxLogical = 0x1p31, kMaxPhysical = 0x1p35, kMaxBox = 0x1p40;
    inline constexpr double kMinScale = 0x1p-4, kMaxScale = 0x1p4;

    namespace detail {
        // All predicates are false for NaN.
        inline bool within(double v, double limit) { return std::fabs(v) <= limit; }
        inline bool length(double v, double limit) { return v > 0 && v <= limit; }
        inline bool nonneg(double v, double limit) { return v >= 0 && v <= limit; }
        inline bool scaleOk(double s) { return s >= kMinScale && s <= kMaxScale; }
        // The extent survives addition to its origin; false would let the edge
        // collapse onto the origin (and, in physicalLocal, a residual of -1).
        inline bool resolved(double x, double w) { return x + w > x; }
        inline bool pointOk(LogicalPoint p) { return within(p.x, kMaxLogical) && within(p.y, kMaxLogical); }
        inline bool logicalOk(LogicalBox d, LogicalPoint e, LogicalPoint w, LogicalPoint f, LogicalPoint m) {
            return within(d.x, kMaxLogical) && within(d.y, kMaxLogical) && length(d.w, kMaxLogical) &&
                length(d.h, kMaxLogical) && pointOk(e) && pointOk(w) && pointOk(f) && pointOk(m);
        }
        inline bool boxOk(LogicalBox b) {
            return within(b.x, kMaxBox) && within(b.y, kMaxBox) && length(b.w, kMaxBox) && length(b.h, kMaxBox) &&
                resolved(b.x, b.w) && resolved(b.y, b.h);
        }
    }

    // D is relative logical geometry; E is the global logical anchor; W/F are
    // presentation offsets; M is the render monitor's global logical origin.
    // Domain: see the constants above. No host projection/rotation/renderModif
    // is applied. Match Hyprutils CBox::round: origin first, then residual
    // extent, including std::round's ties away from zero. Rounding w/h
    // independently is incorrect.
    inline std::optional<PhysicalBox> physicalLocal(LogicalBox d, LogicalPoint e, LogicalPoint w,
                                                    LogicalPoint f, LogicalPoint m, double scale) {
        if (!detail::logicalOk(d, e, w, f, m) || !detail::scaleOk(scale)) return std::nullopt;
        const double x = (d.x + e.x + w.x + f.x - m.x) * scale;
        const double y = (d.y + e.y + w.y + f.y - m.y) * scale;
        if (!detail::resolved(x, d.w * scale) || !detail::resolved(y, d.h * scale)) return std::nullopt;
        const double rx = std::round(x), ry = std::round(y);
        return PhysicalBox{rx, ry, std::round(x + d.w * scale - rx), std::round(y + d.h * scale - ry)};
    }

    // Logical monitor-local bounds. extBottom (e.g. ROUNDING*3) and aaMargin
    // are logical lengths in [0, kMaxLogical]; do not pass physical pixels.
    inline std::optional<LogicalBox> logicalBBox(LogicalBox d, LogicalPoint e, LogicalPoint w,
                                                 LogicalPoint f, LogicalPoint m, double extBottom, double aaMargin) {
        if (!detail::logicalOk(d, e, w, f, m) || !detail::nonneg(extBottom, kMaxLogical) ||
            !detail::nonneg(aaMargin, kMaxLogical))
            return std::nullopt;
        const LogicalBox b{d.x + e.x + w.x + f.x - m.x - aaMargin,
                           d.y + e.y + w.y + f.y - m.y - aaMargin,
                           d.w + 2 * aaMargin, d.h + extBottom + 2 * aaMargin};
        return detail::boxOk(b) ? std::optional<LogicalBox>(b) : std::nullopt;
    }

    // Both boxes must be in the domain (|x|,|y| <= kMaxBox, 0 < w,h <= kMaxBox,
    // x+w > x and y+h > y),
    // logical and in the SAME monitor-local coordinate system. This returns a
    // conservative enclosing rectangle, not a region. The caller owns
    // conversion if the monitor changes.
    inline std::optional<LogicalBox> unionDamage(LogicalBox prev, LogicalBox next) {
        if (!detail::boxOk(prev) || !detail::boxOk(next)) return std::nullopt;
        const double x = std::min(prev.x, next.x), y = std::min(prev.y, next.y);
        const double right = std::max(prev.x + prev.w, next.x + next.w);
        const double bottom = std::max(prev.y + prev.h, next.y + next.h);
        double w = right - x, h = bottom - y;
        // Subtraction followed by addition can lose a fractional boundary.
        if (x + w < right) w = std::nextafter(w, INFINITY);
        if (y + h < bottom) h = std::nextafter(h, INFINITY);
        // Never return a box this function would itself refuse as input.
        const LogicalBox result{x, y, w, h};
        return detail::boxOk(result) ? std::optional<LogicalBox>(result) : std::nullopt;
    }

    // Preview is already screen-space physical monitor-local geometry
    // (|x|,|y| <= kMaxPhysical, 0 < w,h <= kMaxPhysical, resolvable extents). Do not add W/F/M
    // again. Divide once, preserving fractions. If floating point cancellation
    // would lose coverage on floor/ceil after rescaling, widen only the
    // affected boundary by directed representable rounding. Use ordinary IEEE
    // arithmetic (not -ffast-math).
    inline std::optional<LogicalBox> previewBBox(PhysicalBox physical, double scale) {
        if (!detail::within(physical.x, kMaxPhysical) || !detail::within(physical.y, kMaxPhysical) ||
            !detail::length(physical.w, kMaxPhysical) || !detail::length(physical.h, kMaxPhysical) ||
            !detail::resolved(physical.x, physical.w) || !detail::resolved(physical.y, physical.h) ||
            !detail::scaleOk(scale))
            return std::nullopt;
        LogicalBox b{physical.x / scale, physical.y / scale, physical.w / scale, physical.h / scale};
        const auto coverAxis = [scale](double start, double size, double& origin, double& extent) {
            if (std::floor(origin * scale) > start)
                origin = std::nextafter(origin, -INFINITY);
            const double end = start + size;
            if (std::ceil((origin + extent) * scale) < end) {
                const double boundary = std::nextafter(end / scale, INFINITY);
                extent = std::nextafter(boundary - origin, INFINITY);
            }
        };
        coverAxis(physical.x, physical.w, b.x, b.w);
        coverAxis(physical.y, physical.h, b.y, b.h);
        // A result whose extent collapsed (division underflow, sub-ulp sliver) or
        // left the box domain is refused instead of returned.
        return detail::boxOk(b) ? std::optional<LogicalBox>(b) : std::nullopt;
    }
}
