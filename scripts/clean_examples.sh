#!/bin/bash

# Clean script for IncludeBuild examples
# This script removes all build artifacts from example directories

set -e  # Exit on error
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${GREEN}IncludeBuild Examples Cleanup Script${NC}"
echo "========================================"
echo

# Print current directory for debugging
echo "Current directory: $(pwd)"

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
echo -e "${YELLOW}Files will be cleaned from the following examples:${NC}"
find "$EXAMPLES_DIR" -maxdepth 1 -type d | grep -v "^$EXAMPLES_DIR$" | sort | sed 's/^/  - /'
echo

# Save current directory
SCRIPT_DIR=$(pwd)

# Find all example directories
EXAMPLE_DIRS=$(find "${EXAMPLES_DIR}" -maxdepth 1 -type d | grep -v "^${EXAMPLES_DIR}$" || echo "")

if [ -z "$EXAMPLE_DIRS" ]; then
    echo -e "${RED}No example directories found in ${EXAMPLES_DIR}${NC}"
    exit 1
fi

# Track stats
TOTAL_CLEANED=0
TOTAL_FILES_REMOVED=0

for DIR in $EXAMPLE_DIRS; do
    EXAMPLE_NAME=$(basename $DIR)
    echo -e "${CYAN}Cleaning example: ${EXAMPLE_NAME}${NC}"
    
    # Enter example directory
    cd "$DIR"
    
    # Special handling for different examples
    case "$EXAMPLE_NAME" in
        "library")
            # Library example has more artifacts to clean
            echo "  Removing library files..."
            rm -rf lib buildobjects *.o *.a *.so *.exe test_logger run_test_* log_output.txt build 2>/dev/null || true
            ;;
            
        "game")
            # Game example has sound files to remove
            echo "  Removing game files..."
            rm -rf buildobjects main *.o *.exe *.wav *.ogg build 2>/dev/null || true
            ;;
            
        *)
            # Generic example cleanup
            echo "  Removing build artifacts..."
            rm -rf main buildobjects *.o *.exe build 2>/dev/null || true
            ;;
    esac
    
    # Count how many files were removed
    FILES_BEFORE=$(find . -type f | wc -l)
    FILES_AFTER=$(find . -type f -name "*.c" -o -name "*.h" -o -name "README.md" | wc -l)
    FILES_REMOVED=$((FILES_BEFORE - FILES_AFTER))
    
    if [ $FILES_REMOVED -gt 0 ]; then
        echo -e "  ${GREEN}Removed $FILES_REMOVED files/directories${NC}"
        TOTAL_FILES_REMOVED=$((TOTAL_FILES_REMOVED + FILES_REMOVED))
        TOTAL_CLEANED=$((TOTAL_CLEANED + 1))
    else
        echo -e "  ${YELLOW}No files to clean${NC}"
    fi
    
    # Return to script directory
    cd "$SCRIPT_DIR"
    echo
done

# Print summary
echo "========================================"
echo -e "${GREEN}Cleanup Complete!${NC}"
echo "Examples cleaned: $TOTAL_CLEANED"
echo "Total files/directories removed: $TOTAL_FILES_REMOVED"
echo -e "Examples are now ready for a fresh build"

exit 0
