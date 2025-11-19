#!/bin/bash

# R-TYPE Documentation Script
# Build and serve project documentation using MkDocs

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
DOCS_DIR="$PROJECT_ROOT/docs"

# Default action
ACTION="serve"

# Help message
show_help() {
    cat << EOF
Usage: $0 [COMMAND] [OPTIONS]

Build and serve R-TYPE documentation using MkDocs.

COMMANDS:
    serve           Start development server (default)
    build           Build static site
    deploy          Deploy to GitHub Pages
    install         Install MkDocs dependencies
    clean           Remove built documentation

OPTIONS:
    -p, --port PORT     Port for dev server (default: 8000)
    -a, --addr ADDR     Address to bind (default: 127.0.0.1)
    -h, --help          Show this help message

EXAMPLES:
    $0                  # Start dev server
    $0 build            # Build documentation
    $0 serve -p 9000    # Serve on port 9000
    $0 install          # Install dependencies
    $0 deploy           # Deploy to GitHub Pages

EOF
}

# Check if MkDocs is installed
check_mkdocs() {
    if ! command -v mkdocs &> /dev/null; then
        echo -e "${YELLOW}MkDocs not found!${NC}"
        echo -e "${BLUE}Install with: ./scripts/docs.sh install${NC}"
        exit 1
    fi
}

# Install MkDocs and dependencies
install_deps() {
    echo -e "${BLUE}Installing MkDocs dependencies...${NC}\n"
    
    if ! command -v pip3 &> /dev/null; then
        echo -e "${RED}Error: pip3 not found${NC}"
        echo "Install Python 3 and pip first"
        exit 1
    fi
    
    pip3 install -r "$DOCS_DIR/requirements.txt"
    
    echo -e "\n${GREEN}✓ Dependencies installed successfully!${NC}"
}

# Serve documentation locally
serve_docs() {
    check_mkdocs
    
    local port=8000
    local addr="127.0.0.1"
    
    # Parse options
    while [[ $# -gt 0 ]]; do
        case $1 in
            -p|--port)
                port="$2"
                shift 2
                ;;
            -a|--addr)
                addr="$2"
                shift 2
                ;;
            *)
                shift
                ;;
        esac
    done
    
    echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║    Starting MkDocs Dev Server          ║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════╝${NC}\n"
    
    echo -e "${GREEN}Server running at: http://$addr:$port${NC}"
    echo -e "${YELLOW}Press Ctrl+C to stop${NC}\n"
    
    cd "$PROJECT_ROOT"
    mkdocs serve -a "$addr:$port"
}

# Build documentation
build_docs() {
    check_mkdocs
    
    echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║        Building Documentation          ║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════╝${NC}\n"
    
    cd "$PROJECT_ROOT"
    mkdocs build --clean
    
    echo -e "\n${GREEN}✓ Documentation built successfully!${NC}"
    echo -e "${BLUE}Output directory: $PROJECT_ROOT/site${NC}"
}

# Deploy to GitHub Pages
deploy_docs() {
    check_mkdocs
    
    echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║    Deploying to GitHub Pages           ║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════╝${NC}\n"
    
    echo -e "${YELLOW}This will deploy to the gh-pages branch${NC}"
    read -p "Continue? (y/N) " -n 1 -r
    echo
    
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo -e "${YELLOW}Deployment cancelled${NC}"
        exit 0
    fi
    
    cd "$PROJECT_ROOT"
    mkdocs gh-deploy --clean
    
    echo -e "\n${GREEN}✓ Documentation deployed!${NC}"
    echo -e "${BLUE}Visit: https://quent1111.github.io/R-TYPE/${NC}"
}

# Clean built documentation
clean_docs() {
    echo -e "${BLUE}Cleaning documentation build...${NC}"
    
    if [ -d "$PROJECT_ROOT/site" ]; then
        rm -rf "$PROJECT_ROOT/site"
        echo -e "${GREEN}✓ Removed site/ directory${NC}"
    else
        echo -e "${YELLOW}Nothing to clean${NC}"
    fi
}

# Main execution
cd "$PROJECT_ROOT"

# Parse command
if [[ $# -gt 0 ]]; then
    case $1 in
        serve)
            shift
            serve_docs "$@"
            ;;
        build)
            build_docs
            ;;
        deploy)
            deploy_docs
            ;;
        install)
            install_deps
            ;;
        clean)
            clean_docs
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown command: $1${NC}"
            show_help
            exit 1
            ;;
    esac
else
    # Default action: serve
    serve_docs
fi
