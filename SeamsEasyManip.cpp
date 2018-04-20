#include "SeamsEasyManip.h"
#include "SeamsEasyData.h"

#include <maya\MFnManip3D.h>
#include <maya\MGlobal.h>
#include "SCamera.h"
#include <maya\MPointArray.h>
#include <maya\MQuaternion.h>
#include <maya\MPlugArray.h>
#include <maya\MDagPath.h>
#include <maya\MItDependencyGraph.h>
#include <maya\MFnPluginData.h>

MTypeId SeamsEasyManip::id(0x0012789B);

SeamsEasyManip::SeamsEasyManip(){
}

SeamsEasyManip::~SeamsEasyManip(){
}

void SeamsEasyManip::postConstructor() {
	glFirstHandle(firstHandle);
}

void* SeamsEasyManip::creator() {
	return new SeamsEasyManip();
}

MStatus SeamsEasyManip::initialize() {
	return MStatus::kSuccess;
}

void SeamsEasyManip::draw(M3dView& view, const MDagPath& nodePath, M3dView::DisplayStyle style, M3dView::DisplayStatus stat) {
	MStatus status;

	MDagPath camPath;
	view.getCamera(camPath);

	MFnDependencyNode fnNode(pluginNode->thisMObject());
	MFnMesh fnMesh(pluginNode->sourceMesh, &status);
	CHECK_MSTATUS(status);
	MPointArray positions;
	fnMesh.getPoints(positions);

	MItDependencyGraph itGraph(fnNode.findPlug(SeamsEasyNode::aOutMesh), MFn::kMesh);
	MMatrix matrix;
	if (itGraph.currentItem().apiType() == MFn::kMesh) {
		MFnDagNode fnDag(itGraph.currentItem());
		MDagPath path;
		fnDag.getPath(path);
		matrix = path.inclusiveMatrix();
	}

	view.beginGL();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	unsigned int h = firstHandle;
	MPlug edgeLoopArrayPlug = fnNode.findPlug(SeamsEasyNode::aEdgeLoops);
	for (unsigned i = 0; i<edgeLoopArrayPlug.numElements(); i++) {
		MPlug edgeLoopElementPlug = edgeLoopArrayPlug.elementByLogicalIndex(i);
		MObject data;
		edgeLoopElementPlug.getValue(data);
		MFnPluginData fnPluginData(data);

		SeamsEasyData* edgeLoopData = (SeamsEasyData*)fnPluginData.constData(&status);
		CHECK_MSTATUS(status);

		edgeLoopData->edgeLoop.setMeshPtr(&pluginNode->sourceMesh);

		colorAndName(view, h++, true, zColor());
		MIntArray loopVertices;
		MPointArray loopPoints;
		edgeLoopData->edgeLoop.getVertices(loopVertices);

		glLineWidth(2);
		glColor3f(1, 0, 1);
		glBegin(MGL_LINE_STRIP);
		for (unsigned int i = 0; i < loopVertices.length(); i++) {
			loopPoints.append(positions[loopVertices[i]]*matrix);
			glVertex3d(loopPoints[i].x, loopPoints[i].y, loopPoints[i].z);
		}
		glEnd();

		unsigned int base = (unsigned)floor(loopPoints.length() / 2) - 1;
		unsigned int firstIdx = (loopPoints.length() % 2 == 1 && edgeLoopData->edgeLoop.isReversed()) ? base + 1 : base;

		MVector edge = loopPoints[firstIdx + 1] - loopPoints[firstIdx];
		MVector normal;
		fnMesh.getVertexNormal(loopVertices[firstIdx], normal);
		normal *= matrix;
		normal.normalize();
		MVector cross = normal.normal() ^ edge.normal();
		double size = 0.5;
		MPoint center = (loopPoints[firstIdx] + loopPoints[firstIdx + 1]) / 2;

		double scaleFactor = SCamera::scaleFactor(camPath, center);
		double displaySize = scaleFactor*size;

		MPoint topA = center + ((cross * displaySize) + (edge.normal() * displaySize / 2));
		MPoint bottomA = center + ((cross * displaySize) - (edge.normal() * displaySize / 2));
		MPoint topB = center - ((cross * displaySize) + (edge.normal() * displaySize / 2));
		MPoint bottomB = center - ((cross * displaySize) - (edge.normal() * displaySize / 2));

		MPointArray triangleA, triangleB;
		triangleA.append(center);
		triangleA.append(topA);
		triangleA.append(bottomA);

		triangleB.append(center);
		triangleB.append(topB);
		triangleB.append(bottomB);

		glBegin(MGL_TRIANGLES);
		glColor3f(1, 0, 1);
		glVertex3d(center.x, center.y, center.z);
		glVertex3d(topA.x, topA.y, topA.z);
		glVertex3d(bottomA.x, bottomA.y, bottomA.z);
		glColor3f(0, 0, 1);
		glVertex3d(center.x, center.y, center.z);
		glVertex3d(topB.x, topB.y, topB.z);
		glVertex3d(bottomB.x, bottomB.y, bottomB.z);
		glEnd();
	}

	glPopAttrib();
	view.endGL();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Viewport 2.0 ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void SeamsEasyManip::preDrawUI(const M3dView &view){
}

void SeamsEasyManip::drawUI(MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext) const {
	if (pluginNode == NULL)
		return;
	MStatus status;
	
	MDagPath camPath = frameContext.getCurrentCameraPath();
	
	MFnDependencyNode fnNode(pluginNode->thisMObject());
	MFnMesh fnMesh(pluginNode->sourceMesh, &status);
	MPointArray positions;
	fnMesh.getPoints(positions);

	MItDependencyGraph itGraph(fnNode.findPlug(SeamsEasyNode::aOutMesh), MFn::kMesh);
	MMatrix matrix;
	if (itGraph.currentItem().apiType() == MFn::kMesh) {
		MFnDagNode fnDag(itGraph.currentItem());
		MDagPath path;
		fnDag.getPath(path);
		matrix = path.inclusiveMatrix();
	}

	unsigned int h = firstHandle;

	MPlug edgeLoopArrayPlug = fnNode.findPlug(SeamsEasyNode::aEdgeLoops);
	for (unsigned i = 0; i<edgeLoopArrayPlug.numElements(); i++) {
		MPlug edgeLoopElementPlug = edgeLoopArrayPlug.elementByLogicalIndex(i);
		MObject data;
		edgeLoopElementPlug.getValue(data);
		MFnPluginData fnPluginData(data);

		SeamsEasyData* edgeLoopData = (SeamsEasyData*)fnPluginData.constData(&status);
		CHECK_MSTATUS(status);

		edgeLoopData->edgeLoop.setMeshPtr(&pluginNode->sourceMesh);

		MIntArray loopVertices;
		MPointArray loopPoints;
		edgeLoopData->edgeLoop.getVertices(loopVertices);
		for (unsigned int i = 0; i < loopVertices.length(); i++)
			loopPoints.append(positions[loopVertices[i]]*matrix);

		drawManager.beginDrawable();
		drawManager.setLineWidth(2);
		drawManager.setColor(MColor(1, 0, 1));
		drawManager.lineStrip(loopPoints, false);
		drawManager.endDrawable();

		unsigned int base = floor(loopPoints.length() / 2) - 1;
		unsigned int firstIdx = (loopPoints.length() % 2 == 1 && edgeLoopData->edgeLoop.isReversed()) ? base + 1 : base;

		MVector edge = loopPoints[firstIdx + 1] - loopPoints[firstIdx];
		MVector normal;
		fnMesh.getVertexNormal(loopVertices[firstIdx], normal);
		normal *= matrix;
		normal.normalize();
		MVector cross = normal.normal()^edge.normal();
		double size = 0.5;
		MPoint center = (loopPoints[firstIdx] + loopPoints[firstIdx + 1]) / 2;

		double scaleFactor = SCamera::scaleFactor(camPath, center);
		double displaySize = scaleFactor*size;

		MPoint topA = center + ((cross * displaySize) + (edge.normal() * displaySize / 2));
		MPoint bottomA = center + ((cross * displaySize) - (edge.normal() * displaySize / 2));
		MPoint topB = center - ((cross * displaySize) + (edge.normal() * displaySize / 2));
		MPoint bottomB = center - ((cross * displaySize) - (edge.normal() * displaySize / 2));

		MPointArray triangleA, triangleB;
		triangleA.append(center);
		triangleA.append(topA);
		triangleA.append(bottomA);

		triangleB.append(center);
		triangleB.append(topB);
		triangleB.append(bottomB);

		drawManager.beginDrawable(h++, true);
		drawManager.setColor(MColor(1, 0, 1));
		drawManager.mesh(MHWRender::MUIDrawManager::kTriangles, triangleA);
		drawManager.setColor(MColor(0, 0, 1));
		drawManager.mesh(MHWRender::MUIDrawManager::kTriangles, triangleB);
		drawManager.endDrawable();
	}
		
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Event handling /////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

MStatus SeamsEasyManip::doRelease(M3dView& view){
	if (pluginNode == NULL)
		return MS::kSuccess;

	MStatus status;

	MGLuint activeHandle;
	glActiveName(activeHandle);
	unsigned int loopId = activeHandle - firstHandle;

	MFnDependencyNode fnNode(pluginNode->thisMObject());
	MPlug edgeLoopArrayPlug = fnNode.findPlug(SeamsEasyNode::aEdgeLoops);

	if (loopId >= edgeLoopArrayPlug.numElements())
		return MS::kSuccess;

	MPlug edgeLoopElementPlug = edgeLoopArrayPlug.elementByLogicalIndex(loopId);
	MObject data;
	edgeLoopElementPlug.getValue(data);
	MFnPluginData fnPluginData(data);

	SeamsEasyData* edgeLoopData = (SeamsEasyData*)fnPluginData.constData(&status);
	CHECK_MSTATUS(status);
	edgeLoopData->edgeLoop.reverse();

	edgeLoopElementPlug.setValue(data);

	return MS::kSuccess;
}

void SeamsEasyManip::setNodePtr(SeamsEasyNode* ptr) {
	pluginNode = ptr;
}