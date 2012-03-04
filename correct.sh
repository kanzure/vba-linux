cd ./src/gba

g++ -DHAVE_ARPA_INET_H=1  \
    -DHAVE_NETINET_IN_H=1 \
    -fno-exceptions \
    -I. -I../../../src/gba -I../..  -I../../../src \
    -g -O2 \
    -MT remote.o -MD -MP -MF .deps/remote.Tpo -c \
    -o remote.o ../../../src/gba/remote.cpp
