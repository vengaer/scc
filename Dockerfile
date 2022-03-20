FROM archlinux:latest
LABEL maintainer="vilhelm.engstrom@tuta.io"

RUN pacman -Syu --needed --noconfirm make clang gcc git python{,-pytest,-pylint,-sphinx} llvm ruby cmake

ENV CC=gcc

COPY . /scc
WORKDIR /scc
