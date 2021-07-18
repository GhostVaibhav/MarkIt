FROM kalilinux/kali-rolling
RUN apt -y update \
    && apt -y upgrade \
    && apt -y install g++ \
    && apt -y install libcurl4-openssl-dev \
    && apt-get -y install libncurses5-dev libncursesw5-dev \
    && apt -y install cmake \
    && apt -y install git \
    && git clone https://github.com/GhostVaibhav/Todo.git \
    && cd Todo \
    && mkdir build \
    && cd build \
    && cmake .. \
    && cmake --build .
CMD ["echo", "Successfully built application on Kali Linux"]