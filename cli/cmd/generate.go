package cmd

import (
	"fmt"
	"path/filepath"
	"strings"

	"github.com/AlecAivazis/survey/v2"
	"github.com/S1D007/boson/pkg/project"
	"github.com/fatih/color"
	"github.com/spf13/cobra"
)

var componentName string

var generateCmd = &cobra.Command{
	Use:     "generate [component-type] [name]",
	Aliases: []string{"g"},
	Short:   "Generate a Boson component",
	Long: `Generate a Boson component such as controller, model, or middleware.
This command will create the necessary files for the specified component type.

Examples:
  # Generate a controller
  boson generate controller user

  # Generate a model
  boson generate model user

  # Generate middleware
  boson generate middleware auth`,
	Run: func(cmd *cobra.Command, args []string) {
		printLogo()

		headerColor := color.New(color.FgCyan, color.Bold)
		headerColor.Println("🔨 GENERATE A BOSON COMPONENT")
		fmt.Println()

		componentTypes := []string{"controller", "model", "middleware", "service", "router"}

		if len(args) < 1 {
			fmt.Println("🧩 " + color.New(color.Bold).Sprint("Component Type"))

			typeDescriptions := map[string]string{
				"controller": "Handle HTTP requests and responses",
				"model":      "Define data structures and business logic",
				"middleware": "Process requests before they reach controllers",
				"service":    "Implement business logic and operations",
				"router":     "Define routes and URL patterns",
			}

			for _, t := range componentTypes {
				fmt.Printf("  • %s - %s\n", color.CyanString(t), typeDescriptions[t])
			}
			fmt.Println()

			prompt := &survey.Select{
				Message: "  What type of component would you like to generate?",
				Options: componentTypes,
				Default: "controller",
			}
			survey.AskOne(prompt, &componentType)
			fmt.Println()
		} else {
			componentType = args[0]
		}

		if len(args) < 2 {
			fmt.Println("🔤 " + color.New(color.Bold).Sprint("Component Name"))
			prompt := &survey.Input{
				Message: fmt.Sprintf("  What's the name of your %s?", componentType),
				Default: "my_" + componentType,
				Help:    "Use snake_case for the name (e.g., user_auth, product_manager)",
			}
			survey.AskOne(prompt, &componentName)
			fmt.Println()
		} else {
			componentName = args[1]
		}

		componentName = strings.ReplaceAll(strings.ToLower(componentName), " ", "_")
		currentDir, _ := filepath.Abs(".")

		info := color.New(color.FgCyan).SprintFunc()
		headerColor.Println("🔧 GENERATING COMPONENT")
		fmt.Printf("  • Type: %s\n", info(componentType))
		fmt.Printf("  • Name: %s\n", info(componentName))
		fmt.Printf("  • Directory: %s\n", info(currentDir))
		fmt.Println()

		fmt.Printf("  %s Generating component files...", color.YellowString("⟳"))

		err := project.GenerateComponent(componentType, componentName, currentDir)
		if err != nil {
			fmt.Printf("\r  %s Failed to generate component\n", color.RedString("✗"))
			fmt.Println()
			color.Red("Error: %v", err)
			return
		}

		fmt.Printf("\r  %s Component files generated successfully\n", color.GreenString("✓"))
		fmt.Println()

		success := color.New(color.FgGreen, color.Bold).SprintFunc()
		fmt.Println(success("✨ COMPONENT GENERATED SUCCESSFULLY!"))

		fmt.Println()
		headerColor.Println("📁 COMPONENT LOCATION")

		var componentPath string
		switch componentType {
		case "controller":
			componentPath = filepath.Join(currentDir, "src", "controllers", componentName+"_controller.hpp")
		case "model":
			componentPath = filepath.Join(currentDir, "src", "models", componentName+".hpp")
		case "middleware":
			componentPath = filepath.Join(currentDir, "src", "middleware", componentName+"_middleware.hpp")
		case "service":
			componentPath = filepath.Join(currentDir, "src", "services", componentName+"_service.hpp")
		case "router":
			componentPath = filepath.Join(currentDir, "src", "routers", componentName+"_router.hpp")
		}

		cmdColor := color.New(color.FgCyan, color.Bold).SprintFunc()
		fmt.Printf("  • %s\n", cmdColor(componentPath))
		fmt.Println()
	},
}

var componentType string

var controllerCmd = &cobra.Command{
	Use:   "controller [name]",
	Short: "Generate a controller",
	Run: func(cmd *cobra.Command, args []string) {
		printLogo()

		headerColor := color.New(color.FgCyan, color.Bold)
		headerColor.Println("🔨 GENERATE A CONTROLLER")
		fmt.Println()

		if len(args) == 0 {
			fmt.Println("🔤 " + color.New(color.Bold).Sprint("Controller Name"))
			prompt := &survey.Input{
				Message: "  What's the name of your controller?",
				Default: "my_controller",
				Help:    "Use snake_case for the name (e.g., user, product, auth)",
			}
			survey.AskOne(prompt, &componentName)
			fmt.Println()
		} else {
			componentName = args[0]
		}

		componentName = strings.ReplaceAll(strings.ToLower(componentName), " ", "_")
		currentDir, _ := filepath.Abs(".")

		info := color.New(color.FgCyan).SprintFunc()
		headerColor.Println("🔧 GENERATING CONTROLLER")
		fmt.Printf("  • Name: %s\n", info(componentName))
		fmt.Printf("  • Directory: %s\n", info(currentDir))
		fmt.Println()

		fmt.Printf("  %s Generating controller files...", color.YellowString("⟳"))

		err := project.GenerateComponent("controller", componentName, currentDir)
		if err != nil {
			fmt.Printf("\r  %s Failed to generate controller\n", color.RedString("✗"))
			fmt.Println()
			color.Red("Error: %v", err)
			return
		}

		fmt.Printf("\r  %s Controller files generated successfully\n", color.GreenString("✓"))
		fmt.Println()

		success := color.New(color.FgGreen, color.Bold).SprintFunc()
		fmt.Println(success("✨ CONTROLLER GENERATED SUCCESSFULLY!"))

		fmt.Println()
		headerColor.Println("📁 CONTROLLER LOCATION")
		cmdColor := color.New(color.FgCyan, color.Bold).SprintFunc()
		fmt.Printf("  • %s\n", cmdColor(filepath.Join(currentDir, "src", "controllers", componentName+"_controller.hpp")))
		fmt.Println()
	},
}

var modelCmd = &cobra.Command{
	Use:   "model [name]",
	Short: "Generate a model",
	Run: func(cmd *cobra.Command, args []string) {
		printLogo()

		headerColor := color.New(color.FgCyan, color.Bold)
		headerColor.Println("🔨 GENERATE A MODEL")
		fmt.Println()

		if len(args) == 0 {
			fmt.Println("🔤 " + color.New(color.Bold).Sprint("Model Name"))
			prompt := &survey.Input{
				Message: "  What's the name of your model?",
				Default: "my_model",
				Help:    "Use snake_case for the name (e.g., user, product, order)",
			}
			survey.AskOne(prompt, &componentName)
			fmt.Println()
		} else {
			componentName = args[0]
		}

		componentName = strings.ReplaceAll(strings.ToLower(componentName), " ", "_")
		currentDir, _ := filepath.Abs(".")

		info := color.New(color.FgCyan).SprintFunc()
		headerColor.Println("🔧 GENERATING MODEL")
		fmt.Printf("  • Name: %s\n", info(componentName))
		fmt.Printf("  • Directory: %s\n", info(currentDir))
		fmt.Println()

		fmt.Printf("  %s Generating model files...", color.YellowString("⟳"))

		err := project.GenerateComponent("model", componentName, currentDir)
		if err != nil {
			fmt.Printf("\r  %s Failed to generate model\n", color.RedString("✗"))
			fmt.Println()
			color.Red("Error: %v", err)
			return
		}

		fmt.Printf("\r  %s Model files generated successfully\n", color.GreenString("✓"))
		fmt.Println()

		success := color.New(color.FgGreen, color.Bold).SprintFunc()
		fmt.Println(success("✨ MODEL GENERATED SUCCESSFULLY!"))

		fmt.Println()
		headerColor.Println("📁 MODEL LOCATION")
		cmdColor := color.New(color.FgCyan, color.Bold).SprintFunc()
		fmt.Printf("  • %s\n", cmdColor(filepath.Join(currentDir, "src", "models", componentName+".hpp")))
		fmt.Println()
	},
}

var middlewareCmd = &cobra.Command{
	Use:   "middleware [name]",
	Short: "Generate middleware",
	Run: func(cmd *cobra.Command, args []string) {
		printLogo()

		headerColor := color.New(color.FgCyan, color.Bold)
		headerColor.Println("🔨 GENERATE A MIDDLEWARE")
		fmt.Println()

		if len(args) == 0 {
			fmt.Println("🔤 " + color.New(color.Bold).Sprint("Middleware Name"))
			prompt := &survey.Input{
				Message: "  What's the name of your middleware?",
				Default: "my_middleware",
				Help:    "Use snake_case for the name (e.g., auth, logger, cors)",
			}
			survey.AskOne(prompt, &componentName)
			fmt.Println()
		} else {
			componentName = args[0]
		}

		componentName = strings.ReplaceAll(strings.ToLower(componentName), " ", "_")
		currentDir, _ := filepath.Abs(".")

		info := color.New(color.FgCyan).SprintFunc()
		headerColor.Println("🔧 GENERATING MIDDLEWARE")
		fmt.Printf("  • Name: %s\n", info(componentName))
		fmt.Printf("  • Directory: %s\n", info(currentDir))
		fmt.Println()

		fmt.Printf("  %s Generating middleware files...", color.YellowString("⟳"))

		err := project.GenerateComponent("middleware", componentName, currentDir)
		if err != nil {
			fmt.Printf("\r  %s Failed to generate middleware\n", color.RedString("✗"))
			fmt.Println()
			color.Red("Error: %v", err)
			return
		}

		fmt.Printf("\r  %s Middleware files generated successfully\n", color.GreenString("✓"))
		fmt.Println()

		success := color.New(color.FgGreen, color.Bold).SprintFunc()
		fmt.Println(success("✨ MIDDLEWARE GENERATED SUCCESSFULLY!"))

		fmt.Println()
		headerColor.Println("📁 MIDDLEWARE LOCATION")
		cmdColor := color.New(color.FgCyan, color.Bold).SprintFunc()
		fmt.Printf("  • %s\n", cmdColor(filepath.Join(currentDir, "src", "middleware", componentName+"_middleware.hpp")))
		fmt.Println()
	},
}

func init() {
	rootCmd.AddCommand(generateCmd)
	generateCmd.AddCommand(controllerCmd)
	generateCmd.AddCommand(modelCmd)
	generateCmd.AddCommand(middlewareCmd)

	serviceCmd := &cobra.Command{
		Use:   "service [name]",
		Short: "Generate a service",
		Run:   createComponentHandler("service", "SERVICE"),
	}

	routerCmd := &cobra.Command{
		Use:   "router [name]",
		Short: "Generate a router",
		Run:   createComponentHandler("router", "ROUTER"),
	}

	generateCmd.AddCommand(serviceCmd)
	generateCmd.AddCommand(routerCmd)
}

func createComponentHandler(componentType, displayType string) func(*cobra.Command, []string) {
	return func(cmd *cobra.Command, args []string) {
		printLogo()

		headerColor := color.New(color.FgCyan, color.Bold)
		headerColor.Printf("🔨 GENERATE A %s\n", displayType)
		fmt.Println()

		if len(args) == 0 {
			fmt.Println("🔤 " + color.New(color.Bold).Sprintf("%s Name", strings.Title(componentType)))
			prompt := &survey.Input{
				Message: fmt.Sprintf("  What's the name of your %s?", componentType),
				Default: fmt.Sprintf("my_%s", componentType),
				Help:    "Use snake_case for the name",
			}
			survey.AskOne(prompt, &componentName)
			fmt.Println()
		} else {
			componentName = args[0]
		}

		componentName = strings.ReplaceAll(strings.ToLower(componentName), " ", "_")
		currentDir, _ := filepath.Abs(".")

		info := color.New(color.FgCyan).SprintFunc()
		headerColor.Printf("🔧 GENERATING %s\n", displayType)
		fmt.Printf("  • Name: %s\n", info(componentName))
		fmt.Printf("  • Directory: %s\n", info(currentDir))
		fmt.Println()

		fmt.Printf("  %s Generating %s files...", color.YellowString("⟳"), componentType)

		err := project.GenerateComponent(componentType, componentName, currentDir)
		if err != nil {
			fmt.Printf("\r  %s Failed to generate %s\n", color.RedString("✗"), componentType)
			fmt.Println()
			color.Red("Error: %v", err)
			return
		}

		fmt.Printf("\r  %s %s files generated successfully\n",
			color.GreenString("✓"),
			strings.Title(componentType))
		fmt.Println()

		success := color.New(color.FgGreen, color.Bold).SprintFunc()
		fmt.Printf("%s\n", success(fmt.Sprintf("✨ %s GENERATED SUCCESSFULLY!", displayType)))

		fmt.Println()
		headerColor.Printf("📁 %s LOCATION\n", displayType)
		cmdColor := color.New(color.FgCyan, color.Bold).SprintFunc()

		var filePath string
		if componentType == "router" {
			filePath = filepath.Join(currentDir, "src", "routers", componentName+"_router.hpp")
		} else {
			filePath = filepath.Join(currentDir, "src", componentType+"s", componentName+"_"+componentType+".hpp")
		}

		fmt.Printf("  • %s\n", cmdColor(filePath))
		fmt.Println()
	}
}
