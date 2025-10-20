#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Deploy Broadsword Framework to the game directory for testing.

.DESCRIPTION
    Copies the built Broadsword DLLs to the Half Sword game directory.

.PARAMETER GamePath
    Path to the Half Sword game binaries directory.
    Default: C:\Program Files (x86)\Steam\steamapps\common\Half Sword Demo\HalfSwordUE5\Binaries\Win64

.PARAMETER Configuration
    Build configuration to deploy (Debug or Release).
    Default: Debug

.PARAMETER BackupExisting
    Create backups of existing files before overwriting.

.EXAMPLE
    .\deploy.ps1
    Deploy Debug build to default Steam location

.EXAMPLE
    .\deploy.ps1 -Configuration Release
    Deploy Release build to default location

.EXAMPLE
    .\deploy.ps1 -GamePath "D:\Games\Half Sword\HalfSwordUE5\Binaries\Win64"
    Deploy to custom game installation
#>

param(
    [Parameter(Mandatory=$false)]
    [string]$GamePath = "C:\Program Files (x86)\Steam\steamapps\common\Half Sword Demo\HalfSwordUE5\Binaries\Win64",

    [Parameter(Mandatory=$false)]
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",

    [Parameter(Mandatory=$false)]
    [switch]$BackupExisting
)

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Broadsword Framework Deployment" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Get script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# Build output paths
$BinDir = Join-Path $ScriptDir "build\bin\$Configuration"
$ProxyDll = Join-Path $BinDir "dwmapi.dll"
$FrameworkDll = Join-Path $BinDir "Broadsword.dll"

# Verify game path exists
if (-not (Test-Path $GamePath)) {
    Write-Host "Error: Game path does not exist: $GamePath" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please specify the correct path using -GamePath parameter:" -ForegroundColor Yellow
    Write-Host "  .\deploy.ps1 -GamePath ""C:\Your\Game\Path\HalfSwordUE5\Binaries\Win64""" -ForegroundColor Yellow
    exit 1
}

# Verify built files exist
if (-not (Test-Path $ProxyDll)) {
    Write-Host "Error: Proxy DLL not found: $ProxyDll" -ForegroundColor Red
    Write-Host "Please build the framework first using:" -ForegroundColor Yellow
    Write-Host "  cmake --build build --config $Configuration" -ForegroundColor Yellow
    exit 1
}

if (-not (Test-Path $FrameworkDll)) {
    Write-Host "Error: Framework DLL not found: $FrameworkDll" -ForegroundColor Red
    Write-Host "Please build the framework first using:" -ForegroundColor Yellow
    Write-Host "  cmake --build build --config $Configuration" -ForegroundColor Yellow
    exit 1
}

Write-Host "Configuration: $Configuration" -ForegroundColor White
Write-Host "Game Path:     $GamePath" -ForegroundColor White
Write-Host ""

# Function to get file hash (first 8 characters for display)
function Get-ShortHash {
    param([string]$FilePath)

    if (-not (Test-Path $FilePath)) {
        return $null
    }

    try {
        $hash = Get-FileHash -Path $FilePath -Algorithm SHA256
        return $hash.Hash.Substring(0, 8)
    } catch {
        return "ERROR"
    }
}

# Function to backup a file
function Backup-File {
    param([string]$FilePath)

    if (Test-Path $FilePath) {
        $BackupPath = "$FilePath.backup"
        $BackupIndex = 1

        # Find next available backup name
        while (Test-Path $BackupPath) {
            $BackupPath = "$FilePath.backup.$BackupIndex"
            $BackupIndex++
        }

        Copy-Item $FilePath $BackupPath -Force
        Write-Host "  Backed up: $(Split-Path -Leaf $BackupPath)" -ForegroundColor DarkGray
    }
}

# Function to deploy a file with hash comparison
function Deploy-File {
    param(
        [string]$SourcePath,
        [string]$DestPath,
        [string]$DisplayName,
        [bool]$DoBackup
    )

    $fileName = if ($DisplayName) { $DisplayName } else { Split-Path -Leaf $SourcePath }
    $oldHash = Get-ShortHash $DestPath
    $newHash = Get-ShortHash $SourcePath

    if ($null -eq $oldHash) {
        # NEW FILE
        Write-Host "  [NEW]     $fileName" -ForegroundColor Green
        Write-Host "            Hash: $newHash" -ForegroundColor DarkGray
    } else {
        # REPLACING FILE
        if ($oldHash -eq $newHash) {
            Write-Host "  [NO CHG]  $fileName" -ForegroundColor DarkYellow
            Write-Host "            Hash: $newHash (unchanged)" -ForegroundColor DarkGray
        } else {
            Write-Host "  [REPLACE] $fileName" -ForegroundColor Yellow
            Write-Host "            Before: $oldHash" -ForegroundColor DarkGray
            Write-Host "            After:  $newHash" -ForegroundColor DarkGray
        }

        if ($DoBackup) {
            Backup-File $DestPath
        }
    }

    try {
        Copy-Item $SourcePath $DestPath -Force
        return $true
    } catch {
        Write-Host "            ERROR: $_" -ForegroundColor Red
        return $false
    }
}

# Deploy Framework DLLs
Write-Host "Deploying Broadsword Framework..." -ForegroundColor Green
Write-Host ""

$ProxyDest = Join-Path $GamePath "dwmapi.dll"
$FrameworkDest = Join-Path $GamePath "Broadsword.dll"

$proxySuccess = Deploy-File -SourcePath $ProxyDll -DestPath $ProxyDest -DisplayName "dwmapi.dll" -DoBackup $BackupExisting
$frameworkSuccess = Deploy-File -SourcePath $FrameworkDll -DestPath $FrameworkDest -DisplayName "Broadsword.dll" -DoBackup $BackupExisting

# Deploy Mods folder
Write-Host ""
Write-Host "Deploying Mods..." -ForegroundColor Green
Write-Host ""

$ModsSourceDir = Join-Path $BinDir "Mods"
$ModsDestDir = Join-Path $GamePath "Mods"

if (Test-Path $ModsSourceDir) {
    # Create Mods directory if it doesn't exist
    if (-not (Test-Path $ModsDestDir)) {
        New-Item -ItemType Directory -Path $ModsDestDir -Force | Out-Null
        Write-Host "  [CREATE]  Mods directory" -ForegroundColor Green
    }

    # Deploy each mod DLL
    $modDlls = Get-ChildItem -Path $ModsSourceDir -Filter "*.dll"
    foreach ($modDll in $modDlls) {
        $modDest = Join-Path $ModsDestDir $modDll.Name
        Deploy-File -SourcePath $modDll.FullName -DestPath $modDest -DisplayName $modDll.Name -DoBackup $BackupExisting | Out-Null
    }

    if ($modDlls.Count -eq 0) {
        Write-Host "  [INFO]    No mod DLLs found" -ForegroundColor DarkYellow
    } else {
        Write-Host "  Deployed $($modDlls.Count) mod(s)" -ForegroundColor Cyan
    }
} else {
    Write-Host "  [WARN]    Mods directory not found: $ModsSourceDir" -ForegroundColor Yellow
}

# Deploy vcpkg runtime dependencies
Write-Host ""
Write-Host "Deploying Dependencies..." -ForegroundColor Green
Write-Host ""

$VcpkgBinDir = Join-Path $ScriptDir "build\vcpkg_installed\x64-windows\debug\bin"

# MinHook DLL
$MinHookDllName = "minhook.x64d.dll"
$MinHookDll = Join-Path $VcpkgBinDir $MinHookDllName
$MinHookDest = Join-Path $GamePath $MinHookDllName

$minHookSuccess = $true
if (Test-Path $MinHookDll) {
    $minHookSuccess = Deploy-File -SourcePath $MinHookDll -DestPath $MinHookDest -DisplayName $MinHookDllName -DoBackup $BackupExisting
} else {
    Write-Host "  [WARN]    $MinHookDllName not found in vcpkg" -ForegroundColor Yellow
}

# fmt DLL (needed by Logger)
$FmtDllName = "fmtd.dll"
$FmtDll = Join-Path $VcpkgBinDir $FmtDllName
$FmtDest = Join-Path $GamePath $FmtDllName

$fmtSuccess = $true
if (Test-Path $FmtDll) {
    $fmtSuccess = Deploy-File -SourcePath $FmtDll -DestPath $FmtDest -DisplayName $FmtDllName -DoBackup $BackupExisting
} else {
    Write-Host "  [WARN]    $FmtDllName not found in vcpkg" -ForegroundColor Yellow
}

if (-not ($proxySuccess -and $frameworkSuccess -and $minHookSuccess -and $fmtSuccess)) {
    Write-Host ""
    Write-Host "Error copying framework files" -ForegroundColor Red
    Write-Host ""
    Write-Host "The game may be running. Please close the game and try again." -ForegroundColor Yellow
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Deployment Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Installed to: $GamePath" -ForegroundColor Cyan
Write-Host ""
Write-Host "Framework Files:" -ForegroundColor White
Write-Host "  dwmapi.dll (Proxy Loader)" -ForegroundColor DarkGray
Write-Host "  Broadsword.dll (Framework Core)" -ForegroundColor DarkGray
if ($modDlls.Count -gt 0) {
    Write-Host ""
    Write-Host "Mods Installed ($($modDlls.Count)):" -ForegroundColor White
    foreach ($modDll in $modDlls) {
        Write-Host "  Mods\$($modDll.Name)" -ForegroundColor DarkGray
    }
}
Write-Host ""
Write-Host "You can now launch Half Sword to test Broadsword Framework." -ForegroundColor White
Write-Host ""
Write-Host "Expected log file:" -ForegroundColor Gray
Write-Host "  $GamePath\Broadsword_YYYYMMDD_HHMMSS.log" -ForegroundColor DarkGray
Write-Host ""
Write-Host "Expected ImGui window:" -ForegroundColor Gray
Write-Host "  Broadsword Framework (with mod tabs)" -ForegroundColor DarkGray
Write-Host ""
