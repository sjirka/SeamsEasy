#include "StitchEasyCtxCmd.h"
#include "StitchEasyNode.h"
#include "SNode.h"



StitchEasyCtxCmd::StitchEasyCtxCmd()
{
}


StitchEasyCtxCmd::~StitchEasyCtxCmd()
{
}

void *StitchEasyCtxCmd::creator() {
	return new StitchEasyCtxCmd;
}

MPxContext* StitchEasyCtxCmd::makeObj() {
	return m_context = new StitchEasyCtx;
}

MStatus StitchEasyCtxCmd::appendSyntax() {
	MStatus status;

	MSyntax newSyntax = syntax();

	newSyntax.addFlag("-nd", "-node", MSyntax::kString);

	return MS::kSuccess;
}

MStatus StitchEasyCtxCmd::doEditFlags() {
	MStatus status;

	MArgParser argData = parser();

	if (argData.isFlagSet("-nd")) {
		MString nodeName = argData.flagArgumentString("-nd", 0);
		MObject pluginNode;

		status = SNode::getPluginNode(nodeName, StitchEasyNode::id, pluginNode);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		m_context->setNode(nodeName);
	}

	return MS::kSuccess;
}