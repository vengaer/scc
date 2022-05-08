FROM archlinux:latest
LABEL maintainer="vilhelm.engstrom@tuta.io"
ENV PATH="/root/.cargo/bin:/root/.local.bin:${PATH}"

COPY . /scc

RUN pacman -Syu --noconfirm --needed make clang gcc git python{,-sphinx,-pip} llvm ruby cmake rust vim gdb
RUN cargo install --path /scc/submodules/conftool

RUN pip install --no-cache setuptools sphinx-rtd-theme pylint pytest

ENV CC=clang

WORKDIR /scc
