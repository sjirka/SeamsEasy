#include "StitchEasyNode.h"

#include <maya\MFnNumericAttribute.h>
#include <maya\MFnTypedAttribute.h>
#include <maya\MFnCompoundAttribute.h>
#include <maya\MFnUnitAttribute.h>
#include <maya\MItDependencyGraph.h>
#include <maya\MAngle.h>

MTypeId StitchEasyNode::id(0x00127892);

MObject StitchEasyNode::aOutMesh;
MObject StitchEasyNode::aSmoothLevel;

MObject StitchEasyNode::aInMesh;
MObject StitchEasyNode::aInSmoothMesh;
MObject StitchEasyNode::aUseSmoothMesh;
MObject StitchEasyNode::aStitchLines;

MObject StitchEasyNode::aCustomGeometry;
MObject StitchEasyNode::aUseCustomGeometry;

MObject StitchEasyNode::aDistance;
MObject StitchEasyNode::aLength;
MObject StitchEasyNode::aUseLength;
MObject StitchEasyNode::aThickness;
MObject StitchEasyNode::aSkew;
MObject StitchEasyNode::aFlip;
MObject StitchEasyNode::aMultiplier;

MObject StitchEasyNode::aRotate;
MObject StitchEasyNode::aRotateX;
MObject StitchEasyNode::aRotateY;
MObject StitchEasyNode::aRotateZ;

MObject StitchEasyNode::aScale;
MObject StitchEasyNode::aScaleX;
MObject StitchEasyNode::aScaleY;
MObject StitchEasyNode::aScaleZ;

MObject StitchEasyNode::aTranslate;
MObject StitchEasyNode::aTranslateX;
MObject StitchEasyNode::aTranslateY;
MObject StitchEasyNode::aTranslateZ;

StitchEasyNode::StitchEasyNode(){
}

StitchEasyNode::~StitchEasyNode(){
}

void *StitchEasyNode::creator(){
	return new StitchEasyNode;
}

MStatus StitchEasyNode::initialize() {
	MStatus status;

	MFnTypedAttribute tAttr;
	MFnNumericAttribute nAttr;
	MFnUnitAttribute uAttr;
	MFnCompoundAttribute cAttr;

	aOutMesh = tAttr.create("outMesh", "outMesh", MFnData::kMesh);
	tAttr.setWritable(false);
	addAttribute(aOutMesh);

	// Input mesh
	aInMesh = tAttr.create("inMesh", "inMesh", MFnData::kMesh);
	addAttribute(aInMesh);
	attributeAffects(aInMesh, aOutMesh);

	aInSmoothMesh = tAttr.create("inSmoothMesh", "inSmoothMesh", MFnData::kMesh);
	tAttr.setHidden(true);
	addAttribute(aInSmoothMesh);
	attributeAffects(aInSmoothMesh, aOutMesh);

	aUseSmoothMesh = nAttr.create("useSmoothMesh", "useSmoothMesh", MFnNumericData::kBoolean, false);
	nAttr.setHidden(true);
	addAttribute(aUseSmoothMesh);
	attributeAffects(aUseSmoothMesh, aOutMesh);

	aStitchLines = tAttr.create("stitchLines", "stitchLines", MFnData::kComponentList);
	addAttribute(aStitchLines);
	attributeAffects(aStitchLines, aOutMesh);

	aCustomGeometry = tAttr.create("customGeometry", "customGeometry", MFnData::kMesh);
	tAttr.setDisconnectBehavior(MFnAttribute::kNothing);
	addAttribute(aCustomGeometry);
	attributeAffects(aCustomGeometry, aOutMesh);

	aUseCustomGeometry = nAttr.create("useCustom", "useCustom", MFnNumericData::kBoolean, false);
	addAttribute(aUseCustomGeometry);
	attributeAffects(aUseCustomGeometry, aOutMesh);

	// Parameters
	aDistance = uAttr.create("distance", "distance", MFnUnitAttribute::kDistance, 0.5);
	uAttr.setMin(0.001);
	uAttr.setSoftMin(0.1);
	uAttr.setSoftMax(10);
	addAttribute(aDistance);
	attributeAffects(aDistance, aOutMesh);

	aLength = uAttr.create("length", "length", MFnUnitAttribute::kDistance, 0.5);
	uAttr.setMin(0.001);
	uAttr.setSoftMin(0.1);
	uAttr.setSoftMax(10);
	addAttribute(aLength);
	attributeAffects(aLength, aOutMesh);

	aUseLength = nAttr.create("useLength", "useLength", MFnNumericData::kBoolean, false);
	addAttribute(aUseLength);
	attributeAffects(aUseLength, aOutMesh);

	aThickness = uAttr.create("thickness", "thickness", MFnUnitAttribute::kDistance, 0.05);
	uAttr.setMin(0.001);
	uAttr.setSoftMax(1);
	addAttribute(aThickness);
	attributeAffects(aThickness, aOutMesh);

	aSkew = uAttr.create("skew", "skew", MFnUnitAttribute::kDistance, 0.02);
	uAttr.setSoftMin(-1);
	uAttr.setSoftMax(1);
	addAttribute(aSkew);
	attributeAffects(aSkew, aOutMesh);

	aMultiplier = nAttr.create("multiplier", "multiplier", MFnNumericData::kFloat, 1.0);
	nAttr.setMin(0.001);
	nAttr.setSoftMin(0.1);
	nAttr.setSoftMax(10);
	addAttribute(aMultiplier);
	attributeAffects(aMultiplier, aOutMesh);

	aFlip = nAttr.create("flipNormal", "flipNormal", MFnNumericData::kBoolean, false);
	addAttribute(aFlip);
	attributeAffects(aFlip, aOutMesh);

	aSmoothLevel = nAttr.create("smoothLevel", "smoothLevel", MFnNumericData::kInt, 1);
	nAttr.setMin(0);
	nAttr.setSoftMax(3);
	addAttribute(aSmoothLevel);
	attributeAffects(aSmoothLevel, aOutMesh);

	// Transformations
	// Rotate
	aRotate = cAttr.create("rotate", "r");

	aRotateX = uAttr.create("rotateX", "rx", MFnUnitAttribute::kAngle, 0);
	addAttribute(aRotateX);
	attributeAffects(aRotateX, aOutMesh);
	cAttr.addChild(aRotateX);

	aRotateY = uAttr.create("rotateY", "ry", MFnUnitAttribute::kAngle, 0);
	addAttribute(aRotateY);
	attributeAffects(aRotateY, aOutMesh);
	cAttr.addChild(aRotateY);

	aRotateZ = uAttr.create("rotateZ", "rz", MFnUnitAttribute::kAngle, 0);
	addAttribute(aRotateZ);
	attributeAffects(aRotateZ, aOutMesh);
	cAttr.addChild(aRotateZ);

	addAttribute(aRotate);
	attributeAffects(aRotate, aOutMesh);

	// Translate
	aTranslate = cAttr.create("translate", "t");

	aTranslateX = uAttr.create("translateX", "tx", MFnUnitAttribute::kDistance, 0);
	addAttribute(aTranslateX);
	attributeAffects(aTranslateX, aOutMesh);
	cAttr.addChild(aTranslateX);

	aTranslateY = uAttr.create("translateY", "ty", MFnUnitAttribute::kDistance, 0);
	addAttribute(aTranslateY);
	attributeAffects(aTranslateY, aOutMesh);
	cAttr.addChild(aTranslateY);

	aTranslateZ = uAttr.create("translateZ", "tz", MFnUnitAttribute::kDistance, 0);
	addAttribute(aTranslateZ);
	attributeAffects(aTranslateZ, aOutMesh);
	cAttr.addChild(aTranslateZ);

	addAttribute(aTranslate);
	attributeAffects(aTranslate, aOutMesh);

	// Scale
	aScaleX = nAttr.create("scaleX", "scaleX", MFnNumericData::kDouble, 1);
	aScaleY = nAttr.create("scaleY", "sy", MFnNumericData::kDouble, 1);
	aScaleZ = nAttr.create("scaleZ", "sz", MFnNumericData::kDouble, 1);
	aScale = nAttr.create("scale", "s", aScaleX, aScaleY, aScaleZ);
	addAttribute(aScale);
	
	attributeAffects(aScaleX, aOutMesh);
	attributeAffects(aScaleY, aOutMesh);
	attributeAffects(aScaleZ, aOutMesh);


	return MS::kSuccess;
}

MStatus StitchEasyNode::setDependentsDirty(const MPlug &plug, MPlugArray &affected) {

	if (aInMesh == plug)
		m_dirtySourceMesh = true;

	if (aStitchLines == plug)
		m_dirtyComponent = true;

	if (aInSmoothMesh == plug ||
		aSmoothLevel == plug)
		m_dirtySmoothMesh = true;

	if (aDistance == plug ||
		aLength == plug ||
		aUseLength == plug ||
		aThickness == plug ||
		aSkew == plug ||
		aFlip == plug ||
		aMultiplier == plug ||

		aRotateX == plug ||
		aRotateY == plug ||
		aRotateZ == plug ||

		aScaleX == plug ||
		aScaleY == plug ||
		aScaleZ == plug ||

		aTranslateX == plug ||
		aTranslateY == plug ||
		aTranslateZ == plug ||
		
		aCustomGeometry == plug ||
		aUseCustomGeometry == plug)
		m_dirtyStitchMesh = true;

	if (aSmoothLevel == plug)
		m_dirtySmoothStitchMesh = true;

	return MS::kSuccess;
}

MStatus StitchEasyNode::compute(const MPlug &plug, MDataBlock &datablock) {
	MStatus status;
	
	if (plug != aOutMesh)
		return MS::kUnknownParameter;

	if (m_dirtySourceMesh || m_sourceMesh.isNull()) {
		MObject sourceMesh = datablock.inputValue(aInMesh).asMesh();
		if (m_sourceMesh.isNull() || !SMesh::isEquivalent(sourceMesh, m_sourceMesh))
			m_dirtyBaseMesh = true;
		m_sourceMesh = sourceMesh;
	}
	if (m_dirtyComponent || m_component.isNull()) {
		m_dirtyBaseMesh = true;
		m_dirtyComponent = false;
		MObject stichLineData = datablock.inputValue(aStitchLines).data();
		MFnComponentListData compListData(stichLineData, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		m_component = compListData[0];
	}
	if (m_component.apiType() != MFn::kMeshEdgeComponent || m_sourceMesh.isNull())
		return MS::kInvalidParameter;
	
	if (m_dirtySourceMesh || m_dirtyBaseMesh) {
		m_dirtySourceMesh = false;
		m_dirtySmoothMesh = true;
		if(!m_dirtyBaseMesh) {
			status = m_baseMesh.updateMesh(m_sourceMesh);
			CHECK_MSTATUS_AND_RETURN_IT(status);
		}
		else {
			m_dirtyBaseMesh = false;

			MFnSingleIndexedComponent fnComponent(m_component);
			MIntArray edges;
			fnComponent.getElements(edges);

			m_baseMesh = SMesh(m_sourceMesh, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			status = m_baseMesh.setActiveEdges(edges);
			CHECK_MSTATUS_AND_RETURN_IT(status);
		}
	}

	if (m_dirtySmoothMesh) {
		m_dirtyStitchMesh = true;
		m_dirtySmoothMesh = false;

		MObject smoothMesh = datablock.inputValue(aInSmoothMesh).asMesh();
		bool useSmoothMesh = datablock.inputValue(aUseSmoothMesh).asBool();

		m_smoothMesh = SMesh(m_baseMesh);
		if (useSmoothMesh) {
			MFnDependencyNode fnNode(thisMObject());

			MItDependencyGraph itGraph(fnNode.findPlug(aInMesh), MFn::kMesh, MItDependencyGraph::kUpstream, MItDependencyGraph::kBreadthFirst);
			if (itGraph.isDone())
				return MS::kInvalidParameter;

			MObject sourceShape = itGraph.currentItem();
			MFnMesh srcMesh(sourceShape, &status);

			CHECK_MSTATUS_AND_RETURN_IT(status);
			MMeshSmoothOptions smoothOptions;

			status = srcMesh.getSmoothMeshDisplayOptions(smoothOptions);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			status = m_smoothMesh.smoothMesh(smoothOptions);
			CHECK_MSTATUS_AND_RETURN_IT(status);
		}
	}

	if (m_dirtyStitchMesh) {
		m_dirtyStitchMesh = false;
		m_dirtySmoothStitchMesh = true;

		// Stitch parameters
		float multiplier = datablock.inputValue(aMultiplier).asFloat();
		double distance = datablock.inputValue(aDistance).asDistance().as(MDistance::internalUnit())*multiplier;
		double thickness = datablock.inputValue(aThickness).asDistance().as(MDistance::internalUnit())*multiplier;
		double skew = datablock.inputValue(aSkew).asDistance().as(MDistance::internalUnit())*multiplier;
		double length = datablock.inputValue(aLength).asDistance().as(MDistance::internalUnit())*multiplier;
		bool useLength = datablock.inputValue(aUseLength).asBool();
		bool flipNormal = datablock.inputValue(aFlip).asBool();

		MObject customMesh;
		if (datablock.inputValue(aUseCustomGeometry).asBool())
			customMesh = datablock.inputValue(aCustomGeometry).asMesh();

		// Transformations
		double3
			translate,
			rotate,
			&scale = datablock.inputValue(aScale).asDouble3();

		MDataHandle hTranslate = datablock.inputValue(aTranslate);
		translate[0] = hTranslate.child(aTranslateX).asDistance().as(MDistance::internalUnit());
		translate[1] = hTranslate.child(aTranslateY).asDistance().as(MDistance::internalUnit());
		translate[2] = hTranslate.child(aTranslateZ).asDistance().as(MDistance::internalUnit());

		MDataHandle hRotate = datablock.inputValue(aRotate);
		rotate[0] = hRotate.child(aRotateX).asAngle().as(MAngle::internalUnit());
		rotate[1] = hRotate.child(aRotateY).asAngle().as(MAngle::internalUnit());
		rotate[2] = hRotate.child(aRotateZ).asAngle().as(MAngle::internalUnit());

		MTransformationMatrix tMatrix;
		tMatrix.addRotation(rotate, MTransformationMatrix::kXYZ, MSpace::kObject);
		tMatrix.addScale(scale, MSpace::kObject);

		// Generate stitches
		std::vector <SEdgeLoop> activeLoops;
		m_smoothMesh.getActiveLoops(activeLoops);

		// Get mesh info for stitch construction
		MFnMesh fnMesh(m_smoothMesh.getObject());
		MPointArray
			meshPoints;
		MFloatVectorArray
			meshNormals;

		fnMesh.getPoints(meshPoints);
		fnMesh.getVertexNormals(false, meshNormals);

		MPointArray
			stitchPoints;
		MIntArray
			stitchCounts,
			stitchIndices,
			stitchUVCounts,
			stitchUVIndices;
		MFloatArray
			stitchUs,
			stitchVs;

		for (auto &loop : activeLoops) {
			double loopLength;
			status = loop.getLength(loopLength);
			CHECK_MSTATUS_AND_RETURN_IT(status);

			unsigned int numStitches = (unsigned int)ceil(loopLength / distance);
			double stitchLength = loopLength / numStitches;

			unsigned int relEdgeId = 0;
			double remLength = 0;
			int2 vertices;
			bool flipped;

			status = loop.getEdgeVertices(relEdgeId, vertices, flipped);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			MVector currentEdge = meshPoints[vertices[1]] - meshPoints[vertices[0]];


			for (unsigned int s = 0; s < numStitches; s++) {
				MPoint A = meshPoints[vertices[0]] + currentEdge.normal()*remLength;

				double param = remLength + stitchLength;
				while (currentEdge.length() <= param) {
					param -= currentEdge.length();
					relEdgeId++;

					if (relEdgeId < loop.numEdges()) {
						status = loop.getEdgeVertices(relEdgeId, vertices, flipped);
						CHECK_MSTATUS_AND_RETURN_IT(status);
						currentEdge = meshPoints[vertices[1]] - meshPoints[vertices[0]];
					}
					else
						break;
				}
				if (loop.numEdges() <= relEdgeId)
					param = currentEdge.length();

				MPoint B = meshPoints[vertices[0]] + currentEdge.normal()*param;
				MPoint center(
					((A.x + B.x) / 2),
					((A.y + B.y) / 2),
					((A.z + B.z) / 2));

				MVector T = B - A;
				double curStitchLen = T.length();
				T.normalize();
				MVector N = (flipNormal) ? -1 * meshNormals[vertices[0]] : meshNormals[vertices[0]];
				MVector C = (T^N).normal();
				N = T^C;

				double orientation[4][4] = {
					{ T.x, T.y, T.z, 0 },
					{ C.x, C.y,	C.z, 0 },
					{ N.x, N.y,	N.z, 0 },
					{ 0, 0, 0, 1 } };

				MMatrix orientationMatrix(orientation);
				MVector translation(translate[0], translate[1], -translate[2]);
				translation *= orientationMatrix;
				center += translation;

				orientationMatrix = tMatrix.asMatrix()*orientationMatrix;

				appendStitch(
					center,
					orientationMatrix,
					(useLength) ? length : curStitchLen,
					thickness,
					skew,
					stitchPoints,
					stitchCounts,
					stitchIndices,
					stitchUs,
					stitchVs,
					stitchUVCounts,
					stitchUVIndices,
					customMesh);

				remLength = param;
			}
		}

		// Create output mesh
		MFnMeshData fnMeshData;
		m_stitchMesh = fnMeshData.create();
		fnMesh.create(
			stitchPoints.length(),
			stitchCounts.length(),
			stitchPoints,
			stitchCounts,
			stitchIndices,
			stitchUs,
			stitchVs,
			m_stitchMesh,
			&status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		status = fnMesh.assignUVs(stitchUVCounts, stitchUVIndices);
		CHECK_MSTATUS_AND_RETURN_IT(status);
	}

	if (m_dirtySmoothStitchMesh) {
		m_dirtySmoothStitchMesh = false;

		MObject stitchMesh(m_stitchMesh);
		int smoothLevel = datablock.inputValue(aSmoothLevel).asInt();
		
		if (datablock.inputValue(aUseCustomGeometry).asBool() && !datablock.inputValue(aCustomGeometry).asMesh().isNull())
			smoothLevel = 0;

		if (0 < smoothLevel) {
			MFnMesh fnSmoothMesh(stitchMesh, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			MMeshSmoothOptions smoothOptions;

			smoothOptions.setDivisions(smoothLevel);
			smoothOptions.setSmoothUVs(false);

			fnSmoothMesh.generateSmoothMesh(stitchMesh, &smoothOptions, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);
		}

		datablock.outputValue(aOutMesh).set(stitchMesh);
	}

	datablock.setClean(aOutMesh);

	return MS::kSuccess;
}

void StitchEasyNode::appendStitch(MPoint& position, MMatrix& orientation, double length, double thickness, double skew, MPointArray& points, MIntArray& counts, MIntArray& indices, MFloatArray &Us, MFloatArray &Vs, MIntArray &UVCounts, MIntArray &UVIndices, MObject &customGeometry) {
	double radius = thickness / 2;
	double outside = (length+ thickness) / 2;
	double inside = outside - thickness;

	double polyPoints[16][4] = { { -outside,		-radius + skew,		-thickness,		0 },
	{ outside,		-radius - skew,		-thickness,		0 },
	{ -outside,		radius + skew,		-thickness,		0 },
	{ outside,		radius - skew,		-thickness,		0 },
	{ -inside,		radius + skew / 2,	0,				0 },
	{ inside,		radius - skew / 2,	0,				0 },
	{ -inside,		-radius + skew / 2,	0,				0 },
	{ inside,		-radius - skew / 2,	0,				0 },
	{ inside,		-radius,			thickness,		0 },
	{ outside,		-radius,			thickness,		0 },
	{ inside,		radius,				thickness,		0 },
	{ outside,		radius,				thickness,		0 },
	{ -inside,		-radius,			thickness,		0 },
	{ -outside,		-radius,			thickness,		0 },
	{ -outside,		radius,				thickness,		0 },
	{ -inside,		radius,				thickness,		0 } };

	int	polyCounts[12] = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 };
	int	polyIndices[48] = { 15, 12, 6, 4, 14, 15, 4, 2, 13, 14, 2, 0, 12, 13, 0, 6, 11, 9, 1, 3, 10, 11, 3, 5, 8, 10, 5, 7, 9, 8, 7, 1, 0, 1, 7, 6, 6, 7, 5, 4, 4, 5, 3, 2, 2, 3, 1, 0 };

	float polyU[20] = { 0, 0.3f, 0, 0.1f, 0.1f, 0.2f, 0.2f, 0.1f, 0.2f, 0.2f, 0.1f, 0, 0, 0.3f, 0.4f, 0.4f, 0.3f, 0.4f, 0.4f, 0.3f};
	float polyV[20] = { 1, 1, 0.8f, 1, 0.8f, 1, 0.8f, 0, 0, 0.2f, 0.2f, 0, 0.2f, 0.2f, 1, 0.8f, 0, 0, 0.2f, 0.8f};

	int polyUVCounts[12] = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
	int polyUVIds[48] = { 14, 1, 19, 15, 3, 0, 2, 4, 5, 3, 4, 6, 1, 5, 6, 19, 7, 8, 9, 10, 11, 7, 10, 12, 16, 17, 18, 13, 8, 16, 13, 9, 6, 9, 13, 19, 19, 13, 18, 15, 2, 12, 10, 4, 4, 10, 9, 6};

	MPointArray
		geoPoints(polyPoints, 16);
	MIntArray
		geoCounts(polyCounts, 12),
		geoIndices(polyIndices, 48),
		geoUVCounts(polyUVCounts, 12),
		geoUVIds(polyUVIds, 48);
	MFloatArray
		geoUs(polyU,20),
		geoVs(polyV,20);
	unsigned firstIndex = points.length();
	unsigned firstUVIndex = Us.length();

	if (customGeometry.apiType() == MFn::kMeshData){
		MFnMesh fnCustom(customGeometry);
		fnCustom.getPoints(geoPoints);
		fnCustom.getVertices(geoCounts, geoIndices);
		fnCustom.getUVs(geoUs, geoVs);
		fnCustom.getAssignedUVs(geoUVCounts, geoUVIds);
	}

	// Points
	for (unsigned i = 0; i < geoPoints.length(); i++) {
		geoPoints[i] *= orientation;
		geoPoints[i] += position;
		points.append(geoPoints[i]);
	}

	for (unsigned i = 0; i < geoCounts.length(); i++)
		counts.append(geoCounts[i]);

	for (unsigned i = 0; i < geoIndices.length(); i++)
		indices.append(firstIndex + geoIndices[i]);

	// UVs
	for (unsigned i = 0; i < geoUs.length(); i++) {
		Us.append(geoUs[i]);
		Vs.append(geoVs[i]);
	}

	for (unsigned i = 0; i < geoUVCounts.length(); i++)
		UVCounts.append(geoUVCounts[i]);

	for (unsigned i = 0; i < geoUVIds.length(); i++)
		UVIndices.append(firstUVIndex + geoUVIds[i]);
}