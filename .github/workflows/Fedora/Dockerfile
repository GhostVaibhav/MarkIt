FROM fedora:36
RUN dnf -y upgrade \
    && dnf -y install git \
    && dnf -y install gcc-c++ \
    && dnf -y install libcurl-devel \
    && dnf -y install ncurses-devel \
    && dnf -y install cmake \
    && git clone https://github.com/GhostVaibhav/Todo.git \
    && cd Todo \
    && mkdir build \
    && cd build \
    && cmake .. \
    && cmake --build .
CMD ["echo", "Successfully built application on Fedora"]