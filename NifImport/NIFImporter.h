/**********************************************************************
*<
FILE: NifImporter.h

DESCRIPTION:	NIF Importer 

CREATED BY: tazpn (Theo)

HISTORY:

*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/

#ifndef __NIFIMPORTER_H__
#define __NIFIMPORTER_H__

#include "BaseImporter.h"
#include "IniSection.h"
#include "obj/BSLightingShaderProperty.h"

namespace Niflib
{
   class NiTextKeyExtraData;
}

// NIF Importer
class NifImporter : public BaseImporter//, public IniFileSection
{
public:
   // Ini settings
   bool showTextures; // show textures in viewport
   bool removeIllegalFaces;
   bool removeDegenerateFaces;
   bool enableAutoSmooth;
   float autoSmoothAngle;
   bool flipUVTextures;
   bool enableSkinSupport;
   bool goToSkeletonBindPosition;
   bool enableCollision;
   int vertexColorMode;
   int useNiftoolsShader;
   bool mergeNonAccum;
   bool enableLights;
   bool enableCameras;
   bool importUPB;
   bool doNotReuseExistingBones;
   wstring altPathLine;
   bool importBodyTri;
   vector<INode*>imports;

   // Biped/Bones related settings
   bool importBones;
   std::wstring skeleton;
   float bipedHeight;
   std::wstring skeletonCheck;
   float bipedAngle;
   float bipedAnkleAttach;
   bool bipedTrianglePelvis;
   bool importSkeleton;
   bool useBiped;
   bool hasSkeleton;
   bool isBiped;
   bool removeUnusedImportedBones;
   bool forceRotation;
   bool browseForSkeleton;
   std::wstring defaultSkeletonName;
   float minBoneWidth;
   float maxBoneWidth;
   float boneWidthToLengthRatio;
   bool createNubsForBones;
   stringlist dummyNodeMatches;
   bool convertBillboardsToDummyNodes;
   bool uncontrolledDummies;
   bool ignoreRootNode;
   bool autoDetect;
   stringlist rotate90Degrees;
   bool supportPrnStrings;

   // Animation related Settings
   bool replaceTCBRotationWithBezier;
   bool enableAnimations;
   bool requireMultipleKeys;
   bool applyOverallTransformToSkinAndBones;
   bool clearAnimation;
   bool addNoteTracks;
   bool addTimeTags;

   // Collision settings
   float bhkScaleFactor;

   bool weldVertices;
   float weldVertexThresh;

   bool dummyBonesAsLines;

   vector<Niflib::NiObjectRef> blocks;
   vector<Niflib::NiNodeRef> nodes;
   map<std::wstring,int> ctrlCount; // counter for number of controllers referencing a node

   typedef map<Niflib::NiObjectNETRef, INode*> NodeToNodeMap;
   typedef map<std::wstring, INode*, ltstr> NameToNodeMap;
   NodeToNodeMap nodeMap;
   NameToNodeMap nodeNameMap;


   NifImporter(const TCHAR *Name,ImpInterface *I,Interface *GI, BOOL SuppressPrompts);
   virtual void Initialize();
   virtual void ReadBlocks();
   void BuildNodes();

   // Ini File related routines
   virtual void LoadIniSettings();
   virtual void SaveIniSettings();

   void ApplyAppSettings();

   bool HasSkeleton();
   bool IsBiped();
   void ImportBones(vector<Niflib::NiNodeRef>& bones);
   void ImportBones(Niflib::NiNodeRef blocks, bool recurse = true);
   void ImportBipeds(vector<Niflib::NiNodeRef>& blocks);
   void AlignBiped(IBipMaster* master, Niflib::NiNodeRef block);
   bool ImportMeshes(Niflib::NiNodeRef block);
   std::wstring FindImage(const std::wstring& name);

   bool ImportUPB(INode *node, Niflib::NiNodeRef block);

   void SetTriangles(Mesh& mesh, const vector<Niflib::Triangle>& v);
   void SetNormals(Mesh& mesh, const vector<Niflib::Triangle>& t, const vector<Niflib::Vector3>& v);

   bool ImportMesh(Niflib::NiTriShapeRef triShape);
   bool ImportMesh(Niflib::NiTriStripsRef triStrips);
   bool ImportMultipleGeometry(Niflib::NiNodeRef parent, vector<Niflib::NiTriBasedGeomRef>& glist);
   StdMat2 *ImportMaterialAndTextures(ImpNode *node, Niflib::NiAVObjectRef avObject);
   bool ImportMaterialAndTextures(ImpNode *node, vector<Niflib::NiTriBasedGeomRef>& glist);
   bool ImportNiftoolsShader(ImpNode *node, Niflib::NiAVObjectRef avObject, StdMat2 *m);
   bool ImportTransform(ImpNode *node, Niflib::NiAVObjectRef avObject);
   bool ImportMesh(ImpNode *node, TriObject *o, Niflib::NiTriBasedGeomRef triGeom, Niflib::NiTriBasedGeomDataRef triGeomData, vector<Niflib::Triangle>& tris);
   bool ImportVertexColor(INode *tnode, TriObject *o, vector<Niflib::Triangle>& tris, vector<Niflib::Color4> cv, int cv_offset=0);
   bool ImportSkin(ImpNode *node, Niflib::NiTriBasedGeomRef triGeom, int v_start=0);
   Texmap* CreateTexture(Niflib::TexDesc& desc);
   Texmap* CreateTexture(Niflib::NiTexturePropertyRef desc);
   Texmap* CreateTexture(const std::wstring& name);
   Texmap* CreateTexture(const std::wstring& filename, Niflib::BSLightingShaderPropertyRef mode);
   Texmap* CreateNormalBump(LPCTSTR name, Texmap* nmap);
   Texmap* CreateMask(LPCTSTR name, Texmap* nmap, Texmap* mask);
   
   INode *CreateBone(const std::wstring& name, Point3 startPos, Point3 endPos, Point3 zAxis);
   INode *CreateHelper(const std::wstring& name, Point3 startPos);
   INode *CreateCamera(const std::wstring& name);

   INode *CreateImportNode(const wchar_t *name, Object *obj, INode* parent);

   bool ImportLights(Niflib::NiNodeRef node);
   bool ImportLights(vector<Niflib::NiLightRef> lights);

   // Primary Collision entry point.  Tests for bhk objects
   bool ImportCollision(Niflib::NiNodeRef node);

   void RegisterNode(Niflib::NiObjectNETRef node, INode* inode);
   INode *FindNode(Niflib::NiObjectNETRef node);

   INode *GetNode(Niflib::NiNodeRef node);
   INode *GetNode(Niflib::NiObjectNETRef obj);

   void RegisterNode(const std::wstring& name, INode* inode);
   INode *GetNode(const std::wstring& name);
   INode *GetNode(const TSTR& name);

   std::wstring GetSkeleton(AppSettings *appSettings);

   bool ShowDialog();
   virtual bool DoImport();

   // Animation Helpers
   bool ImportAnimation();
   void ClearAnimation();
   void ClearAnimation(INode *node);
   bool AddNoteTracks(float time, std::wstring name, std::wstring target, Niflib::Ref<Niflib::NiTextKeyExtraData> textKeyData, bool loop);

   void WeldVertices(Mesh& mesh);

   bool IsSkyrim() const;
   bool IsFallout3() const;
   bool IsOblivion() const;
   bool IsMorrowind() const;

   protected: 
      NifImporter();
};

#endif
