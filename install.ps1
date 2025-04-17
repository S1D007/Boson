$Repo = "S1D007/boson"
$CliName = "boson.exe"
$InstallDir = "$env:USERPROFILE\.boson"

# Create install directory if it doesn't exist
if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Path $InstallDir | Out-Null
}

Write-Host "Fetching latest release info..." -ForegroundColor Cyan
$headers = @{
    "Accept" = "application/json"
}
$latestRelease = Invoke-RestMethod -Uri "https://api.github.com/repos/$Repo/releases/latest" -Headers $headers
$tag = $latestRelease.tag_name

$asset = "boson-windows.tar.gz"
$assetUrl = "https://github.com/$Repo/releases/download/$tag/$asset"
$tempDir = Join-Path ([System.IO.Path]::GetTempPath()) ([System.Guid]::NewGuid().ToString())
New-Item -ItemType Directory -Path $tempDir | Out-Null

Write-Host "Downloading Boson CLI version $tag..." -ForegroundColor Cyan
$downloadPath = Join-Path $tempDir $asset
Invoke-WebRequest -Uri $assetUrl -OutFile $downloadPath

Write-Host "Extracting files..." -ForegroundColor Cyan
tar -xzf $downloadPath -C $tempDir

Write-Host "Installing Boson CLI..." -ForegroundColor Cyan
Copy-Item -Path (Join-Path $tempDir $CliName) -Destination (Join-Path $InstallDir $CliName) -Force

if (Test-Path (Join-Path $tempDir "templates")) {
    # Remove existing templates directory completely
    if (Test-Path (Join-Path $InstallDir "templates")) {
        Remove-Item -Path (Join-Path $InstallDir "templates") -Recurse -Force
    }
    
    # Copy templates directory with all subdirectories and files
    Copy-Item -Path (Join-Path $tempDir "templates") -Destination $InstallDir -Recurse -Force
    Write-Host "Templates installed successfully to $InstallDir\templates" -ForegroundColor Green
} else {
    Write-Host "Warning: Templates directory not found in the package" -ForegroundColor Yellow
}

# Clean up
Remove-Item -Path $tempDir -Recurse -Force

# Add to PATH if not already present
$currentPath = [Environment]::GetEnvironmentVariable("Path", "User")
if (-not $currentPath.Contains($InstallDir)) {
    Write-Host "Adding Boson CLI to your PATH..." -ForegroundColor Cyan
    [Environment]::SetEnvironmentVariable("Path", "$currentPath;$InstallDir", "User")
    $env:Path = "$env:Path;$InstallDir"
    Write-Host "Boson CLI added to PATH. Note that you may need to restart your terminal for this change to take effect." -ForegroundColor Yellow
}

Write-Host "Boson CLI installed successfully!" -ForegroundColor Green
Write-Host "Run 'boson --help' to get started." -ForegroundColor Green
