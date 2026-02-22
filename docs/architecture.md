# Architecture

Safe Discover follows a three-layer architecture: **Core**, **Backends**, and **UI (QML)**. All process execution flows through a single `CommandRunner` singleton, and all backends expose data via Qt's model/view system.

## High-Level Overview

```mermaid
graph TB
    subgraph UI["UI Layer (QML/Kirigami)"]
        Main["Main.qml<br/>ApplicationWindow"]
        Pages["Pages<br/>Repo | AUR | Flatpak<br/>Firmware | Updates | KDE Add-ons"]
        Components["Components<br/>SearchBar | ListView | DetailsPane<br/>ConfirmDialog | UpdateSection"]
    end

    subgraph Backends["Backend Layer (C++)"]
        PM["PacmanBackend"]
        AUR["AurBackend"]
        FP["FlatpakBackend"]
        FW["FirmwareBackend"]
        UM["UpdateManager"]
    end

    subgraph Core["Core Layer (C++)"]
        CR["CommandRunner"]
        PKG["PackageModel"]
        TC["ToolChecker"]
        LM["LogManager"]
    end

    subgraph External["External Tools"]
        pacman["pacman"]
        paru["paru"]
        flatpak["flatpak"]
        fwupdmgr["fwupdmgr"]
        pkexec["pkexec"]
    end

    Main --> Pages
    Pages --> Components
    Pages --> Backends

    PM --> PKG
    AUR --> PKG
    FP --> PKG

    PM --> CR
    AUR --> CR
    FP --> CR
    FW --> CR
    UM --> CR

    CR --> pacman
    CR --> paru
    CR --> flatpak
    CR --> fwupdmgr
    CR --> pkexec

    UM --> PM
    UM --> AUR
    UM --> FP
    UM --> FW
```

## Layer Responsibilities

### Core Layer

| Component | Role |
|-----------|------|
| **CommandRunner** | Singleton process execution engine. Two modes: `Embedded` (background QProcess) and `Terminal` (Konsole). Handles privilege escalation via `pkexec`, pacman lock detection, job tracking, and timeouts. |
| **PackageModel** | Abstract `QAbstractListModel` base class. Defines `PackageInfo` struct and virtual interface (`search`, `fetchDetails`, `install`, `remove`). All package backends inherit from this. |
| **ToolChecker** | Singleton that detects tool availability (`paru`, `flatpak`, `fwupdmgr`, `konsole`) at startup via `QStandardPaths::findExecutable()`. Properties are `CONSTANT`. |
| **LogManager** | Singleton managing in-memory session log and optional file persistence to `~/.local/state/safe-discover/logs/`. |

### Backend Layer

| Backend | Tool | Inherits |
|---------|------|----------|
| **PacmanBackend** | `pacman` | `PackageModel` |
| **AurBackend** | `paru` | `PackageModel` |
| **FlatpakBackend** | `flatpak` | `PackageModel` |
| **FirmwareBackend** | `fwupdmgr` | `QAbstractListModel` (custom) |
| **UpdateManager** | All tools | `QObject` (orchestrator) |

### UI Layer

QML pages loaded on-demand via `Component` + `pageStack.replace()`. Navigation through a `GlobalDrawer` sidebar. Kirigami provides the adaptive layout framework.

## Singleton Registration

All C++ objects are registered as QML singletons in `main.cpp`:

```mermaid
graph LR
    subgraph QML["QML Module: ca.kinncj.SafeDiscover"]
        direction TB
        S1["CommandRunner"]
        S2["ToolChecker"]
        S3["LogManager"]
        S4["Config (SafeDiscoverConfig)"]
        S5["PacmanBackend"]
        S6["AurBackend"]
        S7["FlatpakBackend"]
        S8["FirmwareBackend"]
        S9["UpdateManager"]
    end
```

## Data Flow: Search and Install

```mermaid
sequenceDiagram
    actor User
    participant Page as QML Page
    participant Backend as PackageBackend
    participant CR as CommandRunner
    participant CLI as CLI Tool

    User->>Page: Types search query
    Page->>Backend: search(query)
    Backend->>CR: runSync(tool, args)
    CR->>CLI: QProcess::start()
    CLI-->>CR: stdout + exit code
    CR-->>Backend: Result{stdout, stderr, exitCode}
    Backend->>Backend: parseSearchOutput(stdout)
    Backend-->>Page: Model updated (countChanged)
    Page-->>User: Package list displayed

    User->>Page: Clicks package
    Page->>Backend: fetchDetails(index)
    Backend->>CR: runSync(tool, detail args)
    CR->>CLI: QProcess::start()
    CLI-->>CR: stdout
    CR-->>Backend: Result
    Backend-->>Page: detailsReady(index, details)
    Page-->>User: Details pane opens

    User->>Page: Clicks Install
    Page->>Backend: install(index)
    Backend-->>Page: confirmationRequired(action, name, msg)
    Page-->>User: Confirm dialog

    User->>Page: Confirms
    Page->>Backend: confirmInstall(index)
    Backend->>CR: runPrivileged(tool, args)
    CR->>CLI: pkexec helper.sh tool args
    CLI-->>CR: exit code
    CR-->>Backend: jobFinished(jobId, exitCode)
    Backend-->>Page: operationFinished(success, message)
    Page-->>User: Toast notification
```

## Update Orchestration

```mermaid
stateDiagram-v2
    [*] --> CheckUpdates: checkAllUpdates()

    CheckUpdates --> PacmanCheck: pacman -Qu
    PacmanCheck --> AURCheck: paru -Qua
    AURCheck --> FlatpakCheck: flatpak remote-ls --updates
    FlatpakCheck --> FirmwareCheck: fwupdmgr get-updates --json
    FirmwareCheck --> Ready: Counts + lists populated

    Ready --> [*]: No updates
    Ready --> Step1: runSafeUpdate()

    state SafeUpdate {
        Step1: Step 1 - Pacman (pacman -Syu)
        Step2: Step 2 - AUR (paru -Sua, terminal)
        Step3: Step 3 - Flatpak (flatpak update)
        Step4: Step 4 - Firmware (fwupdmgr update)

        Step1 --> Step2: Success
        Step2 --> Step3: Continue
        Step3 --> Step4: Success
        Step4 --> [*]: Complete
    }

    Step1 --> Error: Failed
    Step3 --> Error: Failed
    Step4 --> Error: Failed

    Error --> [*]: updateFinished(false)
```

## Project Structure

```
safe-discover/
├── CMakeLists.txt                        # Root: ECM, Qt6, KF6 setup
├── PKGBUILD                              # Arch Linux packaging
├── ca.kinncj.SafeDiscover.desktop      # Desktop entry
├── ca.kinncj.SafeDiscover.metainfo.xml # AppStream metadata
├── ca.kinncj.safediscover.policy       # PolicyKit policy
├── safe-discover-helper.sh               # Privileged helper (whitelist)
├── src/
│   ├── CMakeLists.txt                    # Target, QML module, link libs
│   ├── main.cpp                          # Entry point, singleton registration
│   ├── core/
│   │   ├── commandrunner.h/.cpp          # Process execution engine
│   │   ├── packagemodel.h/.cpp           # Abstract base model
│   │   ├── toolchecker.h/.cpp            # Tool availability detection
│   │   └── logmanager.h/.cpp             # Session + file logging
│   ├── backends/
│   │   ├── pacmanbackend.h/.cpp          # Pacman backend
│   │   ├── aurbackend.h/.cpp             # AUR backend (paru)
│   │   ├── flatpakbackend.h/.cpp         # Flatpak backend
│   │   ├── firmwarebackend.h/.cpp        # Firmware backend (fwupd)
│   │   └── updatemanager.h/.cpp          # Update orchestrator
│   ├── config/
│   │   ├── safediscoverconfig.kcfg       # KDE config schema
│   │   └── safediscoverconfig.kcfgc      # Config compiler settings
│   └── qml/
│       ├── Main.qml                      # ApplicationWindow + navigation
│       ├── pages/                        # Feature pages
│       ├── components/                   # Reusable UI components
│       └── settings/                     # Settings page
├── tests/
│   ├── CMakeLists.txt
│   ├── tst_commandrunner.cpp
│   ├── tst_pacmanbackend.cpp
│   └── tst_flatpakbackend.cpp
└── docs/                                 # Documentation
```
