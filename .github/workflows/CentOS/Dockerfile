FROM centos:latest
RUN yum -y update \
    && yum -y group install "Development Tools" \
    && yum -y install libcurl --allowerasing \
    && yum -y install curl-devel \
    && yum -y install ncurses-devel \
    && yum -y install cmake \
    && git clone https://github.com/GhostVaibhav/Todo.git \
    && cd Todo \
    && mkdir build \
    && cd build \
    && cmake .. \
    && cmake --build .
CMD ["echo", "Successfully built application on CentOS"]