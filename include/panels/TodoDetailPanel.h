#pragma once
#include <curses.h>
#include <string>
#include <vector>
#include "FullScreenPanel.h"
#include "LogoPanel.h"
#include "Todo.h"

enum class TodoDetailAction { Toggle, Delete, Back, QuitApp };

class TodoDetailPanel : public FullScreenPanel {
public:
    explicit TodoDetailPanel();
    void setTodo(const Todo& todo);
    void render() override;
    TodoDetailAction promptAction();

private:
    LogoPanel logoPanel;
    Todo currentTodo;
};
