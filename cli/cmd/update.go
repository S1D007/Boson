package cmd

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"

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

		// --- CLI self-update logic ---
		fmt.Printf("%s Checking for CLI updates...\n", info("ℹ"))
		release, err := bosonfetch.FetchLatestReleaseInfo()
		if err != nil {
			color.Red("Error fetching CLI release info: %v", err)
			return
		}
		arch := runtime.GOARCH
		var cliAssetName string
		if arch == "arm64" {
			cliAssetName = "boson-darwin-arm64"
		} else {
			cliAssetName = "boson-darwin-amd64"
		}
		var cliAssetURL string
		for _, asset := range release.Assets {
			if asset.Name == cliAssetName {
				cliAssetURL = asset.BrowserDownloadURL
				break
			}
		}
		if cliAssetURL == "" {
			color.Red("Could not find CLI binary for your platform in the latest release.")
			return
		}
		tmpPath := filepath.Join(os.TempDir(), cliAssetName)
		err = bosonfetch.DownloadAsset(cliAssetURL, tmpPath)
		if err != nil {
			color.Red("Failed to download latest CLI binary: %v", err)
			return
		}
		_ = os.Chmod(tmpPath, 0755)
		execPath, err := os.Executable()
		if err != nil {
			color.Red("Could not determine current CLI path: %v", err)
			return
		}
		err = os.Rename(tmpPath, execPath)
		if err != nil {
			color.Red("Failed to replace CLI binary: %v", err)
			return
		}
		fmt.Println(color.New(color.FgGreen, color.Bold).Sprint("✓ Boson CLI updated to the latest version!"))
	},
}

func init() {
	rootCmd.AddCommand(updateCmd)
}
