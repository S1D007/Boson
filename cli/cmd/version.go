package cmd

import (
	"fmt"

	"github.com/fatih/color"
	"github.com/spf13/cobra"
)

var (
	Version   = "0.1.0"
	BuildDate = "2025-04-15"
	GitCommit = "development"
)

var versionCmd = &cobra.Command{
	Use:   "version",
	Short: "Print version information",
	Long:  `Print detailed version information about the Boson CLI.`,
	Run: func(cmd *cobra.Command, args []string) {
		info := color.New(color.FgCyan).SprintFunc()
		fmt.Println("Boson CLI Version Information:")
		fmt.Println("----------------------------")
		fmt.Printf("Version:    %s\n", info(Version))
		fmt.Printf("Built:      %s\n", info(BuildDate))
		fmt.Printf("Git commit: %s\n", info(GitCommit))
		fmt.Println("----------------------------")
		fmt.Println("Boson Framework CLI is an open source project")
		fmt.Println("https://github.com/S1D007/boson")
	},
}

func init() {
	rootCmd.AddCommand(versionCmd)
}
