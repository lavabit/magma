#!/bin/bash
# Test script to validate the installation script without running it fully
# This checks for syntax errors and common issues

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_SCRIPT="$SCRIPT_DIR/magmad.install.sh"

echo "=== Testing Installation Script ==="
echo ""

# Test 1: Bash syntax check
echo "[1/5] Checking bash syntax..."
if bash -n "$INSTALL_SCRIPT" 2>&1; then
    echo "  ✓ Syntax check passed"
else
    echo "  ✗ Syntax errors found"
    exit 1
fi

# Test 2: Check for common issues - undefined variables used
echo "[2/5] Checking for undefined variable usage..."
UNDEFINED_VARS=$(grep -n '\$[A-Z_]*[^A-Z_]' "$INSTALL_SCRIPT" | grep -v '^#' | grep -v ':-' | head -20 || true)
echo "  Variables found (verify these are defined before use):"
echo "$UNDEFINED_VARS" | head -5

# Test 3: Check heredoc syntax
echo "[3/5] Checking heredoc syntax..."
HEREDOC_COUNT=$(grep -c "<<" "$INSTALL_SCRIPT" || echo "0")
HEREDOC_END_COUNT=$(grep -cE "^(MYCNF|LOGROTATE|EOF)$" "$INSTALL_SCRIPT" || echo "0")
echo "  Heredoc starts: $HEREDOC_COUNT"
echo "  Heredoc ends: $HEREDOC_END_COUNT"
if [ "$HEREDOC_COUNT" -eq "$HEREDOC_END_COUNT" ]; then
    echo "  ✓ Heredocs appear balanced"
else
    echo "  ✗ Heredoc mismatch - check for unclosed heredocs"
fi

# Test 4: Check iptables commands
echo "[4/5] Checking iptables commands..."
IPTABLES_P=$(grep "iptables -P" "$INSTALL_SCRIPT" | grep -v "^#")
echo "  iptables -P commands:"
echo "$IPTABLES_P" | sed 's/^/    /'
# Check that each -P command only has one chain
BAD_IPTABLES=$(echo "$IPTABLES_P" | grep -E "iptables -P .+ .+ .+" || true)
if [ -z "$BAD_IPTABLES" ]; then
    echo "  ✓ iptables -P commands look correct"
else
    echo "  ✗ Found iptables -P with multiple chains (invalid):"
    echo "$BAD_IPTABLES"
fi

# Test 5: Check for file redirect issues
echo "[5/5] Checking config file redirects..."
# Look for patterns where > follows >> to same file
CONFIG_WRITES=$(grep -n ">> /etc/magmad.config\|> /etc/magmad.config" "$INSTALL_SCRIPT" || true)
echo "  magmad.config writes:"
echo "$CONFIG_WRITES" | sed 's/^/    /'

FIRST_WRITE=$(echo "$CONFIG_WRITES" | head -1)
if echo "$FIRST_WRITE" | grep -q "> /etc/magmad.config"; then
    echo "  ✓ First write uses > (creates file)"
else
    echo "  ⚠ First write should use > to create file"
fi

echo ""
echo "=== Validation Complete ==="
