#pragma once
#include <maya\MPxSelectionContext.h>

class StitchEasyCtx : public MPxSelectionContext
{
public:
	StitchEasyCtx();
	virtual ~StitchEasyCtx();

	void setNode(MString &nodeName);

	virtual void abortAction();
	virtual void deleteAction();
	virtual void completeAction();

	virtual void toolOnSetup(MEvent &);
	virtual void doEnterRegion();
	virtual void toolOffCleanup();
private:
	MString m_nodeName;
};

