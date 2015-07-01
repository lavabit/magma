# Run the magma server

This information relies on having previously followed the steps for building the magma server located [here] (<https://github.com/lavabit/magma.classic/blob/master/docs/magma-build.md>).

## Run magma from the Eclipse Run configuration

A tweak to the Eclipse configuration is needed to use the `Run` configuration for the `magmad` project.  Generate the default run template by selecting the `magmad` project, then select `Run` in the `Run` menu.  The `Console` window will this error message:

```
Could not open the file magma.config for reading. {errno = 2 & strerror = Unknown error}
Unable to load configuration file. Exiting.
magma.init != shutdown {3 != 37}
Magma shutdown complete.

```	

Now we tweak the run configuration.  Select the `Run` menu, `Run Configurations...`.  Verify that under the `C/C++ Application` entry in the left pane that the `magmad` entry is highlighted. In the main dialog window, select the `Arguments` tab and deselect the `Use default` button. Click the `Workspace` button, and select the `magma.classic` project. This will update the `Working directory` entry to read `${workspace_loc:magma.classic}`. In the `Program Arguments` window, enter `res/config/magma.sandbox.config` as the argument that defines where to locate the configuration file. Click the `Apply` button and `Run` to finally run the magma executable. All output is shown in the Eclipse `Console` window:

```
...
Warning: SSL certificate has world-access file permissions! Please fix. { path = res/config/localhost.localdomain.pem }
Warning: SSL certificate has world-access file permissions! Please fix. { path = res/config/localhost.localdomain.pem }
Warning: SSL certificate has world-access file permissions! Please fix. { path = res/config/localhost.localdomain.pem }
Warning: SSL certificate has world-access file permissions! Please fix. { path = res/config/localhost.localdomain.pem }
Warning: DKIM private key has world-access file permissions! Please fix. { path = res/config/dkim.localhost.localdomain.pem }
Magma initialization complete.
```

Demonstrate the magma server is running with:

	> cd ~/bin; tnet
	Trying ::1...
	telnet: connect to address ::1: Connection refused
	Trying 127.0.0.1...
	Connected to localhost.
	Escape character is '^]'.
	220 lavabit.com ESMTP Magma


And alternatively with:

	> ps -ef | grep magmad
	magma     18148  17781 51 22:47 ?        00:03:19 /home/magma/Lavabit/magma.classic/src/.debug/magmad res/config/magma.sandbox.config
	
Kill the magma server with:

	> cd ~/bin; magma.kill

View the Eclipse `Console` window for reports that the magma server is being shutdown as a result of running the magma.kill script.

