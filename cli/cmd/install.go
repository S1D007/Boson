package cmd

import (
	"fmt"

	"github.com/S1D007/boson/pkg/bosonfetch"
	"github.com/fatih/color"
	"github.com/spf13/cobra"
)

var (
	forceInstall bool
)

var installCmd = &cobra.Command{
	Use:   "install",
	Short: "Install Boson framework and dependencies",
	Long: `Install the Boson framework and all necessary dependencies for your platform.
This command will automatically download and install the latest Boson framework release
for your OS and architecture, making setup seamless for you.

Examples:
  boson install
  boson install --force`,
	Run: func(cmd *cobra.Command, args []string) {
		info := color.New(color.FgCyan).SprintFunc()
		fmt.Printf("%s Installing Boson framework for your platform...\n", info("ℹ"))

		err := bosonfetch.InstallOrUpdateBoson(forceInstall)
		if err != nil {
			color.Red("Error installing Boson framework: %v", err)
			return
		}

		success := color.New(color.FgGreen, color.Bold).SprintFunc()
		fmt.Println(success("✓ Boson framework installed successfully!"))
		fmt.Printf("\nNext steps:\n")
		fmt.Printf("  %s\n", info("boson new <project>"))
		fmt.Printf("  %s\n", info("boson run"))
	},
}

func init() {
	rootCmd.AddCommand(installCmd)
	installCmd.Flags().BoolVarP(&forceInstall, "force", "f", false, "Force reinstallation of Boson framework")
}
