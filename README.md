# FastCSV

<img src="resources/icons/pngs/icon_128.png" alt="FastCSV Icon" width="128" height="128">

A minimal, lightning-fast CSV viewer built with C++ and GTK3 for macOS. Designed for developers and data analysts who need to quickly view CSV files without the overhead of spreadsheet applications.

## Features

- **Lightning fast startup** - Opens instantly, no bloat
- **Smart column handling** - Resizable columns with fixed-width display
- **Precise cell selection** - Click individual cells for detailed viewing
- **Flexible copying** - Copy single cells (Cmd+C) or entire datasets (Cmd+A → Cmd+C)
- **Text wrapping** - Toggle with Cmd+W for long content
- **Column sorting** - Click headers to sort data ascending/descending
- **Native macOS integration** - Double-click CSV files to open directly

## Installation

**Note:** You must build FastCSV on the target platform. Cross-compilation is not currently supported.

### macOS
```bash
# Install dependencies
brew install gtk+3

# Clone and build
git clone https://github.com/chris-roerig/fast-csv.git
cd fast-csv
make clean && make macos && make install-mac
```

### Linux
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install libgtk-3-dev build-essential

# Or Fedora/RHEL
sudo dnf install gtk3-devel gcc-c++

# Clone and build
git clone https://github.com/chris-roerig/fast-csv.git
cd fast-csv
make clean && make linux && make install-linux
```

### Windows
```bash
# Install MSYS2 and dependencies
pacman -S mingw-w64-x86_64-gtk3 mingw-w64-x86_64-gcc

# Clone and build
git clone https://github.com/chris-roerig/fast-csv.git
cd fast-csv
make clean && make windows
```

## Building

### Build for Current Platform
```bash
make clean && make all
```

### Platform-Specific Builds
```bash
make macos    # Creates FastCSV.app (macOS only)
make linux    # Creates build/fastcsv (Linux only)
make windows  # Creates build/fastcsv.exe (Windows only)
```

**Important:** Each platform must be built on its respective operating system. The build system detects your platform automatically and creates the appropriate binary format.

## Usage

### Opening Files
- **Double-click** any CSV file in Finder
- **Drag & drop** CSV files onto the FastCSV app icon
- **File → Open** from within the application
- **Right-click** CSV files → "Open with FastCSV"

### Keyboard Shortcuts
- `Cmd+O` - Open file
- `Cmd+C` - Copy selected cell
- `Cmd+A` - Select all data
- `Cmd+W` - Toggle text wrapping
- `Cmd+Q` - Quit application

### Mouse Actions
- **Click** cells to select them
- **Click** column headers to sort
- **Drag** column borders to resize
- **Right-click** for context menu

## Building from Source

```bash
# Install GTK3
brew install gtk+3

# Build the application
make clean && make app

# Install to Applications folder
make install-mac
```

## Project Structure

```
fastcsv/
├── src/                    # C++ source code
├── platform/macos/         # macOS-specific integration
├── platform/linux/         # Linux desktop integration
├── platform/windows/       # Windows resource files
├── resources/icons/        # Application icons
├── build/                  # Build output
├── Makefile               # Cross-platform build configuration
└── sample.csv             # Test data
```

## Technical Details

FastCSV is built with modern C++17 and leverages proven open-source technologies:

- **[GTK3](https://gtk.org/)** - Cross-platform GUI toolkit
- **[GLib](https://gitlab.gnome.org/GNOME/glib)** - Core application building blocks
- **Cocoa Bridge** - Native macOS file association handling

### Architecture
- **GtkApplication** - Proper application lifecycle management
- **GtkTreeView** - Efficient data display with sorting and selection
- **Native file associations** - Seamless integration with macOS Finder

## Open Source Acknowledgments

FastCSV is built on the shoulders of giants. We gratefully acknowledge:

- **GTK Project** - GUI toolkit (LGPL-2.1)
- **GNOME Foundation** - GLib library (LGPL-2.1)  
- **Apple Inc.** - Cocoa framework integration
- **Homebrew** - Package management for dependencies

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Contributing

Contributions welcome! Please feel free to submit issues and pull requests.

