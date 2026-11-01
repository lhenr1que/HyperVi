# Else exist specifically for clang
ifeq ($(CXX),g++)
    EXTRA_FLAGS = --no-gnu-unique
else
    EXTRA_FLAGS =
endif

CXXFLAGS ?= -O2
CXXFLAGS += -shared -fPIC -std=c++2b -Wno-c++11-narrowing
INCLUDES = `pkg-config --cflags pixman-1 libdrm hyprland libinput libudev wayland-server xkbcommon cairo librsvg-2.0`
LIBS = `pkg-config --libs cairo librsvg-2.0`

SRC = main.cpp barDeco.cpp BarPassElement.cpp PreviewPassElement.cpp icon.cpp
TARGET = kwinish-bars.so

all: $(TARGET)

# every header is a prerequisite, so a header change rebuilds the plugin; only the .cpp files are compiled
HDR = $(filter-out test_%,$(wildcard *.hpp))

$(TARGET): $(SRC) $(HDR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(EXTRA_FLAGS) $(INCLUDES) $(SRC) -o $@ $(LIBS)

clean:
	rm ./$(TARGET)

meson-build:
	mkdir -p build
	cd build && meson .. && ninja

.PHONY: all meson-build clean

abi: $(TARGET)
	./abi.sh > kwinish-bars.abi
