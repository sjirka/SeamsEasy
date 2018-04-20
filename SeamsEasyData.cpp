#include "SeamsEasyData.h"

#include <maya\MArgList.h>

const MTypeId SeamsEasyData::id(0x0012789D);
const MString SeamsEasyData::typeName("seamsEasyData");

SeamsEasyData::SeamsEasyData(){
}

SeamsEasyData::~SeamsEasyData(){
}

void *SeamsEasyData::creator()
{
	return new SeamsEasyData;
}

MTypeId SeamsEasyData::typeId() const {
	return SeamsEasyData::id;
}

MString SeamsEasyData::name() const {
	return SeamsEasyData::typeName;
}


MStatus SeamsEasyData::writeASCII(std::ostream &out)
{	
	out << edgeLoop.isReversed() << " ";
	if (out.fail())
		return MStatus::kFailure;
	
	unsigned numberOfEdges = edgeLoop.numEdges();
	out << numberOfEdges << " ";
	if (out.fail())
		return MStatus::kFailure;

	for (unsigned i = 0; i < numberOfEdges; i++)
	{
		out << edgeLoop[i] << " ";
		if (out.fail())
			return MS::kFailure;

		out << edgeLoop.isFlipped(i) << " ";
		if (out.fail())
			return MS::kFailure;
	}

	return MS::kSuccess;
}

MStatus SeamsEasyData::readASCII(const MArgList& args, unsigned int &end){
	MStatus status;

	bool isReversed = args.asBool(end++, &status);
	edgeLoop.setReversed(isReversed);
	
	unsigned numberOfEdges = (unsigned)args.asInt(end++, &status);
	unsigned int numberOfArguments = args.length() - end;

	if (numberOfEdges*2 != numberOfArguments)
		return MS::kUnknownParameter;
	
	for (unsigned i = 0; i < numberOfEdges; i++)
	{
		int index = args.asInt(end++, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		bool isFlipped = args.asBool(end++, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		edgeLoop.pushBack(index, isFlipped);
	}

	return MS::kSuccess;
}

MStatus SeamsEasyData::writeBinary(std::ostream &out){
	
	bool isReversed = edgeLoop.isReversed();
	out.write((char*)&isReversed, sizeof(isReversed));
	if (out.fail())
		return MStatus::kFailure;

	unsigned numberOfEdges = edgeLoop.numEdges();
	out.write((char*)&numberOfEdges, sizeof(numberOfEdges));
	if (out.fail())
		return MStatus::kFailure;
	
	for (unsigned i = 0; i < numberOfEdges; i++)
	{
		unsigned index = edgeLoop[i];
		out.write((char*)&index, sizeof(index));
		if (out.fail())
			return MS::kFailure;

		unsigned isFlipped = edgeLoop.isFlipped(i);
		out.write((char*)&isFlipped, sizeof(isFlipped));
		if (out.fail())
			return MS::kFailure;
	}

	return MS::kSuccess;
}

MStatus SeamsEasyData::readBinary(std::istream &in, unsigned int length)
{
	MStatus status;

	if (length == 0)
		return MS::kSuccess;

	bool isReversed;
	in.read((char*)&isReversed, sizeof(isReversed));
	if (in.fail())
		return MS::kFailure;
	edgeLoop.setReversed(isReversed);
	
	unsigned numberOfEdges;
	in.read((char*)&numberOfEdges, sizeof(numberOfEdges));
	if (in.fail())
		return MS::kFailure;

	for (unsigned i = 0; i < numberOfEdges; i++)
	{
		unsigned index;
		in.read((char*)&index, sizeof(index));
		if (in.fail())
			return MS::kFailure;

		bool isFlipped;
		in.read((char*)&isFlipped, sizeof(isFlipped));
		if (in.fail())
			return MS::kFailure;

		edgeLoop.pushBack(index, isFlipped);
	}

	return MS::kSuccess;
}

void SeamsEasyData::copy(const MPxData &other) {
	
	if (this->typeId() != other.typeId())
		return;

	const SeamsEasyData &otherData = (const SeamsEasyData&)other;
	this->edgeLoop = otherData.edgeLoop;
}