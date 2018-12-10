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

		// ����
		if (strcmp(type, "v") == 0)
		{
			double x, y, z;
			iss >> x >> y >> z;
			vertexPos.push_back(Point3d(x, y, z));
		}

		// ��
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

void HE::HalfedgeMesh::_loopSubdivision()
{
	shared_ptr<Vertex> v1, v2, vUp, vDown;
	shared_ptr<Vertex> vAnother;
	shared_ptr<Halfedge> tmp;
	Point3d sumOfVAround;
	double beta;
	int numOfTri;

	// ����������λ��
	for (int  i = 0; i < edges.size(); i++) {
		shared_ptr<Edge> e = edges[i];
		// ����Ǳ߽�
		if (e->isBoundary) { 
			// �����е�
			v1 = e->he1->v;
			v2 = e->he1->prev.lock()->v;
			e->newPos = 0.5*(v1->pos + v2->pos);
			e->ifCalNewPos = true;
			// ���㶥��������꣬����newPos
			if (!v1->ifCalNewPos) {
				tmp = e->he1->prev.lock();
				while (tmp->twin.lock() !=nullptr)
					tmp = tmp->twin.lock()->prev.lock();
				vAnother = tmp->v;
				v1->newPos = 0.125*(vAnother->pos + v2->pos) + 0.75*v1->pos;
				v1->ifCalNewPos = true;
			}
			if (!v2->ifCalNewPos) {
				tmp = e->he1->next.lock();
				while (tmp->twin.lock() != nullptr)
					tmp = tmp->twin.lock()->next.lock();
				vAnother = tmp->next.lock()->v;
				v2->newPos = 0.125*(vAnother->pos + v1->pos) + 0.75*v2->pos;
				v2->ifCalNewPos = true;
			}
		}
		// ������Ǳ߽�
		else { 
			// ������е�
			v1 = e->he1->v;
			v2 = e->he2->v;
			vUp = e->he1->prev.lock()->v;
			vDown = e->he2->prev.lock()->v;
			e->newPos = 0.375*(v1->pos + v2->pos) + 0.125*(vUp->pos + vDown->pos);
			e->ifCalNewPos = true;
			// ���㶥��������꣬����newPos
			if (!v1->ifCalNewPos && !v1->isOnBoundary)
				_updateVertexInterior(e->he1);

			if (!v2->ifCalNewPos && !v2->isOnBoundary)
				_updateVertexInterior(e->he2);
		}
	}

	// ��������Ƭ
	EdgeList newEdges;
	FaceList newFaces;
	shared_ptr<Vertex> oldV[3];
	shared_ptr<Vertex> newV[3];
	for (int i = 0; i < faces.size(); i++) {
		shared_ptr<Face> currF = faces[i];
		oldV[0] = currF->he->v;
		oldV[1] = currF->he->next.lock()->v;
		oldV[2] = currF->he->prev.lock()->v;
		newV[0] = make_shared<Vertex>(currF->he->e.lock()->newPos);
		newV[1] = make_shared<Vertex>(currF->he->next.lock()->e.lock()->newPos);
		newV[2] = make_shared<Vertex>(currF->he->prev.lock()->e.lock()->newPos);

	}
}

double HE::HalfedgeMesh::_beta(int n)
{
	double nn = 1.0 / n;
	return nn*(0.625 - pow(0.375 + 0.25*cos(2 * PI*nn), 2));
}

void HE::HalfedgeMesh::_updateVertexInterior(shared_ptr<Halfedge>& he)
{
	int numOfTri = 1;
	Point3d sumOfVAround = he->twin.lock()->v->pos;
	shared_ptr<Halfedge> tmp = he->prev.lock()->twin.lock();
	while (tmp != he)
	{
		numOfTri += 1;
		sumOfVAround += tmp->next.lock()->v->pos;
		tmp = tmp->prev.lock()->twin.lock();
	}
	double beta = _beta(numOfTri);
	he->v->newPos = (1 - numOfTri*beta)*he->v->pos + beta*sumOfVAround;
	he->v->ifCalNewPos = true;
}

void HE::HalfedgeMesh::build(vector<Point3d>& vertexPos, vector<Index>& faceIndex, int vn)
{
	// ����vertex
	//vertices.clear();
	shared_ptr<Vertex> v;
	for (size_t i = 0; i < vertexPos.size(); i++) {
		v = make_shared<Vertex>(vertexPos[i]);
		vertices.push_back(v);
	}

	// ����face
	int faceNum = faceIndex.size() / vn;
	Index  *idxs = new Index[vn];
	shared_ptr<Halfedge> *hes = new shared_ptr<Halfedge>[vn];
	shared_ptr<Face> newf;
	for (size_t i = 0; i < faceNum; i++) {
		// ����halfedge
		for (size_t j = 0; j < vn; j++){
			idxs[j] = i*vn + j;
			hes[j] = make_shared<Halfedge>();
		}
		
		newf = make_shared<Face>();
		newf->he = hes[0];
		faces.push_back(newf);
		// ����halfedge�Ķ��㡢�桢ǰ����
		for (size_t j = 0; j < vn; j++) {
			hes[j]->v = vertices[faceIndex[idxs[j]]];
			hes[j]->f = newf;
			hes[j]->prev = hes[(j + vn - 1) % vn];
			hes[j]->next = hes[(j + 1) % vn];
		}
		// ����halfedge��twin�ߡ�Edge
		set<VertexPair>::iterator iter;
		for (size_t j = 0; j < vn; j++) {
			iter = vertexPairSet.find(VertexPair(faceIndex[idxs[(j + 1) % vn]], faceIndex[idxs[j]]));
			// ����ҵ��˶�Ӧ�ıߣ����Ӧ����Ѿ�����
			if (iter != vertexPairSet.end()) {
				shared_ptr<Edge> edge = findEdge((*iter).eID);
				hes[j]->e = edge;
				hes[j]->twin = edge->he1;
				edge->he2 = hes[j];
				edge->he1->twin = hes[j];
				vertexPairSet.erase(iter); //ɾ�������
			}
			// ���û�ҵ������Ӧ���δ����
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
	else {
		cout << "HalfedgeMesh Build : Open surface  !" << endl;
		set<VertexPair>::iterator iter = vertexPairSet.begin();
		shared_ptr<Edge> e;
		while (iter!= vertexPairSet.end())
		{
			e = findEdge((*iter).eID);
			e->isBoundary = true;
			e->he1->v->isOnBoundary = true;
			e->he1->next.lock()->v->isOnBoundary = true;
		}
	}
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
