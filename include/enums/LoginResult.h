#pragma once

enum class LoginResult {
  AlreadyLoggedIn,
  NewUser,
  ExistingUser,
  WrongPassword
};
