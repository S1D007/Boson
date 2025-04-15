package templates

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"
)

func GetTemplateRootPath() string {
	_, filename, _, _ := runtime.Caller(0)
	return filepath.Join(filepath.Dir(filename), "..", "..", "templates")
}

var TemplateRoot = GetTemplateRootPath()

func GetTemplate(templateName string) (string, error) {
	templatePaths := map[string]string{
		"controller.hpp.tmpl": filepath.Join(TemplateRoot, "controller", "controller.hpp.tmpl"),
		"model.hpp.tmpl":      filepath.Join(TemplateRoot, "model", "model.hpp.tmpl"),
		"middleware.hpp.tmpl": filepath.Join(TemplateRoot, "middleware", "middleware.hpp.tmpl"),
		"service.hpp.tmpl":    filepath.Join(TemplateRoot, "service", "service.hpp.tmpl"),
		"router.hpp.tmpl":     filepath.Join(TemplateRoot, "router", "router.hpp.tmpl"),
	}

	path, ok := templatePaths[templateName]
	if !ok {
		return "", fmt.Errorf("template not found: %s", templateName)
	}

	content, err := os.ReadFile(path)
	if err != nil {
		return "", fmt.Errorf("error reading template file %s: %w", path, err)
	}

	return string(content), nil
}

func GetProjectTemplates(projectType string) (map[string]string, error) {
	commonTemplates, err := loadCommonTemplates()
	if err != nil {
		return nil, err
	}

	switch strings.ToLower(projectType) {
	case "basic":
		return commonTemplates, nil

	case "api":
		apiTemplates := copyTemplates(commonTemplates)

		apiModelContent, err := loadFile(filepath.Join(TemplateRoot, "project", "api", "user.hpp"))
		if err != nil {
			return nil, err
		}
		apiTemplates["src/models/user.hpp"] = apiModelContent

		apiServiceContent, err := loadFile(filepath.Join(TemplateRoot, "project", "api", "user_service.hpp"))
		if err != nil {
			return nil, err
		}
		apiTemplates["src/services/user_service.hpp"] = apiServiceContent

		apiControllerContent, err := loadFile(filepath.Join(TemplateRoot, "project", "api", "api_controller.hpp"))
		if err != nil {
			return nil, err
		}
		apiTemplates["src/controllers/api_controller.hpp"] = apiControllerContent

		return apiTemplates, nil

	case "full":
		fullTemplates := copyTemplates(commonTemplates)

		userModelContent, err := loadFile(filepath.Join(TemplateRoot, "project", "api", "user.hpp"))
		if err != nil {
			return nil, err
		}
		fullTemplates["src/models/user.hpp"] = userModelContent

		userServiceContent, err := loadFile(filepath.Join(TemplateRoot, "project", "api", "user_service.hpp"))
		if err != nil {
			return nil, err
		}
		fullTemplates["src/services/user_service.hpp"] = userServiceContent

		layoutViewContent, err := loadFile(filepath.Join(TemplateRoot, "project", "full", "layout.hpp"))
		if err != nil {
			return nil, err
		}
		fullTemplates["src/views/layout.hpp"] = layoutViewContent

		indexHtmlContent, err := loadFile(filepath.Join(TemplateRoot, "project", "full", "index.html"))
		if err != nil {
			return nil, err
		}
		fullTemplates["public/index.html"] = indexHtmlContent

		cssContent, err := loadFile(filepath.Join(TemplateRoot, "project", "full", "styles.css"))
		if err != nil {
			return nil, err
		}
		fullTemplates["public/css/styles.css"] = cssContent

		jsContent, err := loadFile(filepath.Join(TemplateRoot, "project", "full", "app.js"))
		if err != nil {
			return nil, err
		}
		fullTemplates["public/js/app.js"] = jsContent

		return fullTemplates, nil

	case "websocket":
		wsTemplates := copyTemplates(commonTemplates)

		wsHandlerContent, err := loadFile(filepath.Join(TemplateRoot, "project", "websocket", "websocket_handler.hpp"))
		if err != nil {
			return nil, err
		}
		wsTemplates["src/handlers/websocket_handler.hpp"] = wsHandlerContent

		wsHtmlContent, err := loadFile(filepath.Join(TemplateRoot, "project", "websocket", "websocket_html.html"))
		if err != nil {
			return nil, err
		}
		wsTemplates["public/index.html"] = wsHtmlContent

		wsJsContent, err := loadFile(filepath.Join(TemplateRoot, "project", "websocket", "websocket.js"))
		if err != nil {
			return nil, err
		}
		wsTemplates["public/js/websocket.js"] = wsJsContent

		return wsTemplates, nil

	default:
		return nil, fmt.Errorf("unknown project type: %s", projectType)
	}
}

func loadCommonTemplates() (map[string]string, error) {
	commonTemplates := make(map[string]string)

	cmakeContent, err := loadFile(filepath.Join(TemplateRoot, "project", "common", "CMakeLists.txt"))
	if err != nil {
		return nil, err
	}
	commonTemplates["CMakeLists.txt"] = cmakeContent

	mainContent, err := loadFile(filepath.Join(TemplateRoot, "project", "common", "main.cpp"))
	if err != nil {
		return nil, err
	}
	commonTemplates["src/main.cpp"] = mainContent

	helloControllerContent, err := loadFile(filepath.Join(TemplateRoot, "project", "common", "hello_controller.hpp"))
	if err != nil {
		return nil, err
	}
	commonTemplates["src/controllers/hello_controller.hpp"] = helloControllerContent

	gitignoreContent, err := loadFile(filepath.Join(TemplateRoot, "project", "common", ".gitignore"))
	if err != nil {
		return nil, err
	}
	commonTemplates[".gitignore"] = gitignoreContent

	readmeContent, err := loadFile(filepath.Join(TemplateRoot, "project", "common", "README.md"))
	if err != nil {
		return nil, err
	}
	commonTemplates["README.md"] = readmeContent

	return commonTemplates, nil
}

func loadFile(path string) (string, error) {
	if _, err := os.Stat(path); os.IsNotExist(err) {
		execPath, err := os.Executable()
		if err != nil {
			return "", fmt.Errorf("failed to get executable path: %w", err)
		}

		execDir := filepath.Dir(execPath)
		altPath := filepath.Join(execDir, "..", "templates", strings.TrimPrefix(path, TemplateRoot))

		if _, err := os.Stat(altPath); os.IsNotExist(err) {
			return "", fmt.Errorf("template file not found: %s", path)
		}

		path = altPath
	}

	content, err := os.ReadFile(path)
	if err != nil {
		return "", fmt.Errorf("error reading file %s: %w", path, err)
	}

	return string(content), nil
}

func copyTemplates(templates map[string]string) map[string]string {
	result := make(map[string]string, len(templates))
	for k, v := range templates {
		result[k] = v
	}
	return result
}
