FROM alpine:20210804
RUN apk update \
    && apk add g++ git cmake make \
    && git clone --recursive https://github.com/GhostVaibhav/MarkIt.git \
    && cd Todo \
    && mkdir build \
    && cd build \
    && cmake .. -DCOMPILE=0 \
    && cmake --build . \
    && cmake .. -DCOMPILE=1 \
    && cmake --build . --target install \
CMD ["echo", "Successfully built application on Alpine"]