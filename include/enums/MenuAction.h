#pragma once

enum class MainMenuAction { AddTodo, Back };

enum class TodoAction { Delete, ToggleComplete, Back };

enum class OnlineMenuAction {
  AddTodo,
  PushToCloud,
  PullFromCloud,
  Refresh,
  Back
};
