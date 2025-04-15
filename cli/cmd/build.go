package cmd

import (
	"fmt"
	"os"

	"github.com/S1D007/boson/pkg/utils"
	"github.com/fatih/color"
	"github.com/spf13/cobra"
)

var (
	release      bool
	buildDir     string
	cmakeOptions []string
)

var buildCmd = &cobra.Command{
	Use:   "build",
	Short: "Build a Boson application",
	Long: `Build a Boson application.
This command will compile your Boson application.

Examples:
  # Build in debug mode
  boson build

  # Build in release mode
  boson build --release

  # Specify a custom build directory
  boson build --dir build-custom

  # Pass custom CMake options
  boson build --cmake-option="-DCUSTOM_OPTION=ON"`,
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
		fmt.Printf("%s Building Boson application...\n", info("ℹ"))

		buildType := "Debug"
		if release {
			buildType = "Release"
		}

		err = utils.BuildProjectWithOptions(currentDir, buildDir, buildType, cmakeOptions)
		if err != nil {
			color.Red("Error building project: %v", err)
			return
		}

		success := color.New(color.FgGreen, color.Bold).SprintFunc()
		fmt.Println(success("✓ Build completed successfully!"))

		executable := utils.FindExecutable(currentDir)
		if executable != "" {
			fmt.Printf("\nExecutable: %s\n", info(executable))
			fmt.Printf("\nRun your application with:\n  %s\n", info("boson run"))
		}
	},
}

func init() {
	rootCmd.AddCommand(buildCmd)

	buildCmd.Flags().BoolVarP(&release, "release", "r", false, "Build in release mode")
	buildCmd.Flags().StringVarP(&buildDir, "dir", "d", "build", "Directory to build in")
	buildCmd.Flags().StringArrayVar(&cmakeOptions, "cmake-option", []string{}, "Additional CMake options")
}
