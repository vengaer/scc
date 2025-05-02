FROM debian:latest
LABEL maintainer="vilhelm.engstrom@tuta.io"

RUN useradd -m builder

COPY . /home/builder/scc
WORKDIR /home/builder/scc

RUN chown -R builder:builder /home/builder/scc

RUN apt-get update                                                                  &&  \
    apt-get upgrade -y                                                              &&  \
    apt-get install -y make                                                             \
                       clang                                                            \
                       cmake                                                            \
                       curl                                                             \
                       gcc                                                              \
                       gdb                                                              \
                       git                                                              \
                       libclang-dev                                                     \
                       libsqlite3-dev                                                   \
                       llvm                                                             \
                       llvm-dev                                                         \
                       llvm-dev                                                         \
                       ruby                                                             \
                       pylint                                                           \
                       python3-dev                                                      \
                       python3-sphinx                                                   \
                       python3-pytest                                                   \
                       python3-pycparser                                                \
                       python3-pyelftools                                               \
                       sphinx-rtd-theme-common

RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | bash -s -- -y
RUN /root/.cargo/bin/cargo install --path /home/builder/scc/submodules/conftool --root /usr/local

USER builder
