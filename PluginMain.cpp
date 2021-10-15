#include "SeamsEasyNode.h"
#include "SeamsEasyCmd.h"
#include "StitchEasyNode.h"
#include "StitchEasyCmd.h"
#include "StitchEasyCtxCmd.h"

#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject object) {
	MStatus status;

	MFnPlugin fnPlugin(object, "Stepan Jirka", "1.0", "Any", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.registerNode("stitchEasy", StitchEasyNode::id, StitchEasyNode::creator, StitchEasyNode::initialize);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.registerNode("seamsEasy", SeamsEasyNode::id, SeamsEasyNode::creator, SeamsEasyNode::initialize);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.registerCommand("stitchEasy", StitchEasyCmd::creator, StitchEasyCmd::newSyntax);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.registerCommand("seamsEasy", SeamsEasyCmd::creator, SeamsEasyCmd::newSyntax);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.registerContextCommand("stitchEasyCtx", StitchEasyCtxCmd::creator);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return MS::kSuccess;
}

MStatus uninitializePlugin(MObject object) {
	MStatus status;

	MFnPlugin fnPlugin(object);

	status = fnPlugin.deregisterContextCommand("stitchEasyCtx");
	CHECK_MSTATUS_AND_RETURN_IT(status);
	
	status = fnPlugin.deregisterNode(StitchEasyNode::id);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.deregisterNode(SeamsEasyNode::id);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.deregisterCommand("stitchEasy");
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = fnPlugin.deregisterCommand("seamsEasy");
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return MS::kSuccess;
}
