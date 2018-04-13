#pragma once

#include <maya\MPxManipulatorNode.h>
#include "SSeamMesh.h"
#include "SeamsEasyNode.h"

class SeamsEasyManip : MPxManipulatorNode
{
public:
	SeamsEasyManip();
	virtual ~SeamsEasyManip();
	virtual void postConstructor();

	static void*	creator();
	static MStatus	initialize();

	virtual void	draw(M3dView &view, const MDagPath &path, M3dView::DisplayStyle style, M3dView::DisplayStatus status);
	virtual void	preDrawUI(const M3dView &view);
	virtual void	drawUI(MHWRender::MUIDrawManager &drawManager, const MHWRender::MFrameContext &frameContext) const;

	virtual MStatus	doRelease(M3dView &view);

	void setNodePtr(SeamsEasyNode* ptr);

	static MTypeId	id;
private:
	MGLuint firstHandle;
	SeamsEasyNode* pluginNode = NULL;
};

