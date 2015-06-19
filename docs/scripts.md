#Magma Scripts description

All the scripts are in the scripts folder.

```
scripts/linkup.sh
```
Creates soft links in the /usr/bin/ directory to all the magma scripts, allowing for easy calls to said scripts from any location.

##Inside scripts/benchmark

```
scripts/benchmark/build/bench.sh
```
Not in any way critical script, still configured to old admin name and old directory structure, can/should probably be deleted.

```
scripts/benchmark/compression/bench.sh
```
Benchmarks different compression schemes on res/corpus/messages.tar which should be a file with stored messages in them, but is not included in the public repo.

```
scripts/benchmark/mason/mason.sh
```
Benchmarks the mason tool, which I believe has something to do with either populating the database or using tokyo cabinet. It's hard-coded to old paths, which probably means it's unnecessary, as otherwise it would have been edited.

##Inside scripts/builders

```
scripts/builders/build.check.sh
```
Runs the makefile for the unit tests target.

```
scripts/builders/build.docs.sh
```
Runs doxygen on all the source files to generate doc files.

```
scripts/builders/build.doxyfile
Doxygen file.

```
scripts/builders/build.lib.params.sh
```
Parameters included in the script that builds the magmad.so shared object.

```
scripts/builders/build.lib.sh
```
Script that builds the magmad.so shared object.

```
scripts/builders/build.magma.sh
```
Runs the makefile for magmad make target.

```
scripts/builders/build.makefiles.sh
```
Outdated script, no longer relevant was used to localize makefiles.

```
scripts/builders/build.prep.sh
```
Creates a build.h header, which contains the version number and commit-id tail to create the build id.

```
scripts/builders/build.quick.sh
```
Was originally designed in order to allow incremental builds of magmad without the use of eclipse (to be used as an alternative to build.magma.sh). Its paths have been generalized, but its functionality isn't fully verified.

```
scripts/builders/connect/sslmap.sh
```
Connect to the localhost imap port via ssl.

```
scripts/builders/connect/sslnet.sh
```
Connect to the localhost smtp port via ssl.

```
scripts/builders/connect/sslpop.sh
```
Connect to the localhost pop3 port via ssl.

```
scripts/builders/connect/sslweb.sh
```
Connect to the localhost webmail port via ssl.

```
scripts/builders/connect/tlsmap.sh
```
Connect to the localhost imap port via ssl starttls command.

```
scripts/builders/connect/tlsnet.sh
```
Connect to the localhost smtp port via ssl starttls command.

```
scripts/builders/connect/tlspop.sh
```
Connect to the localhost pop3 port via ssl starttls command.

```
scripts/builders/connect/tlsweb.sh
```
Connect to the localhost webmail port via ssl starttls command.

```
scripts/builders/connect/tmap.sh
```
Connect to the localhost imap port via telnet.

```
scripts/builders/connect/tnet.sh
```
Connect to the localhost smtp port via telnet.

```
scripts/builders/connect/tpop.sh
```
Connect to the localhost pop3 port via telnet.

```
scripts/builders/connect/tweb.sh
```
Connect to the localhost webmail port via telnet.

##Inside scripts/database

```
scripts/database/mycon.sh
```
Logs user into the mysql. The username, password and schema are hard-coded in the script.

```
scripts/database/schema.dump.sh
```
Attempts to dump a hardcoded schema from a hard coded user. Needs to be fixed for relative pathing and the location of the required .sql files according to the current directory structure. TODO!!!

```
scripts/database/schema.init.sh
```
Initializes the schema using the .sql files in the folder res/sql/. Requires arguments . These have to match the constants inside the relevant magma.config file located in res/config. This script is already edited for relative pathing and current folder structure.

```
scripts/database/schema.reset.sh
```
Practically same as scripts/database/schema.init.sh.

##Inside scripts/freshen

```
scripts/freshen/freshen.clamav.sh
```
Updates virus signatures for clamav virus scanner.

##Inside scripts/launch

```
scripts/launch/check.cachegrind.sh
```
Runs magmad.check using sandbox config and valgrind in cachegrind mode to determine cache use.

```
scripts/launch/check.callgrind.sh
```
Runs magmad.check using sandbox config and valgrind in callgrind mode to create a function call graph.

```
scripts/launch/check.helgrind.sh
```
Runs magmad.check using sandbox config and valgrind in helgrind mode to determine thread safety and thread related statistics.

```
scripts/launch/check.massif.sh
```
Runs magmad.check using sandbox config and valgrind in massif mode to profile heap usage.

```
scripts/launch/check.run.sh
```
Runs magmad.check using sandbox config without valgrind.

```
scripts/launch/check.vg.sh
```
Runs magmad.check using sandbox config and valgrind in default mode which is memcheck which checks for memory allocation and freeing to avoid memory leaks.

```
scripts/launch/magma.helgrind.sh
```
Runs magma using sandbox config and valgrind in helgrind mode to monitor thread statistics.

```
scripts/launch/magma.killhard.sh
```
Kills magma in a guaranteed fashion.

```
scripts/launch/magma.kill.sh
```
Preferred method for killing magma.

```
scripts/launch/magma.loop.sh
```
Runs magma and restarts it in case of a crash.

```
scripts/launch/magma.mem.sh
```
Monitors overall memory usage by magma.

```
scripts/launch/magma.num.sh
```
Seemingly displays magma process id as well as running configuration file.

```
scripts/launch/magma.run.sh
```
Launches magma using sandbox configuration file.

```
scripts/launch/magma.stat.sh
```
Provides a number of different statistics of magma, including number of threads, connections and etc.

```
scripts/launch/magma.vg.sh
```
Runs magma using sandbox config and valgrind in default mode (memcheck) to help detect memory leaks.

```
scripts/launch/magma.watcher.sh
```
Not sure, don't remember.

```
scripts/launch/runner.sh
```
Not sure, but I believe it attempts to load the data inside res/ folder into magma. Currently failing due to being unable to store messages due to an invalid path. Requires investigation.

```
scripts/launch/testde.sh
```
Used to run tests for some isolated pieces of code, later to be used in magma. Requires compilation of the testde project. Not particularly relevant to development unless we also want to test things in isolation.

##Inside scripts/parsers

```
scripts/parsers/*
```
Unsure about the nature of all of them. Some were used to modify code en-masse during previous development stages. I believe most of this stuff is useless, however of note is the fact that there ARE an uncrustify and a ctags script already there.

##Inside scripts/tests

```
scripts/tests/*
```
Not sure on most of these other than what their names suggest.
