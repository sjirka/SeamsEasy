#pragma once

#include "StitchEasyNode.h"

#include <maya\MPxCommand.h>
#include <maya\MDGModifier.h>
#include <maya\MDagModifier.h>
#include <maya\MDagPath.h>
#include <maya\MDistance.h>
#include <maya\MAngle.h>
#include <maya\MArgDatabase.h>

#include <map>

#define DISTANCE_FLAG_LONG		"-distance"
#define	DISTANCE_FLAG			"-d"

#define LENGTH_FLAG_LONG		"-length"
#define LENGTH_FLAG				"-l"

#define USELENGTH_FLAG_LONG		"-useLength"
#define USELENGTH_FLAG			"-ul"

#define THICKNESS_FLAG_LONG		"-thickness"
#define THICKNESS_FLAG			"-th"

#define SKEW_FLAG_LONG			"-skew"
#define SKEW_FLAG				"-sk"

#define FLIP_FLAG_LONG			"-flipNormal"
#define FLIP_FLAG				"-fn"

#define SMOOTHLEVEL_FLAG_LONG	"-smoothLevel"
#define SMOOTHLEVEL_FLAG		"-sml"

#define ROTATE_FLAG_LONG		"-rotate"
#define ROTATE_FLAG				"-r"

#define SCALE_FLAG_LONG			"-scale"
#define SCALE_FLAG				"-s"

#define TRANSLATE_FLAG_LONG		"-translate"
#define TRANSLATE_FLAG			"-t"

#define SEAMNODE_FLAG_LONG		"-seamNode"
#define SEAMNODE_FLAG			"-sn"

#define MULTIPLIER_FLAG_LONG	"-multiplier"
#define MULTIPLIER_FLAG			"-m"

class StitchEasyCmd : public MPxCommand
{
public:
	StitchEasyCmd();
	virtual ~StitchEasyCmd();

	static void* creator();
	static MSyntax newSyntax();

	virtual MStatus doIt(const MArgList& args);
	virtual MStatus redoIt();
	virtual MStatus undoIt();

	virtual bool isUndoable() const { return true; }

	MStatus queryAttrValue(MPlug& attrPlug);
	MStatus setFlagAttr(MArgDatabase& argData, char *flag, MPlug& attrPlug);

private:
	MDGModifier m_dgMod;
	MDagModifier m_dagMod;

	bool m_isCreation;

	MObject
		m_node,
		m_shape,
		m_transform;

	std::map <char*, MObject> m_attrFlags{
		{ DISTANCE_FLAG, StitchEasyNode::aDistance },
		{ LENGTH_FLAG, StitchEasyNode::aLength },
		{ USELENGTH_FLAG, StitchEasyNode::aUseLength },
		{ THICKNESS_FLAG, StitchEasyNode::aThickness },
		{ SKEW_FLAG, StitchEasyNode::aSkew },
		{ MULTIPLIER_FLAG, StitchEasyNode::aMultiplier },
		{ FLIP_FLAG, StitchEasyNode::aFlip },
		{ SMOOTHLEVEL_FLAG, StitchEasyNode::aSmoothLevel },
		{ ROTATE_FLAG,StitchEasyNode::aRotate },
		{ SCALE_FLAG, StitchEasyNode::aScale },
		{ TRANSLATE_FLAG, StitchEasyNode::aTranslate }
	};
};

