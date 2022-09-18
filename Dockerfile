FROM archlinux:latest
LABEL maintainer="vilhelm.engstrom@tuta.io"

RUN useradd -m builder

COPY . /scc

RUN pacman -Syu --noconfirm --needed make clang gcc git python{,-sphinx,-pip} llvm ruby cmake rust vim gdb
RUN cargo install --path /scc/submodules/conftool --root /usr/local
RUN pip install --no-cache setuptools sphinx-rtd-theme pylint pytest

ENV CC=clang

WORKDIR /scc
RUN chown -R builder:builder /scc
USER builder
