#include "DsaDebug.h"
#include "FileManagement.h"

#pragma once

class Editor {
public:
    Editor();
	void LoadLevel();
    void CreateRenderobject();
    void CreateMoveRuleSet();
private:
    struct directory {
        std::string levels;
    }directory;
};