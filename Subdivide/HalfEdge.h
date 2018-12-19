#pragma once

#include <iostream>
#include <fstream>
#include <strstream>
#include <vector>
#include <memory>
#include <string>
#include <set>
#include "point3.h"
#define PI 3.1415926535898
using namespace std;

typedef size_t Index;

namespace HE {
	class Vertex;
	class Halfedge;
	class Edge;
	class Face;

	typedef vector<shared_ptr<Vertex>> VertexList;
	typedef vector<shared_ptr<Edge>> EdgeList;
	typedef vector<shared_ptr<Face>> FaceList;

	class Vertex
	{
	public:
		Point3d pos;
		bool ifCalNewPos = false;
		Point3d newPos;
		bool isOnBoundary = false;

	public:
		Vertex(Point3d& p) :
			pos(p) {}
	};

	class Halfedge
	{
	public:
		shared_ptr<Vertex> v;
		weak_ptr<Halfedge> twin;
		weak_ptr<Halfedge> next;
		weak_ptr<Halfedge> prev;
		weak_ptr<Edge> e;
		weak_ptr<Face> f;

	public:
		Halfedge() {}
		Halfedge(shared_ptr<Vertex>&  _v) {
			v = _v;
		}
	};

	class Edge
	{
	public:
		Index ID;
		shared_ptr<Halfedge> he1;
		shared_ptr<Halfedge> he2;
		bool ifCalNewPos = false;
		bool ifNew = false;
		Point3d newPos;
		bool isBoundary = false;
		bool ifNeedDelete = false;

	public:
		Edge() {
			static Index id = 0;
			ID = id++;
		}
	};

	class Face
	{
	public:
		shared_ptr<Halfedge> he;
		Point3d normal;
		bool ifNeedDelete = false;

	public:
		Face(){}
		Face(shared_ptr<Halfedge>& _he) {
			he = _he;
		}
	};

	class VertexPair {
	public:
		Index eID;
		Index v[2];

		VertexPair(Index v1, Index v2) {
			eID = -1;
			v[0] = v1;
			v[1] = v2;
		}

		VertexPair(Index eid, Index v1, Index v2) {
			eID = eid;
			v[0] = v1;
			v[1] = v2;
		}

		friend bool operator < (const VertexPair& vp1, const VertexPair& vp2) 
		{
			if (vp1.v[0] < vp2.v[0])
				return true;
			else if (vp1.v[0] == vp2.v[0]) {
				if (vp1.v[1] < vp2.v[1])
					return true;
				else
					return false;
			}
			else
				return false;
		}
	};

	class HalfedgeMesh {
	public:
		Point3d maxCoord, minCoord;
	private:
		VertexList vertices;
		EdgeList edges;
		EdgeList newEdges;
		FaceList faces;
		set<VertexPair> vertexPairSet;
		set<Index> edgeIdxSet;

	public:
		bool Load(string fileName);
		void build(vector<Point3d>& vertexPos, vector<Index>& faceIndex, int vn = 3);
		void LoopSubdivision(int iter);
		//void _loopSubdivision();

		shared_ptr<Edge> findEdge(Index edgeID);
		Point3d MinCoord() const;
		Point3d MaxCoord() const;
		const FaceList Faces() const { return faces; }
	private:
		bool LoadObj(string fileName, vector<Point3d>& vertexPos, vector<Index>& faceIndex);
		void _loopSubdivision();
		double _beta(int n);
		void _updateVertexInterior(shared_ptr<Halfedge>& he);
		bool _splitEdge(shared_ptr<Edge>& eToSplit);
		bool _flipEdge(shared_ptr<Edge>& eToFlip);
		void _linkHEInTriWithFace(shared_ptr<Face>& f, shared_ptr<Halfedge>& he0, shared_ptr<Halfedge>& he1, shared_ptr<Halfedge>& he2);
	};
}

