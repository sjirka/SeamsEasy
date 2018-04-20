#pragma once
#include <maya\MPxData.h>
#include <maya\MTypeId.h>
#include <maya\MString.h>
#include "SEdgeLoop.h"


class SeamsEasyData : public MPxData{
public:
	SeamsEasyData();
	virtual ~SeamsEasyData();
	static void * creator();

	virtual MStatus         readASCII(const MArgList& argList, unsigned int &idx);
	virtual MStatus         readBinary(istream& in, unsigned int length);
	virtual MStatus         writeASCII(ostream& out);
	virtual MStatus         writeBinary(ostream& out);

	virtual void            copy(const MPxData&);

	virtual MTypeId         typeId() const;
	virtual MString         name() const;

	static const MString typeName;
	static const MTypeId id;

	SEdgeLoop edgeLoop;
};