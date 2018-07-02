# th-ghost
Warcraft III Hosting Bot modified from [ghostplusplus](https://code.google.com/archive/p/ghostplusplus/source) and [ent-ghost](https://github.com/uakfdotb/ent-ghost) with support for [W3HMC](https://github.com/triggerhappy187/th-ghost/wiki/W3HMC).

https://github.com/triggerhappy187/th-ghost/wiki

Windows
==========

The easiest way to run th-ghost on a Windows machine is to download and extract [th-ghost-v1.0.0-beta1.zip](https://github.com/triggerhappy187/th-ghost/releases/download/v1.0.0-beta.1/th-ghost-v1.0.0-beta1.zip) from the [releases page](https://github.com/triggerhappy187/th-ghost/releases).

A solution to compile on Windows will be provided eventually.

Debian
==========
Compile:

```
apt-get install screen build-essential g++ libmysql++-dev libbz2-dev libgmp3-dev git libboost-all-dev libcurl-dev
cd ghost-root/bncsutil/src/bncsutil
make
make install
cd ../../../
cd StormLib/stormlib
make
make install
cd ../../
cd ghost
make
```
Running:

MySQL is currently required on Linux for the bot. This means you will need to setup MySQL on your machine and run the provided SQL scripts ([install.sql](https://github.com/triggerhappy187/th-ghost/blob/master/ghost/install.sql) & [mysql_create_tables_v2.sql](https://github.com/triggerhappy187/th-ghost/blob/master/mysql_create_tables_v2.sql)) as well as configure your MySQL info in default.cfg.
