#!/bin/bash
# Deploy benchmark visualizations to GitHub Pages
# Usage: ./deploy.sh [subdirectory]
# Example: ./deploy.sh benchmarks

set -e

# Configuration
GITHUB_REPO="wittenyeh.github.io"
GITHUB_USER="wittenyeh"
SUBDIRECTORY="${1:-gdse-benchmarks}"  # Default to 'gdse-benchmarks' if not specified

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Graph Database Benchmark Deployment${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Check if plots directory exists
if [ ! -d "plots" ]; then
    echo -e "${RED}❌ Error: plots/ directory not found!${NC}"
    echo "Please generate plots first using visualize scripts."
    exit 1
fi

# Check if there are any HTML files in plots
if [ -z "$(ls -A plots/*.html 2>/dev/null)" ]; then
    echo -e "${RED}❌ Error: No HTML files found in plots/ directory!${NC}"
    echo "Please generate plots first using visualize scripts."
    exit 1
fi

echo -e "${GREEN}✓${NC} Found plots directory"

# Generate index page
echo ""
echo -e "${YELLOW}📝 Generating index page...${NC}"
python3 deploy/generate_index.py

if [ ! -f "plots/index.html" ]; then
    echo -e "${RED}❌ Error: Failed to generate index.html${NC}"
    exit 1
fi

echo -e "${GREEN}✓${NC} Index page generated"

# Create temporary directory for deployment
TEMP_DIR=$(mktemp -d)
echo ""
echo -e "${YELLOW}📦 Preparing deployment package...${NC}"
echo "   Temp directory: $TEMP_DIR"

# Clone the GitHub Pages repository
echo ""
echo -e "${YELLOW}📥 Cloning GitHub Pages repository...${NC}"
git clone "https://github.com/${GITHUB_USER}/${GITHUB_REPO}.git" "$TEMP_DIR" 2>&1 | grep -v "Cloning into" || true

if [ ! -d "$TEMP_DIR/.git" ]; then
    echo -e "${RED}❌ Error: Failed to clone repository${NC}"
    echo "Please check:"
    echo "  1. Repository exists: https://github.com/${GITHUB_USER}/${GITHUB_REPO}"
    echo "  2. You have access to the repository"
    echo "  3. Git credentials are configured"
    rm -rf "$TEMP_DIR"
    exit 1
fi

echo -e "${GREEN}✓${NC} Repository cloned"

# Create subdirectory if it doesn't exist
mkdir -p "$TEMP_DIR/$SUBDIRECTORY"

# Copy all HTML files from plots to the subdirectory
echo ""
echo -e "${YELLOW}📋 Copying plot files...${NC}"
cp plots/*.html "$TEMP_DIR/$SUBDIRECTORY/"

# Count files
FILE_COUNT=$(ls -1 "$TEMP_DIR/$SUBDIRECTORY"/*.html | wc -l)
echo -e "${GREEN}✓${NC} Copied $FILE_COUNT HTML files to $SUBDIRECTORY/"

# Change to temp directory
cd "$TEMP_DIR"

# Configure git if needed
git config user.name "Benchmark Deploy Bot" 2>/dev/null || true
git config user.email "deploy@benchmark.local" 2>/dev/null || true

# Add files
echo ""
echo -e "${YELLOW}📤 Committing changes...${NC}"
git add "$SUBDIRECTORY"/*.html

# Check if there are changes to commit
if git diff --staged --quiet; then
    echo -e "${YELLOW}⚠️  No changes to deploy (files are identical)${NC}"
    cd - > /dev/null
    rm -rf "$TEMP_DIR"
    exit 0
fi

# Commit
TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')
git commit -m "Update benchmark visualizations - $TIMESTAMP"

echo -e "${GREEN}✓${NC} Changes committed"

# Push to GitHub
echo ""
echo -e "${YELLOW}🚀 Pushing to GitHub Pages...${NC}"
echo "   Repository: https://github.com/${GITHUB_USER}/${GITHUB_REPO}"
echo "   Branch: main (or master)"

if git push origin HEAD; then
    echo -e "${GREEN}✓${NC} Successfully pushed to GitHub"
else
    echo -e "${RED}❌ Error: Failed to push to GitHub${NC}"
    echo "Please check your GitHub credentials and repository access."
    cd - > /dev/null
    rm -rf "$TEMP_DIR"
    exit 1
fi

# Clean up
cd - > /dev/null
rm -rf "$TEMP_DIR"

# Success message
echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  ✅ Deployment Successful!${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo -e "Your visualizations are now available at:"
echo -e "${BLUE}https://${GITHUB_USER}.github.io/${SUBDIRECTORY}/${NC}"
echo ""
echo -e "Index page:"
echo -e "${BLUE}https://${GITHUB_USER}.github.io/${SUBDIRECTORY}/index.html${NC}"
echo ""
echo -e "${YELLOW}Note: It may take a few minutes for GitHub Pages to update.${NC}"