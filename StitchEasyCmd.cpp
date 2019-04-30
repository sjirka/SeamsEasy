#include "StitchEasyCmd.h"
#include "StitchEasyNode.h"
#include "SeamsEasyNode.h"
#include "../_library/SShadingGroup.h"
#include "SNode.h"

#include <maya\MSyntax.h>
#include <maya\MSelectionList.h>
#include <maya\MItSelectionList.h>
#include <maya\MDagPath.h>
#include <maya\MFnDagNode.h>
#include <maya\MGlobal.h>
#include <maya\MFnComponentListData.h>
#include <maya\MFnSet.h>
#include <maya\MFnTransform.h>
#include <maya\MMatrix.h>
#include <maya\MArgList.h>
#include <maya\MItDependencyGraph.h>

StitchEasyCmd::StitchEasyCmd()
{
}

StitchEasyCmd::~StitchEasyCmd()
{
}

void *StitchEasyCmd::creator()
{
	return new StitchEasyCmd;
}

MSyntax StitchEasyCmd::newSyntax()
{
	MSyntax syntax;

	syntax.addFlag(DISTANCE_FLAG, DISTANCE_FLAG_LONG, MSyntax::kDistance);
	syntax.addFlag(LENGTH_FLAG, LENGTH_FLAG_LONG, MSyntax::kDistance);
	syntax.addFlag(USELENGTH_FLAG, USELENGTH_FLAG_LONG, MSyntax::kBoolean);
	syntax.addFlag(THICKNESS_FLAG, THICKNESS_FLAG_LONG, MSyntax::kDistance);
	syntax.addFlag(SKEW_FLAG, SKEW_FLAG_LONG, MSyntax::kDistance);
	syntax.addFlag(FLIP_FLAG, FLIP_FLAG_LONG, MSyntax::kBoolean);
	syntax.addFlag(SMOOTHLEVEL_FLAG, SMOOTHLEVEL_FLAG_LONG, MSyntax::kLong);
	syntax.addFlag(ROTATE_FLAG, ROTATE_FLAG_LONG, MSyntax::kAngle, MSyntax::kAngle, MSyntax::kAngle);
	syntax.addFlag(SCALE_FLAG, SCALE_FLAG_LONG, MSyntax::kDouble, MSyntax::kDouble, MSyntax::kDouble);
	syntax.addFlag(TRANSLATE_FLAG, TRANSLATE_FLAG_LONG, MSyntax::kDistance, MSyntax::kDistance, MSyntax::kDistance);
	syntax.addFlag(SEAMNODE_FLAG, SEAMNODE_FLAG_LONG, MSyntax::kString);
	syntax.addFlag(MULTIPLIER_FLAG, MULTIPLIER_FLAG_LONG, MSyntax::kDouble);
	syntax.addArg(MSyntax::kString);

	syntax.useSelectionAsDefault(true);
	syntax.setObjectType(MSyntax::kSelectionList, 1);
	syntax.enableEdit(true);
	syntax.enableQuery(true);

	return syntax;
}

MStatus StitchEasyCmd::doIt(const MArgList& args)
{
	MStatus status;

	MArgDatabase argData(syntax(), args, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	bool isEdit = argData.isEdit();
	bool isQuery = argData.isQuery();
	m_isCreation = (isQuery || isEdit) ? false : true;

	MString name = argData.commandArgumentString(0);

	// Creation mode //////////////////////////////////////////////////////////////////////////////
	if (m_isCreation) {
		// Get input mesh node and component
		MSelectionList selection;
		status = argData.getObjects(selection);
		if (status != MS::kSuccess) {
			displayError("Nothing selected");
			setResult(false);
			return status;
		}

		MObject
			inputNode,
			compList;
		MDagPath
			path;

		MPlug pOutSLines;
		
		if (argData.isFlagSet(SEAMNODE_FLAG)){
			MObject seamNode;
			MString nodeName = argData.flagArgumentString(SEAMNODE_FLAG, 0);

			status = SNode::getPluginNode(nodeName, SeamsEasyNode::id, seamNode);
			if (status != MS::kSuccess) {
				displayError("Invalid node name");
				setResult(false);
				return status;
			}

			MFnDependencyNode fnInputNode(seamNode);
			MItDependencyGraph itGraph(fnInputNode.findPlug(SeamsEasyNode::aOutMesh), MFn::kMesh);
			if (itGraph.isDone()) {
				displayError("Source mesh not found");
				setResult(false);
				return status;
			}
				
			MFnDagNode fnOutMesh(itGraph.currentItem());
			fnOutMesh.getPath(path);
			pOutSLines = fnInputNode.findPlug(SeamsEasyNode::aOutStitchLines, true);
		}
		else{
			MObject component;
			selection.getDagPath(0, path, component);
			if (component.apiType() != MFn::kMeshEdgeComponent) {
				displayError("Invalid selection. Select mesh edges and retry");
				setResult(false);
				return status;
			}

			MFnComponentListData compData;
			compList = compData.create();
			compData.add(component);
		}

		// Create nodes
		m_node = m_dgMod.createNode(StitchEasyNode::id, &status);
		status = m_dgMod.doIt();

		name = (name == "") ? "stitchEasy" : name;
		SNode::createDagGroup(name, m_dagMod, m_shape, m_transform);
		
		// Copy original transformation
		MFnTransform
			fnTrans(path.transform()),
			fnNewTrans(m_transform);
		fnNewTrans.set(fnTrans.transformation());

		// Make connections
		MFnDagNode fnMesh(path), fnNewMesh(m_shape);
		MFnDependencyNode fnNode(m_node);

		m_dagMod.connect(fnMesh.findPlug("outMesh", true), fnNode.findPlug(StitchEasyNode::aInMesh, true));
		m_dagMod.connect(fnMesh.findPlug("outSmoothMesh", true), fnNode.findPlug(StitchEasyNode::aInSmoothMesh, true));
		m_dagMod.connect(fnMesh.findPlug("displaySmoothMesh", true), fnNode.findPlug(StitchEasyNode::aUseSmoothMesh, true));
		m_dagMod.connect(fnNode.findPlug(StitchEasyNode::aOutMesh, true), fnNewMesh.findPlug("inMesh", true));
		
		if(!pOutSLines.isNull())
			m_dagMod.connect(pOutSLines, fnNode.findPlug(StitchEasyNode::aStitchLines, true));
		else
			m_dagMod.newPlugValue(fnNode.findPlug(StitchEasyNode::aStitchLines, true), compList);
	}

	if (isQuery || isEdit) {
		status = SNode::getPluginNode(name, StitchEasyNode::id, m_node);
		if (status != MS::kSuccess) {
			displayError("invalid object");
			setResult(false);
			return status;
		}
	}

	MFnDependencyNode fnNode(m_node, &status);
	if (isQuery) {
		for (auto &flag : m_attrFlags)
			if (argData.isFlagSet(flag.first)) {
				queryAttrValue(fnNode.findPlug(flag.second, true));
				break;
			}
	}
	else if (isEdit || m_isCreation){
		for (auto &flag : m_attrFlags)
			if (argData.isFlagSet(flag.first))
				setFlagAttr(argData, flag.first, fnNode.findPlug(flag.second, true));
	}

	return redoIt();
}

MStatus StitchEasyCmd::redoIt()
{
	m_dgMod.doIt();
	m_dagMod.doIt();

	if (m_isCreation) {
		MStringArray resultArray;

		MFnDagNode fnTransform(m_transform);
		resultArray.append(fnTransform.name());

		MFnDependencyNode fnNode(m_node);
		resultArray.append(fnNode.name());

		// Assign shader
		MFnDagNode fnShape(m_shape);
		MDagPath path;
		fnShape.getPath(path);

		MSelectionList      selList;
		MObject             initialSG;

		MGlobal::getSelectionListByName(MString("initialShadingGroup"), selList);
		selList.getDependNode(0, initialSG);
		SShadingGroup::AssignToShadingGroup(initialSG, path);

		setResult(resultArray);

		MGlobal::select(path.node(), MGlobal::kReplaceList);
	}

	return MS::kSuccess;
}

MStatus StitchEasyCmd::undoIt()
{
	m_dagMod.undoIt();
	m_dgMod.undoIt();

	return MS::kSuccess;
}

MStatus StitchEasyCmd::queryAttrValue(MPlug& attrPlug) {
	MStatus status;

	for (unsigned int i = 0; i < ((attrPlug.isCompound()) ? attrPlug.numChildren() : 1); i++) {
		MPlug pAttr = (attrPlug.isCompound()) ? attrPlug.child(i) : attrPlug;
		
		MDataHandle hAttr = pAttr.asMDataHandle();
		MFnNumericData::Type type = hAttr.numericType();

		switch (type) {
		case MFnNumericData::kBoolean:
			appendToResult(pAttr.asBool());
			break;
		case MFnNumericData::kInt:
			appendToResult(pAttr.asInt());
			break;
		case MFnNumericData::kFloat:
			appendToResult(pAttr.asFloat());
			break;
		case MFnNumericData::kDouble:
			appendToResult(pAttr.asDouble());
			break;
		default:
			if (StitchEasyNode::aRotate == attrPlug)
				appendToResult(pAttr.asMAngle().as(MAngle::uiUnit()));
			else 
				appendToResult(pAttr.asMDistance().as(MDistance::uiUnit()));
			break;
		}
	}

	return MS::kSuccess;
}

MStatus StitchEasyCmd::setFlagAttr(MArgDatabase& argData, char *flag, MPlug& attrPlug) {
	MStatus status;

	for (unsigned int i = 0; i < ((attrPlug.isCompound()) ? attrPlug.numChildren() : 1); i++) {
		MPlug pAttr = (attrPlug.isCompound()) ? attrPlug.child(i) : attrPlug;

		MDataHandle hAttr = pAttr.asMDataHandle();
		MFnNumericData::Type type = hAttr.numericType();

		switch (type) {
		case MFnNumericData::kBoolean:
			m_dagMod.newPlugValueBool(pAttr, argData.flagArgumentBool(flag, i));
			break;
		case MFnNumericData::kInt:
			m_dagMod.newPlugValueInt(pAttr, argData.flagArgumentInt(flag, i));
			break;
		case MFnNumericData::kFloat:
			m_dagMod.newPlugValueFloat(pAttr, (float)argData.flagArgumentDouble(flag, i));
			break;
		case MFnNumericData::kDouble:
			m_dagMod.newPlugValueDouble(pAttr, argData.flagArgumentDouble(flag, i));
			break;
		default:
			if (StitchEasyNode::aRotate == attrPlug)
				m_dagMod.newPlugValueMAngle(pAttr, argData.flagArgumentMAngle(flag, i));
			else
				m_dagMod.newPlugValueMDistance(pAttr, argData.flagArgumentMDistance(flag, i));
			break;
		}
	}

	return MS::kSuccess;
}