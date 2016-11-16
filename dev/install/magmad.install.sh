#!/bin/bash

# The domain name this magmad instance is being configured to host.
DOMAIN="example.com"

# Where can we find the [combined] TLS cert (including intermediate chain) and key file.
TLSKEY="/root/example.com.pem"

# Install the EPEL repo.
yum --assumeyes --enablerepo=extras install epel-release

# Add the packages needed to compile/run magma.
yum --assumeyes install valgrind valgrind-devel texinfo autoconf automake libtool \
ncurses-devel gcc-c++ libstdc++-devel gcc cloog-ppl cpp glibc-devel glibc-headers \
kernel-headers libgomp mpfr ppl perl perl-Module-Pluggable perl-Pod-Escapes \
perl-Pod-Simple perl-libs perl-version patch sysstat perl-Time-HiRes cmake \
libbsd libbsd-devel inotify-tools haveged libarchive libevent memcached mysql \
mysql-server perl-DBI perl-DBD-MySQL git rsync perl-Git perl-Error

# Create the magma user, so magmad can drop its root privileges and suid to this account.
useradd magma
usermod --home /var/lib/magma/ magma
usermod --shell /sbin/nologin magma

# Create the following user to avoid spurious errors when compiling clamav.
useradd clamav
usermod --home /var/lib/clamav/ clamav
usermod --shell /sbin/nologin clamav

# Install the clamav package so we can use a distro version of freshclam.
yum --assumeyes install clamav clamav-db

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

# Configure these daemons to autostart at boot, then launch them manually. 
/sbin/chkconfig haveged on
/sbin/service haveged start
/sbin/service memcached start

# The commands, just in case you need to wipe an existing MySQL configuration and then initialize a virgin instance.
# rm -rf /var/lib/mysql/
# mkdir -p /var/lib/mysql/
# chown mysql:mysql /var/lib/mysql/
# chcon system_u:object_r:mysqld_db_t:s0 /var/lib/mysql/
# mysql_install_db
# service mysqld restart

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
#mysqladmin --user=root --password="$PROOT" -h `hostname` password "$PROOT"

# Save the password so the root user can login without having to type it in.
printf "\n\n[mysql]\nuser=root\npassword=$PROOT\n\n" >> /root/.my.cnf 

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

##############################################
# Compile magmad (if necessary).             #
##############################################

git clone https://github.com/lavabit/magma magma-develop
cd magma-develop
dev/scripts/builders/build.lib.sh all
make all


##############################################
# Install magmad.                            #
##############################################

# Copy the magmad and magmad.so files to /usr/libexec
cp magmad magmad.so /usr/libexec
chmod 755 /usr/libexec/magmad
chmod 755 /usr/libexec/magmad.so
chcon system_u:object_r:bin_t:s0 /usr/libexec/magmad
chcon system_u:object_r:bin_t:s0 /usr/libexec/magmad.so

# Create the magmad spool directory.
mkdir -p /var/spool/magma/data
chown -R magma:magma /var/spool/magma
chcon -R system_u:object_r:var_spool_t:s0 /var/spool/magma

# Create the magmad log directory.
mkdir -p /var/log/magma
chown -R magma:magma /var/log/magma
chcon -R system_u:object_r:var_log_t:s0 /var/log/magma

# Create the magmad resources directory.
mkdir -p /var/lib/magma/resources

# Copy in the resources.
cp -R res/fonts /var/lib/magma/resources 
cp -R res/pages /var/lib/magma/resources 
cp -R res/templates /var/lib/magma/resources 

# Create the magmad storage directories.
mkdir -p /var/lib/magma/storage/tanks
mkdir -p /var/lib/magma/storage/local

# Set magma as the owner, and fix the SELinux context for the magma directory.
chown -R magma:magma /var/lib/magma
chcon -R system_u:object_r:var_lib_t:s0 /var/lib/magma

# Set /var/lib/magma as the home directory for the magma user.
usermod --home /var/lib/magma/ magma
usermod --shell /sbin/nologin magma

# Place the TLS cert/key file in the pki directory.
TLSFILE=`basename $TLSKEY`
cp $TLSKEY /etc/pki/tls/private/$TLSFILE
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
printf "magma.iface.database.user = mytool\n" > /etc/magmad.config
printf "magma.iface.database.host = localhost\n" >> /etc/magmad.config
printf "magma.iface.database.schema = Magma\n" >> /etc/magmad.config
printf "magma.iface.database.password = $PMAGMA\n" >> /etc/magmad.config
printf "magma.iface.database.socket_path = /var/lib/mysql/mysql.sock\n\n" >> /etc/magmad.config
printf "magma.iface.cache.host[1].name = localhost\n" >> /etc/magmad.config
printf "magma.iface.cache.host[1].port = 11211\n" >> /etc/magmad.config
printf "magma.relay[1].name = localhost\n" >> /etc/magmad.config
printf "magma.relay[1].port = 2525\n" >> /etc/magmad.config
printf "magma.library.file = /usr/libexec/magmad.so\n" >> /etc/magmad.config
printf "magma.system.worker_threads = $THREADCOUNT\n" >> /etc/magmad.config
printf "magma.secure.memory.length = 268435456\n" >> /etc/magmad.config

# Insert the global config options.
cat magmad.config.sql | sed -e "s/\$DOMAIN/$DOMAIN/g" | mysql -u root Magma
