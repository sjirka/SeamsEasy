#pragma once

#include "SSeamMesh.h"

#include <maya\MPxNode.h>
#include <maya\MItMeshPolygon.h>
#include <maya\MItMeshEdge.h>
#include <maya\MItMeshVertex.h>
#include <maya\MDagPath.h>
#include <maya\MNodeMessage.h>
#include <maya\MObjectArray.h>
#include <maya\MCallbackIdArray.h>

#include <set>
#include <map>
#include <deque>

class OffsetParams {
public:
	OffsetParams() {
		distance = depth = 0 ;
		stitch = false;
		index = 0;
	};

	OffsetParams(float setDistance, float setDepth, bool setStitch, unsigned int setIndex) {
		distance = setDistance;
		depth = setDepth;
		stitch = setStitch;
		index = setIndex;
	};

	OffsetParams(const OffsetParams &params) {
		distance = params.distance;
		depth = params.depth;
		stitch = params.stitch;
		index = params.index;
	}

	OffsetParams& operator=(const OffsetParams& params) {
		distance = params.distance;
		depth = params.depth;
		stitch = params.stitch;
		index = params.index;
		return *this;
	}

	bool operator< (const OffsetParams &op2) const{
		float value1, value2;
		
		switch (compare){
		case kDepth:
			value1 = depth;
			value2 = op2.depth;
			break;
		case kIndex:
			value1 = (float)index;
			value2 = (float)op2.index;
			break;
		default:
			value1 = distance;
			value2 = op2.distance;
			break;
		}

		return (value1 == value2) ? index < op2.index : value1 < value2;
	}

	float
		distance,
		depth;
	bool
		stitch;
	unsigned int
		index;

	enum Compare{
		kDistance,
		kDepth,
		kIndex
	};

	static Compare compare;

private:
};

class SeamsEasyNode : public MPxNode{
public:
	SeamsEasyNode();
	virtual ~SeamsEasyNode();
	virtual void postConstructor();

	static void *creator();
	static MStatus initialize();

	virtual MStatus setDependentsDirty(const MPlug &plug, MPlugArray &otherPlugs);
	virtual MStatus compute(const MPlug &plug, MDataBlock &dataBlock);
	float remap(const float srcMin, const float srcMax, const float trgMin, const float trgMax, float value);

	static void attrChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *data);

	static MTypeId id;

	// Seam attributes
	static MObject aOutMesh;
	static MObject aOutStitchLines;
	
	static MObject aInMesh;
	static MObject aSelectedEdges;
	
	static MObject aExtrudeAllBoundaries;
	static MObject aExtrudeThickness;
	static MObject aExtrudeDivisions;
	
	static MObject aGap;
	
	static MObject aProfileMode;

	static MObject aOffset;
	static MObject aOffsetDistance;
	static MObject aOffsetDepth;
	static MObject aOffsetStitch;

	static MObject aDistanceMultiplier;
	static MObject aDepthMultiplier;

	static MObject aProfileWidth;
	static MObject aProfileDepth;
	static MObject aProfileSubdivs;
	static MObject aProfileCurve;

	static MObject aHardEdgeAngle;

private:
	MObject
		m_sourceMesh,
		m_component;
	SSeamMesh
		m_baseMesh,
		m_profileMesh;
	std::map <unsigned int, MIntArray> m_loopEdges;

	MCallbackIdArray callbackIds;

	bool
		dirtyMesh,
		dirtyBaseMesh,
		dirtyProfileMesh,
		dirtyComponent,
		dirtyExtrude,
		dirtyProfile;

};