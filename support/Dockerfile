FROM alpine:3.17
RUN apk add --no-cache bison coreutils gcc libc-dev make
ncurses-dev libncurses5-dev libncursesw5-dev
WORKDIR /tmp/bash
COPY . .
RUN ./configure --enable-readline --with-curses
RUN make
RUN make tests
