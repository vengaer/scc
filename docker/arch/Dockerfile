FROM archlinux:latest
LABEL maintainer="vilhelm.engstrom@tuta.io"

RUN useradd -m builder

COPY . /home/builder/scc
WORKDIR /home/builder/scc

RUN chown -R builder:builder /home/builder/scc                                      &&  \
    pacman-key --init                                                               &&  \
    pacman -Sy --noconfirm archlinux-keyring                                        &&  \
    pacman -Syu --noconfirm --needed make                                               \
                                     clang                                              \
                                     gcc                                                \
                                     gdb                                                \
                                     git                                                \
                                     python{,-sphinx,-pylint}                           \
                                     python-{pytest,pyelftools,pycparser}               \
                                     python-sphinx_rtd_theme                            \
                                     llvm                                               \
                                     ruby                                               \
                                     cmake                                              \
                                     rust                                           &&  \
    cargo install --path /home/builder/scc/submodules/conftool --root /usr/local

USER builder
