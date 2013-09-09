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

## Using the java bindings

Make sure vba-linux is available within "java.library.path":

```
sudo ln -s ./src/clojure/.libs/libvba.so.0.0.0 /usr/lib/jni/libvba.so
```

Make sure vba-linux bindings are in $CLASSPATH:

```
export CLASSPATH=$CLASSPATH:`pwd`/java/dist/gb-bindings.jar
```

To use the bindings from inside the jvm (like with java, clojure or jython):

```
import com.aurellem.gb.Gb as Gb
Gb.loadVBA()
```

## Which version of VBA is this?

Who knows.

This is a fork of [vba-clojure](http://hg.bortreb.com/vba-clojure) at
b531d490859c, which is a fork of
[vba-rerecording](https://code.google.com/p/vba-rerecording/) v23.5 which is a
fork of some other VBA.
