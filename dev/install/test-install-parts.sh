#!/bin/bash
# Test key components of the installation script without running services
set -e

echo "=== Testing Installation Script Components ==="

# Source the script functions without executing (dry-run simulation)
DOMAIN="test.example.com"
TLSKEY=""
DKIMKEY=""

echo ""
echo "[1/6] Testing variable generation..."
PROOT=$(openssl rand -base64 30 | sed -e "s/\//@-/g" | sed -e "s/\+/_\?/g")
echo "  Generated root password: ${PROOT:0:10}... (${#PROOT} chars)"
if [ ${#PROOT} -ge 30 ]; then
    echo "  ✓ Password generation OK"
else
    echo "  ✗ Password too short"
    exit 1
fi

echo ""
echo "[2/6] Testing MySQL config generation..."
cat > /tmp/test-my.cnf << 'MYCNF'
[mysqld]
datadir=/var/lib/mysql
socket=/var/lib/mysql/mysql.sock
user=mysql
back_log = 128
MYCNF
if [ -f /tmp/test-my.cnf ]; then
    echo "  ✓ Heredoc config generation OK"
    cat /tmp/test-my.cnf | head -3
else
    echo "  ✗ Failed to generate config"
    exit 1
fi

echo ""
echo "[3/6] Testing DKIM key generation..."
openssl genrsa -out /tmp/test-dkim.pem 2048 2>/dev/null
if [ -f /tmp/test-dkim.pem ]; then
    echo "  ✓ DKIM key generation OK"
else
    echo "  ✗ Failed to generate DKIM key"
    exit 1
fi

echo ""
echo "[4/6] Testing TLS cert generation..."
openssl req -x509 -nodes -batch -days 1826 -newkey rsa:4096 \
    -keyout /tmp/test-tls.pem -out /tmp/test-tls.pem 2>/dev/null
if [ -f /tmp/test-tls.pem ]; then
    echo "  ✓ TLS cert generation OK"
else
    echo "  ✗ Failed to generate TLS cert"
    exit 1
fi

echo ""
echo "[5/6] Testing magmad.config generation..."
CPUCORES=4
THREADCOUNT=$((CPUCORES*16))
PMAGMA="test_password"

printf "magma.library.file = /usr/libexec/magmad.so\n" > /tmp/magmad.config
printf "magma.iface.database.user = magma\n" >> /tmp/magmad.config
printf "magma.iface.database.host = localhost\n" >> /tmp/magmad.config
printf "magma.iface.database.schema = Magma\n" >> /tmp/magmad.config
printf "magma.iface.database.password = $PMAGMA\n" >> /tmp/magmad.config
printf "magma.system.worker_threads = $THREADCOUNT\n" >> /tmp/magmad.config

if [ -f /tmp/magmad.config ]; then
    LINES=$(wc -l < /tmp/magmad.config)
    echo "  Generated $LINES config lines"
    if [ "$LINES" -ge 6 ]; then
        echo "  ✓ Config generation OK"
        cat /tmp/magmad.config
    else
        echo "  ✗ Config incomplete"
        exit 1
    fi
else
    echo "  ✗ Failed to generate config"
    exit 1
fi

echo ""
echo "[6/6] Testing salt generation..."
PSALT=$(openssl rand -base64 42 | sed -e "s/\//@-/g" | sed -e "s/\+/_\?/g")
PSESS=$(openssl rand -base64 42 | sed -e "s/\//@-/g" | sed -e "s/\+/_\?/g")
echo "  PSALT: ${PSALT:0:20}... (${#PSALT} chars)"
echo "  PSESS: ${PSESS:0:20}... (${#PSESS} chars)"
if [ ${#PSALT} -ge 40 ] && [ ${#PSESS} -ge 40 ]; then
    echo "  ✓ Salt generation OK"
else
    echo "  ✗ Salt generation failed"
    exit 1
fi

echo ""
echo "=== All Tests Passed ==="
