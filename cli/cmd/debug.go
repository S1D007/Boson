package cmd

import (
	"fmt"
	"os"
	"path/filepath"

	"github.com/S1D007/boson/pkg/templates"
	"github.com/fatih/color"
	"github.com/spf13/cobra"
)

// debugCmd represents the debug command
var debugCmd = &cobra.Command{
	Use:   "debug",
	Short: "Provides debugging information for Boson CLI",
	Long:  `Diagnoses issues with templates and paths for better troubleshooting.`,
}

var debugTemplatesCmd = &cobra.Command{
	Use:   "templates",
	Short: "Shows template path and availability information",
	Long:  `Lists all templates directories and checks for the existence of key template files.`,
	Run: func(cmd *cobra.Command, args []string) {
		// Get the template root path
		templateRoot := templates.GetTemplateRootPath()
		execPath, _ := os.Executable()

		// Print paths
		cyan := color.New(color.FgCyan).SprintFunc()
		green := color.New(color.FgGreen).SprintFunc()
		red := color.New(color.FgRed).SprintFunc()
		yellow := color.New(color.FgYellow).SprintFunc()

		fmt.Println(cyan("Boson CLI Template Diagnostics"))
		fmt.Println("============================")
		fmt.Printf("Executable path:  %s\n", execPath)
		fmt.Printf("Template root:    %s\n", templateRoot)
		fmt.Println()

		fmt.Println(cyan("Project Templates:"))
		projectTypes := []string{"common", "api", "full", "websocket"}
		for _, projType := range projectTypes {
			dirPath := filepath.Join(templateRoot, "project", projType)
			if _, err := os.Stat(dirPath); os.IsNotExist(err) {
				fmt.Printf("  %s: %s\n", projType, red("not found"))
			} else {
				fmt.Printf("  %s: %s\n", projType, green("found"))

				if projType == "full" {
					subdirs := []string{"controllers", "middleware", "models", "views", "services"}
					for _, subdir := range subdirs {
						subdirPath := filepath.Join(dirPath, subdir)
						if _, err := os.Stat(subdirPath); os.IsNotExist(err) {
							fmt.Printf("    - %s: %s\n", subdir, yellow("not found"))
						} else {
							fmt.Printf("    - %s: %s\n", subdir, green("found"))

							files, err := os.ReadDir(subdirPath)
							if err != nil {
								fmt.Printf("      %s\n", red("Error reading directory"))
							} else if len(files) == 0 {
								fmt.Printf("      %s\n", yellow("(empty)"))
							} else {
								for _, file := range files {
									fmt.Printf("      • %s\n", file.Name())
								}
							}
						}
					}
				}
			}
		}

		// Check component templates
		fmt.Println()
		fmt.Println(cyan("Component Templates:"))
		components := []string{"controller", "model", "middleware", "service", "router"}
		for _, comp := range components {
			dirPath := filepath.Join(templateRoot, comp)
			if _, err := os.Stat(dirPath); os.IsNotExist(err) {
				fmt.Printf("  %s: %s\n", comp, red("not found"))
			} else {
				fmt.Printf("  %s: %s\n", comp, green("found"))
			}
		}
	},
}

func init() {
	rootCmd.AddCommand(debugCmd)
	debugCmd.AddCommand(debugTemplatesCmd)
}
