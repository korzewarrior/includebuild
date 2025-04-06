#!/bin/bash

# Test script for IncludeBuild examples
# This script builds and runs all examples to verify they work correctly

set -e  # Exit on error
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}IncludeBuild Examples Test Script${NC}"
echo "========================================"
echo

# Print current directory for debugging
echo "Current directory: $(pwd)"
ls -la ..
echo

# Find examples directory (works from scripts/ directory or project root)
if [ -d "examples" ]; then
    EXAMPLES_DIR="examples"
    echo "Found examples directory in current directory"
elif [ -d "../examples" ]; then
    echo "Found examples directory in parent directory"
    cd ..  # Move to project root
    EXAMPLES_DIR="examples"  # Update path after changing directory
    echo "Changed to directory: $(pwd)"
else
    # Look for examples directory within the repository structure
    PROJECT_ROOT=$(git rev-parse --show-toplevel 2>/dev/null || echo "")
    if [ -n "$PROJECT_ROOT" ] && [ -d "$PROJECT_ROOT/examples" ]; then
        cd "$PROJECT_ROOT"
        EXAMPLES_DIR="examples"
        echo "Found examples directory in Git project root: $PROJECT_ROOT"
    else
        echo -e "${RED}Error: Cannot find 'examples' directory${NC}"
        echo "Directory structure:"
        find . -type d -maxdepth 3 | sort
        echo "This script must be run from either the project root or the scripts directory"
        exit 1
    fi
fi

echo "Using examples directory: $EXAMPLES_DIR"
echo "Contents of examples directory:"
ls -la "$EXAMPLES_DIR"
echo

# Check if Raylib is installed (for game example)
if ! pkg-config --exists raylib; then
    HAS_RAYLIB=0
    echo -e "${YELLOW}Raylib not found. Game example will be skipped.${NC}"
    echo "To install Raylib: sudo apt install libraylib-dev (or equivalent for your system)"
    echo
else
    HAS_RAYLIB=1
    echo -e "${GREEN}Raylib found. Game example will be tested.${NC}"
    echo
fi

TOTAL_EXAMPLES=0
SUCCESSFUL_EXAMPLES=0
FAILED_EXAMPLES=0
SKIPPED_EXAMPLES=0

# Save current directory
SCRIPT_DIR=$(pwd)

# Find all example directories
echo "Finding example directories in ${EXAMPLES_DIR}..."
EXAMPLE_DIRS=$(find "${EXAMPLES_DIR}" -maxdepth 1 -type d | grep -v "^${EXAMPLES_DIR}$" || echo "")

if [ -z "$EXAMPLE_DIRS" ]; then
    echo -e "${RED}No example directories found in ${EXAMPLES_DIR}${NC}"
    echo "Contents of current directory:"
    ls -la
    exit 1
fi

echo "Found example directories: $EXAMPLE_DIRS"
echo

for DIR in $EXAMPLE_DIRS; do
    EXAMPLE_NAME=$(basename $DIR)
    TOTAL_EXAMPLES=$((TOTAL_EXAMPLES + 1))
    
    echo -e "${GREEN}Testing example: ${EXAMPLE_NAME}${NC}"
    echo "----------------------------------------"
    
    # Skip game example if Raylib is not installed
    if [ "$EXAMPLE_NAME" == "game" ] && [ $HAS_RAYLIB -eq 0 ]; then
        echo -e "${YELLOW}Skipping game example (Raylib not installed)${NC}"
        SKIPPED_EXAMPLES=$((SKIPPED_EXAMPLES + 1))
        echo
        continue
    fi
    
    # Enter example directory
    cd "$DIR"
    echo "Entered directory: $(pwd)"
    
    # Clean up any previous builds
    rm -f build main *.o *.a *.so *.exe run_test_* 2>/dev/null || true
    
    # Compile the build script
    echo "Compiling build script..."
    if gcc -o build build.c; then
        echo -e "${GREEN}✓ Build script compiled successfully${NC}"
    else
        echo -e "${RED}✗ Failed to compile build script${NC}"
        FAILED_EXAMPLES=$((FAILED_EXAMPLES + 1))
        cd "$SCRIPT_DIR"
        echo
        continue
    fi
    
    # Run the build script
    echo "Running build script..."
    if ./build; then
        echo -e "${GREEN}✓ Build script executed successfully${NC}"
    else
        echo -e "${RED}✗ Build script failed${NC}"
        FAILED_EXAMPLES=$((FAILED_EXAMPLES + 1))
        cd "$SCRIPT_DIR"
        echo
        continue
    fi
    
    # Special handling for different examples
    case "$EXAMPLE_NAME" in
        "library")
            # For library example, run the test program using the provided script
            echo "Running library test..."
            if [ -f run_test_logger.sh ]; then
                if ./run_test_logger.sh; then
                    echo -e "${GREEN}✓ Library test executed successfully${NC}"
                    SUCCESSFUL_EXAMPLES=$((SUCCESSFUL_EXAMPLES + 1))
                else
                    echo -e "${RED}✗ Library test failed${NC}"
                    FAILED_EXAMPLES=$((FAILED_EXAMPLES + 1))
                fi
            else
                echo -e "${RED}✗ Run script not found${NC}"
                FAILED_EXAMPLES=$((FAILED_EXAMPLES + 1))
            fi
            ;;
            
        "game")
            # For game example, just check if the executable was created
            # (we don't actually run the game in the test)
            if [ -f main ]; then
                echo -e "${GREEN}✓ Game executable created successfully${NC}"
                SUCCESSFUL_EXAMPLES=$((SUCCESSFUL_EXAMPLES + 1))
            else
                echo -e "${RED}✗ Game executable not created${NC}"
                FAILED_EXAMPLES=$((FAILED_EXAMPLES + 1))
            fi
            ;;
            
        *)
            # For other examples, run the main executable if it exists
            if [ -f main ]; then
                echo "Running the executable..."
                if ./main; then
                    echo -e "${GREEN}✓ Executable ran successfully${NC}"
                    SUCCESSFUL_EXAMPLES=$((SUCCESSFUL_EXAMPLES + 1))
                else
                    echo -e "${RED}✗ Executable failed${NC}"
                    FAILED_EXAMPLES=$((FAILED_EXAMPLES + 1))
                fi
            else
                echo -e "${YELLOW}! No 'main' executable found to run${NC}"
                # Still consider it a success if the build worked
                SUCCESSFUL_EXAMPLES=$((SUCCESSFUL_EXAMPLES + 1))
            fi
            ;;
    esac
    
    # Return to script directory
    cd "$SCRIPT_DIR"
    echo
done

# Print summary
echo "========================================"
echo -e "${GREEN}Testing Complete!${NC}"
echo "Total examples: $TOTAL_EXAMPLES"
echo "Successful: $SUCCESSFUL_EXAMPLES"
if [ $FAILED_EXAMPLES -gt 0 ]; then
    echo -e "${RED}Failed: $FAILED_EXAMPLES${NC}"
else
    echo "Failed: $FAILED_EXAMPLES"
fi
if [ $SKIPPED_EXAMPLES -gt 0 ]; then
    echo -e "${YELLOW}Skipped: $SKIPPED_EXAMPLES${NC}"
else
    echo "Skipped: $SKIPPED_EXAMPLES"
fi

if [ $FAILED_EXAMPLES -eq 0 ]; then
    echo -e "\n${GREEN}All tests completed successfully!${NC}"
    exit 0
else
    echo -e "\n${RED}Some tests failed!${NC}"
    exit 1
fi 