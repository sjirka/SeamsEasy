#include "SeamsEasyNode.h"
#include "StitchEasyNode.h"
#include "../_library/SNode.h"
#include "../_library/SPlane.h"

#include <maya\MFnNumericAttribute.h>
#include <maya\MFnTypedAttribute.h>
#include <maya\MFnCompoundAttribute.h>
#include <maya\MFnComponentListData.h>
#include <maya\MFnMesh.h>
#include <maya\MFnMeshData.h>
#include <maya\MFnSingleIndexedComponent.h>
#include <maya\MGlobal.h>
#include <maya\MPointArray.h>
#include <maya\MFloatPointArray.h>
#include <maya\MPlugArray.h>
#include <maya\MFnSet.h>
#include <maya\MFnEnumAttribute.h>
#include <maya\MFnUnitAttribute.h>
#include <maya\MRampAttribute.h>
#include <maya\MDistance.h>
#include <maya\MDGModifier.h>
#include <maya\MDagModifier.h>
#include <maya\MItDependencyGraph.h>

MTypeId SeamsEasyNode::id(0x00127891);

MObject SeamsEasyNode::aOutMesh;
MObject SeamsEasyNode::aOutStitchLines;

MObject SeamsEasyNode::aInMesh;
MObject SeamsEasyNode::aSelectedEdges;

MObject SeamsEasyNode::aExtrudeAllBoundaries;
MObject SeamsEasyNode::aExtrudeThickness;
MObject SeamsEasyNode::aExtrudeDivisions;
MObject SeamsEasyNode::aGap;

MObject SeamsEasyNode::aProfileMode;

MObject SeamsEasyNode::aOffset;
MObject SeamsEasyNode::aOffsetDistance;
MObject SeamsEasyNode::aOffsetDepth;
MObject SeamsEasyNode::aOffsetStitch;

MObject SeamsEasyNode::aDistanceMultiplier;
MObject SeamsEasyNode::aDepthMultiplier;

MObject SeamsEasyNode::aProfileWidth;
MObject SeamsEasyNode::aProfileDepth;
MObject SeamsEasyNode::aProfileSubdivs;
MObject SeamsEasyNode::aProfileCurve;

OffsetParams::Compare OffsetParams::compare = OffsetParams::kDistance;

SeamsEasyNode::SeamsEasyNode(){
}

void SeamsEasyNode::postConstructor(){
	callbackIds.append(MNodeMessage::addAttributeChangedCallback(thisMObject(), attrChanged, NULL));

	// Set default profile curve shape
	MRampAttribute rAttr(thisMObject(), aProfileCurve);
	rAttr.setValueAtIndex(1, 0);

	MFloatArray positions, values;
	MIntArray interpolation;

	positions.append(0.5f);
	values.append(0.85f);
	interpolation.append(MRampAttribute::kSpline);
	
	positions.append(0.85f);
	values.append(0.5f);
	interpolation.append(MRampAttribute::kSpline);
	
	positions.append(1.0f);
	values.append(0.0f);
	interpolation.append(MRampAttribute::kSpline);
	
	rAttr.addEntries(positions, values, interpolation);
}

SeamsEasyNode::~SeamsEasyNode(){
	MMessage::removeCallbacks(callbackIds);
}

void *SeamsEasyNode::creator()
{
	return new SeamsEasyNode;
}

MStatus SeamsEasyNode::initialize()
{
	MStatus status;

	MFnNumericAttribute nAttr;
	MFnTypedAttribute tAttr;
	MFnCompoundAttribute cAttr;
	MFnEnumAttribute eAttr;
	MFnUnitAttribute uAttr;
	MRampAttribute rAttr;

	aOutMesh = tAttr.create("outMesh", "outMesh", MFnData::kMesh, &status);
	tAttr.setWritable(false);
	addAttribute(aOutMesh);

	aOutStitchLines = tAttr.create("stitchLines", "stitchLines", MFnData::kComponentList, &status);
	tAttr.setWritable(false);
	addAttribute(aOutStitchLines);

	aInMesh = tAttr.create("inMesh", "inMesh", MFnData::kMesh, &status);
	addAttribute(aInMesh);
	attributeAffects(aInMesh, aOutMesh);
	attributeAffects(aInMesh, aOutStitchLines);

	aSelectedEdges = tAttr.create("seamLines", "seamLines", MFnData::kComponentList, &status);
	addAttribute(aSelectedEdges);
	attributeAffects(aSelectedEdges, aOutMesh);
	attributeAffects(aSelectedEdges, aOutStitchLines);

	// Extrude attributes /////////////////////////////////////////////////////////////////////////
	aExtrudeAllBoundaries = nAttr.create("extrudeAllBoundaries", "extrudeAllBoundaries", MFnNumericData::kBoolean, 0, &status);
	addAttribute(aExtrudeAllBoundaries);
	attributeAffects(aExtrudeAllBoundaries, aOutMesh);

	aExtrudeThickness = uAttr.create("thickness", "thickness", MFnUnitAttribute::kDistance, 1.0, &status);
	uAttr.setSoftMin(0);
	uAttr.setSoftMax(10);
	addAttribute(aExtrudeThickness);
	attributeAffects(aExtrudeThickness, aOutMesh);

	aExtrudeDivisions = nAttr.create("divisions", "divisions", MFnNumericData::kInt, 0, &status);
	nAttr.setMin(0);
	nAttr.setSoftMax(10);
	addAttribute(aExtrudeDivisions);
	attributeAffects(aExtrudeDivisions, aOutMesh);

	aGap = uAttr.create("gap", "gap", MFnUnitAttribute::kDistance, 0.0, &status);
	uAttr.setMin(0);
	uAttr.setSoftMax(10);
	addAttribute(aGap);
	attributeAffects(aGap, aOutMesh);

	// Profile attributes //////////////////////////////////////////////////////////////////////////
	aProfileMode = eAttr.create("profileMode", "profileMode");
	eAttr.addField("Manual", 0);
	eAttr.addField("Curve", 1);
	addAttribute(aProfileMode);
	attributeAffects(aProfileMode, aOutMesh);
	attributeAffects(aProfileMode, aOutStitchLines);

	aProfileWidth = uAttr.create("profileWidth", "profileWidth", MFnUnitAttribute::kDistance, 1.0);
	uAttr.setMin(0);
	uAttr.setSoftMax(1);
	addAttribute(aProfileWidth);
	attributeAffects(aProfileWidth, aOutMesh);

	aProfileDepth = uAttr.create("profileDepth", "profileDepth", MFnUnitAttribute::kDistance, -1.0);
	uAttr.setSoftMin(-1);
	uAttr.setSoftMax(1);
	addAttribute(aProfileDepth);
	attributeAffects(aProfileDepth, aOutMesh);

	aProfileSubdivs = nAttr.create("profileSubdivisions", "profileSubdivs", MFnNumericData::kInt, 4);
	nAttr.setMin(0);
	nAttr.setSoftMax(10);
	addAttribute(aProfileSubdivs);
	attributeAffects(aProfileSubdivs, aOutMesh);
	
	aProfileCurve = rAttr.createCurveRamp("profileCurve", "profileCurve");
	addAttribute(aProfileCurve);
	attributeAffects(aProfileCurve, aOutMesh);
	
	// Offset attributes //////////////////////////////////////////////////////////////////////////
	
	aOffset = cAttr.create("offset", "offset", &status);
	cAttr.setArray(true);

	aOffsetDistance = uAttr.create("distance", "distance", MFnUnitAttribute::kDistance, 0.0, &status);
	uAttr.setMin(0);
	uAttr.setSoftMax(10);
	addAttribute(aOffsetDistance);
	attributeAffects(aOffsetDistance, aOutMesh);
	attributeAffects(aOffsetDistance, aOutStitchLines);
	cAttr.addChild(aOffsetDistance);

	aOffsetDepth = uAttr.create("depth", "depth", MFnUnitAttribute::kDistance, 0.0, &status);
	uAttr.setSoftMin(-1);
	uAttr.setSoftMax(1);
	addAttribute(aOffsetDepth);
	attributeAffects(aOffsetDepth, aOutMesh);
	cAttr.addChild(aOffsetDepth);

	aOffsetStitch = nAttr.create("stitch", "stitch", MFnNumericData::kBoolean, 0, &status);
	addAttribute(aOffsetStitch);
	attributeAffects(aOffsetStitch, aOutMesh);
	attributeAffects(aOffsetStitch, aOutStitchLines);
	cAttr.addChild(aOffsetStitch);

	addAttribute(aOffset);
	attributeAffects(aOffset, aOutMesh);
	attributeAffects(aOffset, aOutStitchLines);

	aDistanceMultiplier = nAttr.create("distanceMultiplier", "distanceMultiplier", MFnNumericData::kFloat, 1.0, &status);
	nAttr.setMin(0);
	nAttr.setSoftMax(10);
	addAttribute(aDistanceMultiplier);
	attributeAffects(aDistanceMultiplier, aOutMesh);
	attributeAffects(aDistanceMultiplier, aOutStitchLines);

	aDepthMultiplier = nAttr.create("depthMultiplier", "depthMultiplier", MFnNumericData::kFloat, 1.0, &status);
	nAttr.setMin(0);
	nAttr.setSoftMax(10);
	addAttribute(aDepthMultiplier);
	attributeAffects(aDepthMultiplier, aOutMesh);
	attributeAffects(aDepthMultiplier, aOutStitchLines);

	return MS::kSuccess;
}

MStatus SeamsEasyNode::setDependentsDirty(const MPlug &plug, MPlugArray &affected) {
	if (aInMesh == plug)
		dirtyMesh = true;

	if (aSelectedEdges == plug)
		dirtyComponent = true;

	if (aExtrudeAllBoundaries == plug ||
		aExtrudeThickness == plug ||
		aExtrudeDivisions == plug)
		dirtyExtrude = true;

	if (aGap == plug ||
		aProfileMode == plug ||
		aProfileWidth == plug ||
		aProfileDepth == plug ||
		aProfileSubdivs == plug ||
		aProfileCurve == plug.parent().array() ||
		aOffset == plug ||
		aOffsetDepth == plug ||
		aOffsetDistance == plug ||
		aOffsetStitch == plug ||
		aDepthMultiplier == plug||
		aDistanceMultiplier == plug)
		dirtyProfile = true;

	return MS::kSuccess;
}

MStatus SeamsEasyNode::compute(const MPlug &plug, MDataBlock &dataBlock) {
	MStatus status;

	if (plug != aOutMesh)
		return MS::kUnknownParameter;

	// Load attribs ///////////////////////////////////////////////////////////////////////////////
	if (dirtyMesh || m_sourceMesh.isNull()) {
		MObject sourceMesh = dataBlock.inputValue(aInMesh).asMesh();
		if (m_sourceMesh.isNull() || !SMesh::isEquivalent(sourceMesh, m_sourceMesh))
			dirtyBaseMesh = true;
		m_sourceMesh = sourceMesh;
	}
	if (dirtyComponent || m_component.isNull()) {
		dirtyBaseMesh = true;
		dirtyComponent = false;
		MObject oSeamsEasyData = dataBlock.inputValue(aSelectedEdges).data();
		MFnComponentListData compListData(oSeamsEasyData, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		m_component = compListData[0];
	}
	if (m_component.apiType() != MFn::kMeshEdgeComponent || m_sourceMesh.isNull())
		return MS::kInvalidParameter;

	// Load numeric attribs ///////////////////////////////////////////////////////////////////////
	bool extrudeAll = dataBlock.inputValue(aExtrudeAllBoundaries).asBool();
	float thickness = (float)dataBlock.inputValue(aExtrudeThickness).asDistance().as(MDistance::internalUnit());
	unsigned int divisions = dataBlock.inputValue(aExtrudeDivisions).asInt();

	// Load profile settings //////////////////////////////////////////////////////////////////////
	std::set <OffsetParams> offsetParams;
	OffsetParams::compare = OffsetParams::kDistance;

	if (dirtyProfile) {
		std::set <OffsetParams> offsetParamsTmp;

		dirtyProfile = false;
		dirtyProfileMesh = true;

		int profileMode = dataBlock.inputValue(aProfileMode).asInt();

		switch (profileMode)
		{
		case 1: { // profile curve mode
			int subdivisions = dataBlock.inputValue(aProfileSubdivs).asInt();
			float width = (float)dataBlock.inputValue(aProfileWidth).asDistance().as(MDistance::internalUnit());
			float depth = (float)dataBlock.inputValue(aProfileDepth).asDistance().as(MDistance::internalUnit());
			float span = 1 / (float)(subdivisions + 1);

			MRampAttribute rampAttr(thisMObject(), aProfileCurve);

			float firstValue;
			float srcMin = -1, max = -1;

			for (int s = 0; s < subdivisions + 2; s++) {
				OffsetParams newParam;

				float param = s*span;
				newParam.index = s;
				newParam.distance = width - (param*width);

				rampAttr.getValueAtPosition(param, newParam.depth, &status);
				CHECK_MSTATUS_AND_RETURN_IT(status);

				if (s == 0)
					firstValue = newParam.depth;

				if (srcMin == -1 || newParam.depth < srcMin)
					srcMin = newParam.depth;

				if (max == -1 || newParam.depth > max)
					max = newParam.depth;

				offsetParamsTmp.insert(newParam);
			}

			float trgMin = depth, trgMax = 0;
			float srcMax = (firstValue == srcMin) ? max : firstValue;

			for (auto oldParam : offsetParamsTmp) {
				OffsetParams newParam = oldParam;
				newParam.depth = remap(srcMin, srcMax, trgMin, trgMax, newParam.depth);
				offsetParams.insert(newParam);
			}

			break;
		}
		default: { // manual mode
			float distanceMultiplier = dataBlock.inputValue(aDistanceMultiplier).asFloat();
			float depthMultiplier = dataBlock.inputValue(aDepthMultiplier).asFloat();

			MArrayDataHandle hOffsetArray = dataBlock.inputArrayValue(aOffset);
			for (unsigned int i = 0; i < hOffsetArray.elementCount(); i++) {
				OffsetParams newParam;

				hOffsetArray.jumpToArrayElement(i);
				MDataHandle hOffsetElement = hOffsetArray.inputValue();

				newParam.index = i;
				newParam.distance = (float)hOffsetElement.child(aOffsetDistance).asDistance().as(MDistance::internalUnit())*distanceMultiplier;
				newParam.depth = (float)hOffsetElement.child(aOffsetDepth).asDistance().as(MDistance::internalUnit())*depthMultiplier;
				newParam.stitch = hOffsetElement.child(aOffsetStitch).asBool();

				offsetParams.insert(newParam);
			}
			break;
		}
		}
	}

	if (offsetParams.size() == 0 || offsetParams.begin()->distance != 0){
			OffsetParams newParam(0, 0, 0, dataBlock.inputArrayValue(aOffset).elementCount());
			offsetParams.insert(newParam);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// Update base mesh
	if (dirtyMesh || dirtyBaseMesh) {
		dirtyMesh = false;
		dirtyProfileMesh = true;

		if (!dirtyBaseMesh) {
			status = m_baseMesh.updateMesh(m_sourceMesh);
			CHECK_MSTATUS_AND_RETURN_IT(status);
		}
		else {
			dirtyBaseMesh = false;

			// Get splitline IDs
			MFnSingleIndexedComponent fnComponent(m_component, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			MIntArray splitlineArray;
			fnComponent.getElements(splitlineArray);

			// Copy source mesh
			MFnMeshData meshData;
			MObject workMesh = meshData.create();
			MFnMesh fnSourceMesh(m_sourceMesh);
			fnSourceMesh.copy(m_sourceMesh, workMesh, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);

			// Create base mesh
			m_baseMesh = SSeamMesh(workMesh);
			status = m_baseMesh.detachEdges(splitlineArray);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			status = m_baseMesh.transferEdges(m_sourceMesh, splitlineArray);
			CHECK_MSTATUS_AND_RETURN_IT(status);
		}
	}
	// Update profile mesh
	if (dirtyProfileMesh) {
		dirtyProfileMesh = false;

		float gap = (float)dataBlock.inputValue(aGap).asDistance().as(MDistance::internalUnit());

		m_profileMesh = SSeamMesh(m_baseMesh);
		
		// Componend to be passed to stitch node
		MFnComponentListData compListData;
		MObject compList = compListData.create();
		MFnSingleIndexedComponent stitchCompData;
		MObject stitchComponent = stitchCompData.create(MFn::kMeshEdgeComponent);

		// Insert new loops
		std::map <unsigned int, MIntArray> loopEdges;
		for (auto param = offsetParams.rbegin(); param != offsetParams.rend(); param++) {
			m_profileMesh.getActiveEdges(loopEdges[param->index]);
			if (param->stitch)
				stitchCompData.addElements(loopEdges[param->index]);
			status = m_profileMesh.offsetEdgeloops(param->distance + gap / 2, (0 == param->distance && param==--offsetParams.rend()) ? false : true);
			CHECK_MSTATUS_AND_RETURN_IT(status);
		}

		status = compListData.add(stitchComponent);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		status = dataBlock.outputValue(aOutStitchLines).set(compList);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		// Pull loop vertices
		for (auto param : offsetParams){
			MIntArray loopVertices;
			status = m_profileMesh.getEdgeVertices(loopEdges[param.index], loopVertices);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			status = m_profileMesh.pullVertices(loopVertices, param.depth);
			CHECK_MSTATUS_AND_RETURN_IT(status);
		}
	}

	SSeamMesh extrudeMesh(m_profileMesh);
	if (thickness != 0) {
		MIntArray edgesToExtrude;
		if (extrudeAll)
			extrudeMesh.getBoundaryEdges(edgesToExtrude);
		else
			extrudeMesh.getActiveEdges(edgesToExtrude);

		status = extrudeMesh.extrudeEdges(edgesToExtrude, thickness, divisions);
		CHECK_MSTATUS_AND_RETURN_IT(status);
	}

	status = dataBlock.outputValue(aOutMesh).set(extrudeMesh.getObject());
	CHECK_MSTATUS_AND_RETURN_IT(status);
	dataBlock.setClean(aOutMesh);

	return MS::kSuccess;
}

float SeamsEasyNode::remap(const float srcMin, const float srcMax, const float trgMin, const float trgMax, float value) {
	float srcDomain = srcMax - srcMin;
	float trgDomain = trgMax - trgMin;

	if (srcDomain == 0)
		return value;

	return trgMin + (value - srcMin) * trgDomain / srcDomain;
}

void SeamsEasyNode::attrChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *data) {
	if (plug != aOffsetStitch)
		return;

	MPlug pOffset = plug.parent().array();

	bool shouldConnect = false;
	for (unsigned int i = 0; i < pOffset.numElements(); i++)
		if (pOffset.elementByPhysicalIndex(i).child(aOffsetStitch).asBool()) {
			shouldConnect = true;
			break;
		}

	MFnDependencyNode fnNode(plug.node());
	MPlugArray targets;
	bool isConnected = fnNode.findPlug(aOutStitchLines).connectedTo(targets, false, true);

	MDGModifier dgMod;
	MDagModifier dagMod;
	if (!isConnected && shouldConnect)
		dgMod.commandToExecute("stitchEasy -sn "+fnNode.name()+" "+fnNode.name()+"Stitch");
	if (isConnected && !shouldConnect) {
		for (unsigned int t = 0; t < targets.length(); t++) {
			MFnDependencyNode fnStitchNode(targets[t].node());
			if (fnStitchNode.typeId() != StitchEasyNode::id)
				continue;

			MPlugArray outMeshes;
			fnStitchNode.findPlug(StitchEasyNode::aOutMesh).connectedTo(outMeshes, false, true);
			for (unsigned int m = 0; m < outMeshes.length(); m++)
				dagMod.deleteNode(outMeshes[m].node());

			dgMod.deleteNode(targets[t].node());
		}
	}
	dgMod.doIt();
	dagMod.doIt();
	
}