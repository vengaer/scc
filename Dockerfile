FROM archlinux:latest
LABEL maintainer="vilhelm.engstrom@tuta.io"

COPY . /scc
WORKDIR /scc

RUN useradd -m builder                                                  &&  \
    pacman -Syu --noconfirm --needed make                                   \
                                     clang                                  \
                                     gcc                                    \
                                     git                                    \
                                     python{,-sphinx,-pip,-pylint,-pytest}  \
                                     llvm                                   \
                                     ruby                                   \
                                     cmake                                  \
                                     rust                               &&  \
    cargo install --path /scc/submodules/conftool --root /usr/local     &&  \
    pacman -Rns --noconfirm rust                                        &&  \
    pip install --no-cache setuptools                                       \
                           sphinx-rtd-theme                                 \
                           pycparser                                    &&  \
    chown -R builder:builder /scc

USER builder
