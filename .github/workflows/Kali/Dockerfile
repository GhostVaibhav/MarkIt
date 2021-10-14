FROM kalilinux/kali-rolling:latest
RUN apt -y update \
    && apt -y upgrade \
    && apt -y install --no-install-recommends g++ \
    && apt -y install --no-install-recommends libcurl4-openssl-dev \
    && apt-get -y install --no-install-recommends libncurses5-dev libncursesw5-dev \
    && apt -y install --no-install-recommends cmake \
    && apt -y install --no-install-recommends git \
    && git clone https://github.com/GhostVaibhav/MarkIt.git \
    && cd MarkIt \
    && mkdir build \
    && cd build \
    && cmake .. \
    && cmake --build .
CMD ["echo", "Successfully built application on Kali Linux"]