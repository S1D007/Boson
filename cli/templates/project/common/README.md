# {{.ProjectName}}

A Boson framework project.

## Getting Started

### Prerequisites

- CMake 3.14 or higher
- C++17 compatible compiler
- Boson framework installed

### Building the Project

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Running the Project

```bash
./{{.ProjectName}}
```

The server will start at http://localhost:3000

## Project Structure

```
{{.ProjectName}}/
├── CMakeLists.txt            # Project configuration
├── src/                      # Source code
│   ├── main.cpp              # Application entry point
│   ├── controllers/          # Request handlers
│   │   └── hello_controller.hpp
│   ├── models/               # Data structures
│   └── middleware/           # Request/response processors
└── public/                   # Static files (if any)
```

## Adding New Controllers

To add a new controller:

```bash
boson generate controller my_controller
```

## License

This project is licensed under the MIT License - see the LICENSE file for details.