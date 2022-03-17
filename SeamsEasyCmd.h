#pragma once

#include "SeamsEasyNode.h"
#include "SNode.h"

#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>
#include <maya/MDagModifier.h>
#include <maya/MDagPath.h>
#include <maya/MDistance.h>
#include <maya/MArgDatabase.h>
#include <maya/MPlugArray.h>

#include <map>

#define THICKNESS_FLAG_LONG		"-thickness"
#define	THICKNESS_FLAG			"-th"

#define DIVISIONS_FLAG_LONG		"-divisions"
#define DIVISIONS_FLAG			"-div"

#define EXTRUDEALL_FLAG_LONG	"-extrudeAllBoundaries"
#define EXTRUDEALL_FLAG			"-eab"

#define GAP_FLAG_LONG			"-gapWidth"
#define GAP_FLAG				"-gw"

#define MODE_FLAG_LONG			"-profileMode"
#define MODE_FLAG				"-pm"

#define WIDTH_FLAG_LONG			"-profileWidth"
#define WIDTH_FLAG				"-pw"

#define DEPTH_FLAG_LONG			"-profileDepth"
#define DEPTH_FLAG				"-pd"

#define SUBDIVS_FLAG_LONG		"-profileSubdivisions"
#define SUBDIVS_FLAG			"-ps"

#define ADDLOOP_FLAG_LONG		"-addLoop"
#define ADDLOOP_FLAG			"-add"

#define SORT_FLAG_LONG			"-sort"
#define SORT_FLAG				"-srt"

#define DISTANCEMULTI_FLAG_LONG "-distanceMultiplier"
#define DISTANCEMULTI_FLAG		"-dtm"

#define DEPTHMULTI_FLAG_LONG	"-depthMultiplier"
#define DEPTHMULTI_FLAG			"-dpm"

#define HARDEDGEANGLE_FLAG_LONG	"-hardEdgeAngle"
#define HARDEDGEANGLE_FLAG		"-hea"

class SeamsEasyCmd : public MPxCommand
{
public:
	SeamsEasyCmd();
	virtual ~SeamsEasyCmd();

	static void* creator();
	static MSyntax newSyntax();

	virtual MStatus doIt(const MArgList& args);
	virtual MStatus redoIt();
	virtual MStatus undoIt();

	virtual bool isUndoable() const { return true; }

	MStatus queryAttrValue(MPlug& attrPlug);
	MStatus setFlagAttr(MArgDatabase& argData, char *flag, MPlug& attrPlug);
	MStatus AssignToShadingGroup(const MObject& shadingGroup, const MDagPath& dagPath, const MObject& component);
	MStatus RemoveFromShadingGroup(const MObject& shadingGroup, const MDagPath& dagPath, const MObject& component);

private:
	MDGModifier m_dgMod;
	MDagModifier m_dagMod;

	bool m_isCreation;
	
	MObject
		m_shape,
		m_node;

	MObjectArray
		m_groups,
		m_comps;

	std::map <char*, MObject> m_attrFlags{
		{ THICKNESS_FLAG, SeamsEasyNode::aExtrudeThickness },
		{ DIVISIONS_FLAG, SeamsEasyNode::aExtrudeDivisions },
		{ EXTRUDEALL_FLAG, SeamsEasyNode::aExtrudeAllBoundaries },
		{ GAP_FLAG, SeamsEasyNode::aGap },
		{ MODE_FLAG, SeamsEasyNode::aProfileMode },
		{ WIDTH_FLAG,SeamsEasyNode::aProfileWidth },
		{ DEPTH_FLAG, SeamsEasyNode::aProfileDepth },
		{ SUBDIVS_FLAG, SeamsEasyNode::aProfileSubdivs },
		{ DISTANCEMULTI_FLAG, SeamsEasyNode::aDistanceMultiplier },
		{ DEPTHMULTI_FLAG, SeamsEasyNode::aDepthMultiplier },
		{ HARDEDGEANGLE_FLAG, SeamsEasyNode::aHardEdgeAngle },
	};
};

