#pragma once

struct ColorScheme {
  const int success = 1;
  const int error = 2;
  const int accent = 3;
  const int highlight = 4;
  const int warning = 5;
  const int normal = 6;
  const int footer = 7;

  void apply() const;
};
