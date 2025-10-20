#!/usr/bin/env python3
"""
Broadsword Build Automation Tool

A comprehensive build, deployment, and release management system for the Broadsword Framework.
Handles environment validation, configuration, building, deployment, versioning, and releases.
"""

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import winreg
from dataclasses import dataclass
from datetime import datetime
from enum import Enum
from pathlib import Path
from typing import Dict, List, Optional, Tuple


class Color:
    """ANSI color codes for terminal output"""
    RESET = '\033[0m'
    BOLD = '\033[1m'
    RED = '\033[31m'
    GREEN = '\033[32m'
    YELLOW = '\033[33m'
    BLUE = '\033[34m'
    CYAN = '\033[36m'
    GRAY = '\033[90m'


class BuildConfig(Enum):
    """Build configuration types"""
    DEBUG = "Debug"
    RELEASE = "Release"


@dataclass
class Version:
    """Semantic version handling"""
    major: int
    minor: int
    patch: int

    @staticmethod
    def parse(version_str: str) -> 'Version':
        """Parse version string (e.g., '2.1.0')"""
        match = re.match(r'^(\d+)\.(\d+)\.(\d+)$', version_str)
        if not match:
            raise ValueError(f"Invalid version format: {version_str}")
        return Version(int(match.group(1)), int(match.group(2)), int(match.group(3)))

    def __str__(self) -> str:
        return f"{self.major}.{self.minor}.{self.patch}"

    def bump_major(self) -> 'Version':
        """Bump major version and reset minor/patch"""
        return Version(self.major + 1, 0, 0)

    def bump_minor(self) -> 'Version':
        """Bump minor version and reset patch"""
        return Version(self.major, self.minor + 1, 0)

    def bump_patch(self) -> 'Version':
        """Bump patch version"""
        return Version(self.major, self.minor, self.patch + 1)


class Logger:
    """Formatted console output"""

    @staticmethod
    def header(msg: str):
        """Print header message"""
        print(f"\n{Color.BOLD}{Color.CYAN}{'=' * 60}{Color.RESET}")
        print(f"{Color.BOLD}{Color.CYAN}{msg}{Color.RESET}")
        print(f"{Color.BOLD}{Color.CYAN}{'=' * 60}{Color.RESET}\n")

    @staticmethod
    def section(msg: str):
        """Print section header"""
        print(f"\n{Color.BOLD}{Color.BLUE}>> {msg}{Color.RESET}")

    @staticmethod
    def success(msg: str):
        """Print success message"""
        print(f"{Color.GREEN}[OK] {msg}{Color.RESET}")

    @staticmethod
    def error(msg: str):
        """Print error message"""
        print(f"{Color.RED}[ERROR] {msg}{Color.RESET}", file=sys.stderr)

    @staticmethod
    def warning(msg: str):
        """Print warning message"""
        print(f"{Color.YELLOW}[WARN] {msg}{Color.RESET}")

    @staticmethod
    def info(msg: str):
        """Print info message"""
        print(f"{Color.GRAY}  {msg}{Color.RESET}")

    @staticmethod
    def step(msg: str):
        """Print step message"""
        print(f"{Color.CYAN}> {msg}{Color.RESET}")


class Environment:
    """System environment validation and detection"""

    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.vcpkg_root: Optional[Path] = None
        self.cmake_path: Optional[Path] = None
        self.game_path: Optional[Path] = None

    def validate(self) -> bool:
        """Validate build environment"""
        Logger.section("Validating Environment")

        valid = True

        # Check vcpkg
        vcpkg_env = os.getenv('VCPKG_ROOT')
        if vcpkg_env:
            self.vcpkg_root = Path(vcpkg_env)
            if self.vcpkg_root.exists():
                Logger.success(f"vcpkg found: {self.vcpkg_root}")
            else:
                Logger.error(f"VCPKG_ROOT set but path doesn't exist: {self.vcpkg_root}")
                valid = False
        else:
            # Try common locations
            common_paths = [
                Path("C:/vcpkg"),
                Path("C:/Projects/vcpkg"),
                Path.home() / "vcpkg"
            ]
            for path in common_paths:
                if path.exists():
                    self.vcpkg_root = path
                    Logger.warning(f"vcpkg found at {path} but VCPKG_ROOT not set")
                    Logger.info("Run: broadsword.py setup --vcpkg-root <path>")
                    break

            if not self.vcpkg_root:
                Logger.error("vcpkg not found. Install from: https://github.com/microsoft/vcpkg")
                valid = False

        # Check CMake
        cmake_result = subprocess.run(['cmake', '--version'], capture_output=True, text=True)
        if cmake_result.returncode == 0:
            version_line = cmake_result.stdout.split('\n')[0]
            Logger.success(f"CMake found: {version_line}")
            self.cmake_path = Path(shutil.which('cmake'))
        else:
            Logger.error("CMake not found. Install CMake 3.28+")
            valid = False

        # Check MSVC (informational only - CMake will find it via VS generator)
        msvc_result = subprocess.run(['cl'], capture_output=True, text=True, shell=True)
        if 'Microsoft (R) C/C++ Optimizing Compiler' in msvc_result.stderr:
            compiler_match = re.search(r'Version ([\d.]+)', msvc_result.stderr)
            if compiler_match:
                Logger.success(f"MSVC found: Version {compiler_match.group(1)}")
        else:
            Logger.warning("MSVC not in PATH (CMake will locate via Visual Studio)")
            Logger.info("This is normal if not running from Developer Command Prompt")

        # Check vcpkg.json
        vcpkg_json = self.project_root / "vcpkg.json"
        if vcpkg_json.exists():
            Logger.success(f"vcpkg manifest found: {vcpkg_json.name}")
        else:
            Logger.error("vcpkg.json not found in project root")
            valid = False

        # Check CMakePresets.json
        presets_file = self.project_root / "CMakePresets.json"
        if presets_file.exists():
            Logger.success(f"CMake presets found: {presets_file.name}")
        else:
            Logger.warning("CMakePresets.json not found (will be created)")

        return valid

    def detect_game_path(self) -> Optional[Path]:
        """Auto-detect Half Sword installation"""
        Logger.section("Detecting Game Installation")

        # Try Steam registry
        try:
            reg_key = winreg.OpenKey(winreg.HKEY_CURRENT_USER, r"Software\Valve\Steam")
            steam_path, _ = winreg.QueryValueEx(reg_key, "SteamPath")
            winreg.CloseKey(reg_key)

            steam_root = Path(steam_path)
            game_path = steam_root / "steamapps/common/Half Sword Demo/HalfSwordUE5/Binaries/Win64"

            if game_path.exists():
                Logger.success(f"Game found: {game_path}")
                self.game_path = game_path
                return game_path

            # Try library folders
            library_folders = steam_root / "steamapps/libraryfolders.vdf"
            if library_folders.exists():
                content = library_folders.read_text()
                paths = re.findall(r'"path"\s+"([^"]+)"', content)
                for path_str in paths:
                    game_path = Path(path_str) / "steamapps/common/Half Sword Demo/HalfSwordUE5/Binaries/Win64"
                    if game_path.exists():
                        Logger.success(f"Game found: {game_path}")
                        self.game_path = game_path
                        return game_path

        except Exception as e:
            Logger.warning(f"Could not read Steam registry: {e}")

        # Common installation paths
        common_paths = [
            Path("C:/Program Files (x86)/Steam/steamapps/common/Half Sword Demo/HalfSwordUE5/Binaries/Win64"),
            Path("C:/Program Files/Steam/steamapps/common/Half Sword Demo/HalfSwordUE5/Binaries/Win64"),
        ]

        for path in common_paths:
            if path.exists():
                Logger.success(f"Game found: {path}")
                self.game_path = path
                return path

        Logger.warning("Half Sword installation not found")
        Logger.info("Use --game-path to specify manually")
        return None


class Builder:
    """CMake build management"""

    def __init__(self, project_root: Path, env: Environment):
        self.project_root = project_root
        self.env = env
        self.build_dir = project_root / "build"

    def configure(self, preset: str = "debug", clean: bool = False) -> bool:
        """Configure CMake project"""
        Logger.section(f"Configuring CMake ({preset})")

        if clean and self.build_dir.exists():
            Logger.step("Cleaning build directory")
            shutil.rmtree(self.build_dir)
            Logger.success("Build directory cleaned")

        # Ensure CMakePresets.json exists
        self._ensure_presets()

        # Set VCPKG_ROOT for this session
        env_vars = os.environ.copy()
        if self.env.vcpkg_root:
            env_vars['VCPKG_ROOT'] = str(self.env.vcpkg_root)

        Logger.step(f"Running CMake configure with preset: {preset}")
        result = subprocess.run(
            ['cmake', '--preset', preset],
            cwd=self.project_root,
            env=env_vars,
            capture_output=False
        )

        if result.returncode == 0:
            Logger.success("CMake configuration complete")
            return True
        else:
            Logger.error("CMake configuration failed")
            return False

    def build(self, config: BuildConfig, clean: bool = False, parallel: int = 0) -> bool:
        """Build the project"""
        Logger.section(f"Building Broadsword ({config.value})")

        if not self.build_dir.exists():
            Logger.error("Build directory not found. Run configure first.")
            return False

        cmd = ['cmake', '--build', str(self.build_dir), '--config', config.value]

        if clean:
            cmd.extend(['--clean-first'])

        if parallel > 0:
            cmd.extend(['--parallel', str(parallel)])

        Logger.step(f"Running: {' '.join(cmd)}")
        result = subprocess.run(cmd, cwd=self.project_root)

        if result.returncode == 0:
            Logger.success(f"{config.value} build complete")
            self._print_output_summary(config)
            return True
        else:
            Logger.error("Build failed")
            return False

    def _ensure_presets(self):
        """Ensure CMakePresets.json exists"""
        presets_file = self.project_root / "CMakePresets.json"
        if presets_file.exists():
            return

        Logger.warning("CMakePresets.json not found, creating...")

        presets = {
            "version": 3,
            "configurePresets": [
                {
                    "name": "vcpkg",
                    "hidden": True,
                    "cacheVariables": {
                        "CMAKE_TOOLCHAIN_FILE": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
                    }
                },
                {
                    "name": "windows-base",
                    "hidden": True,
                    "generator": "Visual Studio 17 2022",
                    "architecture": {
                        "value": "x64",
                        "strategy": "set"
                    },
                    "cacheVariables": {
                        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
                    }
                },
                {
                    "name": "debug",
                    "displayName": "Debug",
                    "inherits": ["windows-base", "vcpkg"],
                    "binaryDir": "${sourceDir}/build",
                    "cacheVariables": {
                        "CMAKE_BUILD_TYPE": "Debug"
                    }
                },
                {
                    "name": "release",
                    "displayName": "Release",
                    "inherits": ["windows-base", "vcpkg"],
                    "binaryDir": "${sourceDir}/build",
                    "cacheVariables": {
                        "CMAKE_BUILD_TYPE": "Release"
                    }
                }
            ],
            "buildPresets": [
                {
                    "name": "debug",
                    "configurePreset": "debug",
                    "configuration": "Debug"
                },
                {
                    "name": "release",
                    "configurePreset": "release",
                    "configuration": "Release"
                }
            ]
        }

        presets_file.write_text(json.dumps(presets, indent=2))
        Logger.success("Created CMakePresets.json")

    def _print_output_summary(self, config: BuildConfig):
        """Print summary of built files"""
        output_dir = self.build_dir / "bin" / config.value
        if not output_dir.exists():
            return

        Logger.info(f"\nOutput directory: {output_dir}")

        dlls = list(output_dir.glob("*.dll"))
        if dlls:
            Logger.info(f"Built {len(dlls)} DLLs:")
            for dll in dlls:
                size = dll.stat().st_size / 1024 / 1024
                Logger.info(f"  {dll.name} ({size:.2f} MB)")

        mods_dir = output_dir / "Mods"
        if mods_dir.exists():
            mod_dlls = list(mods_dir.glob("*.dll"))
            if mod_dlls:
                Logger.info(f"Built {len(mod_dlls)} mods:")
                for mod in mod_dlls:
                    Logger.info(f"  {mod.name}")


class Deployer:
    """Deployment management"""

    def __init__(self, project_root: Path, env: Environment):
        self.project_root = project_root
        self.env = env
        self.build_dir = project_root / "build"

    def deploy(self, config: BuildConfig, game_path: Optional[Path] = None) -> bool:
        """Deploy built files to game directory"""
        Logger.section(f"Deploying Broadsword ({config.value})")

        # Determine game path
        target_path = game_path or self.env.game_path
        if not target_path:
            target_path = self.env.detect_game_path()

        if not target_path or not target_path.exists():
            Logger.error("Game installation not found")
            Logger.info("Use --game-path to specify location")
            return False

        output_dir = self.build_dir / "bin" / config.value
        if not output_dir.exists():
            Logger.error(f"Build output not found: {output_dir}")
            Logger.info("Run build first")
            return False

        # Deploy framework DLLs
        files_to_deploy = [
            ("dwmapi.dll", "Proxy loader"),
            ("Broadsword.dll", "Framework core"),
            ("minhook.x64d.dll" if config == BuildConfig.DEBUG else "minhook.x64.dll", "MinHook dependency"),
            ("fmtd.dll" if config == BuildConfig.DEBUG else "fmt.dll", "fmt dependency"),
        ]

        deployed = 0
        for filename, description in files_to_deploy:
            source = output_dir / filename
            if source.exists():
                dest = target_path / filename
                shutil.copy2(source, dest)
                Logger.success(f"Deployed {filename} ({description})")
                deployed += 1
            else:
                # Try vcpkg_installed for dependencies
                vcpkg_bin = self.build_dir / "vcpkg_installed/x64-windows/debug/bin" if config == BuildConfig.DEBUG else self.build_dir / "vcpkg_installed/x64-windows/bin"
                source_vcpkg = vcpkg_bin / filename
                if source_vcpkg.exists():
                    dest = target_path / filename
                    shutil.copy2(source_vcpkg, dest)
                    Logger.success(f"Deployed {filename} ({description})")
                    deployed += 1

        # Deploy mods
        mods_source = output_dir / "Mods"
        if mods_source.exists():
            mods_target = target_path / "Mods"
            mods_target.mkdir(exist_ok=True)

            mod_count = 0
            for mod_dll in mods_source.glob("*.dll"):
                dest = mods_target / mod_dll.name
                shutil.copy2(mod_dll, dest)
                Logger.success(f"Deployed mod: {mod_dll.name}")
                mod_count += 1

            if mod_count > 0:
                Logger.info(f"Total mods deployed: {mod_count}")

        Logger.success(f"\nDeployment complete to: {target_path}")
        return True


class VersionManager:
    """Version management and bumping"""

    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.cmake_file = project_root / "CMakeLists.txt"
        self.vcpkg_json = project_root / "vcpkg.json"

    def get_current_version(self) -> Version:
        """Get current version from CMakeLists.txt"""
        content = self.cmake_file.read_text()
        match = re.search(r'project\(Broadsword VERSION ([\d.]+)', content)
        if not match:
            raise ValueError("Could not find version in CMakeLists.txt")
        return Version.parse(match.group(1))

    def set_version(self, new_version: Version, commit: bool = True) -> bool:
        """Update version in all files"""
        Logger.section(f"Updating Version to {new_version}")

        old_version = self.get_current_version()
        Logger.info(f"Current version: {old_version}")
        Logger.info(f"New version: {new_version}")

        # Update CMakeLists.txt
        content = self.cmake_file.read_text()
        content = re.sub(
            r'(project\(Broadsword VERSION )[\d.]+',
            rf'\g<1>{new_version}',
            content
        )
        self.cmake_file.write_text(content)
        Logger.success("Updated CMakeLists.txt")

        # Update vcpkg.json
        vcpkg_data = json.loads(self.vcpkg_json.read_text())
        vcpkg_data['version'] = str(new_version)
        self.vcpkg_json.write_text(json.dumps(vcpkg_data, indent=2) + '\n')
        Logger.success("Updated vcpkg.json")

        if commit:
            self._commit_version_bump(old_version, new_version)

        return True

    def _commit_version_bump(self, old_version: Version, new_version: Version):
        """Create git commit for version bump"""
        Logger.step("Creating git commit")

        # Stage files
        subprocess.run(['git', 'add', 'CMakeLists.txt', 'vcpkg.json'], cwd=self.project_root)

        # Commit
        commit_msg = f"chore: bump version to {new_version}"
        subprocess.run(['git', 'commit', '-m', commit_msg], cwd=self.project_root)

        Logger.success(f"Committed version bump: {old_version} → {new_version}")


class ReleaseManager:
    """Release creation and packaging"""

    def __init__(self, project_root: Path, env: Environment):
        self.project_root = project_root
        self.env = env
        self.version_mgr = VersionManager(project_root)

    def create_release(self, version: Optional[Version] = None, tag: bool = True) -> bool:
        """Create a release package"""
        if not version:
            version = self.version_mgr.get_current_version()

        Logger.header(f"Creating Release v{version}")

        # Create release directory
        release_dir = self.project_root / "releases" / f"Broadsword-v{version}"
        release_dir.mkdir(parents=True, exist_ok=True)

        # Copy Release build
        build_output = self.project_root / "build/bin/Release"
        if not build_output.exists():
            Logger.error("Release build not found. Build Release configuration first.")
            return False

        # Copy DLLs
        for dll in ["dwmapi.dll", "Broadsword.dll"]:
            source = build_output / dll
            if source.exists():
                shutil.copy2(source, release_dir / dll)
                Logger.success(f"Packaged {dll}")

        # Copy Mods
        mods_source = build_output / "Mods"
        if mods_source.exists():
            mods_target = release_dir / "Mods"
            shutil.copytree(mods_source, mods_target, dirs_exist_ok=True)
            Logger.success("Packaged mods")

        # Copy documentation
        docs_to_copy = ["README.md"]
        for doc in docs_to_copy:
            source = self.project_root / doc
            if source.exists():
                shutil.copy2(source, release_dir / doc)

        # Copy BUILD_TOOL.md from docs
        build_doc = self.project_root / "docs/BUILD_TOOL.md"
        if build_doc.exists():
            shutil.copy2(build_doc, release_dir / "BUILD.md")

        # Create archive
        archive_name = f"Broadsword-v{version}-windows-x64"
        archive_path = self.project_root / "releases" / archive_name
        Logger.step(f"Creating archive: {archive_name}.zip")

        shutil.make_archive(str(archive_path), 'zip', release_dir.parent, release_dir.name)
        Logger.success(f"Release archive created: {archive_name}.zip")

        # Create git tag
        if tag:
            self._create_git_tag(version)

        Logger.success(f"\nRelease v{version} created successfully")
        Logger.info(f"Location: {release_dir}")
        Logger.info(f"Archive: {archive_path}.zip")

        return True

    def _create_git_tag(self, version: Version):
        """Create and push git tag"""
        tag_name = f"v{version}"
        Logger.step(f"Creating git tag: {tag_name}")

        # Create tag
        result = subprocess.run(
            ['git', 'tag', '-a', tag_name, '-m', f"Release {tag_name}"],
            cwd=self.project_root,
            capture_output=True
        )

        if result.returncode == 0:
            Logger.success(f"Git tag created: {tag_name}")
            Logger.info("Push with: git push origin --tags")
        else:
            Logger.warning(f"Could not create git tag: {result.stderr.decode()}")


def main():
    """Main CLI entry point"""
    parser = argparse.ArgumentParser(
        description="Broadsword Build Automation Tool",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  broadsword.py validate              - Validate build environment
  broadsword.py build                 - Configure and build (Debug)
  broadsword.py build --release       - Build Release configuration
  broadsword.py deploy                - Build and deploy to game
  broadsword.py deploy --release      - Deploy Release build
  broadsword.py clean                 - Clean build artifacts
  broadsword.py version --patch       - Bump patch version
  broadsword.py release               - Create release package
  broadsword.py all --release         - Full workflow (build + deploy + release)
        """
    )

    subparsers = parser.add_subparsers(dest='command', help='Command to execute')

    # Validate command
    subparsers.add_parser('validate', help='Validate build environment')

    # Setup command
    setup_parser = subparsers.add_parser('setup', help='Setup environment variables')
    setup_parser.add_argument('--vcpkg-root', type=Path, help='Path to vcpkg installation')

    # Build command
    build_parser = subparsers.add_parser('build', help='Build the project')
    build_parser.add_argument('--release', action='store_true', help='Build Release (default: Debug)')
    build_parser.add_argument('--clean', action='store_true', help='Clean before building')
    build_parser.add_argument('--parallel', type=int, default=0, help='Parallel build jobs')
    build_parser.add_argument('--configure-only', action='store_true', help='Only run CMake configure')

    # Deploy command
    deploy_parser = subparsers.add_parser('deploy', help='Deploy to game directory')
    deploy_parser.add_argument('--release', action='store_true', help='Deploy Release (default: Debug)')
    deploy_parser.add_argument('--game-path', type=Path, help='Game installation path')
    deploy_parser.add_argument('--build', action='store_true', help='Build before deploying')

    # Clean command
    subparsers.add_parser('clean', help='Clean build artifacts')

    # Version command
    version_parser = subparsers.add_parser('version', help='Manage version')
    version_parser.add_argument('--show', action='store_true', help='Show current version')
    version_parser.add_argument('--major', action='store_true', help='Bump major version')
    version_parser.add_argument('--minor', action='store_true', help='Bump minor version')
    version_parser.add_argument('--patch', action='store_true', help='Bump patch version')
    version_parser.add_argument('--set', type=str, help='Set specific version (e.g., 2.5.0)')
    version_parser.add_argument('--no-commit', action='store_true', help='Do not create git commit')

    # Release command
    release_parser = subparsers.add_parser('release', help='Create release package')
    release_parser.add_argument('--no-tag', action='store_true', help='Do not create git tag')

    # All command
    all_parser = subparsers.add_parser('all', help='Full workflow: build + deploy + release')
    all_parser.add_argument('--release', action='store_true', help='Use Release configuration')
    all_parser.add_argument('--game-path', type=Path, help='Game installation path')

    args = parser.parse_args()

    # Determine project root
    project_root = Path(__file__).parent.absolute()

    # Initialize environment
    env = Environment(project_root)

    # Handle commands
    try:
        if args.command == 'validate':
            Logger.header("Environment Validation")
            if env.validate():
                Logger.success("\nEnvironment is ready for building")
                sys.exit(0)
            else:
                Logger.error("\nEnvironment validation failed")
                sys.exit(1)

        elif args.command == 'setup':
            Logger.header("Environment Setup")
            if args.vcpkg_root:
                # Set VCPKG_ROOT permanently
                Logger.step(f"Setting VCPKG_ROOT to: {args.vcpkg_root}")
                subprocess.run([
                    'powershell', '-Command',
                    f"[System.Environment]::SetEnvironmentVariable('VCPKG_ROOT', '{args.vcpkg_root}', [System.EnvironmentVariableTarget]::User)"
                ])
                Logger.success("VCPKG_ROOT set successfully")
                Logger.info("Restart your terminal for changes to take effect")
            else:
                Logger.error("No options specified. Use --vcpkg-root <path>")
                sys.exit(1)

        elif args.command == 'build':
            Logger.header("Building Broadsword")
            if not env.validate():
                sys.exit(1)

            builder = Builder(project_root, env)
            config = BuildConfig.RELEASE if args.release else BuildConfig.DEBUG
            preset = "release" if args.release else "debug"

            if not builder.configure(preset, clean=args.clean):
                sys.exit(1)

            if args.configure_only:
                Logger.success("Configuration complete")
                sys.exit(0)

            if not builder.build(config, clean=args.clean, parallel=args.parallel):
                sys.exit(1)

        elif args.command == 'deploy':
            Logger.header("Deploying Broadsword")
            config = BuildConfig.RELEASE if args.release else BuildConfig.DEBUG

            if args.build:
                if not env.validate():
                    sys.exit(1)
                builder = Builder(project_root, env)
                preset = "release" if args.release else "debug"
                if not builder.configure(preset):
                    sys.exit(1)
                if not builder.build(config):
                    sys.exit(1)

            deployer = Deployer(project_root, env)
            if not deployer.deploy(config, args.game_path):
                sys.exit(1)

        elif args.command == 'clean':
            Logger.header("Cleaning Build Artifacts")
            build_dir = project_root / "build"
            if build_dir.exists():
                shutil.rmtree(build_dir)
                Logger.success("Build directory removed")
            else:
                Logger.info("Build directory does not exist")

        elif args.command == 'version':
            version_mgr = VersionManager(project_root)
            current = version_mgr.get_current_version()

            if args.show or not any([args.major, args.minor, args.patch, args.set]):
                Logger.header("Version Information")
                Logger.info(f"Current version: {Color.BOLD}{current}{Color.RESET}")
                sys.exit(0)

            if args.set:
                new_version = Version.parse(args.set)
            elif args.major:
                new_version = current.bump_major()
            elif args.minor:
                new_version = current.bump_minor()
            elif args.patch:
                new_version = current.bump_patch()

            version_mgr.set_version(new_version, commit=not args.no_commit)

        elif args.command == 'release':
            Logger.header("Creating Release")
            release_mgr = ReleaseManager(project_root, env)
            if not release_mgr.create_release(tag=not args.no_tag):
                sys.exit(1)

        elif args.command == 'all':
            Logger.header("Full Build & Deploy Workflow")
            config = BuildConfig.RELEASE if args.release else BuildConfig.DEBUG
            preset = "release" if args.release else "debug"

            # Validate
            if not env.validate():
                sys.exit(1)

            # Build
            builder = Builder(project_root, env)
            if not builder.configure(preset):
                sys.exit(1)
            if not builder.build(config):
                sys.exit(1)

            # Deploy
            deployer = Deployer(project_root, env)
            if not deployer.deploy(config, args.game_path):
                sys.exit(1)

            # Release (only for Release config)
            if args.release:
                release_mgr = ReleaseManager(project_root, env)
                release_mgr.create_release()

            Logger.success("\nWorkflow complete!")

        else:
            parser.print_help()

    except KeyboardInterrupt:
        Logger.error("\nOperation cancelled by user")
        sys.exit(1)
    except Exception as e:
        Logger.error(f"Unexpected error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
