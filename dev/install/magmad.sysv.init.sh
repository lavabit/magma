#! /bin/sh
#
# chkconfig: - 95 05
# description:  The magma daemon provides automagically encrypted email services.
# processname: magmad
# pidfile: /var/run/magmad/magmad.pid
# config: /etc/magmad.config

# Source function library.
. /etc/init.d/functions

# Check that networking is up.
. /etc/sysconfig/network

if [ "$NETWORKING" = "no" ]; then
    exit 0
fi

user=magma
prog="/usr/libexec/magmad"
libfile="/usr/libexec/magmad.so"
configfile="/etc/magmad.config"
lockfile="/var/lock/subsys/magmad"
pidfile="/var/run/magmad/magmad.pid"

# Make sure magmad and magmad.so are installed.
test -x ${prog} || { echo "Could not find the Magma executable ${prog}" 1>&2 ; exit 5 ; }
test -x ${libfile} || { echo "Could not find the Magma shared library ${libfile}" 1>&2 ; exit 5 ; }

# Make sure magmad config file is available.
test -f ${configfile} || { echo "Could not find the Magma config file ${configfile}" 1>&2 ; exit 5 ; }

start () {
    echo -n $"Starting `basename $prog`: "
    # Ensure that /var/run/magmad has proper permissions
    if [ "`stat -c %U /var/run/magmad`" != "$user" ]; then
        chown -R $user /var/run/magmad
    fi
    cd /var/lib/magma
    daemon "${prog} ${configfile} > /var/log/magma/magmad.init.log"
    RETVAL=$?
    echo
    [ $RETVAL -eq 0 ] && touch ${lockfile}
    [ $RETVAL -eq 0 ] && pidof ${prog} > ${pidfile}
    [ $RETVAL -ne 0 -a ! -z "${BOOTUP:-}" ] && cat /var/log/magma/magmad.init.log
}

stop () {
    echo -n $"Stopping `basename $prog`: "
    killproc -p ${pidfile} ${prog}
    RETVAL=$?
    echo
    if [ $RETVAL -eq 0 ]; then
        rm -f ${lockfile} ${pidfile}
    fi
}

reload () {
    echo -n $"Reloading `basename $prog`: "
    killproc -p ${pidfile} ${prog} -HUP
    RETVAL=$?
    echo
}

restart () {
    stop
    start
}

# See how we were called.
case "$1" in
  start)
    start
    ;;
  stop)
    stop
    ;;
  status)
    status -p ${pidfile} magmad
    RETVAL=$?
    ;;
  reload)
    reload
    ;;
  restart|force-reload)
    restart
    ;;
  condrestart|try-restart)
    [ -f ${lockfile} ] && restart || :
    ;;
  *)
    echo $"Usage: $0 {start|stop|status|restart|reload|force-reload|condrestart|try-restart}"
    RETVAL=2
    ;;
esac

exit $RETVAL
