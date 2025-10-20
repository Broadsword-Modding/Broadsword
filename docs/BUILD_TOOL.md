# Broadsword Build Tool Documentation

The `broadsword.py` automation tool provides a comprehensive command-line interface for building, deploying, and releasing the Broadsword Framework.

## Features

- Environment validation and setup
- Automated CMake configuration with vcpkg
- Build management (Debug/Release)
- Game installation detection and deployment
- Version management and bumping
- Release packaging and git tagging
- Full workflow automation

## Installation

The tool requires Python 3.7+ (included with Windows 10+). No additional dependencies needed.

**This is the ONLY supported build method for Broadsword. Manual CMake commands are not supported.**

```powershell
python broadsword.py --help
```

## Quick Start

### First Time Setup

1. Validate your environment:
```powershell
python broadsword.py validate
```

2. If vcpkg is not detected, set it up:
```powershell
python broadsword.py setup --vcpkg-root C:\Projects\vcpkg
```

### Build and Deploy

Build Debug and deploy to game:
```powershell
python broadsword.py all
```

Build Release and create release package:
```powershell
python broadsword.py all --release
```

## Command Reference

### validate

Validates the build environment, checking for:
- vcpkg installation and VCPKG_ROOT
- CMake 3.28+
- MSVC compiler
- vcpkg.json and CMakePresets.json

```powershell
python broadsword.py validate
```

**Output:**
- Green [OK] for each requirement met
- Red [ERROR] for missing requirements
- Yellow [WARN] for non-critical issues

### setup

Configure environment variables permanently.

```powershell
python broadsword.py setup --vcpkg-root <path>
```

**Options:**
- `--vcpkg-root PATH` - Set VCPKG_ROOT environment variable

**Example:**
```powershell
python broadsword.py setup --vcpkg-root C:\Projects\vcpkg
```

Note: Restart your terminal after running setup for changes to take effect.

### build

Configure and build the project.

```powershell
python broadsword.py build [options]
```

**Options:**
- `--release` - Build Release configuration (default: Debug)
- `--clean` - Clean build directory before building
- `--parallel N` - Use N parallel jobs for compilation
- `--configure-only` - Only run CMake configure, skip build

**Examples:**
```powershell
# Build Debug
python broadsword.py build

# Build Release with clean
python broadsword.py build --release --clean

# Build with 8 parallel jobs
python broadsword.py build --parallel 8

# Only configure CMake
python broadsword.py build --configure-only
```

**Output:**
- `build/bin/Debug/` or `build/bin/Release/`
- Shows DLL sizes and mod count

### deploy

Deploy built files to the game directory.

```powershell
python broadsword.py deploy [options]
```

**Options:**
- `--release` - Deploy Release build (default: Debug)
- `--game-path PATH` - Specify game installation path
- `--build` - Build before deploying

**Examples:**
```powershell
# Deploy Debug (auto-detects game path)
python broadsword.py deploy

# Deploy Release to custom path
python broadsword.py deploy --release --game-path "D:\Games\Half Sword\..."

# Build and deploy in one command
python broadsword.py deploy --build
```

**Game Path Detection:**
The tool automatically searches for Half Sword in:
1. Steam registry
2. Steam library folders
3. Common installation paths

**Deployed Files:**
- `dwmapi.dll` - Proxy loader
- `Broadsword.dll` - Framework core
- `minhook.x64d.dll` / `minhook.x64.dll` - MinHook dependency
- `fmtd.dll` / `fmt.dll` - fmt dependency
- `Mods/*.dll` - All built mods

### clean

Remove all build artifacts.

```powershell
python broadsword.py clean
```

Deletes the entire `build/` directory. You'll need to run `build` again to reconfigure and rebuild.

### version

Manage project version.

```powershell
python broadsword.py version [options]
```

**Options:**
- `--show` - Display current version
- `--major` - Bump major version (1.0.0 → 2.0.0)
- `--minor` - Bump minor version (1.0.0 → 1.1.0)
- `--patch` - Bump patch version (1.0.0 → 1.0.1)
- `--set VERSION` - Set specific version (e.g., 2.5.0)
- `--no-commit` - Don't create git commit

**Examples:**
```powershell
# Show current version
python broadsword.py version --show

# Bump patch version (2.1.0 → 2.1.1)
python broadsword.py version --patch

# Bump minor version (2.1.0 → 2.2.0)
python broadsword.py version --minor

# Set specific version
python broadsword.py version --set 3.0.0

# Bump without committing to git
python broadsword.py version --patch --no-commit
```

**What Gets Updated:**
- `CMakeLists.txt` - project(Broadsword VERSION X.Y.Z)
- `vcpkg.json` - "version": "X.Y.Z"
- Git commit created (unless `--no-commit`)

### release

Create a release package.

```powershell
python broadsword.py release [options]
```

**Options:**
- `--no-tag` - Don't create git tag

**What It Does:**
1. Packages Release build into `releases/Broadsword-vX.Y.Z/`
2. Copies DLLs, mods, and documentation
3. Creates ZIP archive: `Broadsword-vX.Y.Z-windows-x64.zip`
4. Creates git tag `vX.Y.Z` (unless `--no-tag`)

**Requirements:**
- Release build must exist (run `python broadsword.py build --release` first)

**Example:**
```powershell
# Build Release first
python broadsword.py build --release

# Create release package
python broadsword.py release
```

**Output:**
- `releases/Broadsword-vX.Y.Z/` - Unpacked release
- `releases/Broadsword-vX.Y.Z-windows-x64.zip` - Distribution archive
- Git tag `vX.Y.Z` ready to push

### all

Run complete workflow: build → deploy → release.

```powershell
python broadsword.py all [options]
```

**Options:**
- `--release` - Use Release configuration (default: Debug)
- `--game-path PATH` - Specify game installation path

**Debug Workflow:**
```powershell
python broadsword.py all
```
1. Validates environment
2. Configures CMake (Debug preset)
3. Builds Debug
4. Deploys to game
5. Success!

**Release Workflow:**
```powershell
python broadsword.py all --release
```
1. Validates environment
2. Configures CMake (Release preset)
3. Builds Release
4. Deploys to game
5. Creates release package
6. Creates git tag
7. Success!

This is the recommended command for creating official releases.

## Common Workflows

### Daily Development

```powershell
# Quick build and test
python broadsword.py all

# Or step by step
python broadsword.py build
python broadsword.py deploy
```

### Testing Changes

```powershell
# Clean build to verify everything compiles
python broadsword.py build --clean

# Deploy and test in-game
python broadsword.py deploy
```

### Creating a Release

```powershell
# Bump version (e.g., 2.1.0 → 2.2.0)
python broadsword.py version --minor

# Full release workflow
python broadsword.py all --release

# Push git tag
git push origin --tags
```

### Troubleshooting

```powershell
# Validate environment
python broadsword.py validate

# Clean and rebuild
python broadsword.py clean
python broadsword.py build --clean

# Check version
python broadsword.py version --show
```

## Environment Variables

### VCPKG_ROOT

The tool uses the `VCPKG_ROOT` environment variable to locate vcpkg.

**Check current value:**
```powershell
echo $env:VCPKG_ROOT
```

**Set permanently:**
```powershell
python broadsword.py setup --vcpkg-root C:\Projects\vcpkg
```

**Set for current session:**
```powershell
$env:VCPKG_ROOT = "C:\Projects\vcpkg"
```

## Directory Structure

```
Broadsword/
├── broadsword.py           # Build automation tool
├── CMakeLists.txt          # CMake configuration
├── CMakePresets.json       # CMake presets (auto-generated)
├── vcpkg.json              # vcpkg manifest
├── build/                  # Build output (generated)
│   ├── bin/
│   │   ├── Debug/
│   │   │   ├── Broadsword.dll
│   │   │   ├── dwmapi.dll
│   │   │   └── Mods/
│   │   └── Release/
│   └── vcpkg_installed/    # vcpkg dependencies
└── releases/               # Release packages (generated)
    └── Broadsword-vX.Y.Z/
```

## Error Handling

The tool provides clear error messages:

**vcpkg not found:**
```
[ERROR] vcpkg not found. Install from: https://github.com/microsoft/vcpkg
```
Solution: Install vcpkg and run `python broadsword.py setup --vcpkg-root <path>`

**CMake configuration failed:**
```
[ERROR] CMake configuration failed
```
Solution: Check VCPKG_ROOT is set, run `python broadsword.py validate`

**Build output not found:**
```
[ERROR] Build output not found: build/bin/Debug
```
Solution: Run `python broadsword.py build` first

**Game installation not found:**
```
[ERROR] Game installation not found
```
Solution: Use `--game-path` to specify location manually

## Advanced Usage

### Custom Build Configuration

The tool automatically generates `CMakePresets.json` if missing. You can customize presets by editing this file.

### Parallel Builds

Speed up compilation with parallel jobs:
```powershell
python broadsword.py build --parallel 12
```

### Selective Deployment

Deploy only framework (without rebuilding):
```powershell
python broadsword.py deploy --no-backup
```

### Version Management

Semantic versioning (MAJOR.MINOR.PATCH):
- **MAJOR**: Breaking changes, incompatible API changes
- **MINOR**: New features, backwards-compatible
- **PATCH**: Bug fixes, backwards-compatible

```powershell
# Bug fix: 2.1.0 → 2.1.1
python broadsword.py version --patch

# New feature: 2.1.1 → 2.2.0
python broadsword.py version --minor

# Breaking change: 2.2.0 → 3.0.0
python broadsword.py version --major
```

## Integration with Git

### Automatic Commits

Version bumps automatically create git commits:
```
chore: bump version to 2.2.0
```

Skip with `--no-commit`:
```powershell
python broadsword.py version --patch --no-commit
```

### Release Tags

The `release` command creates annotated git tags:
```
git tag -a v2.2.0 -m "Release v2.2.0"
```

Push tags to remote:
```powershell
git push origin --tags
```

## Continuous Integration

The tool is designed to work in CI/CD environments:

```yaml
# GitHub Actions example
- name: Setup vcpkg
  run: |
    echo "VCPKG_ROOT=C:/vcpkg" >> $GITHUB_ENV

- name: Build Release
  run: python broadsword.py build --release

- name: Create Release Package
  run: python broadsword.py release --no-tag
```

## Comparison with Old Workflow

### Before (Manual)

```powershell
# Set environment variable manually
$env:VCPKG_ROOT = "C:/Projects/vcpkg"

# Configure
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:/Projects/vcpkg/scripts/buildsystems/vcpkg.cmake"

# Build
cmake --build build --config Debug

# Deploy manually
.\deploy.ps1 -Configuration Debug

# Update version manually in multiple files
# Create release package manually
# Create git tag manually
```

### After (Automated)

```powershell
# One-time setup
python broadsword.py setup --vcpkg-root C:\Projects\vcpkg

# Complete workflow
python broadsword.py all
```

## Support

For issues with the build tool:
1. Run `python broadsword.py validate` to check environment
2. Check `build/CMakeCache.txt` for configuration issues
3. Review error messages for specific problems
4. Report issues at: https://github.com/Broadsword-Modding/Broadsword/issues
