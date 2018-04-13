#include "SeamsEasyManipContainer.h"
#include "SeamsEasyNode.h"

#include <maya\MGlobal.h>
#include <maya\MFnDependencyNode.h>

MTypeId SeamsEasyManipContainer::id(0x0012789C);

SeamsEasyManipContainer::SeamsEasyManipContainer(){
}

SeamsEasyManipContainer::~SeamsEasyManipContainer(){
}

void *SeamsEasyManipContainer::creator(){
	return new SeamsEasyManipContainer();
}

MStatus SeamsEasyManipContainer::initialize(){
	return MPxManipContainer::initialize();
}

MStatus SeamsEasyManipContainer::createChildren(){
	MStatus status;
	
	MPxManipulatorNode *proxyManip = NULL;
	status = addMPxManipulatorNode("seamEasyLoopManip", "seamEasyLoopManip", proxyManip);
	CHECK_MSTATUS(status);
	manipPtr = (SeamsEasyManip*)proxyManip;
	
	return MStatus::kSuccess;
}

MStatus SeamsEasyManipContainer::connectToDependNode(const MObject& node) {

	if (manipPtr != NULL) {
		MFnDependencyNode fnUserNode(node);
		SeamsEasyNode *fnPluginNode = dynamic_cast<SeamsEasyNode*>(fnUserNode.userNode());
		manipPtr->setNodePtr(fnPluginNode);
	}

	return finishAddingManips();
}