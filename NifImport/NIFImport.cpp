/**********************************************************************
*<
FILE: ImporterCore.cpp

DESCRIPTION:	Core Import helper routines

CREATED BY: tazpn (Theo)

HISTORY: 

*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/
#include "stdafx.h"
#include "BodyTri.h"
#include "MaxNifImport.h"

using namespace Niflib;

// Define the standard section names used in the ini file
LPCTSTR NifImportSection = TEXT("MaxNifImport");
LPCTSTR SystemSection = TEXT("System");
LPCTSTR BipedImportSection = TEXT("BipedImport");
LPCTSTR AnimImportSection = TEXT("AnimationImport");
LPCTSTR CollisionSection = TEXT("Collision");

class IBipMaster;
IBipMaster * (_cdecl * Max8CreateNewBiped)(float,float,class Point3 const &,int,int,int,int,int,int,int,int,int,int,int,int,float,int,int,int,int,int,int,int,int) = 0;
IBipMaster * (_cdecl * Max7CreateNewBiped)(float,float,class Point3 const &,int,int,int,int,int,int,int,int,int,int,int,int,float,int,int,int,int) = 0;

NifImporter::NifImporter(const TCHAR *Name,ImpInterface *I,Interface *GI, BOOL SuppressPrompts)
   : BaseImporter()
{
   BaseInit(Name,I,GI,SuppressPrompts);
}

NifImporter::NifImporter()
{
}

INode* NifImporter::CreateImportNode(const wchar_t *name, Object *obj, INode* parent)
{
#if USE_IMPORTNODE
	ImpNode* impNode = i->CreateNode();
	impNode->Reference(obj);
	if (INode *n = impNode->GetINode()) {
		n->SetName(const_cast<TCHAR*>(name));
		n->SetObjectRef(obj);
		i->AddNodeToScene(impNode);
		this->RegisterNode(name, n);
		if (parent)
		{
			parent->AttachChild(impNode->GetINode());

			ASSERT(parent == n->GetParentNode());

		}
	}
	return impNode->GetINode();
#else
	if ( INode* n = gi->CreateObjectNode(obj) )
	{
		n->SetName(const_cast<TCHAR*>(name));
		this->RegisterNode(name, n);
		if (parent)
		{
			parent->AttachChild(n);
			ASSERT(parent == n->GetParentNode());
		}
		return n;
	}
	return NULL;
	
#endif
}

void NifImporter::ReadBlocks()
{
	//blocks = ReadNifList( name );
	Niflib::NifInfo info;
	int opts;
	root = ReadNifTree(narrow(name), &info);
	nifVersion = info.version;
	userVersion = info.userVersion;
	BuildNodes();
}


static void BuildNodes(NiNodeRef object, vector<NiNodeRef>& nodes)
{
   if (!object)
      return;
   nodes.push_back(object);
   vector<NiNodeRef> links = DynamicCast<NiNode>(object->GetChildren());
   for (vector<NiNodeRef>::iterator itr = links.begin(), end = links.end(); itr != end; ++itr)
      BuildNodes(*itr, nodes);
}

void NifImporter::BuildNodes()
{
   ::BuildNodes(root, nodes);
   std::sort(nodes.begin(), nodes.end(), NodeEquivalence());
}

void NifImporter::Initialize()
{
   // Apply post processing checks after reading blocks
   if (isValid()){
      if (goToSkeletonBindPosition && !nodes.empty() && importBones)
         GoToSkeletonBindPosition(nodes);

      // Only support biped if CreateNewBiped can be found.
      useBiped &= (Max8CreateNewBiped != NULL || Max7CreateNewBiped != NULL);

      hasSkeleton = HasSkeleton();
      isBiped = IsBiped();
      skeleton = GetSkeleton(appSettings);
      importSkeleton = (appSettings != NULL) ? appSettings->useSkeleton : false;
      importSkeleton &= hasSkeleton;
	  importSkeleton &= !isBiped;

      // Guess that the skeleton is the same one in the current directory
      if (importSkeleton && !defaultSkeletonName.empty()) {
         TCHAR buffer[MAX_PATH];
         GetFullPathName(name.c_str(), _countof(buffer), buffer, NULL);
         PathRemoveFileSpec(buffer);
         PathAddBackslash(buffer);
         PathAppend(buffer, defaultSkeletonName.c_str());
         if (-1 != _taccess(buffer, 0))
            skeleton = buffer;
      }
   }
}

std::wstring NifImporter::GetSkeleton(AppSettings *appSettings)
{
   std::wstring skeleton = (appSettings != NULL) ? appSettings->Skeleton : _T("");
   // Guess that the skeleton is the same one in the current directory
   if (importSkeleton && !defaultSkeletonName.empty()) {
      TCHAR buffer[MAX_PATH];
      GetFullPathName(name.c_str(), _countof(buffer), buffer, NULL);
      PathRemoveFileSpec(buffer);
      PathAddBackslash(buffer);
      PathAppend(buffer, defaultSkeletonName.c_str());
      if (-1 != _taccess(buffer, 0))
         skeleton = buffer;
   }
   return skeleton;
}

void NifImporter::LoadIniSettings()
{
   TCHAR iniName[MAX_PATH];
   GetIniFileName(iniName);
   this->iniFileName = iniName;
   iniFileValid = (-1 != _waccess(iniName, 0));

   // Locate which application to use. If Auto, find first app where this file appears in the root path list
   appSettings = NULL;
   std::wstring curapp = GetIniValue<std::wstring>(NifImportSection, _T("CurrentApp"), _T("AUTO"));
   if (0 == _wcsicmp(curapp.c_str(), _T("AUTO"))) {
      autoDetect = true;
      // Scan Root paths
      bool versionmatch = false;
      int version = GetNifVersion(narrow(this->name));
      for (AppSettingsMap::iterator itr = TheAppSettings.begin(), end = TheAppSettings.end(); itr != end; ++itr){
         if ((*itr).IsFileInRootPaths(this->name)) {
            appSettings = &(*itr);
            break;
         } else if ( !versionmatch && ParseVersionString(narrow((*itr).NiVersion)) == version ) {
            // Version matching is an ok fit but we want the other if possible. And we want the first match if possible.
            appSettings = &(*itr);
            versionmatch = true;
         }
      }
   } else {
      autoDetect = false;
      appSettings = FindAppSetting(curapp);
   }
   if (appSettings == NULL && !TheAppSettings.empty()){
      appSettings = &TheAppSettings.front();
   }

   // General System level
   useBiped = GetIniValue(NifImportSection, _T("UseBiped"), false);
   skeletonCheck = GetIniValue<wstring>(NifImportSection, _T("SkeletonCheck"), _T("Bip*"));
   showTextures = GetIniValue(NifImportSection, _T("ShowTextures"), true);
   removeIllegalFaces = GetIniValue(NifImportSection, _T("RemoveIllegalFaces"), true);
   removeDegenerateFaces = GetIniValue(NifImportSection, _T("RemoveDegenerateFaces"), true);
   enableAutoSmooth = GetIniValue(NifImportSection, _T("EnableAutoSmooth"), true);
   autoSmoothAngle = GetIniValue(NifImportSection, _T("AutoSmoothAngle"), 30.0f);
   flipUVTextures = GetIniValue(NifImportSection, _T("FlipUVTextures"), true);
   enableSkinSupport = GetIniValue(NifImportSection, _T("EnableSkinSupport"), true);
   enableCollision = GetIniValue(NifImportSection, _T("EnableCollision"), true);
   enableLights = GetIniValue(NifImportSection, _T("Lights"), false);
   enableCameras = GetIniValue(NifImportSection, _T("Cameras"), false);
   vertexColorMode = GetIniValue<int>(NifImportSection, _T("VertexColorMode"), 1);
   useNiftoolsShader = GetIniValue<int>(NifImportSection, _T("UseNiftoolsShader"), 1);
   mergeNonAccum = GetIniValue(NifImportSection, _T("MergeNonAccum"), true);
   importUPB = GetIniValue(NifImportSection, _T("ImportUPB"), true);
   ignoreRootNode = GetIniValue(NifImportSection, _T("IgnoreRootNode"), true);
   weldVertices = GetIniValue(NifImportSection, _T("WeldVertices"), false);
   weldVertexThresh = GetIniValue(NifImportSection, _T("WeldVertexThresh"), 0.0001f);			// Changed Threshold to a smaller value so it only welds UV split verts by default 
   dummyBonesAsLines = GetIniValue(NifImportSection, _T("DummyBonesAsLines"), true);			// Changed to true as default because it just looks nicer
   altPathLine = GetIniValue<wstring>(NifImportSection, _T("AltPaths"), _T(""));				// New INI setting for setting alternative lookup directories (Currently Texture and Tri)
   importBodyTri = GetIniValue(NifImportSection, _T("ImportBodyTri"), true);					// Ini setting for RaceMenu (Skyrim) BodyTri morph load

   // Biped
   importBones = GetIniValue(BipedImportSection, _T("ImportBones"), true);
   bipedHeight = GetIniValue(BipedImportSection, _T("BipedHeight"), 131.90f);
   bipedAngle = GetIniValue(BipedImportSection, _T("BipedAngle"), 90.0f);
   bipedAnkleAttach = GetIniValue(BipedImportSection, _T("BipedAnkleAttach"), 0.2f);
   bipedTrianglePelvis = GetIniValue(BipedImportSection, _T("BipedTrianglePelvis"), false);
   removeUnusedImportedBones = GetIniValue(BipedImportSection, _T("RemoveUnusedImportedBones"), false);
   forceRotation = GetIniValue(BipedImportSection, _T("ForceRotation"), true);
   browseForSkeleton = GetIniValue(BipedImportSection, _T("BrowseForSkeleton"), true);
   defaultSkeletonName = GetIniValue<wstring>(BipedImportSection, _T("DefaultSkeletonName"), _T("Skeleton.Nif"));
   minBoneWidth = GetIniValue(BipedImportSection, _T("MinBoneWidth"), 0.5f);
   maxBoneWidth = GetIniValue(BipedImportSection, _T("MaxBoneWidth"), 3.0f);
   boneWidthToLengthRatio = GetIniValue(BipedImportSection, _T("BoneWidthToLengthRatio"), 0.25f);
   createNubsForBones = GetIniValue(BipedImportSection, _T("CreateNubsForBones"), true);
   dummyNodeMatches = TokenizeString(GetIniValue<wstring>(BipedImportSection, _T("DummyNodeMatches"), _T("")).c_str(), _T(";"));
   convertBillboardsToDummyNodes = GetIniValue(BipedImportSection, _T("ConvertBillboardsToDummyNodes"), true);
   uncontrolledDummies = GetIniValue(BipedImportSection, _T("UncontrolledDummies"), true);

   // Animation
   replaceTCBRotationWithBezier = GetIniValue(AnimImportSection, _T("ReplaceTCBRotationWithBezier"), true);
   enableAnimations = GetIniValue(AnimImportSection, _T("EnableAnimations"), true);
   requireMultipleKeys = GetIniValue(AnimImportSection, _T("RequireMultipleKeys"), true);
   applyOverallTransformToSkinAndBones = GetIniValue(AnimImportSection, _T("ApplyOverallTransformToSkinAndBones"), true);
   clearAnimation = GetIniValue(AnimImportSection, _T("ClearAnimation"), true);
   addNoteTracks = GetIniValue(AnimImportSection, _T("AddNoteTracks"), true);
   addTimeTags = GetIniValue(AnimImportSection, _T("AddTimeTags"), true);

   rotate90Degrees = TokenizeString(GetIniValue<std::wstring>(NifImportSection, _T("Rotate90Degrees"), _T("")).c_str(), _T(";"));

   // Collision
   bhkScaleFactor = GetIniValue<float>(CollisionSection, _T("bhkScaleFactor"), 7.0f);
   ApplyAppSettings();
}
void NifImporter::ApplyAppSettings()
{
   goToSkeletonBindPosition = false;
   // Override specific settings
   if (appSettings) {
      if (appSettings->disableCreateNubsForBones)
         createNubsForBones = false;
      goToSkeletonBindPosition = appSettings->goToSkeletonBindPosition;
      if (!appSettings->dummyNodeMatches.empty())
         dummyNodeMatches = appSettings->dummyNodeMatches;
      if (appSettings->applyOverallTransformToSkinAndBones != -1)
         applyOverallTransformToSkinAndBones = appSettings->applyOverallTransformToSkinAndBones ? true : false;
      if (!appSettings->rotate90Degrees.empty())
         rotate90Degrees = appSettings->rotate90Degrees;
      supportPrnStrings = appSettings->supportPrnStrings;

	  doNotReuseExistingBones = appSettings->doNotReuseExistingBones;

	  if (!appSettings->skeletonCheck.empty())
		skeletonCheck = appSettings->skeletonCheck;
   }
}

void NifImporter::SaveIniSettings()
{
   SetIniValue(NifImportSection, _T("UseBiped"), useBiped);
   SetIniValue(NifImportSection, _T("EnableSkinSupport"), enableSkinSupport);
   SetIniValue(NifImportSection, _T("VertexColorMode"), vertexColorMode);
   SetIniValue(NifImportSection, _T("EnableCollision"), enableCollision);
   SetIniValue(NifImportSection, _T("Lights"), enableLights);
   SetIniValue(NifImportSection, _T("Cameras"), enableCameras);
   
   //SetIniValue(NifImportSection, "EnableFurniture", enableAnimations);

   SetIniValue(NifImportSection, _T("FlipUVTextures"), flipUVTextures);
   SetIniValue(NifImportSection, _T("ShowTextures"), showTextures);
   SetIniValue(NifImportSection, _T("EnableAutoSmooth"), enableAutoSmooth);
   SetIniValue(NifImportSection, _T("RemoveIllegalFaces"), removeIllegalFaces);
   SetIniValue(NifImportSection, _T("RemoveDegenerateFaces"), removeDegenerateFaces);
   SetIniValue(NifImportSection, _T("ImportUPB"), importUPB);
   SetIniValue(NifImportSection, _T("IgnoreRootNode"), ignoreRootNode);

   SetIniValue(BipedImportSection, _T("ImportBones"), importBones);
   SetIniValue(BipedImportSection, _T("RemoveUnusedImportedBones"), removeUnusedImportedBones);  

   SetIniValue(AnimImportSection, _T("EnableAnimations"), enableAnimations);
   SetIniValue(AnimImportSection, _T("ClearAnimation"), clearAnimation);
   SetIniValue(AnimImportSection, _T("AddNoteTracks"), addNoteTracks);
   SetIniValue(AnimImportSection, _T("AddTimeTags"), addTimeTags);
   SetIniValue(NifImportSection, _T("WeldVertices"), weldVertices);
   SetIniValue(NifImportSection, _T("WeldVertexThresh"), weldVertexThresh);
   SetIniValue(NifImportSection, _T("DummyBonesAsLines"), dummyBonesAsLines);

   SetIniValue(NifImportSection, _T("ImportBodyTri"), importBodyTri);

   SetIniValue<std::wstring>(NifImportSection, _T("CurrentApp"), autoDetect ? _T("AUTO") : appSettings->Name );
}

void NifImporter::RegisterNode(Niflib::NiObjectNETRef node, INode* inode)
{
	nodeMap[node] = inode;
}

INode* NifImporter::FindNode(Niflib::NiObjectNETRef node)
{
	// may want to make this a map if its hit a lot
	if (NULL == node) return NULL;

	NodeToNodeMap::iterator itr = nodeMap.find(node);
	if (itr != nodeMap.end())
		return (*itr).second;

	//return gi->GetINodeByName(node->GetName().c_str());
	return GetNode(wide(node->GetName()));
}

INode *NifImporter::GetNode(Niflib::NiNodeRef node)
{
	return FindNode(node);
}

INode *NifImporter::GetNode(Niflib::NiObjectNETRef obj)
{
	if (obj->IsDerivedType(NiNode::TYPE)) {
		NiNodeRef node = StaticCast<NiNode>(obj);
		if (INode *n = GetNode(node)) {
			return n;
		}
	}
	//return gi->GetINodeByName(obj->GetName().c_str());
	return GetNode(wide(obj->GetName()));
}

void NifImporter::RegisterNode(const std::wstring& name, INode* inode) {
	nodeNameMap[name] = inode;
}

INode *NifImporter::GetNode(const std::wstring& name){
	
	NameToNodeMap::iterator itr = nodeNameMap.find(name);
	if (itr != nodeNameMap.end())
		return (*itr).second;

	INode *node  = gi->GetINodeByName(name.c_str());
	if (node != NULL) {
		nodeNameMap[name] = node;
	}
	return node;
}

INode *NifImporter::GetNode(const TSTR& name){
	return GetNode( std::wstring(name.data()) );
}

bool NifImporter::DoImport()
{
   bool ok = true;
   if (!suppressPrompts)
   {
      if (!ShowDialog())
         return true;

      ApplyAppSettings();
      SaveIniSettings();
   }

   // HACK: Skyrim
   if (IsSkyrim())
	   bhkScaleFactor *= 10.0f;

   vector<std::wstring> importedBones;
   if (!isBiped && importSkeleton && importBones)
   {
      if (importSkeleton && !skeleton.empty()) {
         try
         {
            NifImporter skelImport(skeleton.c_str(), i, gi, suppressPrompts);
            if (skelImport.isValid())
            {
               // Enable Skeleton specific items
               skelImport.isBiped = true;
               skelImport.importBones = true;
               // Disable undesirable skeleton items
               skelImport.enableCollision = false;
               skelImport.enableAnimations = false;
               skelImport.suppressPrompts = true;
               skelImport.DoImport();
               if (!skelImport.useBiped && removeUnusedImportedBones)
                  importedBones = GetNamesOfNodes(skelImport.nodes);
            }
         }
         catch (RuntimeError &error)
         {
            // ignore import errors and continue
         }
      }
   } else if (hasSkeleton && useBiped && importBones) {
      ImportBipeds(nodes);
   }

   if (isValid()) {

      if (root->IsDerivedType(NiNode::TYPE))
      {
         NiNodeRef rootNode = root;

		 if (importBones) {
			 if (ignoreRootNode || strmatch(wide(rootNode->GetName()), _T("Scene Root"))) {
				 RegisterNode(root, gi->GetRootNode());
				 ImportBones(DynamicCast<NiNode>(rootNode->GetChildren()));
			 } else {
				 ImportBones(rootNode);
			 }
		 }


         if (enableLights){
            ok = ImportLights(rootNode);
         }

         ok = ImportMeshes(rootNode);

		 // Import Havok Collision Data surrounding node
		 if (enableCollision) {
			 ImportCollision(rootNode);
		 }

         if (importSkeleton && removeUnusedImportedBones){
            vector<std::wstring> importedNodes = GetNamesOfNodes(nodes);
            sort(importedBones.begin(), importedBones.end());
            sort(importedNodes.begin(), importedNodes.end());
            vector<std::wstring> results;
            results.resize(importedBones.size());
            vector<std::wstring>::iterator end = set_difference ( 
               importedBones.begin(), importedBones.end(),
               importedNodes.begin(), importedNodes.end(), results.begin());
            for (vector<std::wstring>::iterator itr = results.begin(); itr != end; ++itr){
               if (INode *node = gi->GetINodeByName((*itr).c_str())){
				   gi->DeleteNode(node, FALSE);
               }
            }
         }
      }

      else if (root->IsDerivedType(NiTriShape::TYPE))
      {
         ok |= ImportMesh(NiTriShapeRef(root));
      }
      else if (root->IsDerivedType(NiTriStrips::TYPE))
      {
         ok |= ImportMesh(NiTriStripsRef(root));
      }
   }

   // BodyTri
   if (importBodyTri) {
	   wstring fileName = getBodyTriPath(root, path, altPathLine);
	   readBodyTri(fileName, imports);
   }

   ClearAnimation();
   ImportAnimation();
   return true;
}
bool NifImporter::IsSkyrim() const {
	return (nifVersion == 0x14020007 && userVersion == 12);
}
bool NifImporter::IsFallout3() const {
	return (nifVersion == 0x14020007 && userVersion == 11);
}
bool NifImporter::IsOblivion() const {
	return ((nifVersion == 0x14000004 || nifVersion == 0x14000005) && (userVersion == 11 || userVersion == 10));
}
bool NifImporter::IsMorrowind() const {
	return ((nifVersion == 0x04000002) && (userVersion == 11 || userVersion == 10) );
}
