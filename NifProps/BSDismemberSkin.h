/**********************************************************************
*<
FILE: iNifProps.h

DESCRIPTION: BSDismember Modifier Interface definitions

CREATED BY: Blabba

HISTORY:

*>	Copyright (c) 2015, All Rights Reserved.
**********************************************************************/

#include <max.h>
#include <meshdlib.h>
#include <string.h>

// Header Definitions
#define BSDSMODIFIER_CLASS_ID			Class_ID(0xe9a0a68e, 0xb091bd48)

//--------------------------- Class Definitions -----------------------------

class BSDSModifier : public Modifier, public FlagUser {
public:
	IParamBlock2 *pblock;

	static IObjParam *ip;
	static BSDSModifier *editMod;
	static SelectModBoxCMode *selectMode;
	static BOOL updateCachePosted;

	// Default Constructor
	BSDSModifier();

	// Animatable Class Overrides
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) { s = GetString(IDS_RB_BSDSMODIFIER); }
	virtual Class_ID ClassID() { return BSDSMODIFIER_CLASS_ID; }
	RefTargetHandle Clone(RemapDir& remap /*= DefaultRemapDir()*/);
	const TCHAR *GetObjectName() { return GetString(IDS_RB_BSDSMODIFIER); }

	// Modifier Class Overrides
	ChannelMask ChannelsUsed()  { return PART_GEOM | PART_TOPO; }
	ChannelMask ChannelsChanged() { return PART_SELECT | PART_SUBSEL_TYPE | PART_TOPO | PART_GEOM; }
	Class_ID InputType() { return triObjectClassID; }
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t);
	Interval GetValidity(TimeValue t);
	BOOL DependOnTopology(ModContext &mc) { return TRUE; }

	// BaseObject Class Overrides
	CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; }
	void BeginEditParams(IObjParam  *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
	void ActivateSubobjSel(int level, XFormModes& modes);
	void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert = FALSE);
	void ClearSelection(int selLevel);
	void SelectAll(int selLevel);
	void InvertSelection(int selLevel);
	void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);
	void ShowEndResultChanged(BOOL showEndResult) { NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE); }
	BOOL SupportsNamedSubSels() { return FALSE; }

	// New SubObjType API
	int NumSubObjTypes();
	ISubObjType *GetSubObjType(int i);

	// IO Class Overrides
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	IOResult SaveLocalData(ISave *isave, LocalModData *ld);
	IOResult LoadLocalData(ILoad *iload, LocalModData **pld);

	int NumParamBlocks() { return 1; }
	IParamBlock2 *GetParamBlock(int i) { return pblock; }
	IParamBlock2 *GetParamBlockByID(short id) { return (pblock->ID() == id) ? pblock : NULL; }
	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i) { return pblock; }

	Animatable* SubAnim(int i) { return GetReference(i); }
	TSTR SubAnimName(int i) { return GetString(IDS_RB_PARAMETERS); }

	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

	// Modifier Methods
	void UpdateSelLevelDisplay();
	void SetEnableStates();
	void SelectByMatID(int id);
	void SetNumSelLabel();
	void UpdateCache(TimeValue t);
	void InvalidateDialogElement(int elem);

	void LocalDataChanged();
	void SelectUnused();
	BSDSData* GetModifierData();
};

class BSDSData : public LocalModData {
private:
	// Temp data used for soft selections, adjacent edge / face lists.
	MeshTempData *temp;

public:
	Tab<BitArray*>
}

//--------------------------- BSDismember Modifier Sub-Class definitions -----------------------------
// Image list used for BSDismember sub-object toolbar:
class BSDSImageHandler {
public:
	HIMAGELIST images;
	BSDSImageHandler() { images = NULL; }
	~BSDSImageHandler() { if (images) ImageList_Destroy(images); }
	// BSDSImageHandler methods.
	HIMAGELIST LoadImages() {
		if (images) return images;
		images = ImageList_Create(24, 23, ILC_COLOR | ILC_MASK, 10, 0);
		HBITMAP hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_EM_SELTYPES));
		HBITMAP hMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_EM_SELMASK));
		ImageList_Add(images, hBitmap, hMask);
		DeleteObject(hBitmap), DeleteObject(hMask);
		return images;
	}
};

// Image Part handler
class BSDSPartImageHandler {
public:
	HIMAGELIST images;
	BSDSPartImageHandler() { images = NULL; }
	~BSDSPartImageHandler() { if (images) ImageList_Destroy(images); }
	HIMAGELIST LoadImages() {
		if (images) return images;
		images = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 0);
		HBITMAP hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_EM_ADDDELPART));
		HBITMAP hMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_EM_ADDDELMASK));
		ImageList_Add(images, hBitmap, hMask);
		DeleteObject(hBitmap), DeleteObject(hMask);
		return images;
	}
};

Modifier *GetOrCreateBSDismemberSkin(INode *node);
Modifier *GetBSDismemberSkin(INode *node);