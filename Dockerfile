FROM archlinux:latest
LABEL maintainer="vilhelm.engstrom@tuta.io"
ENV PATH="/root/.local/bin:${PATH}"

RUN pacman -Syu --noconfirm --needed make clang gcc git python{,-sphinx} llvm ruby cmake

RUN python -m ensurepip
RUN python -m pip install --upgrade pip
RUN pip install --no-cache setuptools sphinx-rtd-theme pylint pytest

ENV CC=clang

COPY . /scc
WORKDIR /scc
