#include "SeamsEasyManip.h"

#include <maya\MFnManip3D.h>
#include <maya\MGlobal.h>
#include "SCamera.h"
#include <maya\MPointArray.h>
#include <maya\MQuaternion.h>
#include <maya\MPlugArray.h>
#include <maya\MDagPath.h>
#include <maya\MItDependencyGraph.h>

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
	
	MDagPath camPath;
	view.getCamera(camPath);

	SSeamMesh* baseMeshPtr = pluginNode->getMeshPtr();
	if (baseMeshPtr->getObject().isNull())
		return;
	std::vector <SEdgeLoop> *edgeLoops = baseMeshPtr->activeLoopsPtr();


	MFnDependencyNode fnNode(pluginNode->thisMObject());
	MPlug outPlug = fnNode.findPlug("outMesh");
	MItDependencyGraph itGraph(outPlug, MFn::kMesh);
	if (itGraph.thisNode().apiType() != MFn::kMesh)
		return;
	MFnDagNode fnDagNode(itGraph.thisNode());
	MDagPath path;
	fnDagNode.getPath(path);
	MMatrix matrix = path.inclusiveMatrix();

	MFnMesh fnMesh(baseMeshPtr->getObject());
	MPointArray positions;
	fnMesh.getPoints(positions, MSpace::kObject);

	for (unsigned int i = 0; i < positions.length(); i++)
		positions[i] *= matrix;

	view.beginGL();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	unsigned int h = firstHandle;
	for (auto &loop : *edgeLoops) {
		colorAndName(view, h++, true, zColor());
		MIntArray loopVertices;
		MPointArray loopPoints;
		loop.getVertices(loopVertices);

		glLineWidth(2);
		glColor3f(1, 0, 1);
		glBegin(MGL_LINE_STRIP);
		for (unsigned int i = 0; i < loopVertices.length(); i++) {
			loopPoints.append(positions[loopVertices[i]]);
			glVertex3d(positions[loopVertices[i]].x, positions[loopVertices[i]].y, positions[loopVertices[i]].z);
		}
		glEnd();

		unsigned int base = floor(loopPoints.length() / 2)-1;
		unsigned int firstIdx = (loopPoints.length() % 2 == 1 && loop.isReversed() ) ? base + 1 : base;

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

	MDagPath camPath = frameContext.getCurrentCameraPath();
	
	SSeamMesh* baseMeshPtr = pluginNode->getMeshPtr();
	if (baseMeshPtr->getObject().isNull())
		return;
	std::vector <SEdgeLoop> *edgeLoops = baseMeshPtr->activeLoopsPtr();
	

	MFnDependencyNode fnNode(pluginNode->thisMObject());
	MPlug outPlug = fnNode.findPlug("outMesh");
	MItDependencyGraph itGraph(outPlug, MFn::kMesh);
	if (itGraph.thisNode().apiType()!=MFn::kMesh)
		return;
	MFnDagNode fnDagNode(itGraph.thisNode());
	MDagPath path;
	fnDagNode.getPath(path);
	MMatrix matrix = path.inclusiveMatrix();
	
	MFnMesh fnMesh(baseMeshPtr->getObject());
	MPointArray positions;
	fnMesh.getPoints(positions, MSpace::kObject);

	for (unsigned int i = 0; i < positions.length(); i++)
		positions[i] *= matrix;

	unsigned int h = firstHandle;
	for (auto &loop : *edgeLoops) {
		MIntArray loopVertices;
		MPointArray loopPoints;
		loop.getVertices(loopVertices);
		for (unsigned int i = 0; i < loopVertices.length(); i++)
			loopPoints.append(positions[loopVertices[i]]);

		drawManager.beginDrawable();
		drawManager.setLineWidth(2);
		drawManager.setColor(MColor(1, 0, 1));
		drawManager.lineStrip(loopPoints, false);
		drawManager.endDrawable();

		unsigned int base = floor(loopPoints.length() / 2) - 1;
		unsigned int firstIdx = (loopPoints.length() % 2 == 1 && loop.isReversed()) ? base + 1 : base;

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

	SSeamMesh* baseMeshPtr = pluginNode->getMeshPtr();
	if (baseMeshPtr->getObject().isNull())
		return MS::kSuccess;
	std::vector <SEdgeLoop> *edgeLoops = baseMeshPtr->activeLoopsPtr();

	MGLuint activeHandle;
	glActiveName(activeHandle);
	unsigned int loopId = activeHandle - firstHandle;

	edgeLoops->at(loopId).reverse();

	MFnDependencyNode fnNode(pluginNode->thisMObject());
	MObject mesh = fnNode.findPlug(SeamsEasyNode::aOutMesh).asMObject();

	return MS::kSuccess;
}

void SeamsEasyManip::setNodePtr(SeamsEasyNode* ptr) {
	pluginNode = ptr;
}