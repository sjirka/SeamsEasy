#include "SeamsEasyCmd.h"
#include "StitchEasyNode.h"

#include <maya\MSyntax.h>
#include <maya\MSelectionList.h>
#include <maya\MItSelectionList.h>
#include <maya\MDagPath.h>
#include <maya\MFnDependencyNode.h>
#include <maya\MFnDagNode.h>
#include <maya\MGlobal.h>
#include <maya\MFnComponentListData.h>
#include <maya\MFnSet.h>
#include <maya\MFnTransform.h>
#include <maya\MMatrix.h>
#include <maya\MItDependencyNodes.h>
#include <maya\MArgList.h>
#include <maya\MAngle.h>

SeamsEasyCmd::SeamsEasyCmd()
{
}

SeamsEasyCmd::~SeamsEasyCmd()
{
}

void *SeamsEasyCmd::creator()
{
	return new SeamsEasyCmd;
}

MSyntax SeamsEasyCmd::newSyntax()
{
	MSyntax syntax;

	syntax.addFlag(THICKNESS_FLAG, THICKNESS_FLAG_LONG, MSyntax::kDistance);
	syntax.addFlag(DIVISIONS_FLAG, DIVISIONS_FLAG_LONG, MSyntax::kLong);
	syntax.addFlag(EXTRUDEALL_FLAG, EXTRUDEALL_FLAG_LONG, MSyntax::kBoolean);
	syntax.addFlag(GAP_FLAG, GAP_FLAG_LONG, MSyntax::kDistance);
	syntax.addFlag(MODE_FLAG, MODE_FLAG_LONG, MSyntax::kLong);
	syntax.addFlag(WIDTH_FLAG, WIDTH_FLAG_LONG, MSyntax::kDistance);
	syntax.addFlag(DEPTH_FLAG, DEPTH_FLAG_LONG, MSyntax::kDistance);
	syntax.addFlag(SUBDIVS_FLAG, SUBDIVS_FLAG_LONG, MSyntax::kLong);
	syntax.addFlag(ADDLOOP_FLAG, ADDLOOP_FLAG_LONG, MSyntax::kDistance, MSyntax::kDistance, MSyntax::kBoolean);
	syntax.makeFlagMultiUse(ADDLOOP_FLAG);
	syntax.addFlag(SORT_FLAG, SORT_FLAG_LONG, MSyntax::kString, MSyntax::kString);
	syntax.addFlag(DISTANCEMULTI_FLAG, DISTANCEMULTI_FLAG_LONG, MSyntax::kDouble);
	syntax.addFlag(DEPTHMULTI_FLAG, DEPTHMULTI_FLAG_LONG, MSyntax::kDouble);
	syntax.addFlag(HARDEDGEANGLE_FLAG, HARDEDGEANGLE_FLAG_LONG, MSyntax::kAngle);

	syntax.addArg(MSyntax::kString);

	syntax.useSelectionAsDefault(true);
	syntax.setObjectType(MSyntax::kSelectionList, 1);
	syntax.enableEdit(true);
	syntax.enableQuery(true);

	return syntax;
}

MStatus SeamsEasyCmd::doIt(const MArgList& args)
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

		MObject component;
		selection.getDagPath(0, m_path, component);
		if (component.apiType() != MFn::kMeshEdgeComponent) {
			displayError("Invalid selection. Select mesh edges and retry");
			setResult(false);
			return status;
		}

		MFnDagNode
			fnMesh(m_path);
		MPlug
			pOutputMesh;

		MPlugArray incomingConnections;
		bool connected = fnMesh.findPlug("inMesh").connectedTo(incomingConnections, true, false);
		if(connected)
			m_dgMod.disconnect(incomingConnections[0], fnMesh.findPlug("inMesh"));

		MIntArray tweakOutPoints;
		int numTweaks = fnMesh.findPlug("pnts").getExistingArrayAttributeIndices(tweakOutPoints);

		MObject tweakNode;
		if (0<numTweaks) {
			tweakNode = m_dgMod.createNode("polyTweak", &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);

			MFnDependencyNode fnTweak(tweakNode, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			for (int t = 0; t < numTweaks; t++) {
				MPlug
					pTweakOutPoint = fnMesh.findPlug("pnts").elementByLogicalIndex(tweakOutPoints[t]),
					pTweakInPoint = fnTweak.findPlug("tweak").elementByLogicalIndex(tweakOutPoints[t]);

				for (unsigned int c = 0; c < pTweakOutPoint.numChildren(); c++) {
					status = m_dgMod.newPlugValueFloat(pTweakInPoint.child(c), pTweakOutPoint.child(c).asFloat());
					CHECK_MSTATUS_AND_RETURN_IT(status);
				}

				status = m_dgMod.removeMultiInstance(pTweakOutPoint, true);
				CHECK_MSTATUS_AND_RETURN_IT(status);
			}
			pOutputMesh = fnTweak.findPlug("output");

			if(connected)
				m_dgMod.connect(incomingConnections[0], fnTweak.findPlug("inputPolymesh"));
		}
		else if(connected)
			pOutputMesh = incomingConnections[0];

		m_node = m_dgMod.createNode(SeamsEasyNode::id, &status);
		status = m_dgMod.doIt();
			
		if (!connected) {
			MFnMesh fnIntermediate;
			m_intermediate = fnIntermediate.copy(m_path.node(), m_path.transform());
			fnIntermediate.setIntermediateObject(true);

			if (!tweakNode.isNull()) {
				MFnDependencyNode fnTweak(tweakNode);
				m_dagMod.connect(fnIntermediate.findPlug("outMesh"), fnTweak.findPlug("inputPolymesh"));
			}
			else
				pOutputMesh = fnIntermediate.findPlug("outMesh");
		}

	
		// Create plugin node and make connections ////////////////////////////////////////////////

		MFnDependencyNode
			fnNode(m_node);
		fnNode.setName((name == "") ? "seamsEasy" : name);

		status = m_dagMod.connect(pOutputMesh, fnNode.findPlug(SeamsEasyNode::aInMesh));
		CHECK_MSTATUS_AND_RETURN_IT(status);
		status = m_dagMod.connect(fnNode.findPlug(SeamsEasyNode::aOutMesh), fnMesh.findPlug("inMesh"));
		CHECK_MSTATUS_AND_RETURN_IT(status);

		MFnComponentListData compData;
		MObject compList = compData.create();
		compData.add(component);

		status = m_dagMod.newPlugValue(fnNode.findPlug(SeamsEasyNode::aSelectedEdges), compList);
		CHECK_MSTATUS_AND_RETURN_IT(status);
	}

	if (isQuery || isEdit) {
		status = SNode::getPluginNode(name, SeamsEasyNode::id, m_node);
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
				queryAttrValue(fnNode.findPlug(flag.second));
				break;
			}
	}
	else if (isEdit || m_isCreation) {
		for (auto &flag : m_attrFlags)
			if (argData.isFlagSet(flag.first))
				setFlagAttr(argData, flag.first, fnNode.findPlug(flag.second));

		if (argData.isFlagSet(ADDLOOP_FLAG)) {
			MPlug offset = fnNode.findPlug(SeamsEasyNode::aOffset);
			MIntArray indices;
			int lowestAvId = (offset.getExistingArrayAttributeIndices(indices) == 0) ? 0 : indices[indices.length() - 1] + 1;

			for (unsigned int i = 0; i < argData.numberOfFlagUses(ADDLOOP_FLAG); i++) {
				MArgList argList;
				argData.getFlagArgumentList(ADDLOOP_FLAG, i, argList);

				MPlug pDistance = offset.elementByLogicalIndex(lowestAvId).child(SeamsEasyNode::aOffsetDistance);
				m_dgMod.newPlugValueMDistance(pDistance, argList.asDistance(0));

				MPlug pDepth = offset.elementByLogicalIndex(lowestAvId).child(SeamsEasyNode::aOffsetDepth);
				m_dgMod.newPlugValueMDistance(pDepth, argList.asDistance(1));

				MPlug pStitch = offset.elementByLogicalIndex(lowestAvId).child(SeamsEasyNode::aOffsetStitch);
				m_dgMod.newPlugValueBool(pDepth, argList.asBool(2));

				lowestAvId++;
			}
		}

		if (argData.isFlagSet(SORT_FLAG)) {
			MString value = argData.flagArgumentString(SORT_FLAG, 0);
			MString order = argData.flagArgumentString(SORT_FLAG, 1);

			if (value == "distance")
				OffsetParams::compare = OffsetParams::kDistance;
			else if (value == "depth")
				OffsetParams::compare = OffsetParams::kDepth;
			else {
				displayError("invalid value");
				setResult(false);
				return status;
			}

			std::set <OffsetParams> offsetParams;

			MPlug pOffset = fnNode.findPlug(SeamsEasyNode::aOffset);
			MIntArray offsetIndices;

			for (unsigned int i = 0; i < pOffset.getExistingArrayAttributeIndices(offsetIndices); i++) {
				MPlug pOffsetElement = pOffset.elementByLogicalIndex(offsetIndices[i]);
				OffsetParams offsetParam(
					pOffsetElement.child(SeamsEasyNode::aOffsetDistance).asFloat(),
					pOffsetElement.child(SeamsEasyNode::aOffsetDepth).asFloat(),
					pOffsetElement.child(SeamsEasyNode::aOffsetStitch).asBool(),
					offsetIndices[i]
				);

				offsetParams.insert(offsetParam);
			}

			if (order == "asc" || order == "desc") {
				auto param = offsetParams.begin();
				auto rparam = offsetParams.rbegin();

				for (unsigned int i = 0; i < offsetIndices.length(); i++) {
					MPlug pOffsetElement = pOffset.elementByLogicalIndex(offsetIndices[i]);
					m_dagMod.newPlugValueFloat(pOffsetElement.child(SeamsEasyNode::aOffsetDistance), (order == "asc") ? param->distance : rparam->distance);
					m_dagMod.newPlugValueFloat(pOffsetElement.child(SeamsEasyNode::aOffsetDepth), (order == "asc") ? param->depth : rparam->depth);
					m_dagMod.newPlugValueBool(pOffsetElement.child(SeamsEasyNode::aOffsetStitch), (order == "asc") ? param->stitch : rparam->stitch);
					param++; rparam++;
				}
			}
			else {
				displayError("invalid value");
				setResult(false);
				return status;
			}
		}
	}

	m_isDoIt = true;

	return redoIt();
}

MStatus SeamsEasyCmd::redoIt()
{
	MStatus status;

	if (m_isDoIt && !m_intermediate.isNull()) {
		MFnDagNode
			fnShape(m_path),
			fnIntermediate(m_intermediate);

		MGlobal::executeCommand("if(`isConnected  \"" + fnIntermediate.name() + ".outMesh\" \"" + fnShape.name() + ".inMesh\"`) disconnectAttr  \"" + fnIntermediate.name() + ".outMesh\" \"" + fnShape.name() + ".inMesh\";", false, false);
	}

	status = m_dgMod.doIt();
	CHECK_MSTATUS_AND_RETURN_IT(status);
	status = m_dagMod.doIt();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	if (m_isCreation) {
		MFnDependencyNode fnNode(m_node);
		setResult(fnNode.name());

		MGlobal::select(m_node, MGlobal::kReplaceList);
	}

	return MS::kSuccess;
}

MStatus SeamsEasyCmd::undoIt()
{
	MStatus status;

	status = m_dagMod.undoIt();
	CHECK_MSTATUS_AND_RETURN_IT(status);
	status = m_dgMod.undoIt();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	if (!m_intermediate.isNull()) {
		MFnDagNode
			fnShape(m_path),
			fnIntermediate(m_intermediate);

		MGlobal::executeCommand("connectAttr \"" + fnIntermediate.name() + ".outMesh\" \"" + fnShape.name() + ".inMesh\";", false, false);
	}

	return MS::kSuccess;
}

MStatus SeamsEasyCmd::queryAttrValue(MPlug& attrPlug) {
	MStatus status;

	MDataHandle hAttr = attrPlug.asMDataHandle();
	MFnNumericData::Type type = hAttr.numericType();

	if (attrPlug == SeamsEasyNode::aProfileMode)
		appendToResult(attrPlug.asShort());
	else if (attrPlug == SeamsEasyNode::aHardEdgeAngle)
		appendToResult(attrPlug.asMAngle().as(MAngle::uiUnit()));
	else {
		switch (type) {
		case MFnNumericData::kBoolean:
			appendToResult(attrPlug.asBool());
			break;
		case MFnNumericData::kInt:
			appendToResult(attrPlug.asInt());
			break;
		case MFnNumericData::kShort:
			appendToResult(attrPlug.asShort());
			break;
		case MFnNumericData::kFloat:
			appendToResult(attrPlug.asFloat());
			break;
		case MFnNumericData::kDouble:
			appendToResult(attrPlug.asDouble());
			break;
		default:
			appendToResult(attrPlug.asMDistance().as(MDistance::uiUnit()));
			break;
		}
	}

	return MS::kSuccess;
}

MStatus SeamsEasyCmd::setFlagAttr(MArgDatabase& argData, char *flag, MPlug& attrPlug) {
	MStatus status;

	MDataHandle hAttr = attrPlug.asMDataHandle();
	MFnNumericData::Type type = hAttr.numericType();

	if (attrPlug == SeamsEasyNode::aProfileMode) {
		int value = argData.flagArgumentBool(flag, 0, &status);
		status = m_dagMod.newPlugValueInt(attrPlug, value);
	}
	else if (attrPlug == SeamsEasyNode::aHardEdgeAngle) {
		MAngle value = argData.flagArgumentMAngle(flag, 0, &status);
		status = m_dagMod.newPlugValueMAngle(attrPlug, value);
	}
	else {
		switch (type) {
		case MFnNumericData::kBoolean: {
			bool value = argData.flagArgumentBool(flag, 0, &status);
			status = m_dagMod.newPlugValueBool(attrPlug, value);
			break; }
		case MFnNumericData::kInt: {
			int value = argData.flagArgumentInt(flag, 0, &status);
			status = m_dagMod.newPlugValueInt(attrPlug, value);
			break; }
		case MFnNumericData::kShort: {
			short value = (short)argData.flagArgumentInt(flag, 0, &status);
			status = m_dagMod.newPlugValueShort(attrPlug, value);
			break; }
		case MFnNumericData::kFloat: {
			float value = (float)argData.flagArgumentDouble(flag, 0, &status);
			status = m_dagMod.newPlugValueFloat(attrPlug, value);
			break; }
		case MFnNumericData::kDouble: {
			double value = argData.flagArgumentDouble(flag, 0, &status);
			status = m_dagMod.newPlugValueDouble(attrPlug, value);
			break; }
		default: {
			MDistance value = argData.flagArgumentMDistance(flag, 0, &status);
			status = m_dagMod.newPlugValueMDistance(attrPlug, value);
			break; }
		}
	}

	return MS::kSuccess;
}