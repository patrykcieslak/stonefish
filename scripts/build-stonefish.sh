#!/bin/bash
set -e

# ----------------------------- GLOBAL VARIABLES -----------------------------
STONEFISH_DIR="${GITHUB_WORKSPACE}/stonefish"
LOG_PREFIX="[$(date +%T)]"

# ----------------------------- HELPER FUNCTIONS -----------------------------
log_info() {
    echo -e "$LOG_PREFIX [INFO] $1"
}

log_error() {
    echo -e "$LOG_PREFIX [ERROR] $1" >&2
}

# ----------------------------- C++ DEPENDENCIES -----------------------------
install_cpp_dependencies() {
    log_info "Installing required C++ dependencies..."
    sudo apt-get update -qq
    sudo apt-get install -y \
        build-essential \
        cmake \
        git \
        libglm-dev \
        libsdl2-dev \
        libfreetype6-dev
    log_info "C++ dependencies installed."
}

# ----------------------------- STONEFISH INSTALLATION -----------------------------
install_stonefish() {
    log_info "Building Stonefish..."
    mkdir -p "$STONEFISH_DIR/build"
    cd "$STONEFISH_DIR/build"

    cmake ..
    make -j"$(nproc)"
    sudo make install

    log_info "Stonefish installation complete."
}


# ----------------------------- EXECUTE INSTALLATION -----------------------------
install_cpp_dependencies
install_stonefish