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
    $currentPath = [Environment]::GetEnvironmentVariable("PATH", [EnvironmentVariableTarget]::User)
    if (-not ($currentPath -like "*$installDir*")) {
        [Environment]::SetEnvironmentVariable("PATH", "$currentPath;$installDir", [EnvironmentVariableTarget]::User)
    }
}

$profilePath = $PROFILE
if (-not (Test-Path $profilePath)) {
    New-Item -ItemType File -Path $profilePath -Force | Out-Null
}

$autoUpdateSnippet = @"
# Boson CLI auto-update
if (Get-Command boson -ErrorAction SilentlyContinue) {
    boson update | Out-Null
}
"@

$profileContent = Get-Content $profilePath -Raw

if (-not ($profileContent -like "*Boson CLI auto-update*")) {
    Add-Content -Path $profilePath -Value $autoUpdateSnippet
}

Write-Host "`n✅ Boson CLI installed successfully!"
Write-Host "Run 'boson --help' to get started."
Write-Host "➡️  PATH updated. Restart your terminal for changes to take effect."
