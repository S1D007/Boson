---
sidebar_position: 2
title: Prerequisites
---

# Prerequisites for Boson Development

Before diving into Boson development, it's important to have a solid foundation in certain technologies and concepts. This page outlines the knowledge and tools you'll need to be successful with the Boson framework.

## Knowledge Prerequisites

### C++ Proficiency

Boson is a C++ framework, so familiarity with modern C++ is essential:

- **Core C++ Knowledge**:
  - Classes and objects
  - Templates and generic programming
  - Lambda expressions
  - Smart pointers
  - Standard library containers and algorithms
  
- **Modern C++ Features** (C++11/14/17):
  - Auto type deduction
  - Range-based for loops
  - Move semantics
  - Variadic templates
  - Standard library additions
  - Structured binding

- **Design Patterns**:
  - Dependency injection
  - Factory pattern
  - Observer pattern
  - Singleton (used sparingly)
  - Strategy pattern

### Web Development Concepts

Understanding these web development fundamentals will help you work effectively with Boson:

- **HTTP Protocol**:
  - Request/response model
  - HTTP methods (GET, POST, PUT, DELETE, etc.)
  - Status codes
  - Headers and cookies
  
- **RESTful API Design**:
  - Resource modeling
  - CRUD operations
  - URL structure
  - Statelessness
  
- **Web Security**:
  - Cross-site scripting (XSS)
  - Cross-site request forgery (CSRF)
  - SQL injection
  - Authentication mechanisms
  - Authorization principles

## Development Tools

### Compilers

Boson requires a C++17-compatible compiler:

| Compiler | Minimum Version | Recommended Version |
|----------|-----------------|---------------------|
| GCC | 7.3+ | 10.0+ |
| Clang | 6.0+ | 12.0+ |
| MSVC | 19.14+ (VS 2017 15.7+) | 19.30+ (VS 2022) |

### Build Systems

Boson uses CMake as its primary build system:

- **CMake** 3.14 or newer
- Build generator (one of the following):
  - **Ninja** (recommended for speed)
  - **GNU Make**
  - **Visual Studio**
  - **Xcode**

### Package Managers

While not strictly required, these package managers make dependency management easier:

- **vcpkg** for Windows
- **Homebrew** for macOS
- **apt/dnf** for Linux distributions
- **Conan** for cross-platform package management

### Development Environment

We recommend using one of these development environments for the best Boson experience:

- **Visual Studio Code** with the following extensions:
  - C/C++ extension
  - CMake Tools
  - Clang-Format
  - C++ TestMate

- **CLion** with the following plugins:
  - C/C++ tools (built-in)
  - ClangFormat

- **Visual Studio** with the following extensions:
  - C++ CMake Tools for Windows

## System Requirements

### Minimum System Requirements

- **CPU**: Dual-core, 2.0 GHz
- **RAM**: 4 GB
- **Disk Space**: 1 GB for Boson and dependencies
- **Operating System**:
  - Windows 10+
  - macOS 10.15+
  - Ubuntu 18.04+/Debian 10+
  - Fedora 30+
  - CentOS/RHEL 8+

### Recommended System Requirements

For an optimal development experience:

- **CPU**: Quad-core, 3.0 GHz or faster
- **RAM**: 8 GB or more
- **Disk Space**: 2+ GB SSD
- **Operating System**: Latest version of your preferred OS

## Dependency Libraries

Boson relies on several open-source libraries:

### Required Dependencies

- **OpenSSL** (1.1.1+): For TLS/SSL support
- **zlib** (1.2.11+): For compression

### Optional Dependencies

- **PostgreSQL client** (libpq 10+): For PostgreSQL database support
- **MySQL client** (libmysqlclient 8+): For MySQL database support
- **SQLite** (3.24+): For SQLite database support
- **RapidJSON** (1.1+): For JSON parsing/generation
- **cURL** (7.58+): For HTTP client functionality
- **Google Test** (1.10+): For unit testing

## Next Steps

Once you have all the prerequisites in place:

1. Follow the [Installation](installation.md) guide to install Boson
2. Complete the [Quickstart](quickstart.md) tutorial
3. Explore the [Hello World](hello-world.md) example

If you're missing any prerequisites, refer to the following resources to get up to speed:

- [Learn Modern C++](https://www.learncpp.com/)
- [HTTP Fundamentals](https://developer.mozilla.org/en-US/docs/Web/HTTP)
- [CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)