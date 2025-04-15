package cmd

import (
	"fmt"
	"os"

	"github.com/S1D007/boson/pkg/utils"
	"github.com/fatih/color"
	"github.com/spf13/cobra"
)

var (
	forceInstall bool
)

var installCmd = &cobra.Command{
	Use:   "install",
	Short: "Install Boson dependencies",
	Long: `Install Boson framework and project dependencies.
This command will install all necessary dependencies for a Boson project,
including the Boson framework library and any other required libraries.

Examples:
  # Install dependencies for the current project
  boson install

  # Force reinstallation of dependencies
  boson install --force`,
	Run: func(cmd *cobra.Command, args []string) {

		currentDir, err := os.Getwd()
		if err != nil {
			color.Red("Error: Could not get current directory: %v", err)
			return
		}

		if !utils.IsBosonProject(currentDir) {
			color.Red("Error: Not in a Boson project directory")
			return
		}

		info := color.New(color.FgCyan).SprintFunc()
		fmt.Printf("%s Installing Boson dependencies...\n", info("ℹ"))

		err = utils.InstallDependencies(currentDir, forceInstall)
		if err != nil {
			color.Red("Error installing dependencies: %v", err)
			return
		}

		success := color.New(color.FgGreen, color.Bold).SprintFunc()
		fmt.Println(success("✓ Dependencies installed successfully!"))

		fmt.Printf("\nNext steps:\n")
		fmt.Printf("  %s\n", info("boson build"))
		fmt.Printf("  %s\n", info("boson run"))
	},
}

func init() {
	rootCmd.AddCommand(installCmd)
	installCmd.Flags().BoolVarP(&forceInstall, "force", "f", false, "Force reinstallation of dependencies")
}
