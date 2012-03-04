cd ./src/gba

g++ -DHAVE_CONFIG_H \
    -I. -I../../../src/gba -I../..  -I../../../src \
    -DSDL -DSYSCONFDIR=\"/usr/local/etc\"  \
    -fno-exceptions  -g -O2 \
    -MT remote.o -MD -MP -MF .deps/remote.Tpo -c \
    -o remote.o ../../../src/gba/remote.cpp
