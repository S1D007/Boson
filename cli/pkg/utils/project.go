package utils

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
)

func IsBosonProject(dir string) bool {

	cmakeFile := filepath.Join(dir, "CMakeLists.txt")
	if _, err := os.Stat(cmakeFile); err == nil {

		content, err := os.ReadFile(cmakeFile)
		if err == nil && strings.Contains(string(content), "Boson") {
			return true
		}
	}
	return false
}

func ExecutableExists(dir string) bool {
	executable := FindExecutable(dir)
	return executable != ""
}

func FindExecutable(dir string) string {

	buildDirs := []string{"build", "cmake-build-debug", "cmake-build-release"}

	projectName := getProjectName(dir)

	if projectName == "" {
		projectName = filepath.Base(dir)
	}

	for _, buildDir := range buildDirs {
		buildPath := filepath.Join(dir, buildDir)

		if _, err := os.Stat(buildPath); err == nil {

			var executable string
			if runtime.GOOS == "windows" {
				executable = filepath.Join(buildPath, projectName+".exe")
			} else {
				executable = filepath.Join(buildPath, projectName)
			}

			if _, err := os.Stat(executable); err == nil {
				return executable
			}
		}
	}

	return ""
}

func BuildProject(dir string) error {
	return BuildProjectWithOptions(dir, "build", "Debug", nil)
}

func BuildProjectWithOptions(dir string, buildDir string, buildType string, cmakeOptions []string) error {

	buildPath := filepath.Join(dir, buildDir)
	if _, err := os.Stat(buildPath); os.IsNotExist(err) {
		err = os.MkdirAll(buildPath, 0755)
		if err != nil {
			return fmt.Errorf("failed to create build directory: %v", err)
		}
	}

	currentDir, err := os.Getwd()
	if err != nil {
		return fmt.Errorf("failed to get current directory: %v", err)
	}

	err = os.Chdir(buildPath)
	if err != nil {
		return fmt.Errorf("failed to change to build directory: %v", err)
	}

	cmakeCmd := exec.Command("cmake", append([]string{
		fmt.Sprintf("-DCMAKE_BUILD_TYPE=%s", buildType),
		".."}, cmakeOptions...)...)

	cmakeCmd.Stdout = os.Stdout
	cmakeCmd.Stderr = os.Stderr

	err = cmakeCmd.Run()
	if err != nil {
		os.Chdir(currentDir)
		return fmt.Errorf("cmake configuration failed: %v", err)
	}

	buildCmd := exec.Command("cmake", "--build", ".")

	buildCmd.Stdout = os.Stdout
	buildCmd.Stderr = os.Stderr

	err = buildCmd.Run()
	if err != nil {
		os.Chdir(currentDir)
		return fmt.Errorf("build failed: %v", err)
	}

	err = os.Chdir(currentDir)
	if err != nil {
		return fmt.Errorf("failed to return to original directory: %v", err)
	}

	return nil
}

func InstallDependencies(dir string, force bool) error {

	var installCmd *exec.Cmd

	switch runtime.GOOS {
	case "darwin":

		if _, err := exec.LookPath("brew"); err == nil {
			installCmd = exec.Command("brew", "install", "openssl", "zlib")
		}
	case "linux":

		if _, err := exec.LookPath("apt"); err == nil {
			installCmd = exec.Command("apt", "install", "-y", "libssl-dev", "zlib1g-dev")

			if os.Geteuid() != 0 {
				installCmd = exec.Command("sudo", "apt", "install", "-y", "libssl-dev", "zlib1g-dev")
			}
		} else if _, err := exec.LookPath("dnf"); err == nil {

			installCmd = exec.Command("dnf", "install", "-y", "openssl-devel", "zlib-devel")
			if os.Geteuid() != 0 {
				installCmd = exec.Command("sudo", "dnf", "install", "-y", "openssl-devel", "zlib-devel")
			}
		}
	case "windows":

		if _, err := exec.LookPath("vcpkg"); err == nil {
			installCmd = exec.Command("vcpkg", "install", "openssl", "zlib")
		}
	}

	if installCmd != nil {
		installCmd.Stdout = os.Stdout
		installCmd.Stderr = os.Stderr

		err := installCmd.Run()
		if err != nil {
			return fmt.Errorf("dependency installation failed: %v", err)
		}
	} else {
		fmt.Println("Warning: Could not detect package manager. Please install dependencies manually:")
		fmt.Println("  - OpenSSL (1.1.1+)")
		fmt.Println("  - zlib (1.2.11+)")
	}

	return nil
}

func getProjectName(dir string) string {
	cmakeFile := filepath.Join(dir, "CMakeLists.txt")
	content, err := os.ReadFile(cmakeFile)
	if err != nil {
		return ""
	}

	lines := strings.Split(string(content), "\n")
	for _, line := range lines {
		line = strings.TrimSpace(line)
		if strings.HasPrefix(line, "project(") {
			start := strings.Index(line, "(") + 1
			end := strings.Index(line[start:], ")") + start
			if end > start {
				parts := strings.Split(line[start:end], " ")
				if len(parts) > 0 {
					return parts[0]
				}
			}
		}
	}

	return ""
}
