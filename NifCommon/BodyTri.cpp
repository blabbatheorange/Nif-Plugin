/**********************************************************************
*<
FILE: BodyTri.cpp

DESCRIPTION:	RaceMenu (skyrim) BodyTri import/export

CREATED BY: blabba

HISTORY:

INFO: See Implementation for minimalist comments

*>	Copyright (c) 2014, All Rights Reserved.
**********************************************************************/

#include "BodyTri.h"

using namespace Niflib;

// Add in early exit from loop once bodytri is found.
wstring getBodyTriPath(NiObjectRef nifNode, wstring filePath, wstring altPathLine) {
	NiNodeRef rootNode = nifNode;
	wstring bodyTriPath;
	TCHAR buffer[MAX_PATH];
	auto children = rootNode->GetChildren();
	for each (auto child in children) {
		if (auto shape = DynamicCast<NiTriShape>(child)) {
			auto dataList = shape->GetExtraData();
			for each (auto extraData in dataList) {
				string compareName = extraData->GetName();
				transform(compareName.begin(), compareName.end(), compareName.begin(), toupper);
				if (compareName == "BODYTRI") {
					NiStringExtraDataRef bodyTri = extraData;
					bodyTriPath = wide(bodyTri->GetData());
				}
			}
		}
	}

	// Check for BodyTri file in directory where nif was opened
	PathCombine(buffer, filePath.c_str(), PathFindFileName(bodyTriPath.c_str()));
	if (-1 != _taccess(buffer, 0))
		return wstring(buffer);

	// Check for BodyTri relative to where .nif's meshes folder is
	string meshRelativePath = narrow(filePath.c_str());
	transform(meshRelativePath.begin(), meshRelativePath.end(), meshRelativePath.begin(), tolower);
	int cut = meshRelativePath.find("meshes\\");
	if (cut != string::npos) {
		cut += 6;
		meshRelativePath.erase(cut, meshRelativePath.length());
	}
	wstring testMeshPath = wide(meshRelativePath);
	PathCombine(buffer, testMeshPath.c_str(), bodyTriPath.c_str());
	if (-1 != _taccess(buffer, 0))
		return wstring(buffer);

	// Check for BodyTri in Alt Path Lookups
	vector<wstring> altPaths = ParseAltPaths(altPathLine);
	for each (wstring selectedAlt in altPaths) {
		selectedAlt += _T("meshes\\");
		PathCombine(buffer, selectedAlt.c_str(), bodyTriPath.c_str());
		if (-1 != _taccess(buffer, 0))
			return wstring(buffer);
	}

	// Returns Empty wstring if no match is found.
	return _T("");
}

BOOL readBodyTri(wstring filepath, vector<INode*>imports) {
	ifstream bodyTri(filepath.c_str(), ios_base::binary);

	if (bodyTri.is_open()) {
		bool packed = false;
		int packedBytes = 4;

		char hdr[4];
		bodyTri.read(hdr, 4);
		if (memcmp(hdr, "PIRT", 4) == 0) {
			packed = true;
			packedBytes = 2;
		}
		else if (memcmp(hdr, "\0IRT", 4) != 0)
			return false;

		Niflib::byte shapeCount = 0;
		bodyTri.read((char*)&shapeCount, packedBytes);

		for (int i = 0; i < shapeCount; i++) {
			unsigned char shapeLength = 0;
			string shapeName;
			vector<MaxMorphModifier> bodyMorphers;
			bodyTri.read((char*)&shapeLength, 1);
			shapeName.resize(shapeLength, ' ');
			bodyTri.read((char*)&shapeName.front(), shapeLength);
			// This is just for getting INode when they are already in the scene. Not for in-progress Imports
			// INode *currentObjectNode = scene->GetINodeByName(wide(shapeName).c_str());
			INode *currentObjectNode = matchImportNode(imports, shapeName);
			BOOL delData = FALSE;
			TriObject *dummyTri = GetTriObjectFromNode(currentObjectNode, delData);
			Mesh dummyMesh = dummyTri->GetMesh();
			TriObject *copyTri = CreateNewTriObject();
			copyTri->mesh = dummyMesh;
			INode *dummyINode = GetCOREInterface()->CreateObjectNode(copyTri);
			if (delData) dummyTri->DeleteMe();
			dummyINode->SetName(_T("test"));
			unsigned int morphCount = 0;
			bodyTri.read((char*)&morphCount, packedBytes);

			for (int ModCount = 1; ModCount <= (int) ceil(morphCount/100.0); ModCount++)
				bodyMorphers.push_back(MaxMorphModifier(GetOrSetBodyTriMorpher(currentObjectNode, ModCount)));

			for (int j = 0; j < morphCount; j++) {
				
				Niflib::byte morphNameLength = 0;
				string morphName;
				bodyTri.read((char*)&morphNameLength, 1);
				morphName.resize(morphNameLength, ' ');
				bodyTri.read((char*)&morphName.front(), morphNameLength); 
				int whichMod = floor((j / 100.0)-.01);
				whichMod = (whichMod < 0) ? 0 : whichMod;
				MaxMorphModifier toAlter = bodyMorphers.at(whichMod);

				MaxMorphChannel currentChannel = toAlter.GetMorphChannel(j);
				currentChannel.SetMorphTarget(dummyINode, 0);
				currentChannel.SetName(wide(morphName).c_str());

				if (packed) {
					float mult = 0.0f;
					unsigned short morphVertCount = 0;
					bodyTri.read((char*)&mult, 4);
					bodyTri.read((char*)&morphVertCount, packedBytes);

					for (int k = 0; k < morphVertCount; k++) {
						unsigned short id = 0;
						short x = 0;
						short y = 0;
						short z = 0;
						bodyTri.read((char*)&id, 2);
						bodyTri.read((char*)&x, 2);
						bodyTri.read((char*)&y, 2);
						bodyTri.read((char*)&z, 2);
						Point3 offset = Point3((x*mult)*.01f, (y*mult)*.01f, (z*mult)*.01f);
						Point3 dest = Point3(copyTri->GetMesh().verts[id].x+(x*mult), copyTri->GetMesh().verts[id].y+(y*mult), copyTri->GetMesh().verts[id].z+(z*mult));
						currentChannel.SetMorphPointDelta(id, offset);
						currentChannel.SetMorphPoint(id, dest);
					}
				}
				else {
					unsigned int morphVertCount = 0;
					bodyTri.read((char*)&morphVertCount, packedBytes);

					for (int k = 0; k < morphVertCount; packedBytes) {
						unsigned int id = 0;
						float x = 0.0f;
						float y = 0.0f;
						float z = 0.0f;
						bodyTri.read((char*)&id, 4);
						bodyTri.read((char*)&x, 4);
						bodyTri.read((char*)&y, 4);
						bodyTri.read((char*)&z, 4);
						Point3 offset = Point3(x*.01f, y*.01f, z*.01f);
						currentChannel.SetMorphPointDelta(id, offset);
					}
				}
			}
			// Unlink Morpher Modifier from dummy object
			dummyINode->DeleteThis();
		}
	}
}

// Use DynamicCast instead of type casting for IDerivedObjects as a null check.
Modifier *GetBodyTriMorpher(INode *node, int index) {
	Object *tObj = node->GetObjectRef();
	if (!tObj) return NULL;
	while (tObj->SuperClassID() == GEN_DERIVOB_CLASS_ID) {
		IDerivedObject* tDerObj = (IDerivedObject *)(tObj);
		int Idx = 0;
		while (Idx < tDerObj->NumModifiers()) {
			// Get the modifier. 
			Modifier* mod = tDerObj->GetModifier(Idx);
			if (mod->ClassID() == MR3_CLASS_ID)
			{
				// is this a Morpher Modifier based on index?
				wstring modName = mod->GetName();
				wstringstream temp;
				temp << index;
				wstring indexAsWstring(temp.str());
				wstring compare = _T("BodyTri_") + indexAsWstring;
				if (mod->GetName() == compare.c_str())	// # Might not show up
					return mod;
			}
			Idx++;
		}
		tObj = tDerObj->GetObjRef();
	}
	return NULL;
}

Modifier *GetOrSetBodyTriMorpher(INode *node, int index) {
	Modifier *bodyTriMorpher = GetBodyTriMorpher(node, index);
	if (bodyTriMorpher)
		return bodyTriMorpher;

	// Find current number of modifiers on Object, to add morphers to bottom of stack
	// Currently Doesn't work, will figure out why later.
	Object *pObj = node->GetObjectRef();
	IDerivedObject *pDerObj = NULL;
	if (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
		pDerObj = static_cast<IDerivedObject*>(pObj);
	else {
		pDerObj = CreateDerivedObject(pObj);
	}
	bodyTriMorpher = (Modifier*)CreateInstance(OSM_CLASS_ID, MR3_CLASS_ID);
	wstringstream temp;
	temp << index;
	wstring indexAsWstring(temp.str());
	wstring modName = _T("BodyTri_") + indexAsWstring;
	bodyTriMorpher->SetName(modName.c_str());
	pDerObj->SetAFlag(A_LOCK_TARGET);
	int numberMod = pDerObj->NumModifiers();
	pDerObj->AddModifier(bodyTriMorpher, NULL, numberMod);
	pDerObj->ClearAFlag(A_LOCK_TARGET);
	node->EvalWorldState(0);	// Build Morpher Cache
	return bodyTriMorpher;
}

INode *matchImportNode(vector<INode*>toSearch, string name) {
	for each (INode *test in toSearch) {
		string testName = narrow(test->GetName()).c_str();
		if (testName == name)
			return test;
	}
}

TriObject *GetTriObjectFromNode(INode *node, BOOL &deleteIt) {
	deleteIt = FALSE;
	Object *obj = node->EvalWorldState(0).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) {
		TriObject *tri = (TriObject *)obj->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0));

		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		if (obj != tri) deleteIt = TRUE;
		return tri;
	}
	else {
		return NULL;
	}
}