#Build the magma.classic project

## Load the Lavabit development environment


The Lavabit development environment is distributed via a VM that is referenced near the bottom of the <https://github.com/Lavabit/magma.classic> page, **Downloads section**, **Magma Development Machine, v1.0.0**. Download and install the VM into your favorite VM manager. These instructions have been tested with `VMWare Fusion Pro 7` in Yosemite OSX, but several other VM managers have been used successfully on different OS platforms. Note: credentials for the `magma` user and `root` user are in the VM info file.

## Remove the original magma installation

The VM comes loaded with an outdated version of the magma server that we'll remove prior to installing the newest magma server available on the master branch in the github repository. We start the cleanup by removing utility links to the supporting scripts; we'll recreate these links later. Note: If you've made use of this VM to customize your development environment, be careful not to remove any additional tools that you may have installed the `~/bin` directory. Use a terminal to remove all links in the user's `~/bin` directory:

	> cd ~; mv bin{,.z}; mkdir bin

Remove the original Eclipse projects from when this VM was created. Run Eclipse, remove all projects in the project view by highlighting, right-clicking and deleting the projects, complete with files. Exit Eclipse. 

Move the existing Eclipse workspace out of the way before installing a fresh github clone. In a terminal, notice there is a `~/Lavabit/magma.messages` directory. `Lavabit` is the default Eclipse workspace and `magma.messages` is a collection of files that aren't currently in github but will be used later when testing the magma server. For now, move the Eclipse workspace out of the way so Eclipse creates a default workspace from scratch on the next launch. 
 
	> cd ~; mv Lavabit{,.z}

A tweak is necessary in the Eclipse workspace prior to cloning the latest magma repository. Launch Eclipse. Close the `Welcome` tab if you haven't already. In the Project menu, deselect `Build Automatically`. Verify in a terminal that a new workspace directory has been created.

	> ls ~/Lavabit
	RemoteSystemsTempFiles

## Clone the latest magma.classic repo

We will use the Eclipse github plugin to clone the latest `magma.classic` repository from github. In Eclipse click the `Open Perspective` icon on the right side of the menu bar area. Select and add the `git` perspective. In the `Git Repositories` tab click the link `Clone a Git repository`, click `GitHub`, `Next`. Enter `magma.classic` in the search field, `Search`. Highlight the result for the search `lavabit/magma.classic`, `Next`, `Next`, For `Destination directory`, browse to `~magma/Lavabit`, `OK`, `OK`. Take the defaults for the other options, `Finish`. This process will currently import > 3000 objects. The Git repository view should still be open and now list the `magma.classic` repository. On the file system, verify that the github repository has been cloned to this location: 

	> cd ~/Lavabit/magma.classic/
	> ls -la
	total 64
	...
	drwxrwxr-x. 7 magma magma 4096 Jun 7 08:50 .git
	...

## Import the magma.classic projects

In Eclipse, using the `Git Repositories` perspective, right-click the `magma.classic` repo, select `Import Projects`, don't modify the defaults, `Next`. Deselect all, then select the projects `magma`, `magma.check`, `magma.classic`, `magma.so` and `magma.web`. Verify that `Search for nested projects` is selected. `Finish`. Note: In the lower right section of the Eclipse app the C/C++ Indexer is busy indexing the new project files.

In Eclipse, if the `C/C++` perspective is not already an option in the perspective buttons, select the `Open perspective` button and add the `C/C++` perspective. You can remove the `Java` perspective since it won't be used for this project. You should see the projects selected in the previous section in the `C/C++` perspective in the leftmost pane. The projects are:

- magma - the magmad daemon
- magma.check - for regression testing
- magma.classic - a view of the magma source code that is used to manage files on the github repository
- magma.so - shared library placeholder
- magma.web - web stuff

Exit Eclipse.

## Build the magmad.so library

Use a terminal to perform these steps and to kick off the build of the `magmad.so` shared library. The `linkup.sh` script reference below recreates links to utility scripts in the `~/bin` directory. These scripts are used to build and test the magma process. 

	> cd ~/Lavabit/magma.classic/scripts/; ./linkup.sh
	> cd ~/bin
	> ./build.lib all

Get a coffee. Wait ~30 minutes, witness no red feedback in the terminal listing. Wait for the 10 beeps or the prompt to return and you're done.

## Build the magmad executable

Run Eclipse. Right click the `magma` project. Select `Build Configurations`, `Build Selected...`. Select the `.debug` configuration, verify that both the `Clean selected configurations` and `Build selected configurations` are checked and click `Ok`. Builing this configuration takes ~1 minute to complete. Upon completion the `Console`	 tab will report:

	...
	Finished buildling target: magmad

## Build the libmagma.a library

The `libmagma.a` static library is used by the tests maintained in the `magma.check` project's testing framework.  Right click the `magma` project. Select `Build Configurations`, `Build Selected...`. Select the `.check` configuration, verify both the `Clean selected configurations` and `Build selected configurations` are checked and click `Ok`. This configuration takes ~1 minute to complete. Upon completion the `Console` tab will report:

	...
	Finished buildling target: libmagma.a

## Compile the check tests

Right click the `magma.check` project. Select `Build Configurations`, `Build Selected...`. Verify the `.check` configuration is selected. Verify both the `Clean selected configurations` and `Build selected configurations` are checked and click `Ok`. This configuration takes ~10 seconds to complete. Upon completion the `Console` tab should report:

	...
	Finished buildling target: magmad.check

## Run the check tests
	
The `mysql` schema needs to be updated prior to running the check tests. In a terminal, run the `schema.reset` script to define the appropriate schema. Running the script with no arguments displays the script's usage: 

	> cd ~/bin; schema.reset
	Usage: schema.reset <mysql_user> <mysql_password> <mysql_schema>

The values for `<mysql_user>`, `<mysql_password>` and `<mysql_schema>` are located in the file: `/res/config/magma.sandbox.config`.

	> cd ~/bin; 
	> schema.reset mytool aComplex1 Lavabit

Check tests are now ready to run from a command line or the Eclipse IDE. Run the test from the command line with this instruction:

	> cd ~/bin; check.run 

From Eclipse:



