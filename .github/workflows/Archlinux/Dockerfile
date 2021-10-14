FROM archlinux:latest
RUN pacman -Syy --noconfirm \
    && pacman -Syu --noconfirm \
    && pacman -Sy gcc --noconfirm \
    && pacman -S curl --noconfirm \
    && pacman -S ncurses --noconfirm \
    && pacman -S git --noconfirm \
    && pacman -S cmake --noconfirm \
    && pacman -S make --noconfirm \
    && git clone https://github.com/GhostVaibhav/Todo.git \
    && cd Todo \
    && mkdir build \
    && cd build \
    && cmake .. \
    && cmake --build .
CMD ["echo", "Successfully built application on Arch Linux"]