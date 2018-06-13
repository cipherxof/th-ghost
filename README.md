# th-ghost
Warcraft III Hosting Bot with support for [W3HMC](https://github.com/triggerhappy187/th-ghost/wiki/W3HMC).

https://github.com/triggerhappy187/th-ghost/wiki

Compiling (Debian)
==========

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