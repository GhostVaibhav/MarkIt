FROM opensuse/leap
RUN zypper refresh \
    && zypper update -y \
    && zypper install -y gcc-c++ \
    && zypper install -y libcurl-devel \
    && zypper install -y ncurses-devel \
    && zypper install -y cmake \
    && zypper install -y git \
    && git clone https://github.com/GhostVaibhav/Todo.git \
    && cd Todo \
    && mkdir build \
    && cd build \
    && cmake .. \
    && cmake --build .
CMD ["echo", "Successfully built application on openSUSE Leap"]