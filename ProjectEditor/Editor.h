#include "DsaDebug.h"
#include "FileManagement.h"
#include "Environment.h"

#include "LevelEditor.h"
#include "Renderer.h"
#include "Time.h"

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