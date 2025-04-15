package cmd

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/AlecAivazis/survey/v2"
	"github.com/S1D007/boson/pkg/bosonfetch"
	"github.com/S1D007/boson/pkg/project"
	"github.com/fatih/color"
	"github.com/spf13/cobra"
)

var (
	projectName       string
	projectType       string
	skipDependencies  bool
	availableTypes    = []string{"basic", "api", "full", "websocket"}
	defaultProjectDir string
)

var newCmd = &cobra.Command{
	Use:   "new [project-name]",
	Short: "Create a new Boson project",
	Long: `Create a new Boson project with the specified structure.
This command will set up a new Boson project with all necessary files and dependencies.

Examples:
  # Create a new project with interactive prompts
  boson new

  # Create a new project with a specific name
  boson new my-boson-app

  # Create a new project with a specific type
  boson new my-boson-app --type api`,
	Run: func(cmd *cobra.Command, args []string) {
		printLogo()

		headerColor := color.New(color.FgCyan, color.Bold)
		headerColor.Println("📦 CREATE A NEW BOSON PROJECT")
		fmt.Println()

		if projectName == "" && len(args) > 0 {
			projectName = args[0]
		}

		if projectName == "" {
			fmt.Println("🔤 " + color.New(color.Bold).Sprint("Project Name"))
			prompt := &survey.Input{
				Message: "  What's the name of your project?",
				Default: "my-boson-app",
				Help:    "The name will be used as the directory name and in various files",
			}
			survey.AskOne(prompt, &projectName)
			fmt.Println()
		}

		projectName = strings.ReplaceAll(strings.ToLower(projectName), " ", "-")
		projectDir := filepath.Join(defaultProjectDir, projectName)

		if projectType == "" {
			fmt.Println("🧩 " + color.New(color.Bold).Sprint("Project Type"))

			typeDescriptions := map[string]string{
				"basic":     "A simple Boson application with minimal setup",
				"api":       "A RESTful API application with JSON endpoints",
				"full":      "A complete web application with HTML views and API endpoints",
				"websocket": "A real-time application with WebSocket support",
			}

			for _, t := range availableTypes {
				fmt.Printf("  • %s - %s\n", color.CyanString(t), typeDescriptions[t])
			}
			fmt.Println()

			prompt := &survey.Select{
				Message: "  What type of project would you like to create?",
				Options: availableTypes,
				Default: availableTypes[0],
			}
			survey.AskOne(prompt, &projectType)
			fmt.Println()
		}

		info := color.New(color.FgCyan).SprintFunc()
		headerColor.Println("🔧 CREATING PROJECT")
		fmt.Printf("  • Name: %s\n", info(projectName))
		fmt.Printf("  • Type: %s\n", info(projectType))
		fmt.Printf("  • Location: %s\n", info(projectDir))
		var installStatus string
		if skipDependencies {
			installStatus = "No"
		} else {
			installStatus = "Yes"
		}
		fmt.Printf("  • Installing dependencies: %s\n", info(installStatus))

		fmt.Println()

		if !skipDependencies {
			installDir, err := bosonfetch.GetInstallDir()
			frameworkExists := false
			if err == nil {
				if _, err := os.Stat(filepath.Join(installDir, "libboson.a")); err == nil {
					frameworkExists = true
				} else if _, err := os.Stat(filepath.Join(installDir, "boson.lib")); err == nil {
					frameworkExists = true
				}
			}
			if !frameworkExists {
				info := color.New(color.FgCyan).SprintFunc()
				fmt.Printf("%s Boson framework not found. Installing...\n", info("ℹ"))
				err := bosonfetch.InstallOrUpdateBoson(false)
				if err != nil {
					color.Red("Error installing Boson framework: %v", err)
					return
				}
				fmt.Println(color.New(color.FgGreen, color.Bold).Sprint("✓ Boson framework installed!"))
			}
		}

		fmt.Printf("  %s Creating project files...", color.YellowString("⟳"))

		err := project.Create(projectName, projectDir, projectType, !skipDependencies)
		if err != nil {
			fmt.Printf("\r  %s Failed to create project\n", color.RedString("✗"))
			fmt.Println()
			color.Red("Error: %v", err)
			return
		}

		fmt.Printf("\r  %s Project files created successfully\n", color.GreenString("✓"))

		if !skipDependencies {
			fmt.Printf("  %s Installing dependencies...", color.YellowString("⟳"))
			fmt.Printf("\r  %s Dependencies installed successfully\n", color.GreenString("✓"))
		}

		fmt.Println()
		success := color.New(color.FgGreen, color.Bold).SprintFunc()
		fmt.Println(success("🚀 PROJECT CREATED SUCCESSFULLY!"))

		fmt.Println()
		headerColor.Println("📝 NEXT STEPS")
		cmdColor := color.New(color.FgCyan, color.Bold).SprintFunc()

		fmt.Printf("  1. Change to project directory:\n     %s\n\n", cmdColor("cd "+projectName))

		if skipDependencies {
			fmt.Printf("  2. Install dependencies:\n     %s\n\n", cmdColor("boson install"))
			fmt.Printf("  3. Run the project:\n     %s\n", cmdColor("boson run"))
		} else {
			fmt.Printf("  2. Run the project:\n     %s\n", cmdColor("boson run"))
		}

		fmt.Println()
	},
}

func init() {
	defaultProjectDir, _ = filepath.Abs(".")

	rootCmd.AddCommand(newCmd)

	newCmd.Flags().StringVarP(&projectType, "type", "t", "", "Project type (basic, api, full, websocket)")
	newCmd.Flags().BoolVarP(&skipDependencies, "skip-install", "s", false, "Skip installing dependencies")
	newCmd.Flags().StringVarP(&defaultProjectDir, "dir", "d", defaultProjectDir, "Directory to create the project in")
}
