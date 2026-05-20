# share_client.ps1
# Automates packaging the compiled game client into a clean archive for testers.

$ErrorActionPreference = "Stop"

# 1. Define paths
$RootDir = Get-Item $PSScriptRoot
$ExportDir = Join-Path $RootDir.FullName "client_exports"
$Timestamp = Get-Date -Format "yyyyMMdd_HHmmss"

$RarName = "MuClient_Release_$Timestamp.rar"
$RarPath = Join-Path $ExportDir $RarName

$ZipName = "MuClient_Release_$Timestamp.zip"
$ZipPath = Join-Path $ExportDir $ZipName

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "   MU Online Client Exporter for Testers  " -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan

# 2. Find the Release build folder
$SearchPaths = @(
    "out/build/windows-x86/src/Release",
    "out/build/windows-x64/src/Release"
)

$SourceDir = $null
foreach ($path in $SearchPaths) {
    $fullPath = Join-Path $RootDir.FullName $path
    if (Test-Path $fullPath) {
        $SourceDir = $fullPath
        break
    }
}

if ($null -eq $SourceDir) {
    Write-Error "Could not find a compiled Release build. Please compile in Release mode first."
    exit 1
}

Write-Host "Found Release build source at: $SourceDir" -ForegroundColor Green

# 3. Create clean output directory & temporary staging folder
if (!(Test-Path $ExportDir)) {
    New-Item -ItemType Directory -Path $ExportDir | Out-Null
    Write-Host "Created export directory: $ExportDir" -ForegroundColor Yellow
}

$StagingDir = Join-Path $ExportDir "temp_client_staging"
if (Test-Path $StagingDir) {
    Remove-Item -Recurse -Force $StagingDir | Out-Null
}
New-Item -ItemType Directory -Path $StagingDir | Out-Null

Write-Host "Staging files for compression..." -ForegroundColor Yellow

# 4. Copy required files and folders
$FilesToCopy = @(
    "Main.exe",
    "MUnique.Client.Library.dll",
    "glew32.dll",
    "ogg.dll",
    "vorbisfile.dll"
)

$FoldersToCopy = @(
    "Data",
    "Translations"
)

foreach ($file in $FilesToCopy) {
    $src = Join-Path $SourceDir $file
    if (Test-Path $src) {
        Copy-Item -Path $src -Destination $StagingDir | Out-Null
    } else {
        Write-Warning "Required file not found in build output: $file"
    }
}

foreach ($folder in $FoldersToCopy) {
    $src = Join-Path $SourceDir $folder
    if (Test-Path $src) {
        Copy-Item -Path $src -Destination $StagingDir -Recurse -Force | Out-Null
    } else {
        Write-Warning "Required folder not found in build output: $folder"
    }
}

# 5. Generate a clean config.ini without credentials
$SourceConfig = Join-Path $SourceDir "config.ini"
$DestConfig = Join-Path $StagingDir "config.ini"

if (Test-Path $SourceConfig) {
    Write-Host "Cleaning credentials from config.ini..." -ForegroundColor Yellow
    $configLines = Get-Content $SourceConfig
    $cleanLines = @()
    
    foreach ($line in $configLines) {
        $trimmed = $line.Trim()
        if ($trimmed -like "EncryptedUsername=*") {
            continue
        }
        if ($trimmed -like "EncryptedPassword=*") {
            continue
        }
        if ($trimmed -like "RememberMe=*") {
            $cleanLines += "RememberMe=0"
        } else {
            $cleanLines += $line
        }
    }
    
    [System.IO.File]::WriteAllLines($DestConfig, $cleanLines)
    Write-Host "Cleaned config.ini written to staging." -ForegroundColor Green
} else {
    Write-Warning "config.ini not found. Testers will need to create their own configuration."
}

# 6. Create the Archive
$RarExe = "C:\Program Files\WinRAR\Rar.exe"
$OutputFilePath = $null

if (Test-Path $RarExe) {
    Write-Host "Compressing files into $RarName using WinRAR..." -ForegroundColor Cyan
    if (Test-Path $RarPath) {
        Remove-Item $RarPath -Force
    }
    
    # Run WinRAR command-line to create the RAR archive
    & $RarExe a -ep1 -r $RarPath "$StagingDir\*"
    $OutputFilePath = $RarPath
} else {
    Write-Host "WinRAR not found at $RarExe. Falling back to native Windows tar..." -ForegroundColor Cyan
    if (Test-Path $ZipPath) {
        Remove-Item $ZipPath -Force
    }
    
    # Change location into the staging directory and use native tar to zip
    Push-Location $StagingDir
    & tar -a -c -f $ZipPath *
    Pop-Location
    $OutputFilePath = $ZipPath
}

# 7. Clean up staging folder
Write-Host "Cleaning up staging files..." -ForegroundColor Yellow
Remove-Item -Recurse -Force $StagingDir | Out-Null

Write-Host "==========================================" -ForegroundColor Green
Write-Host "   EXPORT COMPLETE!                       " -ForegroundColor Green
Write-Host "   Archive file created at:               " -ForegroundColor Green
Write-Host "   $OutputFilePath" -ForegroundColor White
Write-Host "==========================================" -ForegroundColor Green
