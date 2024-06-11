#pragma once
#include"../cadApplication.h"

enum class SaveResult
{
	Undefined = 1,
	Success = 0, NoFile = -1, InvalidFile = -2
};
SaveResult save(const CadApplication& app, const char* filepath);
SaveResult load(CadApplication& app, const char* filepath);