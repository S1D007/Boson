package cmd

import (
	"fmt"
	"os"
	"strings"

	"github.com/fatih/color"
	"github.com/spf13/cobra"
	"github.com/spf13/viper"
)

var (
	cfgFile string
	verbose bool
)

var rootCmd = &cobra.Command{
	Use:   "boson",
	Short: "Boson CLI - A modern CLI for the Boson Framework",
	Long: `Boson CLI is a comprehensive command line interface tool for creating and managing
Boson Framework projects. It provides commands for creating new projects, generating
components, and building applications with ease.

The CLI streamlines the development workflow for Boson applications and helps
ensure best practices are followed.`,

	Run: func(cmd *cobra.Command, args []string) {
		if len(args) == 0 {
			printLogo()
			fmt.Println(getFormattedCommands())
		}
	},
}

func Execute() error {
	cobra.AddTemplateFunc("StyleHeading", func(s string) string {
		return color.New(color.FgCyan, color.Bold).Sprint(s)
	})

	rootCmd.SetUsageTemplate(getCustomUsageTemplate())

	return rootCmd.Execute()
}

func init() {
	cobra.OnInitialize(initConfig)

	rootCmd.PersistentFlags().StringVar(&cfgFile, "config", "", "config file (default is $HOME/.boson.yaml)")
	rootCmd.PersistentFlags().BoolVarP(&verbose, "verbose", "v", false, "verbose output")
}

func initConfig() {
	if cfgFile != "" {
		viper.SetConfigFile(cfgFile)
	} else {
		home, err := os.UserHomeDir()
		cobra.CheckErr(err)

		viper.AddConfigPath(home)
		viper.SetConfigType("yaml")
		viper.SetConfigName(".boson")
	}

	viper.AutomaticEnv()

	if err := viper.ReadInConfig(); err == nil && verbose {
		fmt.Fprintln(os.Stderr, "Using config file:", viper.ConfigFileUsed())
	}
}

func printLogo() {
	bold := color.New(color.FgCyan, color.Bold).SprintFunc()
	logo := `
    ____                        
   / __ )____  _________  ____ 
  / __  / __ \/ ___/ __ \/ __ \
 / /_/ / /_/ (__  ) /_/ / / / /
/_____/\____/____/\____/_/ /_/ 
                               
`
	fmt.Println(bold(logo))
	fmt.Println(bold("High-Performance C++ Web Framework"))
	fmt.Println()
}

func getFormattedCommands() string {
	cmdColor := color.New(color.FgCyan).SprintFunc()
	descColor := color.New(color.FgWhite).SprintFunc()

	commands := []struct {
		name string
		desc string
	}{
		{"new", "Create a new Boson project"},
		{"generate", "Generate project components"},
		{"run", "Run the Boson project"},
		{"build", "Build the Boson project"},
		{"install", "Install dependencies"},
		{"version", "Display version information"},
	}

	var output strings.Builder
	output.WriteString(color.New(color.Bold).Sprint("AVAILABLE COMMANDS:\n\n"))

	for _, cmd := range commands {
		output.WriteString(fmt.Sprintf("  %s\t%s\n", cmdColor(cmd.name), descColor(cmd.desc)))
	}

	output.WriteString("\n")
	output.WriteString("Use \"boson [command] --help\" for more information about a command.\n")

	return output.String()
}

func getCustomUsageTemplate() string {
	return `{{ StyleHeading "Usage:" }}{{if .Runnable}}
  {{.UseLine}}{{end}}{{if .HasAvailableSubCommands}}
  {{.CommandPath}} [command]{{end}}{{if gt (len .Aliases) 0}}

{{ StyleHeading "Aliases:" }}
  {{.NameAndAliases}}{{end}}{{if .HasExample}}

{{ StyleHeading "Examples:" }}
{{.Example}}{{end}}{{if .HasAvailableSubCommands}}

{{ StyleHeading "Available Commands:" }}{{range .Commands}}{{if (or .IsAvailableCommand (eq .Name "help"))}}
  {{rpad .Name .NamePadding }} {{.Short}}{{end}}{{end}}{{end}}{{if .HasAvailableLocalFlags}}

{{ StyleHeading "Flags:" }}
{{.LocalFlags.FlagUsages | trimTrailingWhitespaces}}{{end}}{{if .HasAvailableInheritedFlags}}

{{ StyleHeading "Global Flags:" }}
{{.InheritedFlags.FlagUsages | trimTrailingWhitespaces}}{{end}}{{if .HasHelpSubCommands}}

{{ StyleHeading "Additional help topics:" }}{{range .Commands}}{{if .IsAdditionalHelpTopicCommand}}
  {{rpad .CommandPath .CommandPathPadding}} {{.Short}}{{end}}{{end}}{{end}}{{if .HasAvailableSubCommands}}

Use "{{.CommandPath}} [command] --help" for more information about a command.{{end}}
`
}
