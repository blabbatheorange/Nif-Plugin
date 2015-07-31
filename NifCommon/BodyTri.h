/**********************************************************************
*<
FILE: BodyTri.h

DESCRIPTION:	RaceMenu (skyrim) BodyTri import/export

CREATED BY: blabba

HISTORY:

INFO: See Implementation for minimalist comments

*>	Copyright (c) 2014, All Rights Reserved.
**********************************************************************/

#include <string>
#include <sstream>
#include <vector>
#include <niflib.h>
#include <obj\NiObject.h>
#include <obj\NiNode.h>
#include <fstream>
#include "niutils.h"

#include <Max.h>
#include <include\MorpherApi.h>
#include <include\MorpherClassID.h>
#include <include\MorpherExport.h>
#include "wm3.h"

// Returns a null string if fails. Figure out exceptions later.
extern wstring getBodyTriPath(Niflib::NiObjectRef nifNode, std::wstring filePath, std::wstring altPathLine);
extern BOOL readBodyTri(wstring filePath, vector<INode*>imports);

extern Modifier *GetBodyTriMorpher(INode *node, int index);
extern Modifier *GetOrSetBodyTriMorpher(INode *node, int index);
extern INode *matchImportNode(vector<INode*>toSearch, string name);
extern TriObject *GetTriObjectFromNode(INode *node, BOOL &deleteIt);