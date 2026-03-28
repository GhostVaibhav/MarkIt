#pragma once
#include "Panel.h"
#include <string>

class AddTodoPanel : public Panel {
public:
    explicit AddTodoPanel();
    
    void render() override;
    
    void promptInput();
    
    std::string getEnteredName() const;
    std::string getEnteredDesc() const;

private:
    std::string name;
    std::string desc;
};
