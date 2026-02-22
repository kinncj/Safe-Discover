# Safe Discover Documentation

**Safe Discover** is a Kirigami-based package management GUI for CachyOS and Arch Linux. It provides a unified interface for managing Pacman, AUR (via paru), Flatpak, firmware updates (via fwupd), and KDE add-ons (via KNewStuff).

**Version**: 0.1.0
**License**: GPL-3.0-or-later
**Author**: Kinn Coelho Juliao <kinncj@protonmail.com>

## Documentation Index

| Document | Description |
|----------|-------------|
| [Architecture](architecture.md) | System architecture, layer design, and component interactions |
| [Backends](backends.md) | Package backend implementations and CLI tool integration |
| [Security Model](security.md) | Privilege escalation, PolicyKit policy, and helper script whitelist |
| [QML UI](qml-ui.md) | QML module, page hierarchy, component library, and signal flow |
| [Configuration](configuration.md) | KConfig schema, settings, and runtime options |
| [Build & Development](build.md) | Build instructions, dependencies, testing, and packaging |
| [Contributing](contributing.md) | Development workflow, conventions, and code style |

## Quick Start

```bash
# Build
cmake -B build && cmake --build build -j$(nproc)

# Run
./build/bin/safe-discover

# Test
cmake -B build -DBUILD_TESTING=ON && cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

## Key Design Principles

- **No PackageKit**: All operations use direct CLI tool invocation for transparency and reliability.
- **Explicit Actions**: Every install, remove, and update requires user confirmation.
- **Sequential Updates**: The Safe Update feature runs backends one at a time, stopping on any error.
- **Strict Privilege Escalation**: Only whitelisted commands can run as root, enforced by a helper script.
- **Conditional Features**: Backend pages are only shown when their tools are installed.
