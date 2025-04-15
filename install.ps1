$repo = "S1D007/boson"
$cliName = "boson.exe"
$installDir = "$env:USERPROFILE\.boson\bin"
$asset = "boson-windows.zip"

$tag = (Invoke-RestMethod "https://api.github.com/repos/$repo/releases/latest").tag_name

New-Item -ItemType Directory -Force -Path $installDir | Out-Null

$tmp = New-TemporaryFile
$zipPath = "$($tmp.FullName).zip"
Invoke-WebRequest -Uri "https://github.com/$repo/releases/download/$tag/$asset" -OutFile $zipPath
Expand-Archive -Path $zipPath -DestinationPath $installDir -Force
Remove-Item $zipPath

if (-not ($env:PATH -like "*$installDir*")) {
    [Environment]::SetEnvironmentVariable("PATH", "$env:PATH;$installDir", [EnvironmentVariableTarget]::User)
    Write-Host "Added $installDir to PATH. Please restart your terminal."
}

Write-Host "Boson CLI installed successfully!"
Write-Host "Run 'boson --help' to get started."