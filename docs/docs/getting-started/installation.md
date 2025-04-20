---
sidebar_position: 1
title: Installation
---

# Installing Boson Framework

This guide will help you install the Boson framework and set up your development environment. Boson supports all major operating systems and can be installed through various methods to suit your workflow.

## System Requirements

Before installing Boson, ensure your system meets these requirements:

### Minimum Requirements
- **C++ Compiler** with C++17 support:
  - GCC 7.3+
  - Clang 6.0+
  - MSVC 2017+ (Visual Studio 2017 15.7+)
- **CMake** 3.14 or higher
- **OpenSSL** 1.1.1 or higher (for HTTPS support)
- **git** (for downloading source and examples)
- At least 1GB free disk space

### Recommended
- **C++ Compiler**: Latest stable GCC 10+, Clang 12+, or MSVC 2022
- **Build System**: Ninja (faster builds than Make)
- **Editor**: Visual Studio Code with C++ extension, CLion, or other C++ IDE
- **Package Manager**: vcpkg (Windows), Homebrew (macOS), or apt/dnf (Linux)

## Quick Installation

### Using the Boson CLI Installer (Recommended)

#### macOS/Linux

```bash
curl -fsSL https://raw.githubusercontent.com/S1D007/boson/main/install.sh | bash
```

This script will:
- Download the latest Boson CLI and framework for your platform
- Install it to `~/.local/bin` (or another directory in your PATH)
- Install the Boson library for development
- Set up auto-update capabilities

#### Windows

```powershell
Invoke-WebRequest -Uri https://raw.githubusercontent.com/S1D007/boson/main/install.ps1 -UseBasicParsing | Invoke-Expression
```

This script will:
- Download the latest Boson CLI release for Windows
- Install it to `%USERPROFILE%\.boson\bin`
- Add the install path to your system `PATH`
- Install the Boson library for development
- Set up auto-update capabilities

> ℹ️ **Note:** You may need to restart your terminal after installation for the `boson` command to be recognized.


## Building from Source

For those who prefer to build from source or need the latest development version:

### Step 1: Clone the Repository

```bash
git clone https://github.com/S1D007/boson.git
cd boson
```

### Step 2: Create Build Directory

```bash
mkdir build && cd build
```

### Step 3: Configure with CMake

```bash
# Basic configuration
cmake ..

# Or for a release build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Or with custom options
cmake .. -DCMAKE_BUILD_TYPE=Release -DBOSON_WITH_SQLITE=ON -DBOSON_BUILD_EXAMPLES=ON
```

### Step 4: Build and Install

```bash
# Build the project
cmake --build .

# Install system-wide (may require sudo/administrator)
cmake --install .
```

### Available CMake Options

| Option | Description | Default |
|--------|-------------|---------|
| `BOSON_BUILD_EXAMPLES` | Build example applications | ON |
| `BOSON_WITH_SQLITE` | Enable SQLite database support | OFF |
| `BUILD_TESTS` | Build tests | OFF |

## Verifying Your Installation

### Using the CLI

After installation, open a new terminal and run:

```bash
boson --version
```

You should see the Boson CLI version information.

### Testing a Simple Application

Create a file named `test.cpp`:

```cpp
#include <boson/boson.hpp>
#include <iostream>

int main() {
    std::cout << "Boson installed successfully!" << std::endl;
    
    boson::initialize();
    boson::Server app;
    
    app.get("/", [](const boson::Request& req, boson::Response& res) {
        res.send("Hello, Boson!");
    });
    
    std::cout << "Server configured successfully!" << std::endl;
    
    // Don't actually start the server for this test
    return 0;
}
```

Compile with:

```bash
# Using g++
g++ -std=c++17 test.cpp -lboson -o test

# Or using clang++
clang++ -std=c++17 test.cpp -lboson -o test

# Or using MSVC
cl /std:c++17 test.cpp /I path\to\boson\include /link boson.lib
```

Run the test:

```bash
./test  # or test.exe on Windows
```

If you see "Boson installed successfully!" and "Server configured successfully!" messages, your installation is working correctly.

## Keeping Boson Up to Date

### Using the CLI

```bash
# Check for updates
boson update

```

## Troubleshooting

### Common Issues

#### Missing Libraries

If you encounter errors about missing libraries during compilation:

```bash
# Ubuntu/Debian
sudo apt install libssl-dev build-essential pkg-config

# Fedora/RHEL/CentOS
sudo dnf install openssl-devel gcc-c++ make

# macOS
brew install openssl

# Windows (vcpkg)
vcpkg install openssl:x64-windows
```

#### CMake Can't Find Boson

Add the installation directory to your CMAKE_PREFIX_PATH:

```bash
cmake .. -DCMAKE_PREFIX_PATH=/path/to/boson/installation
```

#### Path Issues on Windows

If Windows can't find the Boson executable or libraries:

1. Check if the Boson bin directory is in your PATH
2. Ensure the DLLs are either in your PATH or in the same directory as your executable
3. Try restarting your terminal or system

### Getting Help

If you encounter issues not covered here:

- Check the [GitHub issues](https://github.com/S1D007/boson/issues) for similar problems
- Post a question on [Stack Overflow](https://stackoverflow.com/questions/tagged/boson-framework) with the `boson-framework` tag

### Manual Uninstallation

#### macOS/Linux
```bash
# Remove CLI and framework files
rm -rf ~/.local/bin/boson ~/.boson
```

#### Windows
```powershell
# Remove CLI and framework files
Remove-Item -Recurse -Force "$env:USERPROFILE\.boson"
```

You may also need to remove the installation directory from your PATH environment variable.

## Next Steps

Now that you have Boson installed, you're ready to start building web applications!

- Continue to the [Quick Start Guide](quickstart) to create your first Boson application
- Learn about the [Boson CLI](using-cli) to boost your productivity
- Explore the [Hello World Tutorial](hello-world) for a more detailed walkthrough

Ready to begin? [Create your first project →](./hello-world)