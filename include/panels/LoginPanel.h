#pragma once
#include <string>

#include "DimensionConfig.h"
#include "FullScreenPanel.h"
#include "LoadingPanel.h"
#include "LogoPanel.h"

class LoginPanel : public FullScreenPanel {
 public:
  explicit LoginPanel(WINDOW* win);
  ~LoginPanel();

  void render() override;
  void promptInput();
  std::string getEnteredUsername() const;
  std::string getEnteredPassword() const;
  void captureInput(WINDOW*, std::string&, bool);
  void clearUsername();
  void reset();

  int getMinWidth() const override { return Dimensions::LoginMinWidth; }
  int getMinHeight() const override { return Dimensions::LoginMinHeight; }

  void showLoading(const std::string& msg);
  void showError(const std::string& error);

 private:
  LogoPanel logoPanel;
  LoadingPanel loadingPanel;
  std::string username;
  std::string password;
  std::string errorMessage;
  bool isLoading = false;

  WINDOW* title = nullptr;
  WINDOW* userNameWindow = nullptr;
  WINDOW* passwordWindow = nullptr;
  WINDOW* information = nullptr;
  WINDOW* wrongPassword = nullptr;
  void recreateWindows();
};
