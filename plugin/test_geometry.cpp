// SPDX-License-Identifier: MIT
// Part of HyperVi (Copyright (c) 2026 lhenr1que).
#include "geometry.hpp"
#include <cfloat>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <optional>
#include <type_traits>

using namespace KwGeometry;
static_assert(!std::is_convertible_v<PhysicalBox, LogicalBox>);
static_assert(!std::is_convertible_v<LogicalBox, PhysicalBox>);
// Relations the numeric contract depends on (see GEOMETRY.md section 4).
static_assert(kMaxPhysical == kMaxLogical * kMaxScale);
static_assert(8 * kMaxLogical <= kMaxBox);          // bbox of the widest valid input
static_assert(kMaxPhysical / kMinScale <= kMaxBox); // widest valid preview result
// Measured loss of coverage starts near 2^50 pixels; edges (x+w <= 2*kMaxBox)
// scaled by kMaxScale must stay >= 4 bits below it.
static_assert(2 * kMaxBox * kMaxScale <= 0x1p46);

namespace {
    int checks = 0, negative = 0;
    void check(bool ok, const char* name) {
        ++checks;
        if (!ok) {
            std::printf("FALHOU - %s (variante %d)\n", name, negative);
            std::exit(1);
        }
    }
    bool near(double a, double b) { return std::isfinite(a) && std::isfinite(b) && std::abs(a - b) < 1e-9; }
    template <typename A, typename B> bool same(A a, B b) {
        return near(a.x, b.x) && near(a.y, b.y) && near(a.w, b.w) && near(a.h, b.h);
    }
    template <typename A, typename B> bool covers(A a, B b) {
        return a.x <= b.x && a.y <= b.y && a.x + a.w >= b.x + b.w && a.y + a.h >= b.y + b.h;
    }
    template <typename A> bool finite(A a) {
        return std::isfinite(a.x) && std::isfinite(a.y) && std::isfinite(a.w) && std::isfinite(a.h);
    }
    PhysicalBox outward(LogicalBox b, double s) {
        const double x = std::floor(b.x * s), y = std::floor(b.y * s);
        return {x, y, std::ceil((b.x + b.w) * s) - x, std::ceil((b.y + b.h) * s) - y};
    }
    struct Fixture {
        LogicalBox d;
        LogicalPoint e, w, f, m;
        PhysicalBox expected[4];
    };
    constexpr double scales[]{1, 1.25, 1.5, 2};
    const Fixture fixtures[]{
        {{.25, -.5, 100.5, 23.25}, {100, 200}, {13, -17}, {-4, 6}, {-1920, -1080},
         {{2029,1269,101,23}, {2537,1586,125,29}, {3044,1903,151,35}, {4059,2537,201,47}}},
        {{-.5, -.5, 1, 1}, {}, {}, {}, {},
         {{-1,-1,2,2}, {-1,-1,2,2}, {-1,-1,2,2}, {-1,-1,2,2}}},
        {{2.3, 7.1, 80.2, 18.7}, {-90.5, -30.2}, {41.2, 12.3}, {-5.1, 9.7}, {1280, -720},
         {{-1332,719,80,19}, {-1665,899,100,23}, {-1998,1078,120,28}, {-2664,1438,160,37}}}
    };
    // Fault injection is confined to this test executable. Each variant runs
    // the SAME assertions as production and must exit 1, not merely differ.
    PhysicalBox physical(Fixture q, double s) {
        if (negative == 1) { q.w.x *= 2; q.w.y *= 2; }
        if (negative == 2) { q.f.x *= 2; q.f.y *= 2; }
        if (negative == 3) q.m = {};
        if (negative == 4) s *= s;
        if (negative == 5) { q.m.x = -q.m.x; q.m.y = -q.m.y; }
        const auto r = physicalLocal(q.d, q.e, q.w, q.f, q.m, s);
        check(r.has_value(), "P1 in-domain physicalLocal must be accepted");
        auto p = *r;
        if (negative == 11) { p.w = std::round(q.d.w * s); p.h = std::round(q.d.h * s); }
        if (negative == 12) p.w = p.h; // inconsistent dimensions
        return p;
    }
    LogicalBox bbox(Fixture q, double ext, double aa, double s) {
        if (negative == 6) q.f = {};
        if (negative == 8) { ext = 0; aa = 10; }
        const auto r = logicalBBox(q.d, q.e, q.w, q.f, q.m, ext, aa);
        check(r.has_value(), "P2 in-domain logicalBBox must be accepted");
        auto b = *r;
        if (negative == 7) { b.x *= s; b.y *= s; b.w *= s; b.h *= s; }
        return b;
    }
    LogicalBox damage(LogicalBox prev, LogicalBox next) {
        if (negative == 9) return next;
        const auto r = unionDamage(prev, next);
        check(r.has_value(), "P4 in-domain unionDamage must be accepted");
        return *r;
    }
    LogicalBox preview(PhysicalBox p, double s) {
        if (negative == 10) return {p.x, p.y, p.w, p.h};
        if (negative == 13) return {p.x / s, p.y / s, p.w, p.h};
        const auto r = previewBBox(p, s);
        check(r.has_value(), "P5 in-domain previewBBox must be accepted");
        return *r;
    }
    void p1() {
        for (const auto& q : fixtures)
            for (int i = 0; i < 4; ++i)
                check(same(physical(q, scales[i]), q.expected[i]), "P1 literal numeric oracle / residual rounding");
    }

    // ---- P3 numeric domain -------------------------------------------------
    struct Args {
        LogicalBox d;
        LogicalPoint e, w, f, m;
        double s;
    };
    constexpr int kFields = 13; // 12 logical components + scale
    double& field(Args& a, int i) {
        double* p[]{&a.d.x, &a.d.y, &a.d.w, &a.d.h, &a.e.x, &a.e.y, &a.w.x, &a.w.y, &a.f.x, &a.f.y, &a.m.x, &a.m.y, &a.s};
        return *p[i];
    }
    bool isLength(int i) { return i == 2 || i == 3; }
    const Args base{fixtures[0].d, fixtures[0].e, fixtures[0].w, fixtures[0].f, fixtures[0].m, 1.5};
    bool physicalOk(const Args& a) { return physicalLocal(a.d, a.e, a.w, a.f, a.m, a.s).has_value(); }
    bool bboxOk(const Args& a, double ext = 0, double aa = 0) {
        return logicalBBox(a.d, a.e, a.w, a.f, a.m, ext, aa).has_value();
    }
    // Coverage of one drawn bottom-extended box by the bbox of the same box.
    bool covered(const Args& a, double ext, double aa) {
        auto draw = a.d;
        draw.h += ext;
        if (draw.h > kMaxLogical) return true; // the drawn box itself is outside the domain; physicalLocal rejects it (tested separately)
        const auto p = physicalLocal(draw, a.e, a.w, a.f, a.m, a.s);
        const auto b = logicalBBox(a.d, a.e, a.w, a.f, a.m, ext, aa);
        return p && b && finite(*p) && finite(*b) && p->w >= 0 && p->h >= 0 && covers(outward(*b, a.s), *p);
    }
    std::uint64_t rng = 0x9E3779B97F4A7C15ull;
    double uni() { // deterministic xorshift64*, [0, 1)
        rng ^= rng >> 12; rng ^= rng << 25; rng ^= rng >> 27;
        return double((rng * 0x2545F4914F6CDD1Dull) >> 11) * 0x1p-53;
    }
    double pick(double lim) { // near the limit half the time, small otherwise
        const double sign = uni() < .5 ? -1 : 1;
        return sign * (uni() < .5 ? lim * (.5 + .5 * uni()) : 200 * uni());
    }
    double pickScale() { return kMinScale * std::pow(kMaxScale / kMinScale, uni()); }

    void domain() {
        // The ORIGINAL review counterexample (origin 2^53, width 1, scale 1.5):
        // it is outside the supported domain and must be rejected EXPLICITLY by
        // every entry point that would silently lose the extent.
        const LogicalBox big{0x1p53, 0, 1, 1};
        check(!physicalLocal(big, {}, {}, {}, {}, 1.5).has_value(), "P3 original counterexample 2^53 rejected by physicalLocal");
        check(!logicalBBox(big, {}, {}, {}, {}, 0, 0).has_value(), "P3 original counterexample 2^53 rejected by logicalBBox");
        check(!unionDamage(big, big).has_value(), "P3 original counterexample 2^53 rejected by unionDamage");
        check(!previewBBox({0x1p53, 0, 1, 1}, 1.5).has_value(), "P3 original counterexample 2^53 rejected by previewBBox");
        // Largest in-domain analogue is accepted and covered.
        Args edge = base;
        edge.d = {kMaxLogical, 0, 1, 1}; edge.e = edge.w = edge.f = edge.m = {};
        check(physicalOk(edge) && covered(edge, 0, 0), "P3 largest in-domain analogue of the counterexample covered");

        // Extents too small to move their own origin (x+w == x) are degenerate:
        // before this guard physicalLocal({0.5,0,2^-54,1}) returned width -1, and
        // unionDamage of such a box returned width 0 that it then refused as input.
        // Edges are the double values x+w, as in the host's CBox.
        for (bool vertical : {false, true}) {
            const auto box = [vertical](double origin, double extent) {
                return vertical ? LogicalBox{0, origin, 1, extent} : LogicalBox{origin, 0, extent, 1};
            };
            const auto phys = [vertical](double origin, double extent) {
                return vertical ? PhysicalBox{0, origin, 1, extent} : PhysicalBox{origin, 0, extent, 1};
            };
            check(!physicalLocal(box(.5, 0x1p-54), {}, {}, {}, {}, 1).has_value(), "P1 physicalLocal rejects an extent absorbed by the origin (was width -1)");
            check(!logicalBBox(box(1, 0x1p-54), {}, {}, {}, {}, 0, 0).has_value(), "P2 logicalBBox rejects an extent absorbed by the origin");
            check(!unionDamage(box(1, 0x1p-54), box(1, 0x1p-54)).has_value(), "P4 unionDamage rejects an extent absorbed by the origin");
            check(!unionDamage(box(0, 1), box(1, 0x1p-54)).has_value(), "P4 unionDamage rejects an absorbed extent in the second operand");
            check(!previewBBox(phys(1, 0x1p-54), 1).has_value(), "P5 previewBBox rejects an extent absorbed by the origin");
            check(!previewBBox(phys(-3, 0x1p-52), 1.5).has_value(), "P5 previewBBox rejects an extent absorbed by a negative origin");
            // Smallest resolvable extents are accepted, never negative, and stay valid inputs.
            const auto pl = physicalLocal(box(.5, 0x1p-53), {}, {}, {}, {}, 1);
            check(pl && pl->w >= 0 && pl->h >= 0, "P1 smallest resolvable extent at a rounding tie gives a nonnegative width");
            const auto un = unionDamage(box(1, 0x1p-52), box(1, 0x1p-52));
            check(un && covers(*un, box(1, 0x1p-52)) && unionDamage(*un, *un).has_value(), "P4 union of the smallest resolvable extent is itself a valid input");
            const auto pv = previewBBox(phys(1, 0x1p-52), 1);
            check(pv && covers(outward(*pv, 1), phys(1, 0x1p-52)), "P5 preview of the smallest resolvable extent keeps coverage");
        }

        // Every field: every non-finite / overflow / just-outside value is rejected.
        for (int i = 0; i < kFields; ++i) {
            const double lim = i == 12 ? kMaxScale : kMaxLogical;
            for (double bad : {double(NAN), double(INFINITY), -double(INFINITY), DBL_MAX, -DBL_MAX, 0x1p53, -0x1p53,
                               std::nextafter(lim, INFINITY), -std::nextafter(lim, INFINITY)}) {
                if (i != 12 && !isLength(i) && std::abs(bad) <= lim) continue;
                Args a = base;
                field(a, i) = bad;
                check(!physicalOk(a), "P3 physicalLocal rejects non-finite/overflow/out-of-domain field");
                if (i != 12) check(!bboxOk(a), "P3 logicalBBox rejects non-finite/overflow/out-of-domain field");
            }
            // Lower edge of the positive quantities.
            if (isLength(i) || i == 12) {
                std::initializer_list<double> badLow = i == 12
                    ? std::initializer_list<double>{0, -1, -0., std::nextafter(kMinScale, 0), 0x1p-1074}
                    : std::initializer_list<double>{0, -0., -1, -0x1p-1074};
                for (double bad : badLow) {
                    Args a = base;
                    field(a, i) = bad;
                    check(!physicalOk(a), "P3 physicalLocal rejects zero/negative length or scale below minimum");
                    if (i != 12) check(!bboxOk(a), "P3 logicalBBox rejects zero/negative length");
                    // AA and extension must not make a nonpositive D length acceptable.
                    if (i != 12) check(!bboxOk(a, 5, 5), "P3 logicalBBox rejects zero/negative length even when AA/extension would widen it");
                }
            }
            // Exactly at the limit: accepted (the bound is closed).
            for (double good : {lim, i == 12 || isLength(i) ? kMinScale : -lim}) {
                Args a = base;
                field(a, i) = good;
                check(physicalOk(a), "P3 physicalLocal accepts the closed domain limit");
                if (i != 12) check(bboxOk(a), "P3 logicalBBox accepts the closed domain limit");
            }
        }
        // extBottom / aaMargin: finite, nonnegative, <= kMaxLogical.
        for (double bad : {double(NAN), double(INFINITY), DBL_MAX, -1., -0x1p-1074, std::nextafter(kMaxLogical, INFINITY)}) {
            check(!bboxOk(base, bad, 0), "P3 logicalBBox rejects bad extBottom");
            check(!bboxOk(base, 0, bad), "P3 logicalBBox rejects bad aaMargin");
        }
        check(bboxOk(base, kMaxLogical, kMaxLogical) && bboxOk(base, 0, 0), "P3 logicalBBox accepts extBottom/aaMargin at 0 and at the limit");
        check(!bboxOk(base, -0.0 - 1e-300, 0), "P3 logicalBBox rejects tiny negative extBottom");

        // Scale bounds for the other two scale-bearing functions.
        const PhysicalBox ph{10, 20, 30, 40};
        for (double bad : {double(NAN), double(INFINITY), DBL_MAX, 0., -1., 0x1p-1074, std::nextafter(kMinScale, 0), std::nextafter(kMaxScale, INFINITY)})
            check(!previewBBox(ph, bad).has_value(), "P5 previewBBox rejects bad scale");
        for (double good : {kMinScale, kMaxScale, 1.5})
            check(previewBBox(ph, good).has_value(), "P5 previewBBox accepts closed scale limits");
        // Preview physical box bounds.
        const double lim[]{kMaxPhysical, kMaxPhysical, kMaxPhysical, kMaxPhysical};
        for (int i = 0; i < 4; ++i)
            for (double bad : {double(NAN), double(INFINITY), -double(INFINITY), DBL_MAX, 0x1p53, std::nextafter(lim[i], INFINITY),
                               -std::nextafter(lim[i], INFINITY), (i >= 2 ? 0. : 1e300), (i >= 2 ? -1. : 1e300)}) {
                if (i < 2 && std::abs(bad) <= lim[i]) continue;
                double v[]{10, 20, 30, 40};
                v[i] = bad;
                check(!previewBBox({v[0], v[1], v[2], v[3]}, 1.5).has_value(), "P5 previewBBox rejects bad physical component");
            }
        // unionDamage components: |x|,|y| <= kMaxBox, 0 < w,h <= kMaxBox.
        for (int i = 0; i < 4; ++i)
            for (double bad : {double(NAN), double(INFINITY), -double(INFINITY), DBL_MAX, 0x1p53, std::nextafter(kMaxBox, INFINITY),
                               -std::nextafter(kMaxBox, INFINITY), 0., -1.}) {
                if (i < 2 && (std::abs(bad) <= kMaxBox)) continue;
                double v[]{1, 2, 3, 4};
                v[i] = bad;
                const LogicalBox odd{v[0], v[1], v[2], v[3]}, ok{1, 2, 3, 4};
                check(!unionDamage(odd, ok).has_value() && !unionDamage(ok, odd).has_value(), "P4 unionDamage rejects bad component in either operand");
            }
        // Closed limit: the union is exactly kMaxBox wide. One beyond is refused
        // by the producing call, never returned as a box it would later reject.
        check(unionDamage({-kMaxBox, -kMaxBox, kMaxBox / 2, kMaxBox / 2}, {-kMaxBox / 2, -kMaxBox / 2, kMaxBox / 2, kMaxBox / 2}).has_value(), "P4 unionDamage accepts a union exactly kMaxBox wide");
        check(!unionDamage({-kMaxBox, -kMaxBox, kMaxBox, kMaxBox}, {kMaxBox, kMaxBox, kMaxBox, kMaxBox}).has_value(), "P4 unionDamage refuses a union wider than the box domain");
        check(!unionDamage({-kMaxBox, -kMaxBox, kMaxBox / 2, kMaxBox / 2}, {-kMaxBox / 2, -kMaxBox / 2, std::nextafter(kMaxBox / 2, INFINITY), kMaxBox / 2}).has_value(), "P4 unionDamage refuses a union one ulp wider than kMaxBox");
        // previewBBox results must be valid boxes (reviewer: collapsed extents).
        check(!previewBBox({1.5, 0, 3 * 0x1p-54, 1}, 1.5).has_value(), "P5 previewBBox refuses a result whose extent collapsed by division");
        check(!previewBBox({-0x1p-1074, 0, 0x1p-1074, 1}, 16).has_value(), "P5 previewBBox refuses a result whose extent underflowed to zero");

        // Boundary coverage: signed corners of the closed domain, every scale
        // that matters, with and without extension and AA.
        const double ss[]{kMinScale, 0.1, 1, 1.25, 1.5, 2, 3.7, 10, kMaxScale, std::nextafter(kMaxScale, 0), std::nextafter(kMinScale, 1)};
        for (int mask = 0; mask < 32; ++mask) {
            Args a = base;
            const double sg[]{mask & 1 ? 1. : -1., mask & 2 ? 1. : -1., mask & 4 ? 1. : -1., mask & 8 ? 1. : -1., mask & 16 ? 1. : -1.};
            a.d.x = sg[0] * kMaxLogical; a.d.y = -sg[0] * kMaxLogical;
            a.e = {sg[1] * kMaxLogical, -sg[1] * kMaxLogical};
            a.w = {sg[2] * kMaxLogical, sg[2] * kMaxLogical};
            a.f = {sg[3] * kMaxLogical, -sg[3] * kMaxLogical};
            a.m = {sg[4] * kMaxLogical, sg[4] * kMaxLogical};
            for (double dw : {1., .3, 1234.5, kMaxLogical}) {
                a.d.w = dw; a.d.h = dw == 1. ? 1 : dw * .7;
                for (double s : ss)
                    for (double ext : {0., 96., kMaxLogical})
                        for (double aa : {0., 1.25, kMaxLogical}) {
                            a.s = s;
                            check(covered(a, ext, aa), "P3 coverage at the signed corners of the closed domain");
                        }
            }
        }
        // Deterministic fuzz inside the domain, concentrated near its limits.
        for (int n = 0; n < 60000; ++n) {
            Args a{{pick(kMaxLogical), pick(kMaxLogical), 0, 0}, {pick(kMaxLogical), pick(kMaxLogical)}, {pick(kMaxLogical), pick(kMaxLogical)},
                   {pick(kMaxLogical), pick(kMaxLogical)}, {pick(kMaxLogical), pick(kMaxLogical)}, pickScale()};
            a.d.w = .01 + std::abs(pick(kMaxLogical)); a.d.h = .01 + std::abs(pick(kMaxLogical));
            a.d.w = std::min(a.d.w, kMaxLogical); a.d.h = std::min(a.d.h, kMaxLogical);
            const double ext = uni() < .5 ? 0 : 200 * uni(), aa = uni() < .5 ? 0 : 3 * uni();
            check(covered(a, ext, aa), "P3 fuzz coverage over the closed domain");
            // P4: damage of two independent in-domain frames covers both drawings.
            Args b = a;
            b.d.x = pick(kMaxLogical); b.d.y = pick(kMaxLogical); b.f = {pick(kMaxLogical), pick(kMaxLogical)};
            const auto ba = logicalBBox(a.d, a.e, a.w, a.f, a.m, ext, aa), bb = logicalBBox(b.d, b.e, b.w, b.f, b.m, ext, aa);
            const auto u = ba && bb ? unionDamage(*ba, *bb) : std::nullopt;
            check(u.has_value() && unionDamage(*u, *u).has_value(), "P4 fuzz: union of two in-domain bboxes accepted and re-acceptable");
            check(covers(outward(*u, a.s), outward(*ba, a.s)) && covers(outward(*u, a.s), outward(*bb, a.s)), "P4 fuzz: union covers both frames");
            // P5: any in-domain preview round-trips with coverage.
            const PhysicalBox p{pick(kMaxPhysical), pick(kMaxPhysical), .01 + std::abs(pick(kMaxPhysical)), .01 + std::abs(pick(kMaxPhysical))};
            const PhysicalBox pc{p.x, p.y, std::min(p.w, kMaxPhysical), std::min(p.h, kMaxPhysical)};
            const auto pv = previewBBox(pc, a.s);
            check(pv.has_value() && finite(*pv) && covers(outward(*pv, a.s), pc), "P5 fuzz: preview coverage over the closed domain");
            check(unionDamage(*pv, *pv).has_value(), "P5 fuzz: preview result is a valid unionDamage input");
        }
    }

    void p2p3() {
        if (negative == 8) {
            auto draw = fixtures[0];
            draw.d.h += 96;
            check(covers(outward(bbox(fixtures[0], 96, 0, 1.25), 1.25), physical(draw, 1.25)),
                  "P3 fixed expand(10) cannot cover large bottom extension");
        }
        const LogicalBox expected{2028, 1267.25, 103, 121.75};
        check(same(bbox(fixtures[0], 96, 1.25, 1.5), expected), "P2 logical F / bottom extension / AA");
        // Zero and large rounding; AA is logical and not a substitute for ext.
        for (const auto& q : fixtures)
            for (double s : scales)
                for (double ext : {0., 3., 96., 240.})
                    for (double aa : {0., .5, 1.25}) {
                        auto draw = q;
                        draw.d.h += ext;
                        const auto b = bbox(q, ext, aa, s);
                        const auto p = physical(draw, s);
                        check(covers(outward(b, s), p), "P3 outward bbox covers every drawn pixel");
                        check(near(b.w, q.d.w + 2*aa) && near(b.h, q.d.h + ext + 2*aa), "P2 dimensions stay logical");
                    }
        // Dense fractional origins near rounding boundaries, both signs.
        for (int i = -32; i <= 32; ++i)
            for (double s : scales)
                for (int base : {0, 1}) {
                    auto q = fixtures[base];
                    q.d.x = i / 16.; q.d.y = -i / 16.; q.d.w = .3; q.d.h = .7;
                    check(covers(outward(bbox(q, 0, 0, s), s), physical(q, s)), "P3 fractional boundary sweep");
                }
    }
    void p4() {
        for (bool vertical : {false, true}) {
            const LogicalBox a = vertical ? LogicalBox{0,-1,1,1} : LogicalBox{-1,0,1,1};
            const LogicalBox b = vertical ? LogicalBox{0,.5,1,0x1.0000000000002p-1} : LogicalBox{.5,0,0x1.0000000000002p-1,1};
            const auto both = damage(a, b);
            check(covers(both, a) && covers(both, b), "P4 fractional union preserves logical edges (review regression)");
            check(covers(outward(both, 1), b), "P4 fractional union preserves pixel coverage");
        }
        const LogicalBox prev{-200, -90, 40, 30}, next{70, 80, 100, 60};
        check(same(damage(prev, next), LogicalBox{-200,-90,370,230}), "P4 literal union of disjoint frames");
        check(same(damage(next, prev), damage(prev, next)), "P4 commutativity");
        check(same(damage(prev, prev), prev), "P4 idempotence");
        const LogicalBox inside{-190,-85,2,3};
        check(same(damage(prev, inside), prev), "P4 nested frame");
        for (double s : scales) {
            const auto& q = fixtures[0];
            auto moved = q;
            moved.w = {-300, 150}; moved.f = {50,-20}; moved.d.w = 280;
            const auto both = damage(bbox(q, 96, 1, s), bbox(moved, 96, 1, s));
            auto a = q, b = moved; a.d.h += 96; b.d.h += 96;
            check(covers(outward(both, s), physical(a, s)) && covers(outward(both, s), physical(b, s)), "P4 damage covers old and new drawings");
        }
    }
    void p5() {
        const PhysicalBox edge{1, 0, 0x1.0000000000002p0, 1};
        check(covers(outward(preview(edge, 1.5), 1.5), edge), "P5 fractional right edge above integer (review regression)");
        for (double s : scales)
            for (const PhysicalBox p : {PhysicalBox{251,-37,123,49}, PhysicalBox{-17,21,1,1}, PhysicalBox{0,0,120,40}}) {
                const auto b = preview(p, s);
                check(same(PhysicalBox{b.x*s,b.y*s,b.w*s,b.h*s}, p), "P5 all components divide by scale exactly once");
                check(covers(outward(b, s), p), "P5 no one-pixel gap on round trip");
            }
        for (double s : {1., 1.1, 1.25, 1.3, 1.5, 2., 2.3, 3.})
            for (int i = -8; i <= 8; ++i)
                for (int width = 1; width <= 4; ++width)
                    for (double direction : {-INFINITY, INFINITY}) {
                        const PhysicalBox p{i / 3., -i / 3., std::nextafter(double(width), direction),
                                            std::nextafter(double(width), -direction)};
                        const auto b = preview(p, s);
                        check(same(PhysicalBox{b.x*s,b.y*s,b.w*s,b.h*s}, p), "P5 fractional components preserved within numeric error");
                        check(covers(outward(b, s), p), "P5 fractional edges preserve strict coverage");
                    }
        // Origins and edges one ulp either side of an integer: the division can round
        // across the integer, which is when the directed widening is needed.
        for (int k = 1; k <= 30; ++k) {
            const double s = 1 + k / 10.; // 1.1 .. 4.0, mostly inexact in binary
            for (int i = -4; i <= 4; ++i)
                for (double dx : {-INFINITY, INFINITY})
                    for (double dw : {-INFINITY, INFINITY}) {
                        const PhysicalBox p{std::nextafter(double(i), dx), std::nextafter(double(-i), -dx),
                                            std::nextafter(double(3), dw), std::nextafter(double(5), -dw)};
                        const auto b = preview(p, s);
                        check(covers(outward(b, s), p), "P5 ulp-adjacent origins and edges preserve strict coverage");
                    }
        }
        check(same(preview({251,-37,123,49}, 1.25), LogicalBox{200.8,-29.6,98.4,39.2}), "P5 literal fractional oracle");
    }
}
int main(int argc, char** argv) {
    if (argc != 1) {
        if (argc != 3 || std::strcmp(argv[1], "--negative") != 0) return 2;
        char* end = nullptr;
        const long mode = std::strtol(argv[2], &end, 10);
        if (*end || mode < 1 || mode > 13) return 2;
        negative = static_cast<int>(mode);
    }
    p1(); p2p3(); p4(); p5();
    domain(); // last: earlier properties keep their own first-failure messages
    std::printf("OK - P1-P5: %d verificacoes (variante %d)\n", checks, negative);
}
