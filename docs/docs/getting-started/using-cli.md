---
sidebar_position: 3
title: Using the CLI
---

# Using the Boson CLI

The Boson CLI is a powerful command-line tool that simplifies the development workflow for Boson applications. This guide will walk you through all the CLI features and show you how to use them effectively.

## Overview

The Boson CLI provides a comprehensive set of commands to:
- Create new projects
- Generate components (controllers, models, middleware)
- Build your application
- Run development servers
- Update the framework
- Install dependencies

## Command Reference

### Getting Help

To see all available commands and general help:

```bash
boson
# OR
boson --help
```

This displays the Boson logo and a list of available commands.

### Create a New Project

To create a new Boson project:

```bash
boson new <project-name>
```

Options:
- `--template` or `-t`: Specify a project template (default: "basic")
- `--path` or `-p`: Specify a custom path for the project

Example:
```bash
# Create a basic project
boson new my-app

# Create a project using the API template
boson new my-api --template api

# Create a project in a specific directory
boson new my-app --path /path/to/directory
```

When you create a new project, the CLI sets up the project structure, initializes a Git repository, and installs the necessary dependencies.

### Generate Components

Quickly scaffold new components for your project:

```bash
boson generate <component-type> <name>
```

Component types:
- `controller`: Generate a new controller
- `model`: Generate a new data model
- `middleware`: Generate a request middleware
- `service`: Generate a service class

Examples:
```bash
# Generate a user controller
boson generate controller User

# Generate a user model
boson generate model User

# Generate authentication middleware
boson generate middleware Auth
```

Generated components follow Boson best practices and are automatically integrated into your project.

### Run Your Application

Start a development server with:

```bash
boson run
```

Options:
- `--port` or `-p`: Specify the port (default: 8080)
- `--watch` or `-w`: Enable hot-reloading for development

Example:
```bash
# Run on port 3000 with hot-reloading
boson run --port 3000 --watch
```

### Build Your Project

Compile your Boson application:

```bash
boson build
```

Options:
- `--release` or `-r`: Create a release build with optimizations
- `--output` or `-o`: Specify output directory

Example:
```bash
# Create a release build
boson build --release

# Specify a custom output directory
boson build --output ./dist
```

### Install Dependencies

Install Boson dependencies for your project:

```bash
boson install
```

This command installs or updates the Boson framework and any other required dependencies for your project.

### Update the CLI and Framework

Keep your Boson CLI and framework up to date:

```bash
boson update
```

This checks for the latest version and updates both the CLI tool and framework libraries if newer versions are available.

### Check Version Information

View version information:

```bash
boson version
```

This displays the current version of the CLI and framework.

## Configuration

The Boson CLI can be configured using a `.boson.yaml` file in your home directory or project root. You can also specify a configuration file with the `--config` flag.

Example configuration file:
```yaml
# .boson.yaml
project:
  default_template: api
build:
  compiler: clang++
  flags: "-std=c++17 -O2"
  auto_install_deps: true
```

## Common Workflows

### Starting a New Project

```bash
# Create a new project
boson new my-app

# Navigate to the project directory
cd my-app

# Start the development server
boson run --watch
```

### Adding a New Feature

```bash
# Generate a controller
boson generate controller Product

# Generate a model
boson generate model Product

# Run the server to test your changes
boson run
```

### Building for Production

```bash
# Create an optimized release build
boson build --release

# The built application will be in the specified output directory
```

## Troubleshooting

### Common Issues

If you encounter issues with the CLI, try these solutions:

1. **Command not found**: Ensure the CLI is installed correctly and is in your PATH
2. **Build failures**: Check your C++ compiler version and ensure all dependencies are installed
3. **Permission errors**: You might need to run with sudo/administrator privileges

For more help, use the `--verbose` flag with any command to see detailed output:

```bash
boson build --verbose
```

## Next Steps

Now that you understand how to use the Boson CLI, learn about the [Project Structure](./project-structure) or check out some [Examples](../examples/basic-server) to see Boson in action.