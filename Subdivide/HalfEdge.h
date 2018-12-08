#pragma once
#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <set>
using namespace std;

typedef size_t Index;

namespace HE {
	class Point;
	class Vertex;
	class Halfedge;
	class Edge;
	class Face;

	class Point 
	{
	public:
		double p[3];
	};

	class Vertex 
	{
	public:
		Point pos;
		weak_ptr<Halfedge> he;
		bool isNew;
		Point newPos;

	public:
		Vertex(Point& p) :
			pos(p) {}
	};

	class Halfedge
	{
	public:
		weak_ptr<Halfedge> twin;
		shared_ptr<Halfedge> next;
		weak_ptr<Halfedge> prev;
		shared_ptr<Vertex> v;
		weak_ptr<Edge> e;
		weak_ptr<Face> f;
	};

	class Edge 
	{
	public:
		Index ID;
		shared_ptr<Halfedge> he1;
		shared_ptr<Halfedge> he2;
		bool isNew;
		Point newPos;

	public:
		Edge() {
			static Index id = 0;
			ID = id++;
			isNew = false;
		}
	};

	class Face
	{
	public:
		shared_ptr<Halfedge> he;
		Point normal;
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
		vector<shared_ptr<Vertex>> vertices;
		vector<shared_ptr<Edge>> edges;
		vector<shared_ptr<Face>> faces;
	private:
		set<VertexPair> vertexPairSet;

	public:
		void build(vector<Point>& vertexPos, vector<Index>& faceIndex, int vn = 3);
		shared_ptr<Edge> findEdge(Index edgeID);
	};
}

