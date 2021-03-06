/**********************************************************************
*<
FILE: NIUtils.h

DESCRIPTION:	NifImporter Utilities

CREATED BY: tazpn (Theo)

HISTORY: 

INFO: See Implementation for minimalist comments

*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/
#ifndef _NIUTILS_H_
#define _NIUTILS_H_

#ifndef _WINDOWS_
#  include <windows.h>
#endif
#include <tchar.h>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <list>
#include <map>

// Max Headers
#include <Max.h>
#include <strclass.h>
#include <color.h>

// Niflib Headers
#include <niflib.h>
#include <obj\NiObject.h>
#include <obj\NiAVObject.h>
#include <obj\NiObjectNET.h>
#include <obj\NiNode.h>
#include <obj\NiTriBasedGeom.h>
#include <gen\QuaternionXYZW.h>
#include <nif_math.h>

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif

const unsigned int IntegerInf = 0x7f7fffff;
const unsigned int IntegerNegInf = 0xff7fffff;
const float FloatINF = *(float*)&IntegerInf;
const float FloatNegINF = *(float*)&IntegerNegInf;

// Trim whitespace before and after a string
inline TCHAR *Trim(TCHAR*&p) { 
   while(_istspace(*p)) *p++ = 0; 
   TCHAR *e = p + _tcslen(p) - 1;
   while (e > p && _istspace(*e)) *e-- = 0;
   return p;
}

inline std::wstring Trim(std::wstring& p)
{
	std::wstring str = p;
	size_t found;
	found = str.find_last_not_of(' ');

	if (found != std::wstring::npos)
		str.erase(found + 1);
	else
		str.clear(); // str is all whitespace

	return str;
}

// Case insensitive string equivalence test for collections
struct ltstr
{
	bool operator()(const wchar_t* s1, const wchar_t* s2) const { return _wcsicmp(s1, s2) < 0; }
	bool operator()(const std::wstring& s1, const std::wstring& s2) const { return _wcsicmp(s1.c_str(), s2.c_str()) < 0; }
	bool operator()(const std::wstring& s1, const wchar_t* s2) const { return _wcsicmp(s1.c_str(), s2) < 0; }
	bool operator()(const wchar_t* s1, const std::wstring& s2) const { return _wcsicmp(s1, s2.c_str()) < 0; }
};


// Case insensitive string equivalence but numbers are sorted together
struct NumericStringEquivalence
{
   bool operator()(const TCHAR* s1, const TCHAR* s2) const
   { return numstrcmp(s1, s2) < 0; }

   bool operator()(const std::wstring& s1, const TCHAR* s2) const
   { return numstrcmp(s1.c_str(), s2) < 0; }

   bool operator()(const TCHAR* s1, const std::wstring& s2) const
   { return numstrcmp(s1, s2.c_str()) < 0; }

   bool operator()(const std::wstring& s1, const std::wstring& s2) const
   { return numstrcmp(s1.c_str(), s2.c_str()) < 0; }

   static int numstrcmp(const TCHAR *str1, const TCHAR *str2)
   {
      TCHAR *p1, *p2;
      int c1, c2, lcmp;
      for(;;)
      {
         c1 = tolower(*str1), c2 = tolower(*str2);
         if ( c1 == 0 || c2 == 0 )
            break;
         else if (isdigit(c1) && isdigit(c2))
         {			
            lcmp = wcstol(str1, &p1, 10) - wcstol(str2, &p2, 10);
            if ( lcmp == 0 )
               lcmp = (p2 - str2) - (p1 - str1);
            if ( lcmp != 0 )
               return (lcmp > 0 ? 1 : -1);
            str1 = p1, str2 = p2;
         }
         else
         {
            lcmp = (c1 - c2);
            if (lcmp != 0)
               return (lcmp > 0 ? 1 : -1);
            ++str1, ++str2;
         }
      }
      lcmp = (c1 - c2);
      return ( lcmp < 0 ) ? -1 : (lcmp > 0 ? 1 : 0);
   }
};

// Common collections that I use
typedef std::map<std::wstring, std::wstring, ltstr> NameValueCollection;
typedef std::pair<std::wstring, std::wstring> KeyValuePair;
typedef std::vector<std::wstring> stringlist;

extern int wildcmp(const TCHAR *wild, const TCHAR *string);
extern int wildcmpi(const TCHAR *wild, const TCHAR *string);

inline bool strmatch(const std::wstring& lhs, const std::wstring& rhs) {
   return (0 == _wcsicmp(lhs.c_str(), rhs.c_str()));
}
inline bool strmatch(const TCHAR* lhs, const std::wstring& rhs) {
   return (0 == _wcsicmp(lhs, rhs.c_str()));
}
inline bool strmatch(const wstring& lhs, const TCHAR* rhs) {
   return (0 == _wcsicmp(lhs.c_str(), rhs));
}
inline bool strmatch(const TCHAR* lhs, const TCHAR* rhs) {
   return (0 == _wcsicmp(lhs, rhs));
}

bool wildmatch(const TCHAR* match, const TCHAR* value);
bool wildmatch(const std::wstring& match, const std::wstring& value);
bool wildmatch(const stringlist& matches, const std::wstring& value);

// Generic IniFile reading routine
template<typename T>
inline T GetIniValue(LPCTSTR Section, LPCTSTR Setting, T Default, LPCTSTR iniFileName){
   T v;
   TCHAR buffer[1024];
   std::wstringstream sstr;
   sstr << Default;
   buffer[0] = 0;
   if (0 < GetPrivateProfileString(Section, Setting, sstr.str().c_str(), buffer, sizeof(buffer), iniFileName)){
      std::wstringstream sstr(buffer);
      sstr >> v;
      return v;
   }
   return Default;
}

// Specific override for int values
template<>
inline int GetIniValue<int>(LPCTSTR Section, LPCTSTR Setting, int Default, LPCTSTR iniFileName){
   return GetPrivateProfileInt(Section, Setting, Default, iniFileName);
}

// Specific override for string values
template<>
inline std::wstring GetIniValue<std::wstring>(LPCTSTR Section, LPCTSTR Setting, std::wstring Default, LPCTSTR iniFileName){
   TCHAR buffer[1024];
   buffer[0] = 0;
   if (0 < GetPrivateProfileString(Section, Setting, Default.c_str(), buffer, sizeof(buffer), iniFileName)){
      return std::wstring(buffer);
   }
   return Default;
}

// Specific override for TSTR values
template<>
inline TSTR GetIniValue<TSTR>(LPCTSTR Section, LPCTSTR Setting, TSTR Default, LPCTSTR iniFileName){
   TCHAR buffer[1024];
   buffer[0] = 0;
   if (0 < GetPrivateProfileString(Section, Setting, Default.data(), buffer, sizeof(buffer), iniFileName)){
      return TSTR(buffer);
   }
   return Default;
}

// Generic IniFile reading routine
template<typename T>
inline void SetIniValue(LPCTSTR Section, LPCTSTR Setting, T value, LPCTSTR iniFileName){
   std::wstringstream sstr;
   sstr << value;
   WritePrivateProfileString(Section, Setting, sstr.str().c_str(), iniFileName);
}

// Specific override for string values
template<>
inline void SetIniValue<std::wstring>(LPCTSTR Section, LPCTSTR Setting, std::wstring value, LPCTSTR iniFileName){
   WritePrivateProfileString(Section, Setting, value.c_str(), iniFileName);
}

// Specific override for TSTR values
template<>
inline void SetIniValue<TSTR>(LPCTSTR Section, LPCTSTR Setting, TSTR value, LPCTSTR iniFileName){
   WritePrivateProfileString(Section, Setting, value.data(), iniFileName);
}

extern TSTR FormatText(const TCHAR* format,...);
extern std::wstring FormatString(const TCHAR* format,...);

extern stringlist TokenizeString(LPCWSTR str, LPCWSTR delims, bool trim = false);
extern stringlist TokenizeCommandLine(LPCWSTR str, bool trim);
extern std::wstring JoinCommandLine(stringlist args);

extern std::wstring GetIndirectValue(LPCWSTR path);
extern NameValueCollection ReadIniSection(LPCWSTR Section, LPCWSTR iniFileName);
extern std::wstring ExpandQualifiers(const std::wstring& src, const NameValueCollection& map);
extern std::wstring ExpandEnvironment(const std::wstring& src);

extern void FindImages(NameValueCollection& images, const std::wstring& rootPath, const stringlist& searchpaths, const stringlist& extensions);

extern void RenameNode(Interface *gi, LPCTSTR SrcName, LPCTSTR DstName);

enum PosRotScale
{
   prsPos = 0x1,
   prsRot = 0x2,
   prsScale = 0x4,
   prsDefault = prsPos | prsRot | prsScale,
};
extern void PosRotScaleNode(INode *n, Point3 p, Quat& q, float s, PosRotScale prs = prsDefault, TimeValue t = 0);
extern void PosRotScaleNode(Control *c, Point3 p, Quat& q, float s, PosRotScale prs = prsDefault, TimeValue t = 0);
extern void PosRotScaleNode(INode *n, Matrix3& m3, PosRotScale prs = prsDefault, TimeValue t = 0);
extern void PosRotScaleNode(Control *c, Matrix3& m3, PosRotScale prs = prsDefault, TimeValue t = 0);
extern Matrix3 GetNodeLocalTM(INode *n, TimeValue t = 0);

extern Niflib::NiNodeRef FindNodeByName( const vector<Niflib::NiNodeRef>& blocks, const string& name );
extern std::vector<Niflib::NiNodeRef> SelectNodesByName( const vector<Niflib::NiNodeRef>& blocks, LPCTSTR match);
extern int CountNodesByName( const vector<Niflib::NiNodeRef>& blocks, LPCTSTR match );
extern std::vector<std::wstring> GetNamesOfNodes( const vector<Niflib::NiNodeRef>& blocks );
extern std::vector<Niflib::NiNodeRef> SelectNodesByName( const vector<Niflib::NiNodeRef>& blocks, LPCTSTR match);

extern INode* FindINode(Interface *i, const string& name);
extern INode* FindINode(Interface *i, Niflib::NiObjectNETRef node);

struct NodeEquivalence
{
   bool operator()(const Niflib::NiNodeRef& lhs, const Niflib::NiNodeRef& rhs) const{
      return (!lhs || !rhs) ? (lhs < rhs) : (lhs->GetName() < rhs->GetName());
   }
   bool operator()(const Niflib::NiNodeRef& lhs, const std::string& rhs) const{
      return (lhs->GetName() < rhs);
   }
   bool operator()(const std::string& lhs, const Niflib::NiNodeRef& rhs) const{
      return (lhs < rhs->GetName());
   }
};

inline Niflib::NiNodeRef BinarySearch(vector<Niflib::NiNodeRef> &nodes, const string& name)
{
   typedef std::pair<vector<Niflib::NiNodeRef>::iterator, vector<Niflib::NiNodeRef>::iterator> NiNodePair;
   NiNodePair pair = std::equal_range(nodes.begin(), nodes.end(), name, NodeEquivalence());
   if (pair.first != pair.second) {
      return (*pair.first);
   }
   return Niflib::NiNodeRef();
}

// Simple conversion helpers
static inline float TODEG(float x) { return x * 180.0f / PI; }
static inline float TORAD(float x) { return x * PI / 180.0f; }

inline string narrow(std::wstring const& text) {
	std::locale loc("");
	wchar_t const* from = text.c_str();
	std::size_t const len = text.size();
	std::vector<char> buffer(len + 1);
	std::use_facet<std::ctype<wchar_t> >(loc).narrow(from, from + len, '_', &buffer[0]);
	return std::string(&buffer[0], &buffer[len]);
}

inline wstring wide(std::string const& text) {
	std::locale loc("");
	char const* from = text.c_str();
	std::size_t const len = text.size();
	std::vector<char> buffer(len + 1);
	std::use_facet<std::ctype<char> >(loc).widen(from, from + len, &buffer[0]);
	return std::wstring(&buffer[0], &buffer[len]);
}

inline vector<wstring> ParseAltPaths(wstring const& paths) {
	vector<wstring> tokens;
	string item;
	stringstream toParse(narrow(paths.c_str()));
	while (getline(toParse, item, ',')) {
		if (item.back() != '\\')
			item += "\\";
		tokens.push_back(wide(item));
	}
	return tokens;
}

static inline Color TOCOLOR(const Niflib::Color3& c3) {
   return Color(c3.r, c3.g, c3.b);
}

static inline Niflib::Color3 TOCOLOR3(const Color& c3) {
   return Niflib::Color3(c3.r, c3.g, c3.b);
}

static inline Niflib::Color3 TOCOLOR3(const Point3& c3) {
   return Niflib::Color3(c3.x, c3.y, c3.z);
}

static inline Point3 TOPOINT3(const Niflib::Color3& c3){
   return Point3(c3.r, c3.g, c3.b);
}

static inline Point3 TOPOINT3(const Niflib::Vector3& v){
   return Point3(v.x, v.y, v.z);
}

static inline Point3 TOPOINT3(const Niflib::Vector4& v){
	return Point3(v.x, v.y, v.z);
}

static inline Niflib::Vector3 TOVECTOR3(const Point3& v){
   return Niflib::Vector3(v.x, v.y, v.z);
}

static inline Niflib::Vector3 TOVECTOR3(const Niflib::Vector4& v){
	return Niflib::Vector3(v.x, v.y, v.z);
}

static inline Niflib::Vector4 TOVECTOR4(const Point3& v, float w = 0.0){
	return Niflib::Vector4(v.x, v.y, v.z, w);
}


static inline Quat TOQUAT(const Niflib::Quaternion& q, bool inverse = false){
   Quat qt(q.x, q.y, q.z, q.w);
   return (inverse && q.w != FloatNegINF) ? qt.Inverse() : qt;
}

static inline Quat TOQUAT(const Niflib::QuaternionXYZW& q, bool inverse = false){
   Quat qt(q.x, q.y, q.z, q.w);
   return (inverse && q.w != FloatNegINF) ? qt.Inverse() : qt;
}

static inline Niflib::Quaternion TOQUAT(const Quat& q, bool inverse = false){
   return (inverse && q.w != FloatNegINF) ? TOQUAT(q.Inverse(), false) : Niflib::Quaternion(q.w, q.x, q.y, q.z);
}

static inline Niflib::QuaternionXYZW TOQUATXYZW(const Niflib::Quaternion& q){
   Niflib::QuaternionXYZW qt;
   qt.x = q.x; qt.y = q.y; qt.z = q.z; qt.w = q.w;
   return qt;
}

static inline Niflib::QuaternionXYZW TOQUATXYZW(const Quat& q){
   Niflib::QuaternionXYZW qt;
   qt.x = q.x; qt.y = q.y; qt.z = q.z; qt.w = q.w;
   return qt;
}

static inline AngAxis TOANGAXIS(const Niflib::Quaternion& q, bool inverse = false){
   Quat qt(q.x, q.y, q.z, q.w);
   if (inverse && q.w != FloatNegINF) qt.Invert();
   return AngAxis(q.x, q.y, q.z, q.w);
}

static inline Matrix3 TOMATRIX3(const Niflib::Matrix44 &tm, bool invert = false){
   Niflib::Vector3 pos; Niflib::Matrix33 rot; float scale;
   tm.Decompose(pos, rot, scale);
   Matrix3 m(rot.rows[0].data, rot.rows[1].data, rot.rows[2].data, Point3());
   if (invert) m.Invert();
   m.Scale(Point3(scale, scale, scale));
   m.SetTrans(Point3(pos.x, pos.y, pos.z));
   return m;
}

static inline Niflib::Matrix33 TOMATRIX33(const Matrix3 &tm, bool invert = false){
	Niflib::Matrix33 m3(tm.GetRow(0)[0], tm.GetRow(0)[1], tm.GetRow(0)[2],
		tm.GetRow(1)[0], tm.GetRow(1)[1], tm.GetRow(1)[2],
		tm.GetRow(2)[0], tm.GetRow(2)[1], tm.GetRow(2)[2]);
	return m3;
}

static inline Matrix3 TOMATRIX3(Niflib::Vector3& trans, Niflib::QuaternionXYZW quat, float scale){
	Matrix3 tm(true), qm;
	Quat q(quat.x, quat.y, quat.z, quat.w);
	q.MakeMatrix(qm);
	tm.SetTranslate(TOPOINT3(trans));
	tm *= qm;
	tm *= ScaleMatrix(Point3(scale, scale, scale));
	return tm;
}

static inline Matrix3 TOMATRIX3(Niflib::Vector3& trans, Niflib::Quaternion quat, float scale){
	Matrix3 tm, qm;
	Quat q(quat.x, quat.y, quat.z, quat.w);
	q.MakeMatrix(qm);
	tm.SetTranslate(TOPOINT3(trans));
	tm *= qm;
	tm *= ScaleMatrix(Point3(scale, scale, scale));
	return tm;
}

static inline Niflib::Matrix44 TOMATRIX4(const Matrix3 &tm, bool invert = false){
   Niflib::Matrix33 m3(tm.GetRow(0)[0], tm.GetRow(0)[1], tm.GetRow(0)[2],
                       tm.GetRow(1)[0], tm.GetRow(1)[1], tm.GetRow(1)[2],
                       tm.GetRow(2)[0], tm.GetRow(2)[1], tm.GetRow(2)[2]);
   Niflib::Matrix44 m4(TOVECTOR3(tm.GetTrans()), m3, 1.0f);
   return m4;
}

static inline Point3 GetScale(const Matrix3& mtx){
   return Point3( fabs(mtx.GetRow(0)[0]), fabs(mtx.GetRow(1)[1]), fabs(mtx.GetRow(2)[2]) );
}

static inline float Average(const Point3& val) {
   return (val[0] + val[1] + val[2]) / 3.0f;
}

static inline float Average(const Niflib::Vector3& val) {
   return (val.x + val.y + val.z) / 3.0f;
}

template <typename U, typename T>
inline Niflib::Ref<U> SelectFirstObjectOfType( vector<Niflib::Ref<T> > const & objs ) {
   for (vector<Niflib::Ref<T> >::const_iterator itr = objs.begin(), end = objs.end(); itr != end; ++itr) {
      Niflib::Ref<U> obj = DynamicCast<U>(*itr);
      if (obj) return obj;
   }
   return Niflib::Ref<U>();
}

template <typename U, typename T>
inline Niflib::Ref<U> SelectFirstObjectOfType( list<Niflib::Ref<T> > const & objs ) {
   for (list<Niflib::Ref<T> >::const_iterator itr = objs.begin(), end = objs.end(); itr != end; ++itr) {
      Niflib::Ref<U> obj = DynamicCast<U>(*itr);
      if (obj) return obj;
   }
   return Niflib::Ref<U>();
}

TSTR PrintMatrix3(Matrix3& m);
TSTR PrintMatrix44(Niflib::Matrix44& m);

extern Modifier *GetOrCreateSkin(INode *node);
extern Modifier *GetSkin(INode *node);
extern TriObject* GetTriObject(Object *o);

extern TSTR GetFileVersion(const wchar_t *fileName);

template<typename T>
inline Niflib::Ref<T> CreateNiObject() {
   return Niflib::StaticCast<T>(T::Create());
}

void CollapseGeomTransform(Niflib::NiTriBasedGeomRef shape);
void CollapseGeomTransforms(std::vector<Niflib::NiTriBasedGeomRef>& shapes);
void FixNormals(std::vector<Niflib::Triangle>& tris, std::vector<Niflib::Vector3>& verts, std::vector<Niflib::Vector3>& norms);

Modifier *GetbhkCollisionModifier(INode* node);
Modifier *CreatebhkCollisionModifier(
	INode* node
	, int type
	, Niflib::HavokMaterial material
	, Niflib::OblivionLayer layer
	, byte filter
	);

void GetIniFileName(wchar_t *iniName);

Matrix3 GetLocalTM(INode *node);

// Morph related routines in nimorph.cpp
extern Modifier *GetMorpherModifier(INode* node);
extern Modifier *CreateMorpherModifier(INode* node);
extern void MorpherBuildFromNode(Modifier* mod, int index, INode *target);
extern void MorpherSetName(Modifier* mod, int index, TSTR& name);
extern void MorpherRebuild(Modifier* mod, int index);
extern TSTR MorpherGetName(Modifier* mod, int index);
extern bool MorpherIsActive(Modifier* mod, int index);
extern bool MorpherHasData(Modifier* mod, int index);
extern int MorpherNumProgMorphs(Modifier* mod, int index);
extern INode *MorpherGetProgMorph(Modifier* mod, int index, int morphIdx);
extern void MorpherGetMorphVerts(Modifier* mod, int index, vector<Niflib::Vector3>& verts);

#pragma region Enumeration support
// Enumeration support
typedef struct EnumLookupType {
   int value;
   const wchar_t *name;
} EnumLookupType;

extern TSTR EnumToString(int value, const EnumLookupType *table);
extern int StringToEnum(TSTR value, const EnumLookupType *table);
extern int EnumToIndex(int value, const EnumLookupType *table);

extern TSTR FlagsToString(int value, const EnumLookupType *table);
extern int StringToFlags(TSTR value, const EnumLookupType *table);
#pragma endregion

extern unsigned long Crc32Array(const void *data, size_t size);

#endif // _NIUTILS_H_