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

void HE::HalfedgeMesh::_loopSubdivision()
{
	shared_ptr<Vertex> v1, v2, vUp, vDown;
	shared_ptr<Vertex> vAnother;
	shared_ptr<Halfedge> tmp;
	Point3d sumOfVAround;
	double beta;
	int numOfTri;

	// 计算所有新位置
	for (int  i = 0; i < edges.size(); i++) {
		shared_ptr<Edge> e = edges[i];
		// 如果是边界
		if (e->isBoundary) { 
			// 计算中点
			v1 = e->he1->v;
			v2 = e->he1->prev.lock()->v;
			e->newPos = 0.5*(v1->pos + v2->pos);
			e->ifCalNewPos = true;
			// 计算顶点更新坐标，存入newPos
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
		// 如果不是边界
		else { 
			// 计算边中点
			v1 = e->he1->v;
			v2 = e->he2->v;
			vUp = e->he1->prev.lock()->v;
			vDown = e->he2->prev.lock()->v;
			e->newPos = 0.375*(v1->pos + v2->pos) + 0.125*(vUp->pos + vDown->pos);
			e->ifCalNewPos = true;
			// 计算顶点更新坐标，存入newPos
			if (!v1->ifCalNewPos && !v1->isOnBoundary)
				_updateVertexInterior(e->he1);

			if (!v2->ifCalNewPos && !v2->isOnBoundary)
				_updateVertexInterior(e->he2);
		}
	}

	// split 
	cout << "_loopSubdivision Old edges number : " << edges.size() << endl;
	int oldEdgeNum = edges.size();
	for (int i = 0; i < oldEdgeNum; i++) {
		if (!_splitEdge(edges[i]))
			cout << "Split Edge " << i << " failed!" << endl;
	}
	// 删除旧边
	edges.erase(edges.begin(), edges.begin() + oldEdgeNum);
	cout << "_loopSubdivision New edges number : " << edges.size() << endl;
	cout << "_loopSubdivision Face number : " << faces.size() << endl;
	// 删除旧面
	vector<shared_ptr<Face>>::iterator iter = faces.begin();
	while (iter!=faces.end())
	{
		if ((*iter)->ifNeedDelete == true)
			iter = faces.erase(iter);
		else
			iter++;
	}
	// flip
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

bool HE::HalfedgeMesh::_splitEdge(shared_ptr<Edge>& eToSplit)
{
	if (!eToSplit->ifCalNewPos)
		return false;
	shared_ptr<Vertex>  newV;
	shared_ptr<Face> oldF[2];
	shared_ptr<Face> newF[4];
	shared_ptr<Halfedge> oldHE[4];
	shared_ptr<Halfedge> newHE[8];
	shared_ptr<Edge> newE[4];

	// 如果是边界
	if (eToSplit->isBoundary) {
		newV = make_shared<Vertex>(eToSplit->newPos);
		oldHE[0] = eToSplit->he1->next.lock();
		oldHE[1] = eToSplit->he1->prev.lock();
		// RightTri
		newHE[0] = make_shared<Halfedge>(oldHE[1]->v);
		newHE[1] = make_shared<Halfedge>(newV);
		newF[0] = make_shared<Face>();
		_linkHEInTriWithFace(newF[0], newHE[0], newHE[1], oldHE[0]);
		// LeftTri
		newHE[2] = make_shared<Halfedge>(eToSplit->he1->v);
		newHE[3] = make_shared<Halfedge>(newV);
		newF[1] = make_shared<Face>();
		_linkHEInTriWithFace(newF[1], newHE[2], newHE[3], oldHE[1]);
		// 插入的新边
		newE[0] = make_shared<Edge>();
		newE[0]->he1 = newHE[0];
		newE[0]->he2 = newHE[3];
		newHE[0]->e = newE[0];
		newHE[3]->e = newE[0];
		newHE[0]->twin = newHE[3];
		newHE[3]->twin = newHE[0];
		// 断开形成的两条新边
		newE[1] = make_shared<Edge>();
		newE[1]->he1 = newHE[1];
		newE[1]->isBoundary = true;
		newHE[1]->e = newE[1];
		newE[2] = make_shared<Edge>();
		newE[2]->he1 = newHE[2];
		newE[2]->isBoundary = true;
		newHE[2]->e = newE[2];
		// 断开原Face和Edge的相关link，确保不会再访问
		eToSplit->he1->f.lock()->he = nullptr;
		eToSplit->he1->f.lock()->ifNeedDelete = true;
		eToSplit->he1 = nullptr;
		eToSplit->ifNeedDelete = true;
		// 加入新Face和Edge
		for (size_t i = 0; i < 3; i++)
			edges.push_back(newE[i]);
		for (size_t i = 0; i < 2; i++)
			faces.push_back(newF[i]);
	}
	// 如果不是边界
	else {
		newV = make_shared<Vertex>(eToSplit->newPos);
		oldHE[0] = eToSplit->he2->next.lock();
		oldHE[1] = eToSplit->he2->prev.lock();
		oldHE[2] = eToSplit->he1->next.lock();
		oldHE[3] = eToSplit->he1->prev.lock();
		for (size_t i = 0; i < 4; i++) {
			newHE[2 * i] = make_shared<Halfedge>(oldHE[(i + 1) % 4]->v);
			newHE[2 * i + 1] = make_shared<Halfedge>(newV);
			newF[i] = make_shared<Face>();
			_linkHEInTriWithFace(newF[i], newHE[2 * i], newHE[2 * i + 1], oldHE[i]);
		}
		for (size_t i = 0; i < 4; i++) {
			newE[i] = make_shared<Edge>();
			newE[i]->he1 = newHE[2 * i];
			newE[i]->he2 = newHE[(2 * i + 3) % 8];
			newHE[2 * i]->e = newE[i];
			newHE[(2 * i + 3) % 8]->e = newE[i];
			newHE[2 * i]->twin = newHE[(2 * i + 3) % 8];
			newHE[(2 * i + 3) % 8]->twin = newHE[2 * i];
		}
		// 断开原Face和Edge的相关link，确保不会再访问
		eToSplit->he1->f.lock()->he = nullptr;
		eToSplit->he2->f.lock()->he = nullptr;
		eToSplit->he1->f.lock()->ifNeedDelete = true;
		eToSplit->he1 = nullptr;
		eToSplit->he2 = nullptr;
		eToSplit->ifNeedDelete = true;
		// 加入新Face和Edge
		for (size_t i = 0; i < 4; i++) {
			edges.push_back(newE[i]);
			faces.push_back(newF[i]);
		}
	}

	return true;
}

void HE::HalfedgeMesh::_linkHEInTriWithFace(shared_ptr<Face>& f, shared_ptr<Halfedge>& he0, shared_ptr<Halfedge>& he1, shared_ptr<Halfedge>& he2)
{
	he0->next = he1;
	he1->next = he2;
	he2->next = he0;
	he0->prev = he2;
	he1->prev = he0;
	he2->prev = he1;
	he0->f = f;
	he1->f = f;
	he2->f = f;
	f->he = he0;
}

void HE::HalfedgeMesh::build(vector<Point3d>& vertexPos, vector<Index>& faceIndex, int vn)
{
	// 加载vertex
	//vertices.clear();
	shared_ptr<Vertex> v;
	for (size_t i = 0; i < vertexPos.size(); i++) {
		v = make_shared<Vertex>(vertexPos[i]);
		vertices.push_back(v);
	}

	// 加载face
	int faceNum = faceIndex.size() / vn;
	Index  *idxs = new Index[vn];
	shared_ptr<Halfedge> *hes = new shared_ptr<Halfedge>[vn];
	shared_ptr<Face> newf;
	for (size_t i = 0; i < faceNum; i++) {
		// 创建halfedge
		for (size_t j = 0; j < vn; j++){
			idxs[j] = i*vn + j;
			hes[j] = make_shared<Halfedge>();
		}
		
		newf = make_shared<Face>();
		newf->he = hes[0];
		faces.push_back(newf);
		// 链接halfedge的顶点、面、前后半边
		for (size_t j = 0; j < vn; j++) {
			hes[j]->v = vertices[faceIndex[idxs[j]]];
			hes[j]->f = newf;
			hes[j]->prev = hes[(j + vn - 1) % vn];
			hes[j]->next = hes[(j + 1) % vn];
		}
		// 链接halfedge的twin边、Edge
		set<VertexPair>::iterator iter;
		for (size_t j = 0; j < vn; j++) {
			iter = vertexPairSet.find(VertexPair(faceIndex[idxs[(j + 1) % vn]], faceIndex[idxs[j]]));
			// 如果找到了对应的边，则对应半边已经加载
			if (iter != vertexPairSet.end()) {
				shared_ptr<Edge> edge = findEdge((*iter).eID);
				hes[j]->e = edge;
				hes[j]->twin = edge->he1;
				edge->he2 = hes[j];
				edge->he1->twin = hes[j];
				vertexPairSet.erase(iter); //删除顶点对
			}
			// 如果没找到，则对应半边未加载
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
		cout << "Boudary Number : " << vertexPairSet .size() << endl;
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
