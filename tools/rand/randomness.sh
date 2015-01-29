#!/bin/bash

echo '#!/bin/bash
#
# rng-tool:     Daemon and tools to use a Hardware TRNG to feed /dev/random
#
# chkconfig: - 99 1
# description:  The rngd daemon acts as a bridge between a Hardware TRNG
#               (true random number generator) such as the ones in some
#               Intel/AMD/VIA chipsets, and the kernel PRNG (peusdo
#               (pseudo-random number generator).
# processname: /sbin/rngd
#
### BEGIN INIT INFO
# Provides: rngd
# Required-Start: $syslog
# Required-Start: $syslog
# Required-Stop:
# Default-Start: 2 3 5
# Default-Stop: 0 6
# Short-Description: Starts the rngd hardware random number generator
# Description:  The rngd daemon acts as a bridge between a Hardware TRNG
#               (true random number generator) such as the ones in some
#               Intel/AMD/VIA/Geode chipsets, and the kernel PRNG
#               (pseudo-random number generator).
### END INIT INFO
 
PIDFILE=/var/run/rngd.pid
KERNMODS=""
DEVICELIST=""
HRNGDEVICE=/dev/urandom
 
# Source function library.
. /etc/init.d/functions
 
[ -f /sbin/rngd ] || exit 1
 
probehardware() {
	for i in ${KERNMODS} ; do
		modprobe -q $i
	done
}
 
finddevice () {
        [ -c "${HRNGDEVICE}" ] && return 0
        for i in ${DEVICELIST} ; do
                if [ -c "/dev/$i" ] ; then
                        HRNGDEVICE="/dev/$i"
                        return 0
                fi
                if [ -c "/dev/misc/$i" ] ; then
                        HRNGDEVICE="/dev/misc/$i"
                        return 0
                fi
        done
 
        echo "(Hardware RNG device inode not found)"
        echo "$0: Cannot find a hardware RNG device to use." >&2
        exit 1
}
 
start() {
	echo "Probing for HRNG module"
	probehardware
	finddevice
        echo -n $"Starting rngd: "
	daemon /sbin/rngd --background --fill-watermark=4096 --rng-device=/dev/urandom --random-device=/dev/random
	RETVAL=$?
	pidof rngd > $PIDFILE
	echo
	return $RETVAL
}
 
stop() {
	echo -n $"Shutting down rngd service: "
	killproc rngd
	RETVAL=$?
	if [ $RETVAL -eq 0 ] ; then
	  rm -f /var/lock/subsys/rngd
	  rm -f $PIDFILE
	fi
	echo
	touch /var/lock/subsys/rngd
	return $RETVAL
}
 
restart() {
	stop
	start
}
 
RETVAL=0
 
# See how we were called.
case "$1" in
  start)
	start
	;;
  stop)
	stop
	;;
  restart)
	restart
	;;
  condrestart)
        [ -f /var/lock/subsys/rngd ] && restart || :
	;;
  *)
	echo $"Usage: $0 {start|stop|restart|condrestart}"
	exit 1
esac
 
exit $?
' > /etc/init.d/rngd

/bin/chmod 755 /etc/init.d/rngd
/sbin/chkconfig --add rngd
/sbin/chkconfig rngd on
/sbin/service rngd start

