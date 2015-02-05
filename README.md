# Description

Magma was originally designed and developed by Ladar Levison for lavabit.com. This Magma Classic release is the based on the development branch for Lavabit's service. Because we started with the development branch, some of the features and functions are unstable. It needs work. Happy hacking.

# Downloads

##### Magma Classic v6.0.1

https://darkmail.info/downloads/magma-classic-6.0.1.tar.gz     
https://darkmail.info/downloads/magma-classic-6.0.1.tar.gz.sha512

5bbafc8778ef1da9a4afc1c8cb8b17dca19c98d370d9fbdbc1ee6f136abaed76a04d86df4d4b63660e4c79cc261d22fbca781e775f7db9ed83de9358c62fb681

##### Magma Development Machine, v1.0.0  

https://darkmail.info/downloads/dark-mail-development-machine-1.0.0.tar.gz
https://darkmail.info/downloads/dark-mail-development-machine-1.0.0.tar.gz.sha512
https://darkmail.info/downloads/dark-mail-development-machine-1.0.0.torrent

5bbafc8778ef1da9a4afc1c8cb8b17dca19c98d370d9fbdbc1ee6f136abaed76a04d86df4d4b63660e4c79cc261d22fbca781e775f7db9ed83de9358c62fb681

# Credits

Greg Brown  
Ivan Tolkachav  
Ladar Levison  
Princess Levison  
Ryan Crites  
Sean Benson  
Stephen Watt

And the army of Kickstarter supporters who contributed to this project.

# Tarball Contents

magma.distribution/  
    bin/  
	check/  
	docs/  
	lib/  
	res/  
	scripts/  
	src/  
	tools/  
	web/  
	INSTALL  
	README  

# Installation Instructions

These instructions are targeted at systems running CentOS 6.6.

### Prerequisites

List of packages to be installed: mysql-server, memcached, gettext-devel

**MySQL**  

To install MySQL and configure the magma username run the commands below. The supplied password should be replaced with value unique to your environment.  
  
```shell
yum install mysql-server
service mysqld start
chkconfig mysqld on

mysql -u root < "create user 'magma'@'localhost' identified by 'volcano';"
mysql -u root < "grant all privileges on *.* to 'magma'@'localhost' with grant;"
```

**Memcached**  

To install Memcached run the commands below.

```shell
yum install memcached
service memcached start
chkconfig memcached on
```

**EnvSubst**
  
```shell
yum install gettext-devel
```

**ClamAV**  

The freshen.clamav.sh script will update the ClamAV definitions.

### Compiling

After running through the prerequisites above, load the project into Eclipse and build magma.so, magma.check and magma.

### Deploying    

Run the INSTALL script.

```shell
./INSTALL -d ~/ -u magma -p volcano -s Lavabit
```

### Development 

To link up the development scripts run linkup.sh. 

```shell
magma.distribution/scripts/linkup.sh
```

To launch magma using the sandbox configuration.

```shell
magma.run
```

# Webmail

Inside the resources/pages/ directory is a copy of the webmail code. Locate 
script.js file and change the magma.portalUrl = true variable to false, and 
it will use a set of hard coded test requests/responses. Its useful for 
checking/developing the webmail code without involving a server. Currently 
the files are configured to access the JSON-RPC interface using the hostname 
"localhost" and the HTTP port 10000. This should work using the default magma.config file.




