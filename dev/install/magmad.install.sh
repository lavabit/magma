#!/bin/bash

# Name: magmad.install.sh
# Author: Ladar Levison
#
# Description: A very rough installation script designed to install the various magma components. 



# The domain name this magmad instance is being configured to host.
DOMAIN="example.com"

# The [combined] TLS cert (including intermediate chain) and key file. Leave commented out to 
# generate a new key and self-signed cert.
# TLSKEY="/root/example.com.pem"

# The DKIM key file. Leave commented out to generate a new key and print the associated DNS record.
# DKIMKEY="/root/dkim.example.com.pem"










# Operating System Check
grep -E "CentOS release 6|Red Hat Enterprise.*release 6" /etc/system-release  >& /dev/null
if [ $? != 0 ]; then
	tput setaf 1; tput bold
	printf "This script is designed for CentOS/Red Hat Enterprise version 6.\n"
	tput sgr0
	exit 1
fi

# Update the system.
yum --assumeyes update

# Override the default run levels for the entropy gathering daemon. We'd like it to start before 
# OpenSSH and magmad, so those processes don't spend as much time waiting for randomness.
printf "# chkconfig: - 54 25\n" > /etc/chkconfig.d/haveged

# Install the EPEL repo.
yum --assumeyes --enablerepo=extras install epel-release

# Add the packages needed to compile/run magma.
yum --assumeyes install valgrind valgrind-devel texinfo autoconf automake libtool \
ncurses-devel gcc-c++ libstdc++-devel gcc cloog-ppl cpp glibc-devel glibc-headers \
kernel-headers libgomp mpfr ppl perl perl-Module-Pluggable perl-Pod-Escapes \
perl-Pod-Simple perl-libs perl-version patch sysstat perl-Time-HiRes cmake \
libbsd libbsd-devel inotify-tools haveged libarchive libevent memcached mysql \
mysql-server perl-DBI perl-DBD-MySQL git rsync perl-Git perl-Error perl-libintl \
perl-Text-Unidecode policycoreutils checkpolicy

# Configure the entropy gathering daemon to autostart, then launch it. Extra entropy will 
# speed a number of randomness intensive operations. 
/sbin/chkconfig haveged on
/sbin/service haveged start

# Create the following user to avoid spurious errors when compiling clamav.
useradd clamav
usermod --home /var/lib/clamav/ clamav
usermod --shell /sbin/nologin clamav

# Install the clamav package so we can use a distro version of freshclam.
yum --assumeyes install clamav clamav-db clamav-lib clamav-data clamav-update clamav-filesystem

# Use a more reliable freshclam.conf configuration.
printf "Bytecode yes\n" > /etc/freshclam.conf
printf "LogSyslog yes\n" >> /etc/freshclam.conf
printf "SafeBrowsing yes\n" >> /etc/freshclam.conf 
printf "LogFileMaxSize 8M\n" >> /etc/freshclam.conf 
printf "DatabaseOwner clam\n" >> /etc/freshclam.conf 
printf "CompressLocalDatabase no\n" >> /etc/freshclam.conf 
printf "DatabaseDirectory /var/lib/clamav\n" >> /etc/freshclam.conf 
printf "DatabaseMirror database.clamav.net\n" >> /etc/freshclam.conf 
printf "UpdateLogFile /var/log/clamav/freshclam.log\n" >> /etc/freshclam.conf 

# Copy the update script into cron.hourly directory so we update every hour, instead of once a day.
cp /etc/cron.daily/freshclam /etc/cron.hourly/

# Update the database.
/etc/cron.hourly/freshclam

# The commands, just in case you need to wipe an existing MySQL configuration and then initialize a virgin instance.
# rm -rf /var/lib/mysql/
# mkdir -p /var/lib/mysql/
# chown mysql:mysql /var/lib/mysql/
# chcon system_u:object_r:mysqld_db_t:s0 /var/lib/mysql/
# mysql_install_db
# service mysqld restart

yum --assumeyes install mysql mysql-server mariadb mariadb-server

# Configure the mysqld instance to autostart during boot, then start the daemon.
/sbin/chkconfig mysqld on
/sbin/service mysqld start

# Drop the test database.
mysqladmin --force=true --user=root drop test

# Create the magma database.
mysqladmin --force=true --user=root create Magma

# Configure the mysql root password to be a random base64 string, which will be at least 40 characters in length.
PROOT=`openssl rand -base64 30 | sed -e "s/\//@-/g" | sed -e "s/\+/_\?/g"`
mysqladmin --user=root password "$PROOT"

# Save the password so the root user can login without having to type it in.
printf "\n\n[mysql]\nuser=root\npassword=$PROOT\ndatabase=Magma\n\n" >> /root/.my.cnf 

# Find out how much RAM is installed, and what 50% would be in KB.
TOTALMEM=`free -k | grep -E "^Mem:" | awk -F' ' '{print $2}'`
HALFMEM=`echo $(($TOTALMEM/2))`

# Update the limits to allow users to lock 50% of memory.
printf "root     soft    memlock    $HALFMEM\n" > /etc/security/limits.d/50-magmad.conf
printf "root     hard    memlock    $HALFMEM\n\n" >> /etc/security/limits.d/50-magmad.conf
printf "magma    soft    memlock    $HALFMEM\n" >> /etc/security/limits.d/50-magmad.conf
printf "magma    hard    memlock    $HALFMEM\n\n" >> /etc/security/limits.d/50-magmad.conf

# Fix the SELinux context.
chcon system_u:object_r:etc_t:s0 /etc/security/limits.d/50-magmad.conf

# Disable IPv6 and the iptables module used to firewall IPv6.
/sbin/chkconfig ip6tables off
printf "\n\nnet.ipv6.conf.all.disable_ipv6 = 1\n" >> /etc/sysctl.conf

# Open up the firewall. 
iptables -P INPUT OUTPUT FORWARD
iptables -F
iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
iptables -A INPUT -p icmp -j ACCEPT
iptables -A INPUT -i lo -j ACCEPT
iptables -A INPUT -m state --state NEW -m tcp -p tcp --dport 22 -j ACCEPT
iptables -A INPUT -m state --state NEW -m tcp -p tcp --dport 25 -j ACCEPT
iptables -A INPUT -m state --state NEW -m tcp -p tcp --dport 53 -j ACCEPT
iptables -A INPUT -m state --state NEW -m udp -p udp --dport 53 -j ACCEPT
iptables -A INPUT -m state --state NEW -m tcp -p tcp --dport 80 -j ACCEPT
iptables -A INPUT -m state --state NEW -m tcp -p tcp --dport 110 -j ACCEPT
iptables -A INPUT -m state --state NEW -m tcp -p tcp --dport 143 -j ACCEPT
iptables -A INPUT -m state --state NEW -m tcp -p tcp --dport 443 -j ACCEPT
iptables -A INPUT -m state --state NEW -m tcp -p tcp --dport 465 -j ACCEPT
iptables -A INPUT -m state --state NEW -m tcp -p tcp --dport 587 -j ACCEPT
iptables -A INPUT -m state --state NEW -m tcp -p tcp --dport 993 -j ACCEPT
iptables -A INPUT -m state --state NEW -m tcp -p tcp --dport 995 -j ACCEPT
iptables -A INPUT -j REJECT --reject-with icmp-host-prohibited 
iptables -A FORWARD -j REJECT --reject-with icmp-host-prohibited 

# Save the new rules, and restart the firewall.
/sbin/service iptables save
/sbin/service iptables restart

# Find out how much RAM is installed, and what 25% would be in MB.
TOTALMEM=`free -m | grep -E "^Mem:" | awk -F' ' '{print $2}'`
QUARTERMEM=`echo $(($TOTALMEM/4))`

# Update the memcached config file, and allow memcached to use up to 25% of available RAM.
sed -i -e "s/CACHESIZE=\"[0-9]*\"/CACHESIZE=\"$QUARTERMEM\"/g" /etc/sysconfig/memcached

# Configure memcached to start during the system boot up. Then restart the daemon
# to capture the updated settings. Note the stop command may fail, if memcached isn't 
# already running.
/sbin/chkconfig memcached on
/sbin/service memcached stop
/sbin/service memcached start

# Install postfix for outbound relays.
yum --assumeyes install postfix

# Modify the selinux rules so that postfix may bind to port 2525.
checkmodule -M -m -o postfix.selinux.mod postfix.selinux.te
semodule_package -o postfix.selinux.pp -m postfix.selinux.mod
semodule -i postfix.selinux.pp

# Setup logrotate so it only stores 7 days worth of logs.
printf "/var/log/maillog {\n\tdaily\n\trotate 7\n\tmissingok\n}\n" > /etc/logrotate.d/postfix

# Fix the SELinux context for the postfix logrotate config.
chcon system_u:object_r:etc_t:s0 /etc/logrotate.d/postfix

# Configure postfix to listen for relays on port 2525 so it doesn't conflict with magma.
sed -i -e "s/^smtp\([ ]*inet\)/127.0.0.1:2525\1/" /etc/postfix/master.cf 

# Configure the postfix hostname and origin parameters.
printf "\nmyhostname = relay.$DOMAIN\nmyorigin = $DOMAIN\n" >> /etc/postfix/main.cf 

# Ensure postfix auto-starts during boot, and then launch the daemon.
/sbin/chkconfig postfix on
/sbin/service postfix stop 
/sbin/service postfix start

#############################################################################
# Compile magmad (if necessary).                                            #
#                                                                           #
# git clone https://github.com/lavabit/magma magma-develop                  #
# cd magma-develop                                                          #
# dev/scripts/builders/build.lib.sh all                                     #
# make all                                                                  #
#                                                                           #
#############################################################################

#############################################################################
# Install magmad.                                                           #
#############################################################################

# Create the magma user, so magmad can drop privileges and switch to this unprivileged.
useradd magma
usermod --home /var/lib/magma/ magma
usermod --shell /sbin/nologin magma

# Copy the magmad and magmad.so files to /usr/libexec.
cp magmad magmad.so /usr/libexec
chmod 755 /usr/libexec/magmad
chmod 755 /usr/libexec/magmad.so
chcon system_u:object_r:bin_t:s0 /usr/libexec/magmad
chcon system_u:object_r:bin_t:s0 /usr/libexec/magmad.so

# Create the magmad spool directory.
mkdir -p /var/spool/magma/data/
mkdir -p /var/spool/magma/scan/
chown -R magma:magma /var/spool/magma
chcon -R system_u:object_r:var_spool_t:s0 /var/spool/magma

# Create the magmad log directory.
mkdir -p /var/log/magma/
chown -R magma:magma /var/log/magma
chcon -R system_u:object_r:var_log_t:s0 /var/log/magma

# Create the magmad resources directory.
mkdir -p /var/lib/magma/resources/

# Copy in the resources.
cp -R res/fonts /var/lib/magma/resources 
cp -R res/pages /var/lib/magma/resources 
cp -R res/templates /var/lib/magma/resources 

# Create the magmad storage directories.
mkdir -p /var/lib/magma/storage/tanks/
mkdir -p /var/lib/magma/storage/local/

# Setup a cron job for purging stale magmad logs. Currently we purge anything older than 7 days.
printf "32 0 * * * root find /var/log/magma/ -name magmad.[0-9]*.log -mmin +8639 -exec rm --force {} \;\n" > /etc/cron.d/magma-log-cleanup
chmod 600 /etc/cron.d/magma-log-cleanup
chcon system_u:object_r:system_cron_spool_t:s0 /etc/cron.d/magma-log-cleanup

# Set magma as the owner, and fix the SELinux context for the magma directory.
chown -R magma:magma /var/lib/magma
chcon -R system_u:object_r:var_lib_t:s0 /var/lib/magma

# Set /var/lib/magma as the home directory for the magma user.
usermod --home /var/lib/magma/ magma
usermod --shell /sbin/nologin magma

# Create the directory used to hold the DKIM key.
mkdir -p /etc/pki/dkim/private/

# We need to copy the DKIM key file... or generate a new one.
if [[ "$DKIMKEY" != "" ]]; then
	DKIMFILE=`basename $DKIMKEY`
	cp $DKIMKEY /etc/pki/dkim/private/$DKIMFILE
else
	DKIMKEY="/etc/pki/dkim/private/dkim.$DOMAIN.pem"
	DKIMFILE=`basename $DKIMKEY`
	openssl genrsa -out "/etc/pki/dkim/private/$DKIMFILE" 2048
fi

# Fix the permissions and selinux context for the DKIM key.
chmod 600 "/etc/pki/dkim/private/$DKIMFILE"
chcon unconfined_u:object_r:cert_t:s0 "/etc/pki/dkim/private/$DKIMFILE"

# Calculate a selector based on the organizational domain name.
SELECTOR=`echo $DOMAIN | awk -F'.' '{ print $(NF-1) }'`

# Print the DKIM DNS record.
tput setaf 1; tput bold
printf "\n\nPublish the following record to ensure DKIM signatures operate properly.\n\n"
tput sgr0
openssl rsa -in "/etc/pki/dkim/private/$DKIMFILE" -pubout -outform PEM 2> /dev/null | \
sed -r "s/-----BEGIN PUBLIC KEY-----$//" | sed -r "s/-----END PUBLIC KEY-----//" | tr -d [:space:] | \
awk "{ print \"$SELECTOR._domainkey IN TXT \\\"v=DKIM1; k=rsa; p=\" substr(\$1, 1, 208) \"\\\" \\\"\" substr(\$1, 209) \"\\\" ; ----- DKIM $DOMAIN\" }"
printf "\n\n"

# We need to copy the TKS key file... or generate a new one.
if [[ "$TLSKEY" != "" ]]; then
	TLSFILE=`basename "$TLSKEY"`
	cp "$TLSKEY" "/etc/pki/tls/private/$TLSFILE"
else
	TLSKEY="/etc/pki/tls/private/$DOMAIN.pem"
	TLSFILE=`basename "$TLSKEY"`
	openssl req -x509 -nodes -batch -days 1826 -newkey rsa:4096 -keyout "$TLSKEY" -out "$TLSKEY"
fi

# Fix the permissions and selinux context for the TLS key.
chmod 600 /etc/pki/tls/private/$TLSFILE
chcon unconfined_u:object_r:cert_t:s0 /etc/pki/tls/private/$TLSFILE

# In case an existing magma user needs to be removed.
# mysql --execute="DROP USER 'magma'@'localhost'" 

# Generate another random string for use as the magma database user password.
PMAGMA=`openssl rand -base64 30 | sed -e "s/\//@-/g" | sed -e "s/\+/_\?/g"`

# Create the magma user and grant the required permissions.
mysql --execute="CREATE USER 'magma'@'localhost' IDENTIFIED BY '$PMAGMA'"
mysql --execute="GRANT ALL ON *.* TO 'magma'@'localhost'"

# Initialize the new database schema.
dev/scripts/database/schema.init.sh magma "$PMAGMA" Magma

# Figure out how many CPUs are on the host so we know how many worker threads to spawn.
CPUCORES=`nproc --all`
THREADCOUNT=`echo $(($CPUCORES*16))`

# Write the database config information out to the magmad config file.
printf "magma.library.file = /usr/libexec/magmad.so\n" >> /etc/magmad.config
printf "magma.iface.database.user = magma\n" > /etc/magmad.config
printf "magma.iface.database.host = localhost\n" >> /etc/magmad.config
printf "magma.iface.database.schema = Magma\n" >> /etc/magmad.config
printf "magma.iface.database.password = $PMAGMA\n" >> /etc/magmad.config
printf "magma.iface.database.socket_path = /var/lib/mysql/mysql.sock\n" >> /etc/magmad.config
printf "magma.iface.database.pool.connections = $CPUCORES\n\n" >> /etc/magmad.config
printf "magma.relay[1].port = 2525\n" >> /etc/magmad.config
printf "magma.relay[1].name = localhost\n" >> /etc/magmad.config
printf "magma.iface.cache.host[1].port = 11211\n" >> /etc/magmad.config
printf "magma.iface.cache.host[1].name = localhost\n\n" >> /etc/magmad.config
printf "magma.library.file = /usr/libexec/magmad.so\n" >> /etc/magmad.config
printf "magma.system.worker_threads = $THREADCOUNT\n" >> /etc/magmad.config
printf "magma.secure.memory.length = 268435456\n" >> /etc/magmad.config

# Generate another a random site specific salt value to protect legacy password hashes, and session tokens.
PSALT=`openssl rand -base64 42 | sed -e "s/\//@-/g" | sed -e "s/\+/_\?/g"`
PSESS=`openssl rand -base64 42 | sed -e "s/\//@-/g" | sed -e "s/\+/_\?/g"`

# Insert the global config options.
cat magmad.config.sql | \
sed -e "s/\$PSALT/$PSALT/g" | \
sed -e "s/\$PSESS/$PSESS/g" | \
sed -e "s/\$DOMAIN/$DOMAIN/g" | \
sed -e "s/\$TLSFILE/$TLSFILE/g" | \
sed -e "s/\$DKIMFILE/$DKIMFILE/g" | \
sed -e "s/\$SELECTOR/$SELECTOR/g" | \
mysql -u root Magma

# Install the Sys V init script.
cp magmad.sysv.init.sh /etc/init.d/magmad
chmod 755 /etc/init.d/magmad
chcon system_u:object_r:initrc_exec_t:s0 /etc/init.d/magmad

# Setup the run director used by the init script.
mkdir -p /var/run/magmad/
chown magma:magma /var/run/magmad/
chcon system_u:object_r:var_run_t:s0 /var/run/magmad/

# Add the magmad init script to the system configuration.
chkconfig --add magmad
chkconfig magmad on

