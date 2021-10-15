#pragma once

#include "SMesh.h"

#include <maya/MPxNode.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MMatrix.h>

class StitchEasyNode : public MPxNode
{
public:
	StitchEasyNode();
	virtual ~StitchEasyNode();

	static void *creator();
	static MStatus initialize();

	virtual MStatus setDependentsDirty(const MPlug &plug, MPlugArray &otherPlugs);
	virtual MStatus compute(const MPlug &plug, MDataBlock &datablock);
	void appendStitch(MPoint& position, MMatrix& orientation, double length, double thickness, double skew, MPointArray& points, MIntArray& counts, MIntArray& indices, MFloatArray &Us, MFloatArray &Vs, MIntArray &UVCounts, MIntArray &UVIndices, MObject &customGeometry = MObject::kNullObj);

	static MTypeId id;

	static MObject aOutMesh;
	static MObject aSmoothLevel;

	static MObject aInMesh;
	static MObject aInSmoothMesh;
	static MObject aUseSmoothMesh;
	static MObject aStitchLines;

	static MObject aCustomGeometry;
	static MObject aUseCustomGeometry;

	static MObject aDistance;
	static MObject aLength;
	static MObject aUseLength;
	static MObject aThickness;
	static MObject aSkew;
	static MObject aFlip;
	static MObject aMultiplier;

	static MObject aRotate;
	static MObject aRotateX;
	static MObject aRotateY;
	static MObject aRotateZ;

	static MObject aScale;
	static MObject aScaleX;
	static MObject aScaleY;
	static MObject aScaleZ;

	static MObject aTranslate;
	static MObject aTranslateX;
	static MObject aTranslateY;
	static MObject aTranslateZ;

private:
	MObject
		m_sourceMesh,
		m_component,
		m_stitchMesh;

	SMesh
		m_baseMesh,
		m_smoothMesh;

	bool
		m_dirtySourceMesh,
		m_dirtyBaseMesh,
		m_dirtyComponent,
		m_dirtySmoothMesh,
		m_dirtyStitchMesh,
		m_dirtySmoothStitchMesh;
};

