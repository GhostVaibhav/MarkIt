#pragma once
#include <string>

#include "DimensionConfig.h"
#include "FullScreenPanel.h"
#include "LogoPanel.h"

class LoadingPanel : public FullScreenPanel {
 public:
  explicit LoadingPanel(WINDOW* win);
  void setLoadingText(const std::string& text);
  void render() override;

  int getMinWidth() const override { return Dimensions::LoadingMinWidth; }
  int getMinHeight() const override { return Dimensions::LoadingMinHeight; }

 private:
  LogoPanel logoPanel;
  std::string loadingText = "Loading...";
};
