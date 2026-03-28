#pragma once

struct ColorScheme {
    int success = 1;
    int error = 2;
    int accent = 3;
    int highlight = 4;
    int warning = 5;
    int normal = 6;
    int footer = 7;

    void apply() const;
};
