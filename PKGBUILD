pkgname=safe-discover
pkgver=0.1.0
pkgrel=1
pkgdesc='Kirigami-based package management GUI for Arch Linux (pacman, AUR, Flatpak, fwupd)'
arch=('x86_64' 'aarch64')
license=('GPL-3.0-or-later')
url='https://github.com/kinncj/Safe-Discover'

# Build dependencies - only needed at compile time
makedepends=(
    'cmake'
    'extra-cmake-modules'
    'gcc'
)

# Runtime + build dependencies
depends=(
    'qt6-base'
    'qt6-declarative'       # Includes QtQuick, QuickControls2, Qml
    'kirigami'
    'kirigami-addons'
    'kcoreaddons'
    'ki18n'
    'kconfig'
    'knewstuff'
    'pacman'             # Always present on Arch, but explicit
    'polkit'             # For pkexec privilege escalation
)

# Optional runtime dependencies
optdepends=(
    'paru: AUR package support'
    'flatpak: Flatpak application support'
    'fwupd: Firmware update support'
    'konsole: Terminal execution mode for AUR builds'
)

source=()
sha256sums=()

build() {
    cmake -B build -S "$startdir" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DBUILD_TESTING=OFF
    cmake --build build -j"$(nproc)"
}

check() {
    cmake -B build -S "$startdir" \
        -DBUILD_TESTING=ON
    cmake --build build -j"$(nproc)"
    ctest --test-dir build --output-on-failure
}

package() {
    DESTDIR="$pkgdir" cmake --install build
}
