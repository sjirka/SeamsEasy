#pragma once

#include "StitchEasyCtx.h"

#include <maya\MPxContextCommand.h>

class StitchEasyCtxCmd : public MPxContextCommand
{
public:
	StitchEasyCtxCmd();
	virtual ~StitchEasyCtxCmd();

	static void *creator();
	virtual MPxContext* makeObj();

	virtual MStatus appendSyntax();
	virtual MStatus doEditFlags();

private:
	StitchEasyCtx *m_context;
};