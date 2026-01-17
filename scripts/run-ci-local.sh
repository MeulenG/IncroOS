#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

ARTIFACTS_DIR_DEFAULT="$ROOT_DIR/.artifacts/bochs-output"
ARTIFACTS_DIR="${ARTIFACTS_DIR:-$ARTIFACTS_DIR_DEFAULT}"
UPLOAD_DISK_IMAGE="${UPLOAD_DISK_IMAGE:-false}"
KEEP_BUILD="${KEEP_BUILD:-false}"
SKIP_DEPS="${SKIP_DEPS:-false}"
SKIP_BOCHS_BUILD="${SKIP_BOCHS_BUILD:-false}"
AUTO_SETUP="${AUTO_SETUP:-true}"

usage() {
  cat <<EOF
Usage: $(basename "$0") [options]

Options:
  --artifacts-dir PATH     Where to store logs/artifacts (default: $ARTIFACTS_DIR_DEFAULT)
  --upload-disk-image      Copy disk.img into artifacts dir
  --keep-build             Do not delete build artifacts and bochs outputs
  --skip-deps              Skip apt-get dependency installation
  --skip-bochs-build       Skip building/installing Bochs
  --no-auto-setup          Skip git submodule init/update
  -h, --help               Show this help

Environment variables:
  ARTIFACTS_DIR, UPLOAD_DISK_IMAGE, KEEP_BUILD, SKIP_DEPS, SKIP_BOCHS_BUILD, AUTO_SETUP
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --artifacts-dir)
      ARTIFACTS_DIR="$2"
      shift 2
      ;;
    --upload-disk-image)
      UPLOAD_DISK_IMAGE=true
      shift
      ;;
    --keep-build)
      KEEP_BUILD=true
      shift
      ;;
    --skip-deps)
      SKIP_DEPS=true
      shift
      ;;
    --skip-bochs-build)
      SKIP_BOCHS_BUILD=true
      shift
      ;;
    --no-auto-setup)
      AUTO_SETUP=false
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 1
      ;;
  esac
done

log() {
  echo "[$(date +"%H:%M:%S")] $*"
}

ensure_dotnet() {
  if command -v dotnet >/dev/null 2>&1; then
    return 0
  fi

  if [[ -x "$ROOT_DIR/vendor/dotnet-install.sh" ]]; then
    log "Installing .NET 6 (local user)"
    "$ROOT_DIR/vendor/dotnet-install.sh" -Channel 6.0 -InstallDir "$HOME/.dotnet"
    export DOTNET_ROOT="$HOME/.dotnet"
    export PATH="$DOTNET_ROOT:$PATH"
  else
    echo "dotnet not found and vendor/dotnet-install.sh is missing." >&2
    exit 1
  fi

  if ! command -v dotnet >/dev/null 2>&1; then
    echo "dotnet installation failed." >&2
    exit 1
  fi
}

ensure_submodules() {
  if [[ "$AUTO_SETUP" != "true" ]]; then
    log "Skipping submodule setup"
    return 0
  fi

  log "Ensuring submodules are initialized"
  git -C "$ROOT_DIR" submodule update --init --recursive
}

install_deps() {
  if [[ "$SKIP_DEPS" == "true" ]]; then
    log "Skipping dependency install"
    return 0
  fi

  log "Installing build dependencies (requires sudo)"
  sudo apt-get update
  sudo apt-get install -y \
    build-essential \
    nasm \
    cmake \
    ninja-build \
    python3 \
    git \
    curl
}

build_bochs() {
  if [[ "$SKIP_BOCHS_BUILD" == "true" ]]; then
    log "Skipping Bochs build"
    return 0
  fi

  if command -v bochs >/dev/null 2>&1; then
    log "Bochs already available; skipping build"
    return 0
  fi

  log "Building and installing Bochs 2.7"
  pushd /tmp >/dev/null
  rm -rf bochs-2.7 bochs-2.7.tar.gz
  curl -L -o bochs-2.7.tar.gz https://sourceforge.net/projects/bochs/files/bochs/2.7/bochs-2.7.tar.gz
  tar -xzf bochs-2.7.tar.gz
  cd bochs-2.7
  ./configure \
    --enable-cpu-level=6 \
    --enable-x86-64 \
    --enable-smp \
    --enable-svm \
    --enable-avx \
    --enable-long-phy-address \
    --enable-all-optimizations \
    --enable-pci \
    --enable-usb \
    --enable-usb-ohci \
    --enable-usb-ehci \
    --enable-usb-xhci \
    --enable-ne2000 \
    --enable-pnic \
    --enable-e1000 \
    --enable-raw-serial \
    --with-nogui
  make -j"$(nproc)"
  sudo make install
  popd >/dev/null
  rm -rf /tmp/bochs-2.7 /tmp/bochs-2.7.tar.gz
}

build_diskbuilder() {
  log "Building diskbuilder"
  pushd "$ROOT_DIR/External/diskbuilder" >/dev/null
  mkdir -p build
  cd build
  cmake .. -G Ninja
  ninja
  popd >/dev/null
}

build_os() {
  log "Building OS"
  mkdir -p "$ROOT_DIR/build"
  pushd "$ROOT_DIR/build" >/dev/null
  cmake ..
  make -j"$(nproc)"
  popd >/dev/null
}

create_bochsrc() {
  log "Creating headless bochsrc"
  cat > "$ROOT_DIR/bochs/bochsrc.ci" <<'EOF'
# Headless configuration for CI testing
display_library: nogui

# CPU configuration
cpu: model=core2_penryn_t9600, count=1, ips=50000000, reset_on_triple_fault=1, ignore_bad_msrs=1
cpuid: x86_64=1, mmx=1, sep=1, simd=sse4_2, apic=xapic, aes=1, movbe=1, xsave=1
cpuid: family=6, model=0x1a, stepping=5, level=6

# Memory
memory: guest=4096, host=256

# ROM images
romimage: file=$BXSHARE/BIOS-bochs-latest
vgaromimage: file=$BXSHARE/VGABIOS-lgpl-latest

# PCI
pci: enabled=1, chipset=i440fx

# ATA controllers
ata0: enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0, irq=14
ata1: enabled=1, ioaddr1=0x170, ioaddr2=0x370, irq=15

# Disk image
ata0-master: type=disk, mode=flat, path=disk.img

# Boot from disk
boot: disk

# Serial port for output capture
com1: enabled=1, mode=file, dev=serial.out

# Logging
log: bochsout.txt
debugger_log: -

# Log levels
panic: action=report
error: action=report
info: action=report
debug: action=ignore

# Magic breakpoint disabled for automated testing
magic_break: enabled=0

# Port E9 hack for debug output
port_e9_hack: enabled=1

# Mouse disabled for headless operation
mouse: enabled=0

# Private colormap
private_colormap: enabled=0
EOF
}

run_bochs() {
  log "Running Bochs (headless)"
  pushd "$ROOT_DIR/bochs" >/dev/null
  if [[ -f "$ROOT_DIR/disk.img" ]]; then
    cp "$ROOT_DIR/disk.img" .
  else
    echo "Error: disk.img not found in repo root." >&2
    ls -la "$ROOT_DIR" >&2
    ls -la "$ROOT_DIR/build" >&2 || true
    popd >/dev/null
    return 1
  fi

  export BXSHARE=/usr/local/share/bochs
  timeout 30s bochs -q -f bochsrc.ci || true
  echo "=== Bochs run completed ==="
  popd >/dev/null
}

display_logs() {
  log "Displaying Bochs output"
  pushd "$ROOT_DIR/bochs" >/dev/null

  echo "=== Bochs Output Log ==="
  if [[ -f bochsout.txt ]]; then
    echo "File size: $(wc -c < bochsout.txt) bytes"
    cat bochsout.txt
  else
    echo "No bochsout.txt found"
  fi

  echo ""
  echo "=== Port E9 Debug Output (from bochsout.txt) ==="
  if [[ -f bochsout.txt ]]; then
    grep -i "port 0x0e9" bochsout.txt || echo "No E9 output found"
  fi

  popd >/dev/null

  log "Dumping serial output"
  pushd "$ROOT_DIR/bochs" >/dev/null
  echo "========================================"
  echo "     SERIAL OUTPUT FROM COM1 PORT      "
  echo "========================================"
  echo ""

  if [[ -f serial.out ]]; then
    echo "Serial output file found!"
    echo "File size: $(wc -c < serial.out) bytes"
    echo "Line count: $(wc -l < serial.out) lines"
    echo ""
    echo "--- Serial Output Content ---"
    cat serial.out
    echo ""
    echo "--- End of Serial Output ---"

    if [[ -s serial.out ]]; then
      echo ""
      echo "--- Serial Output (Hex Dump - First 512 bytes) ---"
      xxd -l 512 serial.out || hexdump -C serial.out | head -32
    fi
  else
    echo "ERROR: No serial output file found!"
    echo "This means the OS did not write to the serial port."
  fi

  echo ""
  echo "Serial output capture location: $(pwd)/serial.out"
  popd >/dev/null
}

verify_os() {
  log "Verifying OS started"
  pushd "$ROOT_DIR/bochs" >/dev/null

  local success=0

  if [[ -f bochsout.txt ]]; then
    if grep -q "Bochs x86 Emulator" bochsout.txt; then
      echo "SUCCESS: Bochs emulator started successfully"
      success=1
    fi
    if grep -q "Booting from" bochsout.txt; then
      echo "SUCCESS: Boot process initiated"
      success=1
    fi
  fi

  if [[ -f serial.out ]] && [[ -s serial.out ]]; then
    echo "SUCCESS: Serial output captured"
    echo "Serial output content:"
    cat serial.out
    success=1
  fi

  if [[ "$success" -eq 0 ]]; then
    echo "ERROR: Could not verify OS boot - no serial output or boot indicators found"
    echo "The script expects either:"
    echo "  1. Serial output in serial.out, OR"
    echo "  2. Boot indicators in bochsout.txt (e.g., 'Booting from')"
    echo ""
    echo "Current status:"
    echo "  - Bochs log exists: $([ -f bochsout.txt ] && echo 'Yes' || echo 'No')"
    echo "  - Serial file exists: $([ -f serial.out ] && echo 'Yes' || echo 'No')"
    echo "  - Serial file size: $([ -f serial.out ] && wc -c < serial.out || echo '0') bytes"
    popd >/dev/null
    return 1
  fi

  echo "SUCCESS: OS verification successful"
  popd >/dev/null
  return 0
}

collect_artifacts() {
  mkdir -p "$ARTIFACTS_DIR"

  if [[ -f "$ROOT_DIR/bochs/bochsout.txt" ]]; then
    cp "$ROOT_DIR/bochs/bochsout.txt" "$ARTIFACTS_DIR/"
  fi
  if [[ -f "$ROOT_DIR/bochs/serial.out" ]]; then
    cp "$ROOT_DIR/bochs/serial.out" "$ARTIFACTS_DIR/"
  fi
  if [[ -f "$ROOT_DIR/bochs/bochsrc.ci" ]]; then
    cp "$ROOT_DIR/bochs/bochsrc.ci" "$ARTIFACTS_DIR/"
  fi

  if [[ "$UPLOAD_DISK_IMAGE" == "true" ]] && [[ -f "$ROOT_DIR/disk.img" ]]; then
    cp "$ROOT_DIR/disk.img" "$ARTIFACTS_DIR/"
  fi
}

cleanup() {
  if [[ "$KEEP_BUILD" == "true" ]]; then
    log "Keeping build artifacts"
    return 0
  fi

  log "Cleaning up build artifacts"
  rm -rf "$ROOT_DIR/build"
  rm -rf "$ROOT_DIR/External/diskbuilder/build"
  rm -rf "$ROOT_DIR/bochs/bochsrc.ci" "$ROOT_DIR/bochs/bochsout.txt" "$ROOT_DIR/bochs/serial.out" "$ROOT_DIR/bochs/disk.img"
}

main() {
  ensure_submodules
  install_deps
  ensure_dotnet
  if [[ "$SKIP_BOCHS_BUILD" == "true" ]] && ! command -v bochs >/dev/null 2>&1; then
    echo "Bochs is not available but --skip-bochs-build was set." >&2
    echo "Install Bochs or rerun without --skip-bochs-build." >&2
    return 1
  fi
  build_bochs
  build_diskbuilder
  build_os
  create_bochsrc
  run_bochs
  display_logs
  if ! verify_os; then
    return 1
  fi
}

status=0
if ! main; then
  status=1
fi
collect_artifacts
cleanup

if [[ "$status" -ne 0 ]]; then
  exit 1
fi

log "Done. Artifacts: $ARTIFACTS_DIR"
