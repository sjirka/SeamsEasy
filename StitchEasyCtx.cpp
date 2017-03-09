#include "StitchEasyCtx.h"
#include "StitchEasyNode.h"

#include <maya\MGlobal.h>
#include <maya\MFnDependencyNode.h>
#include <maya\MSelectionList.h>
#include <maya\MItSelectionList.h>
#include <maya\MDagPath.h>

StitchEasyCtx::StitchEasyCtx(){
	setTitleString("StitchEasy");
	setImage("seamsEasy.xpm", MPxContext::kImage1);
}

StitchEasyCtx::~StitchEasyCtx()
{
}

void StitchEasyCtx::setNode(MString &nodeName) {
	m_nodeName = nodeName;
}

void StitchEasyCtx::toolOnSetup(MEvent &) {
	setHelpString("Select custom stitch geometry and press enter.");
}

void StitchEasyCtx::doEnterRegion() {
	setHelpString("Select custom stitch geometry and press enter.");
}

void StitchEasyCtx::abortAction() {
	MGlobal::executeCommand("setToolTo moveSuperContext;");
}

void StitchEasyCtx::deleteAction() {
	MGlobal::executeCommand("setToolTo moveSuperContext;");
}

void StitchEasyCtx::completeAction(){
	if (m_nodeName!="") {
		MSelectionList selection;
		MGlobal::getActiveSelectionList(selection, true);

		MItSelectionList itSelected(selection, MFn::kMesh);
		if (!itSelected.isDone()) {
			MDagPath path;
			itSelected.getDagPath(path);

			MFnDagNode fnShape(path);
			MGlobal::executeCommand("connectAttr -f \"" + fnShape.name() + ".outMesh\" \"" + m_nodeName + ".customGeometry\"; setAttr \"" + m_nodeName + ".useCustom\" 1;", true, true);
		}
		else
			MGlobal::displayInfo("Invalid selection");
	}
	else
		MGlobal::displayInfo("Invalid target node");

	MGlobal::executeCommand("setToolTo moveSuperContext;");
}

void StitchEasyCtx::toolOffCleanup() {
	MPxSelectionContext::toolOffCleanup();
}