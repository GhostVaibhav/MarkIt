FROM debian:stable
RUN apt update && apt -y install g++ libcurl4-openssl-dev libncurses5-dev libncursesw5-dev git cmake && git clone https://github.com/GhostVaibhav/Todo.git && cd Todo && mkdir build && cd build && cmake .. && cmake --build .
CMD ["echo", "Successfully built application on Debian"]