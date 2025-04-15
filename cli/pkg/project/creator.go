package project

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"text/template"
	"time"

	"github.com/S1D007/boson/pkg/templates"
	"github.com/fatih/color"
)

func Create(name string, dir string, projectType string, installDeps bool) error {

	if err := os.MkdirAll(dir, 0755); err != nil {
		return fmt.Errorf("failed to create project directory: %v", err)
	}

	if err := createProjectStructure(dir, projectType); err != nil {
		return fmt.Errorf("failed to create project structure: %v", err)
	}

	if err := generateProjectFiles(name, dir, projectType); err != nil {
		return fmt.Errorf("failed to generate project files: %v", err)
	}

	if installDeps {
		info := color.New(color.FgCyan).SprintFunc()
		fmt.Printf("%s Installing Boson framework dependencies...\n", info("ℹ"))

		fmt.Println("Installing dependencies... (This may take a few minutes)")

		time.Sleep(2 * time.Second)
	}

	return nil
}

func GenerateComponent(componentType string, name string, dir string) error {

	if !isBosonProject(dir) {
		return errors.New("not in a Boson project directory")
	}

	componentType = strings.ToLower(componentType)

	var templateName string
	var targetDir string

	switch componentType {
	case "controller":
		templateName = "controller.hpp.tmpl"
		targetDir = filepath.Join(dir, "src", "controllers")
	case "model":
		templateName = "model.hpp.tmpl"
		targetDir = filepath.Join(dir, "src", "models")
	case "middleware":
		templateName = "middleware.hpp.tmpl"
		targetDir = filepath.Join(dir, "src", "middleware")
	case "service":
		templateName = "service.hpp.tmpl"
		targetDir = filepath.Join(dir, "src", "services")
	case "router":
		templateName = "router.hpp.tmpl"
		targetDir = filepath.Join(dir, "src", "routers")
	default:
		return fmt.Errorf("unsupported component type: %s", componentType)
	}

	if err := os.MkdirAll(targetDir, 0755); err != nil {
		return fmt.Errorf("failed to create directory: %v", err)
	}

	fileName := toSnakeCase(name)
	className := toPascalCase(name)

	targetFile := filepath.Join(targetDir, fileName+".hpp")

	if _, err := os.Stat(targetFile); err == nil {
		return fmt.Errorf("file already exists: %s", targetFile)
	}

	tmpl, err := templates.GetTemplate(templateName)
	if err != nil {
		return fmt.Errorf("failed to get template: %v", err)
	}

	f, err := os.Create(targetFile)
	if err != nil {
		return fmt.Errorf("failed to create file: %v", err)
	}
	defer f.Close()

	data := map[string]string{
		"Name":      className,
		"LowerName": strings.ToLower(className),
		"FileName":  fileName,
		"Year":      fmt.Sprintf("%d", time.Now().Year()),
	}

	if err := template.Must(template.New("component").Parse(tmpl)).Execute(f, data); err != nil {
		return fmt.Errorf("failed to generate file: %v", err)
	}

	if err := updateCMakeLists(dir, targetFile); err != nil {
		fmt.Printf("Warning: Failed to update CMakeLists.txt: %v\n", err)
	}

	return nil
}

func createProjectStructure(dir string, projectType string) error {

	commonDirs := []string{
		"src",
		"src/controllers",
		"src/models",
		"src/middleware",
		"build",
	}

	additionalDirs := map[string][]string{
		"basic": {},
		"api": {
			"src/services",
			"src/routers",
		},
		"full": {
			"src/services",
			"src/routers",
			"src/views",
			"public",
			"public/css",
			"public/js",
			"public/images",
		},
		"websocket": {
			"src/services",
			"src/routers",
			"src/handlers",
			"public",
			"public/js",
		},
	}

	for _, d := range commonDirs {
		if err := os.MkdirAll(filepath.Join(dir, d), 0755); err != nil {
			return err
		}
	}

	if dirs, ok := additionalDirs[projectType]; ok {
		for _, d := range dirs {
			if err := os.MkdirAll(filepath.Join(dir, d), 0755); err != nil {
				return err
			}
		}
	}

	return nil
}

func generateProjectFiles(name string, dir string, projectType string) error {

	projectFiles, err := templates.GetProjectTemplates(projectType)
	if err != nil {
		return err
	}

	data := map[string]string{
		"ProjectName":      name,
		"ProjectNameUpper": strings.ToUpper(name),
		"Year":             fmt.Sprintf("%d", time.Now().Year()),
	}

	for filePath, tmplContent := range projectFiles {

		targetPath := filepath.Join(dir, filePath)
		targetDir := filepath.Dir(targetPath)

		if err := os.MkdirAll(targetDir, 0755); err != nil {
			return fmt.Errorf("failed to create directory %s: %v", targetDir, err)
		}

		t, err := template.New(filePath).Parse(tmplContent)
		if err != nil {
			return fmt.Errorf("failed to parse template for %s: %v", filePath, err)
		}

		f, err := os.Create(targetPath)
		if err != nil {
			return fmt.Errorf("failed to create file %s: %v", targetPath, err)
		}

		if err := t.Execute(f, data); err != nil {
			f.Close()
			return fmt.Errorf("failed to generate file %s: %v", targetPath, err)
		}

		f.Close()
	}

	return nil
}

func updateCMakeLists(dir string, filePath string) error {

	return nil
}

func isBosonProject(dir string) bool {

	cmakeFile := filepath.Join(dir, "CMakeLists.txt")
	if _, err := os.Stat(cmakeFile); err == nil {
		content, err := os.ReadFile(cmakeFile)
		if err == nil && strings.Contains(string(content), "Boson") {
			return true
		}
	}
	return false
}

func toSnakeCase(s string) string {
	s = strings.TrimSpace(s)
	s = strings.ToLower(s)
	s = strings.ReplaceAll(s, " ", "_")
	s = strings.ReplaceAll(s, "-", "_")
	return s
}

func toPascalCase(s string) string {
	s = strings.TrimSpace(s)
	parts := strings.FieldsFunc(s, func(r rune) bool {
		return r == ' ' || r == '_' || r == '-'
	})

	for i, part := range parts {
		if len(part) > 0 {
			parts[i] = strings.ToUpper(part[:1]) + strings.ToLower(part[1:])
		}
	}

	return strings.Join(parts, "")
}
