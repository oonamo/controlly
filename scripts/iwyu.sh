#!/bin/bash

# TODO: This script is untested

FIX_SCRIPT="${FIX_SCRIPT:-fix_includes.py}"
BUILD_DIR="${BUILD_DIR:-build}"

LOG_FILE="$BUILD_DIR/iwyu.log"

echo -e "Starting IWYU automated"

if [ ! -d "$BUILD_DIR" ]; then
  echo -e "Error: Build directory '$BUILD_DIR' not found. Please run CMake configuration first."
  exit 1
fi

if [ ! -f "$FIX_SCRIPT" ]; then
    FALLBACKS=(
        "/usr/bin/fix_includes.py"
        "/usr/local/bin/fix_includes.py"
        "/c/msys64/mingw64/bin/fix_includes.py" # WSL
        "/c/msys64/mingw64/share/include-what-you-use/fix_includes.py" # WSL
    )

    FOUND=false
    for fallback in "${FALLBACKS[@]}"; do
      if [ -f "$fallback" ]; then
        echo -e "Note: Using fallback '$MsysBinFallback'"
        FIX_SCRIPT="$fallback"
        FOUND=true
        break
      fi
    done

    if [ "$FOUND" = false]; then
      echo -e "Error: scripts '$FIX_SCRIPT' and fallbacks were not found. Install the script before using this script"
      exit 1
    fi
fi

echo -e "Cleaning build directory..."
cmake --build "$BUILD_DIR" --target clean

echo -e "Building and Capturing output to $LogFile"
cmake --build "$BUILD_DIR" > "$LOG_FILE" 2>&1

echo -e "Forcing angle brackets for <controlly/*> prefix"
# matches "controlly/*" and replaces it to <controlly/*>
sed -e 's/"controlly\/\([^"]*\)"/<controlly\/\1>/g' "$LOG_FILE" > "${LOG_FILE}.tmp"
mv "${LOG_FILE}.tmp" "$LOG_FILE"

echo -e "Applying fixes"
cat "$LOG_FILE" | python3 "$FIX_SCRIPT"

echo -e "Done!"
