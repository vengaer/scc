FROM archlinux:latest
LABEL maintainer="vilhelm.engstrom@tuta.io"

RUN pacman -Syu --needed --noconfirm make clang gcc git python llvm asciidoc ruby cmake

ENV CC=gcc

COPY . /scc
WORKDIR /scc
