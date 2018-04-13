#pragma once

#include "SeamsEasyManip.h"
#include <maya\MPxManipContainer.h>

class SeamsEasyManipContainer : public MPxManipContainer
{
public:
	SeamsEasyManipContainer();
	virtual ~SeamsEasyManipContainer();

	static void *	creator();
	static MStatus	initialize();
	virtual MStatus createChildren();
	virtual MStatus connectToDependNode(const MObject& node);

	static MTypeId		id;

private:
	SeamsEasyManip* manipPtr = NULL;
};