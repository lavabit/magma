# -*- mode: ruby -*-
# vi: set ft=ruby :

# All Vagrant configuration is done below. The "2" in Vagrant.configure
# configures the configuration version (we support older styles for
# backwards compatibility). Please don't change it unless you know what
# you're doing.
Vagrant.configure(2) do |config|
  # The most common configuration options are documented and commented below.
  # For a complete reference, please see the online documentation at
  # https://docs.vagrantup.com.

  # Every Vagrant development environment requires a box. You can search for
  # boxes at https://atlas.hashicorp.com/search.
  config.vm.box = "chef/centos-6.6"

  # Disable automatic box update checking. If you disable this, then
  # boxes will only be checked for updates when the user runs
  # `vagrant box outdated`. This is not recommended.
  # config.vm.box_check_update = false

  # Create a forwarded port mapping which allows access to a specific port
  # within the machine from a port on the host machine. In the example below,
  # accessing "localhost:8080" will access port 80 on the guest machine.
  # config.vm.network "forwarded_port", guest: 80, host: 8080

  # Create a private network, which allows host-only access to the machine
  # using a specific IP.
  # config.vm.network "private_network", ip: "192.168.33.10"

  # Create a public network, which generally matched to bridged network.
  # Bridged networks make the machine appear as another physical device on
  # your network.
  # config.vm.network "public_network"

  # Share an additional folder to the guest VM. The first argument is
  # the path on the host to the actual folder. The second argument is
  # the path on the guest to mount the folder. And the optional third
  # argument is a set of non-required options.
  # config.vm.synced_folder "../data", "/vagrant_data"

  # Provider-specific configuration so you can fine-tune various
  # backing providers for Vagrant. These expose provider-specific options.
  # Example for VirtualBox:
  #
  # config.vm.provider "virtualbox" do |vb|
  #   # Display the VirtualBox GUI when booting the machine
  #   vb.gui = true
  #
  #   # Customize the amount of memory on the VM:
  #   vb.memory = "1024"
  # end
  #
  # View the documentation for the provider you are using for more
  # information on available options.

  # Define a Vagrant Push strategy for pushing to Atlas. Other push strategies
  # such as FTP and Heroku are also available. See the documentation at
  # https://docs.vagrantup.com/v2/push/atlas.html for more information.
  # config.push.define "atlas" do |push|
  #   push.app = "YOUR_ATLAS_USERNAME/YOUR_APPLICATION_NAME"
  # end

  config.vm.provision "shell", inline: <<-SHELL
    # Enable EHEL repos (libbsd-dev)
    wget http://download.fedoraproject.org/pub/epel/6/i386/epel-release-6-8.noarch.rpm
    sudo rpm -ivh epel-release-6-8.noarch.rpm

    # Install build dependencies
    sudo yum install autoconf
    sudo yum install automake
    sudo yum install libtool
    sudo yum install check-devel
    sudo yum install ncurses-devel # (mysql)
    sudo yum install gcc-c++
    sudo yum install libbsd-devel # (dkim)

    # Install test dependencies
    sudo yum install valgrind-devel

    # Install runtime dependencies, start them, and configure them to start on boot
    sudo yum install mysql-server
    sudo yum install memcached
    sudo /sbin/service mysqld start
    sudo chkconfig mysqld on
    sudo /sbin/service memcached start
    sudo chkconfig memcached on

    # Make sure we're in the source directory
    cd /vagrant

    # Set up database
    # Note that this will currently overwrite some sql files under version control
    ./scripts/database/schema.init.sh root "" Lavabit

    # Set up sandbox config
    cp res/config/magma.sandbox.config.vagrant res/config/magma.sandbox.config

    # Change memlock limits for mmap
    echo "* hard memlock 1024" | sudo tee -a /etc/security/limits.conf
    echo "* soft memlock 1024" | sudo tee -a /etc/security/limits.conf
  SHELL
end
