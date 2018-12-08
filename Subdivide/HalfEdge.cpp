#include "HalfEdge.h"
using namespace HE;

bool HE::HalfedgeMesh::Load(string fileName)
{
	vector<Point3d> vertexPosList;
	vector<Index> faceIndexList;
	string modelExtensions = fileName.substr(fileName.length() - 3, fileName.length());
	if (strcmp(modelExtensions.c_str(), "obj") == 0) {
		if (LoadObj(fileName, vertexPosList, faceIndexList)) {
			cout << "Load " << fileName << endl;
			build(vertexPosList, faceIndexList, 3);
			minCoord = MinCoord();
			maxCoord = MaxCoord();
		}
		else return false;
	}
	//vector<Point3d>().swap(vertexPosList);
	//vector<Index>().swap(faceIndexList);
	return true;
}

bool HE::HalfedgeMesh::LoadObj(string fileName, vector<Point3d>& vertexPos, vector<Index>& faceIndex)
{
	if (fileName.empty())
		return false;
	ifstream ifs(fileName);
	if (ifs.fail())
		return false;

	char buf[1024], type[1024];
	do
	{
		ifs.getline(buf, 1024);
		istrstream iss(buf);
		iss >> type;

		// 顶点
		if (strcmp(type, "v") == 0)
		{
			double x, y, z;
			iss >> x >> y >> z;
			vertexPos.push_back(Point3d(x, y, z));
		}

		// 面
		else if (strcmp(type, "f") == 0)
		{
			Index index[3];
			iss >> index[0] >> index[1] >> index[2];
			faceIndex.push_back(Index(index[0] - 1));
			faceIndex.push_back(Index(index[1] - 1));
			faceIndex.push_back(Index(index[2] - 1));
		}
	} while (!ifs.eof());
	ifs.close();

	return true;
}

void HE::HalfedgeMesh::build(vector<Point3d>& vertexPos, vector<Index>& faceIndex, int vn)
{
	//加载vertex
	//vertices.clear();
	shared_ptr<Vertex> v;
	for (size_t i = 0; i < vertexPos.size(); i++) {
		v = make_shared<Vertex>(vertexPos[i]);
		vertices.push_back(v);
	}

	//加载face
	int faceNum = faceIndex.size() / vn;
	Index  *idxs = new Index[vn];
	shared_ptr<Halfedge> *hes = new shared_ptr<Halfedge>[vn];
	shared_ptr<Face> newf;
	for (size_t i = 0; i < faceNum; i++) {
		///创建halfedge
		for (size_t j = 0; j < vn; j++){
			idxs[j] = i*vn + j;
			hes[j] = make_shared<Halfedge>();
		}
		
		newf = make_shared<Face>();
		newf->he = hes[0];
		faces.push_back(newf);
		///链接halfedge的顶点、面、前后半边
		for (size_t j = 0; j < vn; j++) {
			hes[j]->v = vertices[faceIndex[idxs[j]]];
			hes[j]->f = newf;
			hes[j]->prev = hes[(j + vn - 1) % vn];
			hes[j]->next = hes[(j + 1) % vn];
		}
		///链接halfedge的twin边、Edge
		set<VertexPair>::iterator iter;
		for (size_t j = 0; j < vn; j++) {
			iter = vertexPairSet.find(VertexPair(faceIndex[idxs[(j + 1) % vn]], faceIndex[idxs[j]]));
			///如果找到了对应的边，则对应半边已经加载
			if (iter != vertexPairSet.end()) {
				shared_ptr<Edge> edge = findEdge((*iter).eID);
				hes[j]->e = edge;
				hes[j]->twin = edge->he1;
				edge->he2 = hes[j];
				edge->he1->twin = hes[j];
				vertexPairSet.erase(iter);///删除顶点对
			}
			///如果没找到，则对应半边未加载
			else {
				shared_ptr<Edge> edge = make_shared<Edge>();
				hes[j]->e = edge;
				edge->he1 = hes[j];
				edges.push_back(edge);
				vertexPairSet.insert(VertexPair(edge->ID, faceIndex[idxs[j]], faceIndex[idxs[(j + 1) % vn]]));
			}
		}
	}
	delete[] idxs;
	delete[] hes;

	if (vertexPairSet.size() == 0) 
		cout << "HalfedgeMesh Build : Close surface !" << endl;
	else
		cout << "HalfedgeMesh Build : Open surface  !" << endl;
}

shared_ptr<Edge> HE::HalfedgeMesh::findEdge(Index edgeID)
{
	for (size_t i = 0; i < edges.size(); i++) {
		if (edges[i]->ID == edgeID)
			return edges[i];
	}
	return nullptr;
}

Point3d HE::HalfedgeMesh::MinCoord() const
{
	Point3d minCoord(1e20);
	for (size_t i = 0; i < vertices.size(); i++)
		minCoord = minCoord.Min(vertices[i]->pos);
	return minCoord;
}

Point3d HE::HalfedgeMesh::MaxCoord() const
{
	Point3d maxCoord(-1e20);
	for (size_t i = 0; i < vertices.size(); i++)
		maxCoord = maxCoord.Max(vertices[i]->pos);
	return maxCoord;
}
