<!-- vim: set filetype=markdown: -->

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
  
88d38c8c1f64fa03611f635ad478e75e7e3911a1717e5ca899ffd5f14bb1fa7083a1040aa5bb6bfd908796842a9eed0390afb8abd1ec9607227e8064d7115afa

# Credits

Greg Brown  
Ivan Tolkachev  
Ladar Levison  
Princess Levison  
Ryan Crites  
Sean Benson  
Stephen Watt

And the army of Kickstarter supporters who contributed to this project.

# Tarball Contents

```
magma/  
	bin/
	check/
	dev/
		docs/  
		sandbox/  
		scripts/  
		tools/    
	lib/  
	res/  
	src/  
	web/  
	configure.ac  
	INSTALL  
	Makefile.am
	README.md  
```

# Installation Instructions

These instructions are targeted at systems running CentOS 6.

### Prerequisites

List of packages to be installed: mysql-server, memcached, gettext-devel

**MySQL**  

To install MySQL and configure the magma username run the commands below. The supplied password should be replaced with value unique to your environment. You may also want to limit the permissions of the magma database user to the database it will need to access. The global permission is only needed to setup the table schema.
  
```shell
yum install mysql-server
service mysqld start
chkconfig mysqld on

mysql -u root < echo "CREATE USER 'magma'@'localhost' IDENTIFIED BY 'volcano';"
mysql -u root < echo "GRANT ALL PRIVILEGES ON *.* TO 'magma'@'localhost' WITH GRANT;"
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

The freshen.clamav.sh script will update, or download, the ClamAV virus definitions.

```shell
magma/dev/scripts/freshen/freshen.clamav.sh
```

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
magma/dev/scripts/linkup.sh
```

Development environments need to create the database manually.

```shell
# Usage: schema.init <mysql_user> <mysql_password> <mysql_schema>
schema.init magma volcano Lavabit
```

To launch magma using the sandbox configuration.

```shell
magma.run
```

# Webmail

Inside the resources/pages/ directory is a copy of the webmail code. Locate script.js file and change the magma.portalUrl = true variable to false, and it will use a set of hard coded test requests/responses. Its useful for checking/developing the webmail code without involving a server. Currently the files are configured to access the JSON-RPC interface using the hostname "localhost" and the HTTP port 10000. This should work using the default magma.config file.




