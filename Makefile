# Safe Discover - Development Makefile
# Copyright (C) 2026 Kinn Coelho Juliao <kinncj@protonmail.com>
# License: GPL-3.0-or-later

.PHONY: build build-clean build-release configure test install uninstall \
        package package-install package-clean run lint clean version bump-version \
        aur-update aur-publish help

# Version is sourced from CMakeLists.txt (single source of truth)
VERSION := $(shell grep 'project.*safe-discover' CMakeLists.txt | grep -o '[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*')
BUILD_DIR := build

# BUILDDIR isolates makepkg's working directories (src/, pkg/) from the
# project tree, preventing collision with our own src/ directory.
MAKEPKG_BUILDDIR := /tmp/safe-discover-makepkg

##@ Development

help: ## Show this help
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m<target>\033[0m\n"} \
		/^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2 } \
		/^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) }' $(MAKEFILE_LIST)

configure: ## Configure cmake build
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug

build: ## Build the project
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
	cmake --build $(BUILD_DIR) -j$$(nproc)

build-release: ## Build in release mode
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
	cmake --build $(BUILD_DIR) -j$$(nproc)

build-clean: ## Clean and rebuild from scratch
	rm -rf $(BUILD_DIR)
	$(MAKE) build

run: build ## Build and run the application
	./$(BUILD_DIR)/bin/safe-discover

##@ Testing

test: build ## Run the test suite
	ctest --test-dir $(BUILD_DIR) --output-on-failure

##@ Installation

install: build-release ## Install to system via cmake (requires sudo)
	sudo cmake --install $(BUILD_DIR)

uninstall: ## Uninstall via pacman (if installed as package)
	sudo pacman -Rns safe-discover

##@ Packaging

package: ## Build Arch Linux package (makepkg)
	BUILDDIR=$(MAKEPKG_BUILDDIR) makepkg -sf --noconfirm

package-install: ## Build and install Arch Linux package
	BUILDDIR=$(MAKEPKG_BUILDDIR) makepkg -sif --noconfirm

package-clean: ## Remove built package artifacts
	rm -f safe-discover-*.pkg.tar.zst
	rm -rf $(MAKEPKG_BUILDDIR)/safe-discover

##@ Maintenance

lint: ## Validate PKGBUILD with namcap
	namcap PKGBUILD
	@if ls safe-discover-*.pkg.tar.zst 1>/dev/null 2>&1; then \
		namcap safe-discover-*.pkg.tar.zst; \
	fi

clean: ## Remove cmake build artifacts
	rm -rf $(BUILD_DIR)

clean-all: clean package-clean ## Remove all build and package artifacts

##@ AUR

aur-update: ## Update AUR PKGBUILD version (usage: make aur-update AUR_VERSION=0.2.0)
ifndef AUR_VERSION
	$(error AUR_VERSION is required. Usage: make aur-update AUR_VERSION=0.2.0)
endif
	@echo "Updating AUR PKGBUILD to version $(AUR_VERSION)..."
	sed -i 's/^pkgver=.*/pkgver=$(AUR_VERSION)/' aur/PKGBUILD
	sed -i 's/^pkgrel=.*/pkgrel=1/' aur/PKGBUILD
	cd aur && makepkg --printsrcinfo > .SRCINFO
	@echo "AUR PKGBUILD updated. Review and run: make aur-publish"

aur-publish: ## Push AUR PKGBUILD to AUR git repository
	@if [ ! -d aur/.git ]; then \
		echo "Error: aur/ is not an AUR git repo."; \
		echo "First run: cd aur && git init && git remote add origin ssh://aur@aur.archlinux.org/safe-discover.git"; \
		exit 1; \
	fi
	cd aur && git add PKGBUILD .SRCINFO && git commit -m "Update to $$(grep ^pkgver= PKGBUILD | cut -d= -f2)" && git push

version: ## Show current version
	@echo $(VERSION)

bump-version: ## Bump version (usage: make bump-version NEW_VERSION=0.2.0)
ifndef NEW_VERSION
	$(error NEW_VERSION is required. Usage: make bump-version NEW_VERSION=0.2.0)
endif
	@echo "Bumping version from $(VERSION) to $(NEW_VERSION)..."
	sed -i 's/project(safe-discover VERSION $(VERSION)/project(safe-discover VERSION $(NEW_VERSION)/' CMakeLists.txt
	sed -i 's/pkgver=$(VERSION)/pkgver=$(NEW_VERSION)/' PKGBUILD
	sed -i 's/pkgrel=[0-9]*/pkgrel=1/' PKGBUILD
	sed -i 's/version="$(VERSION)"/version="$(NEW_VERSION)"/' ca.kinncj.SafeDiscover.metainfo.xml
	sed -i 's/^pkgver=.*/pkgver=$(NEW_VERSION)/' aur/PKGBUILD
	sed -i 's/^pkgrel=.*/pkgrel=1/' aur/PKGBUILD
	cd aur && makepkg --printsrcinfo > .SRCINFO
	@echo "Version bumped to $(NEW_VERSION). Review changes and commit."
