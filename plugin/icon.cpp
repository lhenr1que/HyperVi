#include "icon.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>

#include <librsvg/rsvg.h>

namespace fs = std::filesystem;

static std::string lower(std::string s) {
    std::ranges::transform(s, s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

static std::vector<fs::path> dataDirs() {
    std::vector<fs::path> out;
    const char*           home = std::getenv("HOME");
    const char*           xdh  = std::getenv("XDG_DATA_HOME");
    if (xdh && *xdh)
        out.emplace_back(xdh);
    else if (home)
        out.emplace_back(fs::path(home) / ".local/share");

    std::string xdd = std::getenv("XDG_DATA_DIRS") ? std::getenv("XDG_DATA_DIRS") : "/usr/local/share:/usr/share";
    size_t      pos = 0;
    while (pos <= xdd.size()) {
        auto end = xdd.find(':', pos);
        if (end == std::string::npos)
            end = xdd.size();
        if (end > pos)
            out.emplace_back(xdd.substr(pos, end - pos));
        pos = end + 1;
    }
    out.emplace_back("/var/lib/flatpak/exports/share");
    if (home)
        out.emplace_back(fs::path(home) / ".local/share/flatpak/exports/share");
    return out;
}

// window class (lower case) -> Icon= value, from every desktop entry
static const std::unordered_map<std::string, std::string>& desktopIcons() {
    static const auto map = [] {
        std::unordered_map<std::string, std::string> m;
        for (const auto& d : dataDirs()) {
            std::error_code ec;
            for (const auto& entry : fs::directory_iterator(d / "applications", ec)) {
                if (entry.path().extension() != ".desktop")
                    continue;
                std::ifstream f(entry.path());
                std::string   line, icon, wmclass;
                bool          inEntry = false;
                while (std::getline(f, line)) {
                    if (line.starts_with("[")) {
                        inEntry = line == "[Desktop Entry]";
                        continue;
                    }
                    if (!inEntry)
                        continue;
                    if (line.starts_with("Icon="))
                        icon = line.substr(5);
                    else if (line.starts_with("StartupWMClass="))
                        wmclass = line.substr(15);
                }
                if (icon.empty())
                    continue;
                const auto stem = lower(entry.path().stem().string());
                // first entry wins (user directories come first); keys: WM class, file stem, last part of a reverse-DNS stem
                if (!wmclass.empty())
                    m.emplace(lower(wmclass), icon);
                m.emplace(stem, icon);
                if (auto dot = stem.rfind('.'); dot != std::string::npos)
                    m.emplace(stem.substr(dot + 1), icon);
            }
        }
        return m;
    }();
    return map;
}

static std::string themeLookup(const std::string& name, int wantPx) {
    if (name.empty())
        return "";
    if (name.front() == '/')
        return fs::exists(name) ? name : "";

    std::vector<fs::path> roots;
    const char*           home = std::getenv("HOME");
    if (home) {
        roots.emplace_back(fs::path(home) / ".local/share/icons");
        roots.emplace_back(fs::path(home) / ".icons");
    }
    roots.emplace_back("/usr/share/icons");
    roots.emplace_back("/usr/share/pixmaps");

    // sizes nearest above the wanted one first, then the smaller ones
    std::vector<int> sizes = {16, 22, 24, 32, 48, 64, 96, 128, 256, 512};
    std::ranges::sort(sizes, [&](int a, int b) { return (a >= wantPx) != (b >= wantPx) ? a >= wantPx : (a >= wantPx ? a < b : a > b); });

    std::error_code ec;
    for (const auto& root : roots) {
        if (root.filename() == "pixmaps") {
            for (const char* ext : {".png", ".svg"})
                if (fs::exists(root / (name + ext), ec))
                    return (root / (name + ext)).string();
            continue;
        }
        std::vector<fs::path> themes;
        if (fs::exists(root / "hicolor", ec))
            themes.push_back(root / "hicolor");
        for (const auto& t : fs::directory_iterator(root, ec))
            if (t.is_directory(ec) && t.path().filename() != "hicolor")
                themes.push_back(t.path());
        for (const auto& t : themes) {
            for (int s : sizes)
                for (const char* ext : {".png", ".svg"}) {
                    const auto p = t / (std::to_string(s) + "x" + std::to_string(s)) / "apps" / (name + ext);
                    if (fs::exists(p, ec))
                        return p.string();
                }
            const auto sv = t / "scalable/apps" / (name + ".svg");
            if (fs::exists(sv, ec))
                return sv.string();
        }
    }
    return "";
}

std::string KwIcon::findIconPath(const std::string& windowClass, int wantPx) {
    if (windowClass.empty())
        return "";
    const auto  key = lower(windowClass);
    const auto& m   = desktopIcons();
    if (auto it = m.find(key); it != m.end())
        if (auto p = themeLookup(it->second, wantPx); !p.empty())
            return p;
    return themeLookup(key, wantPx);
}

cairo_surface_t* KwIcon::load(const std::string& path, int size) {
    if (path.empty() || size < 1)
        return nullptr;

    auto* out = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size, size);
    auto* cr  = cairo_create(out);

    bool  ok = false;
    if (path.ends_with(".svg")) {
        GError*     err    = nullptr;
        RsvgHandle* handle = rsvg_handle_new_from_file(path.c_str(), &err);
        if (handle) {
            RsvgRectangle vp = {0, 0, (double)size, (double)size};
            ok               = rsvg_handle_render_document(handle, cr, &vp, &err);
            g_object_unref(handle);
        }
        if (err)
            g_error_free(err);
    } else {
        auto* png = cairo_image_surface_create_from_png(path.c_str());
        if (cairo_surface_status(png) == CAIRO_STATUS_SUCCESS) {
            const double w = cairo_image_surface_get_width(png), h = cairo_image_surface_get_height(png);
            if (w > 0 && h > 0) {
                const double s = std::min(size / w, size / h);
                cairo_translate(cr, (size - w * s) / 2, (size - h * s) / 2);
                cairo_scale(cr, s, s);
                cairo_set_source_surface(cr, png, 0, 0);
                cairo_paint(cr);
                ok = true;
            }
        }
        cairo_surface_destroy(png);
    }

    cairo_destroy(cr);
    if (!ok) {
        cairo_surface_destroy(out);
        return nullptr;
    }
    return out;
}
