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
                                     python{,-sphinx,-pip,-pylint}                      \
                                     python-{pytest,aiofiles,pyelftools}                \
                                     llvm                                               \
                                     ruby                                               \
                                     cmake                                              \
                                     rust                                           &&  \
    cargo install --path /home/builder/scc/submodules/conftool --root /usr/local    &&  \
    pip install --no-cache setuptools                                                   \
                           sphinx-rtd-theme                                             \
                           pycparser

USER builder
