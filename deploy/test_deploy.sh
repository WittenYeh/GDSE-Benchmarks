#!/bin/bash
# Test deployment script (dry run - no actual push to GitHub)
# Usage: ./test_deploy.sh

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Deployment Test (Dry Run)${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Check plots directory
if [ ! -d "plots" ]; then
    echo "❌ Error: plots/ directory not found!"
    exit 1
fi

echo -e "${GREEN}✓${NC} Found plots directory"

# Count HTML files
HTML_COUNT=$(ls -1 plots/*.html 2>/dev/null | grep -v index.html | wc -l)
echo -e "${GREEN}✓${NC} Found $HTML_COUNT plot files"

# Generate index page
echo ""
echo -e "${YELLOW}📝 Generating index page...${NC}"
python3 deploy/generate_index.py

if [ ! -f "plots/index.html" ]; then
    echo "❌ Error: Failed to generate index.html"
    exit 1
fi

echo -e "${GREEN}✓${NC} Index page generated successfully"

# Show index page stats
INDEX_SIZE=$(du -h plots/index.html | cut -f1)
echo -e "${GREEN}✓${NC} Index page size: $INDEX_SIZE"

# List all files that would be deployed
echo ""
echo -e "${YELLOW}📋 Files ready for deployment:${NC}"
ls -lh plots/*.html | awk '{printf "   %s  %s\n", $5, $9}'

# Show total size
TOTAL_SIZE=$(du -sh plots/ | cut -f1)
echo ""
echo -e "${BLUE}Total deployment size: $TOTAL_SIZE${NC}"

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  ✅ Test Passed!${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "To actually deploy, run:"
echo -e "${BLUE}./deploy/deploy.sh${NC}"