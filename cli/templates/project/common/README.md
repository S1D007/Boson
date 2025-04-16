# {{.ProjectName}}

A web application built with the Boson C++ Web Framework.

## Getting Started

### Prerequisites

- CMake 3.14 or higher
- C++17 compatible compiler
- Boson Framework installed

### Building the Project

```bash
mkdir build && cd build
cmake ..
make
```

### Running the Application

From the build directory:

```bash
./{{.ProjectName}}
```

The server will start at http://localhost:3000

## Project Structure

- `src/` - Source code
  - `controllers/` - Controller classes
  - `models/` - Data models
  - `middleware/` - Custom middleware
- `public/` - Static files (if applicable)

## API Endpoints

- `GET /api` - Welcome message
- `GET /api/hello` - Returns a hello world message
- `GET /api/hello/:name` - Returns a personalized hello message

## License

This project is licensed under the MIT License