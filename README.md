# Description

Magma was originally designed and developed by Ladar Levison for lavabit.com. The current release is currently under heavy development, and some of the features and functions contained herein are unstable. The SMTP, POP, and IMAP protocol handlers are reasonably mature. The DMTP and HTTP (and the bundled webmail system) are still in development. Happy hacking.

# Downloads

##### Magma Classic v6.0.1

https://darkmail.info/downloads/magma-classic-6.0.1.tar.gz    
https://darkmail.info/downloads/magma-classic-6.0.1.tar.gz.sha256    

d4c5b2d3e435eaae5a1c320cec5ec44db3818c67fe23819abcecab7d223e482e

##### Magma Development Machine, v1.0.0

The development machine is a pre-built virtual machine with a graphical desktop and various development tools, and dependencies installed.

https://darkmail.info/downloads/dark-mail-development-machine-1.0.0.tar.gz    
https://darkmail.info/downloads/dark-mail-development-machine-1.0.0.tar.gz.sha256    

33808e4ed81859cb076ae879fed7ad85164a2561a1b1cd96f66f65f7e3bf7bd7

##### Magma Build Machines, v0.0.8

For those looking for a slim virtual machine pre-configured to build and run magma, consider the following Vagrant boxes which have been created specifically for that purpose. Images have been created to support the VirtualBox, VMware, and libvirt providers. An official Docker image is on the roadmap, but for the time being you might want to consider one of the community supported images. Use the appropriate command below to download and provision a Vagrant instance.

```shell
# VMware
vagrant init lavabit/magma; vagrant up --provider vmware_desktop

# VirtualBox
vagrant init lavabit/magma; vagrant up --provider virtualbox

# libvirt
vagrant init lavabit/magma; vagrant up --provider libvirt
```

Or you can download the Vagrant box image manually.

https://darkmail.info/downloads/magma-centos-vmware.box   
https://darkmail.info/downloads/magma-centos-vmware.box.sha256    

7ab90989877c3d996b015391819aa3872328bda9b97d8f252cef1dd6ac5a7915

https://darkmail.info/downloads/magma-centos-virtualbox.box    
https://darkmail.info/downloads/magma-centos-virtualbox.box.sha256    

a134a57886ccec5690c143dceda0d48dc5f7cddb3c5944ae4d65ee78a280970b

https://darkmail.info/downloads/magma-centos-libvirt.box    
https://darkmail.info/downloads/magma-centos-libvirt.box.sha256    

b9339d535a3e0630f84152e272570e0af082124a4023e76cc8ea30390ef54ff5   

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
	check/
	dev/
		docs/
		scripts/
		tools/
			cryptex/
			mason/
			pwtool/
			rand/
			runner/
			stringer/
			testde/
	lib/
	res/
	sandbox/
	src/
	web/
	COPYRIGHT
	INSTALL
	LICENSE
	Makefile
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

### Compiling

To link up the development and build scripts run the linkup.sh. This will create a bin folder in your home directory, if it doesn't already exist, and create symbolic links to the scripts and tools used to build, run and test magma. The commands below assume the bin directory is in your PATH. If it isn't, or you simply don't want to create the symbolic links, you can also run the shell scripts directly from their location in the dev/scripts folder. To execute the linkup.sh script:


```shell
magma/dev/scripts/linkup.sh
```

To build the dependencies and create the magmad.so library, run the build.lib script. Run the script without any parameters to see the possible command line options. To run all of the steps in a single operation:


```shell
build.lib all
```

Once the dependencies have been compiled the bundled Makefile can be used to compile magma. It points to the relevant folders created by the previous step when searching for the necessary header files. If the Makefile has trouble finding the necessary include files, odds are its because the previous step hasn't been run. To compile magmad and magmad.check:

```shell
build.magma
build.check
```

To configure a sandbox database which can be used to run the unit tests, or experiment with magma, run:

```shell
schema.reset
```

To launch the magma unit tests, or magma using the sandbox configuration, run:

```shell
check.run
magma.run
```

To download the ClamAV virus definitions into the sandbox environment, run:

```shell
freshen.clamav
```

### Deploying

To deploy magma, run the INSTALL script. Note the script is slightly out of date, and may need to tweaking to operate perfectly against a copy of the current magma development branch cloned directly via git.

```shell
./INSTALL -d ~/ -u magma -p volcano -s Lavabit
```

### Development

The best way to get an issue fixed is to create a pull request with a unit test added to the check folder which reproduces the issue and checks for the expected output. In general, please be sure to run the check.vg and magma.vg scripts before creating a pull request to make sure the newly submitted code doesn't introduce a memory leak, or invalid memory operation.


# Webmail

Inside the res/pages/webmail directory is a compiled copy of the webmail code. Locate script.js file and change the magma.portalUrl = true variable to false, and it will use a set of hard coded test requests/responses. These hard coded requests, and responses are useful for checking/developing the webmail code without a running version of the magma server. Currently the files are configured to access the JSON-RPC interface using the hostname "localhost" and the HTTP port 10000. This should work using the default magma.config and magma.sandbox.config files.

The static files inside the res/pages/webmail folder are compiled using the files inside the web directory. See the web/WORKFLOW.md file for details.




