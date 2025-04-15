package bosonfetch

import (
	"archive/tar"
	"archive/zip"
	"compress/gzip"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"net/http"
	"os"
	"os/user"
	"path/filepath"
	"runtime"
	"strings"
)

type ReleaseAsset struct {
	Name               string `json:"name"`
	BrowserDownloadURL string `json:"browser_download_url"`
}

type ReleaseInfo struct {
	TagName string         `json:"tag_name"`
	Assets  []ReleaseAsset `json:"assets"`
}

func getInstallDir() (string, error) {
	u, err := user.Current()
	if err != nil {
		return "", err
	}
	return filepath.Join(u.HomeDir, ".boson"), nil
}

func GetInstallDir() (string, error) {
	return getInstallDir()
}

func getPlatformAssetName() string {
	osName := runtime.GOOS
	if osName == "darwin" {
		return "boson-macos.tar.gz"
	} else if osName == "linux" {
		return "boson-linux.tar.gz"
	} else if osName == "windows" {
		return "boson-windows.tar.gz"
	}
	return ""
}

func fetchLatestRelease() (*ReleaseInfo, error) {
	resp, err := http.Get("https://api.github.com/repos/S1D007/boson/releases/latest")
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	var release ReleaseInfo
	if err := json.NewDecoder(resp.Body).Decode(&release); err != nil {
		return nil, err
	}
	return &release, nil
}

func downloadAsset(url, dest string) error {
	resp, err := http.Get(url)
	if err != nil {
		return err
	}
	defer resp.Body.Close()
	f, err := os.Create(dest)
	if err != nil {
		return err
	}
	defer f.Close()
	_, err = io.Copy(f, resp.Body)
	return err
}

func extractTarGz(src, dest string) error {
	f, err := os.Open(src)
	if err != nil {
		return err
	}
	defer f.Close()
	gz, err := gzip.NewReader(f)
	if err != nil {
		return err
	}
	tr := tar.NewReader(gz)
	for {
		hdr, err := tr.Next()
		if err == io.EOF {
			break
		}
		if err != nil {
			return err
		}
		path := filepath.Join(dest, hdr.Name)
		if hdr.FileInfo().IsDir() {
			os.MkdirAll(path, 0755)
			continue
		}
		os.MkdirAll(filepath.Dir(path), 0755)
		out, err := os.Create(path)
		if err != nil {
			return err
		}
		if _, err := io.Copy(out, tr); err != nil {
			out.Close()
			return err
		}
		out.Close()
	}
	return nil
}

func InstallOrUpdateBoson(force bool) error {
	installDir, err := getInstallDir()
	if err != nil {
		return err
	}
	os.MkdirAll(installDir, 0755)

	assetName := getPlatformAssetName()
	if assetName == "" {
		return errors.New("unsupported platform")
	}
	release, err := fetchLatestRelease()
	if err != nil {
		return err
	}
	var assetURL string
	for _, asset := range release.Assets {
		if asset.Name == assetName {
			assetURL = asset.BrowserDownloadURL
			break
		}
	}
	if assetURL == "" {
		return fmt.Errorf("could not find asset %s in latest release", assetName)
	}
	tmpFile := filepath.Join(os.TempDir(), assetName)
	if err := downloadAsset(assetURL, tmpFile); err != nil {
		return err
	}
	if strings.HasSuffix(assetName, ".tar.gz") {
		if err := extractTarGz(tmpFile, installDir); err != nil {
			return err
		}
	} else if strings.HasSuffix(assetName, ".zip") {
		if err := extractZip(tmpFile, installDir); err != nil {
			return err
		}
	} else {
		return errors.New("unknown asset archive format")
	}
	return nil
}

func extractZip(src, dest string) error {
	zr, err := zip.OpenReader(src)
	if err != nil {
		return err
	}
	defer zr.Close()
	for _, f := range zr.File {
		path := filepath.Join(dest, f.Name)
		if f.FileInfo().IsDir() {
			os.MkdirAll(path, 0755)
			continue
		}
		os.MkdirAll(filepath.Dir(path), 0755)
		out, err := os.Create(path)
		if err != nil {
			return err
		}
		in, err := f.Open()
		if err != nil {
			out.Close()
			return err
		}
		if _, err := io.Copy(out, in); err != nil {
			in.Close()
			out.Close()
			return err
		}
		in.Close()
		out.Close()
	}
	return nil
}
