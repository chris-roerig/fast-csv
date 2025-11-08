CXX = g++
CXXFLAGS = -std=c++17 -O2 `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`
TARGET = fastcsv
SRCDIR = src
SOURCES = $(SRCDIR)/main.cpp
BUILD_DIR = build

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    PLATFORM = macos
    SOURCES += platform/macos/mac_open_bridge.mm
    LDFLAGS += -framework Cocoa
    APP_BUNDLE = FastCSV.app
else ifeq ($(UNAME_S),Linux)
    PLATFORM = linux
    TARGET_EXT = 
else
    PLATFORM = windows
    TARGET_EXT = .exe
endif

# Default target - show help
help:
	@echo "FastCSV Build System"
	@echo "===================="
	@echo ""
	@echo "Platform detected: $(PLATFORM)"
	@echo ""
	@echo "Available targets:"
	@echo "  help        Show this help message"
	@echo "  all         Build for current platform"
	@echo "  macos       Build macOS app bundle (macOS only)"
	@echo "  linux       Build Linux binary (Linux only)"
	@echo "  windows     Build Windows executable (Windows only)"
	@echo "  install-mac Install to /Applications (macOS only)"
	@echo "  install-linux Install system-wide (Linux only)"
	@echo "  clean       Remove build artifacts"
	@echo ""
	@echo "Quick start:"
	@echo "  make all && make install-$(PLATFORM)"

$(TARGET): $(SOURCES)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$(TARGET)$(TARGET_EXT) $(SOURCES) $(LDFLAGS)

# Platform-specific builds
macos: $(TARGET)
ifeq ($(PLATFORM),macos)
	mkdir -p $(APP_BUNDLE)/Contents/MacOS
	mkdir -p $(APP_BUNDLE)/Contents/Resources
	cp $(BUILD_DIR)/$(TARGET) $(APP_BUNDLE)/Contents/MacOS/fastcsv
	cp resources/icons/fastcsv.icns $(APP_BUNDLE)/Contents/Resources/
	cp platform/macos/Info.plist $(APP_BUNDLE)/Contents/
	@echo "macOS app bundle created: $(APP_BUNDLE)"
else
	@echo "Error: macOS build only supported on macOS"
endif

linux: $(TARGET)
ifeq ($(PLATFORM),linux)
	@echo "Linux build created: $(BUILD_DIR)/$(TARGET)"
	@echo "Desktop file available: platform/linux/fastcsv.desktop"
else
	@echo "Error: Linux build only supported on Linux"
endif

windows: $(TARGET)
ifeq ($(PLATFORM),windows)
	@echo "Windows build created: $(BUILD_DIR)/$(TARGET)$(TARGET_EXT)"
else
	@echo "Error: Windows build only supported on Windows"
endif

# Legacy targets
app: macos

install-mac: macos
ifeq ($(PLATFORM),macos)
	cp -R $(APP_BUNDLE) /Applications/
	@echo "FastCSV installed to /Applications/"
else
	@echo "Error: macOS install only supported on macOS"
endif

install-linux: linux
ifeq ($(PLATFORM),linux)
	sudo cp $(BUILD_DIR)/$(TARGET) /usr/local/bin/
	sudo cp platform/linux/fastcsv.desktop /usr/share/applications/
	sudo cp resources/icons/pngs/icon_64.png /usr/share/pixmaps/fastcsv.png
	sudo update-desktop-database
	@echo "FastCSV installed system-wide on Linux"
else
	@echo "Error: Linux install only supported on Linux"
endif

# Build for current platform
all: $(TARGET)
	@echo "Built for $(PLATFORM): $(BUILD_DIR)/$(TARGET)$(TARGET_EXT)"

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(APP_BUNDLE)

.PHONY: help clean app macos linux windows all install-mac install-linux

# Make help the default target
.DEFAULT_GOAL := help
