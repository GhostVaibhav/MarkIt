FROM alpine
RUN apk update \
    && apk upgrade \
    && apk add g++ ncurses-dev curl-dev git cmake make \
    && git clone https://github.com/GhostVaibhav/Todo.git \
    && cd Todo \
    && mkdir build \
    && cd build \
    && cmake .. \
    && cmake --build .
CMD ["echo", "Successfully built application on Alpine"]