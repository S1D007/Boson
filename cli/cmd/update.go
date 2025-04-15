package cmd

import (
	"fmt"

	"github.com/S1D007/boson/pkg/bosonfetch"
	"github.com/fatih/color"
	"github.com/spf13/cobra"
)

var updateCmd = &cobra.Command{
	Use:   "update",
	Short: "Update Boson framework to the latest version",
	Long: `Download and install the latest Boson framework release for your platform.
This command will always fetch and install the newest version, replacing any existing installation.`,
	Run: func(cmd *cobra.Command, args []string) {
		info := color.New(color.FgCyan).SprintFunc()
		fmt.Printf("%s Updating Boson framework to the latest version...\n", info("ℹ"))
		err := bosonfetch.InstallOrUpdateBoson(true)
		if err != nil {
			color.Red("Error updating Boson framework: %v", err)
			return
		}
		fmt.Println(color.New(color.FgGreen, color.Bold).Sprint("✓ Boson framework updated successfully!"))
	},
}

func init() {
	rootCmd.AddCommand(updateCmd)
}
