#include "Max.h"
#include "MeshDelta.h"
#include "nifqhull.h"

using namespace Niflib;

void get_convex_hull(Mesh& mesh, Mesh& outmesh)
{  
	MNMesh mn;
	map<int, int> ptmap;
	vector<Vector3> vertmap;
	for (int i = 0; i < mesh.getNumVerts(); ++i)
	{
		Vector3 temp(mesh.getVert(i).x, mesh.getVert(i).y, mesh.getVert(i).z);
		vertmap.push_back(temp);
	}
	vector<Triangle> outshape = NifQHull::compute_convex_hull(vertmap);
	vector<Triangle>::size_type sz = outshape.size();
	for (int i = 0; i < mesh.getNumVerts(); ++i)
	{
		mn.NewVert(Point3(mesh.getVert(i).x, mesh.getVert(i).y, mesh.getVert(i).z));
	}
	for (unsigned i = 0; i < sz; i++)
	{
		mn.NewTri(outshape[i].v1, outshape[i].v2, outshape[i].v3);
	}
	mn.EliminateIsoMapVerts();
	mn.MakeConvex();
	mn.FillInMesh();
	mn.EliminateBadVerts(0);
	mn.Triangulate();
	mn.OutToTri(outmesh);
}