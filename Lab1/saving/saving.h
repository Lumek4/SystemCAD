#pragma once
#include"../cadApplication.h"

enum class GEOMETRY_TYPE
{
	torus,
	bezierC0,
	bezierC2,
	interpolatedC2,
	bezierSurfaceC0,
	bezierSurfaceC2,
	gregoryPatch,
	INVALID = -1,
};
enum class SaveResult
{
	Undefined = 1,
	Success = 0, NoFile = -1, InvalidFile = -2
};
SaveResult save(const CadApplication& app, const char* filepath);
SaveResult load(CadApplication& app, const char* filepath);