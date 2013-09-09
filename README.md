# vba-linux

This is VBA for Linux except with rerecording capabilities and jvm bindings
(jni).

## building

Configure some environment variables:

```
export JAVA_INCLUDE_PATH=/usr/lib/jvm/java-6-openjdk-amd64/include/
export JAVA_INCLUDE_PATH2=$JAVA_INCLUDE_PATH
```

Next do these things:

```
cd java/
ant all
cd ../

autoreconf -i
./configure
make
```

Build products are dumped into: ./src/clojure/.libs/

## Which version of VBA is this?

Who knows.

This is a fork of [vba-clojure](http://hg.bortreb.com/vba-clojure) at
b531d490859c, which is a fork of
[vba-rerecording](https://code.google.com/p/vba-rerecording/) v23.5 which is a
fork of some other VBA.
