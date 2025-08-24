#!/bin/bash

# Final version of code formatting and checking script
# Based on guard_function/.clang-format and .arclint configuration

# Don't exit on error, handle errors manually

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color
BOLD='\033[1m'

target_dir="perception_app"

# Separator functions
print_separator() {
    echo -e "${BOLD}${CYAN}==========================================${NC}"
}

print_separator
echo -e "${BOLD}${CYAN}Code Formatting and Checking Tool${NC}"
echo -e "${BOLD}${CYAN}Based on Google C++ Style Guide${NC}"
print_separator
echo ""

# Check required tools
check_tools() {
    echo -e "${BOLD}${BLUE}🔧 Checking tools...${NC}"
    
    # Check clang-format
    if ! command -v clang-format &> /dev/null; then
        echo -e "${YELLOW}Installing clang-format...${NC}"
        sudo apt-get update && sudo apt-get install -y clang-format
    fi
    
    # Check cpplint
    if ! command -v cpplint &> /dev/null; then
        echo -e "${YELLOW}Installing cpplint...${NC}"
        pip install cpplint
    fi
    
    echo -e "${GREEN}✓ Tools ready${NC}"
    echo ""
}

# Format code
format_code() {
    echo -e "${BOLD}${BLUE}🎨 Formatting code...${NC}"
    
    # Find all C++ files
    FILES=$(find $target_dir -type f \( -name "*.cpp" -o -name "*.cc" -o -name "*.h" \))
    TOTAL_FILES=$(echo "$FILES" | wc -l)
    
    # Formatting counters
    FORMATTED_COUNT=0
    ERROR_COUNT=0
    PASSED_COUNT=0
    
    # Arrays to store file names
    FORMATTED_FILES=()
    ERROR_FILES=()
    
    # Process each file for formatting
    while IFS= read -r file; do
        if [ -f "$file" ]; then
            # Create temporary file
            temp_file=$(mktemp)
            
            # Format file
            if clang-format --style=file "$file" > "$temp_file" 2>/dev/null; then
                # Check if file has changes
                if ! cmp -s "$file" "$temp_file"; then
                    # Has changes, replace original file
                    mv "$temp_file" "$file"
                    echo -e " ${GREEN}[FORMATTED]${NC} $(basename "$file")"
                    FORMATTED_FILES+=("$(basename "$file")")
                    ((FORMATTED_COUNT++))
                else
                    rm "$temp_file"
                    ((PASSED_COUNT++))
                fi
            else
                echo -e " ${RED}[ERROR]${NC} $(basename "$file")"
                ERROR_FILES+=("$(basename "$file")")
                rm "$temp_file"
                ((ERROR_COUNT++))
            fi
        fi
    done <<< "$FILES"

    echo -e "${GREEN}✓ Formatted: $FORMATTED_COUNT files${NC}"
    echo -e "${YELLOW}- Passed: $PASSED_COUNT files${NC}"
    if [ $ERROR_COUNT -gt 0 ]; then
        echo -e "${RED}✗ Errors: $ERROR_COUNT files${NC}"
        if [ ${#ERROR_FILES[@]} -gt 0 ]; then
            echo -e "  ${RED}Failed: ${ERROR_FILES[*]}${NC}"
        fi
    fi
    echo ""
}

# Parse .arclint configuration
parse_arclint_config() {
    if [ ! -f ".arclint" ]; then
        echo -e "${RED}Error: .arclint file not found${NC}"
        return 1
    fi
    
    # Extract linelength from .arclint (simple parsing)
    LINELENGTH=$(grep -o '"linelength":[0-9]*' .arclint | grep -o '[0-9]*' || echo "120")
    echo -e "${CYAN}Line length limit: $LINELENGTH${NC}"
    
    # Build filter string from disabled rules
    FILTER_PARTS=()
    if grep -q '"build/c++11": "disabled"' .arclint; then
        FILTER_PARTS+=("-build/c++11")
    fi
    if grep -q '"build/include_order": "disabled"' .arclint; then
        FILTER_PARTS+=("-build/include_order")
    fi
    if grep -q '"runtime/references": "disabled"' .arclint; then
        FILTER_PARTS+=("-runtime/references")
    fi
    if grep -q '"legal/copyright": "disabled"' .arclint; then
        FILTER_PARTS+=("-legal/copyright")
    fi
    if grep -q '"whitespace/line_length": "disabled"' .arclint; then
        FILTER_PARTS+=("-whitespace/line_length")
    fi
    
    # Join filter parts
    if [ ${#FILTER_PARTS[@]} -gt 0 ]; then
        FILTER=$(IFS=','; echo "${FILTER_PARTS[*]}")
    else
        FILTER=""
    fi
    
    echo -e "${CYAN}Filter rules: $FILTER${NC}"
}

# Run cpplint check
run_cpplint_check() {
    echo -e "${BOLD}${BLUE}🔍 Running cpplint...${NC}"
    
    # Parse .arclint configuration
    parse_arclint_config
    
    # Find all C++ files in src directory
    FILES=$(find $target_dir -type f \( -name "*.h" -o -name "*.cc" -o -name "*.cpp" \))
    
    if [ -z "$FILES" ]; then
        echo -e "${YELLOW}No C++ files found.${NC}"
        return 0
    fi
    
    # Build cpplint command
    CMD="cpplint --root=\"$target_dir\""
    if [ -n "$FILTER" ]; then
        CMD="$CMD --filter=\"$FILTER\""
    fi
    CMD="$CMD --linelength=\"$LINELENGTH\""
    CMD="$CMD $FILES"
    
    echo -e "${CYAN}Running: $CMD${NC}"
    
    # Run cpplint
    cpplint_output=$(eval $CMD 2>&1)
    cpplint_exit_code=$?
    
    # Count errors from output
    error_count=$(echo "$cpplint_output" | grep -c "Total errors found:" || echo "0")
    if [ "$error_count" -gt 0 ]; then
        error_number=$(echo "$cpplint_output" | grep "Total errors found:" | sed 's/.*Total errors found: \([0-9]*\).*/\1/')
    else
        error_number=0
    fi
    
    # Display results
    if [ $cpplint_exit_code -eq 0 ]; then
        echo -e "${GREEN}✅ cpplint passed!${NC}"
    else
        echo -e "${RED}❌ cpplint found $error_number issues${NC}"
        # Filter output to show only error lines, not "Done processing" messages
        echo "$cpplint_output" | grep -v "Done processing" | grep -v "^$"
    fi
    echo ""
}

# Show final summary
show_final_summary() {
    print_separator
    echo -e "${BOLD}${GREEN}🎉 All operations completed!${NC}"
    
    # Show git status if available
    if command -v git &> /dev/null && [ -d ".git" ]; then
        echo -e "${CYAN}📝 Git Status:${NC}"
        git status --porcelain
    fi
    
    print_separator
}

# Main function
main() {
    check_tools
    format_code
    run_cpplint_check
    show_final_summary
}

# Run main function
main "$@" 