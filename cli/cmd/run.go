package cmd

import (
	"fmt"
	"os"
	"os/exec"
	"os/signal"
	"syscall"
	"time"

	"github.com/S1D007/boson/pkg/utils"
	"github.com/fatih/color"
	"github.com/spf13/cobra"
)

var (
	watchMode  bool
	buildFirst bool
	port       int
	host       string
)

var runCmd = &cobra.Command{
	Use:   "run",
	Short: "Run a Boson application",
	Long: `Run a Boson application in development mode.
This command will build (if necessary) and run your Boson application.

Examples:
  # Run the application
  boson run

  # Run the application with hot reloading
  boson run --watch

  # Run the application on a specific port
  boson run --port 8080`,
	Run: func(cmd *cobra.Command, args []string) {
		printLogo()

		headerColor := color.New(color.FgCyan, color.Bold)
		headerColor.Println("🚀 RUNNING BOSON APPLICATION")
		fmt.Println()

		currentDir, err := os.Getwd()
		if err != nil {
			fmt.Printf("  %s Could not get current directory\n", color.RedString("✗"))
			fmt.Println()
			color.Red("Error: %v", err)
			return
		}

		if !utils.IsBosonProject(currentDir) {
			fmt.Printf("  %s Not in a Boson project directory\n", color.RedString("✗"))
			fmt.Println()
			color.Red("Error: This command must be run from a Boson project directory")
			fmt.Println("Hint: Make sure you're in a directory with a CMakeLists.txt file for a Boson project")
			return
		}

		info := color.New(color.FgCyan).SprintFunc()
		headerColor.Println("⚙️  CONFIGURATION")
		fmt.Printf("  • Host: %s\n", info(host))
		fmt.Printf("  • Port: %s\n", info(port))
		fmt.Printf("  • Watch mode: %s\n", info(watchMode))
		fmt.Printf("  • Directory: %s\n", info(currentDir))
		fmt.Println()

		// Build the project if needed
		if buildFirst || !utils.ExecutableExists(currentDir) {
			headerColor.Println("🔨 BUILDING APPLICATION")
			fmt.Printf("  %s Building project...", color.YellowString("⟳"))

			startTime := time.Now()
			err := utils.BuildProject(currentDir)
			if err != nil {
				fmt.Printf("\r  %s Failed to build project\n", color.RedString("✗"))
				fmt.Println()
				color.Red("Error: %v", err)
				return
			}

			duration := time.Since(startTime).Seconds()
			fmt.Printf("\r  %s Project built successfully (%.2fs)\n", color.GreenString("✓"), duration)
			fmt.Println()
		}

		executable := utils.FindExecutable(currentDir)
		if executable == "" {
			fmt.Printf("  %s Could not find executable\n", color.RedString("✗"))
			fmt.Println()
			color.Red("Error: No executable was found after building")
			fmt.Println("Hint: Check the build logs for compilation errors")
			return
		}

		headerColor.Println("▶️  STARTING APPLICATION")
		if watchMode {
			fmt.Println()
			headerColor.Println("👀 WATCH MODE ENABLED")
			fmt.Println("  File changes will automatically rebuild and restart the application")
			watchAndReload(currentDir, executable, port, host, headerColor)
			return
		}

		fmt.Printf("  %s Launching executable...", color.YellowString("⟳"))
		execCmd := exec.Command(executable, fmt.Sprintf("--port=%d", port), fmt.Sprintf("--host=%s", host))
		execCmd.Stdout = os.Stdout
		execCmd.Stderr = os.Stderr
		if err := execCmd.Start(); err != nil {
			fmt.Printf("\r  %s Failed to start application\n", color.RedString("✗"))
			fmt.Println()
			color.Red("Error: %v", err)
			return
		}
		fmt.Printf("\r  %s Application started successfully\n", color.GreenString("✓"))
		fmt.Println()
		success := color.New(color.FgGreen, color.Bold).SprintFunc()
		urlColor := color.New(color.Bold).SprintFunc()
		fmt.Println(success("💻 APPLICATION IS RUNNING"))
		fmt.Printf("  🔗 URL: %s\n", urlColor(fmt.Sprintf("http://%s:%d", host, port)))
		fmt.Println()
		fmt.Println("  Press Ctrl+C to stop the application")
		fmt.Println()
		if err := execCmd.Wait(); err != nil {
			if err.Error() != "signal: interrupt" {
				fmt.Printf("\n  %s Application terminated unexpectedly\n", color.RedString("✗"))
				fmt.Println()
				color.Red("Error: %v", err)
				return
			} else {
				fmt.Printf("\n  %s Application terminated\n", color.YellowString("⚠️"))
			}
		}
	},
}

func init() {
	rootCmd.AddCommand(runCmd)

	runCmd.Flags().BoolVarP(&watchMode, "watch", "w", false, "Watch for file changes and automatically rebuild")
	runCmd.Flags().BoolVarP(&buildFirst, "build", "b", true, "Build the application before running")
	runCmd.Flags().IntVarP(&port, "port", "p", 3000, "Port to run the application on")
	runCmd.Flags().StringVarP(&host, "host", "H", "127.0.0.1", "Host to run the application on")
}

// Add file watcher and hot reload logic
func watchAndReload(projectDir, executable string, port int, host string, headerColor *color.Color) {
	watchDirs := []string{"src", "include"}
	var lastCmd *exec.Cmd
	var watcher *utils.FileWatcher
	var err error

	watcher, err = utils.NewFileWatcher(projectDir, watchDirs)
	if err != nil {
		headerColor.Println("Failed to start file watcher:", err)
		return
	}
	defer watcher.Close()

	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)

	gracefulStop := func() {
		if lastCmd != nil && lastCmd.Process != nil {
			headerColor.Println("🛑 Stopping application...")

			if err := lastCmd.Process.Signal(syscall.SIGTERM); err != nil {
				headerColor.Printf("Could not send SIGTERM: %v\n", err)
				if err := lastCmd.Process.Kill(); err != nil {
					headerColor.Printf("Could not kill process: %v\n", err)
				}
			}

			done := make(chan error, 1)
			go func() {
				done <- lastCmd.Wait()
			}()

			select {
			case <-done:
			case <-time.After(2 * time.Second):
				headerColor.Println("Process didn't terminate in time, forcing kill...")
				if err := lastCmd.Process.Kill(); err != nil {
					headerColor.Printf("Could not force kill: %v\n", err)
				}
			}

			time.Sleep(500 * time.Millisecond)
		}
	}

	restart := func() {
		// Stop any running instance
		gracefulStop()

		headerColor.Println("🔨 Rebuilding application due to file change...")
		if err := utils.BuildProject(projectDir); err != nil {
			headerColor.Println("Build failed:", err)
			return
		}

		time.Sleep(500 * time.Millisecond)

		// Start the new instance
		cmd := exec.Command(executable, fmt.Sprintf("--port=%d", port), fmt.Sprintf("--host=%s", host))
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		if err := cmd.Start(); err != nil {
			headerColor.Println("Failed to start application:", err)
			return
		}
		lastCmd = cmd
	}

	restart()

	go func() {
		<-sigChan
		headerColor.Println("\n🛑 Terminating watch mode...")
		gracefulStop()
		os.Exit(0)
	}()

	for {
		changed := watcher.WaitForChange()
		if changed {
			restart()
		}
	}
}
