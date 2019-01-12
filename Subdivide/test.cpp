#pragma once

#include <iostream>
#include <string>
#include <time.h>
#include <GL/freeglut.h>
#include "TrackBall.h"
#include "HalfEdge.h"
#define MAX_CHAR        128

using namespace std;
using namespace HE;

void help() {
	cout << "Help:" << endl;
	cout << "   > Subdivide.exe [objPath] [subdivideTimes]" << endl;
	cout << "   > ¡ª¡ª [objPath] \t the path of the obj file" << endl;
	cout << "   > ¡ª¡ª [subdivideTimes]  the times want to subdivide" << endl;
	cout << "   cmd example > Subdivide.exe test1.obj" << endl;
	cout << "   cmd example > Subdivide.exe test2.obj 3" << endl;
	cout << "   You could use 'D' to subdivde the mesh, 'S' to save the new mesh as obj files.\n" << endl;
}
void InitGL();
void ScreenToNCC(int x, int y, float & nccX, float & nccY);
void SetBoundaryBox(const Point3d & bmin, const Point3d & bmax);
void LoadMesh(HalfedgeMesh & mesh, string & modelName);

HalfedgeMesh testMesh;
string meshName = "obj/spoon.obj";
string newObjName;
int subdivdeTimes = 0;
double duration;
clock_t start, finish;
double usingTime = 0;
int main(int argc, char *argv[]) {
	if (argc == 2) {
		meshName = argv[1];
	}
	else if (argc == 1) {}
	else if (argc == 3) {
		meshName = argv[1];
		subdivdeTimes = atoi(argv[2]);
	}
	else {
		help();
		return 0;
	}
	help();

	LoadMesh(testMesh, meshName);
	cout << "Original Mesh : " << testMesh.Faces().size() << " faces." << endl;
	if (subdivdeTimes > 0) {
		start = clock();
		testMesh.LoopSubdivision(subdivdeTimes);
		finish = clock();
		usingTime += (double)(finish - start) / CLOCKS_PER_SEC;
		cout << "After " << subdivdeTimes << " subdivition : "
			<< testMesh.Faces().size() << " faces   "
			<< "using " << (double)(finish - start) / CLOCKS_PER_SEC << " seconds" << endl;
	}

	glutInit(&argc, argv);
	InitGL();
	glutMainLoop();
	return 0;
}

// Load mesh file
double xtrans = 0, ytrans = 0;
void LoadMesh(HalfedgeMesh & mesh, string & modelName)
{
	cout << "LoadMesh : " << modelName << endl;
	mesh.Load(modelName);
	SetBoundaryBox(mesh.minCoord, mesh.maxCoord);
	xtrans = ytrans = 0.0;
}

// Print words
void drawString(const char * str)
{
	static bool isFirstCall = true;
	static GLuint lists;

	if (isFirstCall) {
		isFirstCall = false;
		lists = glGenLists(MAX_CHAR);
		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);
	}
	for (; *str != '\n'; ++str)
		glCallList(lists + *str);
}


/**********************************************************************************/
/****************************** OpenGL Part **************************************/
/**********************************************************************************/
int mainMenu, displayMenu;						// glut menu handlers
int winWidth = 800, winHeight = 600;
int between = 35;
double winAspect;
string winName = "Subdivide Test";

double zNear = 1.0, zFar = 100.0;		
double g_fov = 45.0;
Point3d g_center;
double g_sdepth, sdepth = 10, lastNccX = 0.0, lastNccY = 0.0;
double zDelta;	
int viewport[4];
bool isLeftDown = false, isRightDown = false, isMidDown = false;
TrackBall trackball;

void DisplayFunc();
void KeyboardFunc(unsigned char ch, int x, int y);
void ReshapeFunc(int width, int height);
void MotionFunc(int x, int y);
void MouseWheelFunc(int button, int dir, int x, int y);
void MouseFunc(int button, int state, int x, int y);
void DrawFlatShaded(HalfedgeMesh & mesh);

void DrawFlatShaded(HalfedgeMesh & mesh)
{
	FaceList fList = mesh.Faces();
	glShadeModel(GL_FLAT);
	glEnable(GL_LIGHTING);
	glColor3f(0.4f, 0.4f, 0.4f);
	glBegin(GL_TRIANGLES);
	for (size_t i = 0; i<fList.size(); i++) {
		shared_ptr<Face> f = fList[i];
		const Point3d & pos1 = f->he->v->pos;
		const Point3d & pos2 = f->he->next.lock()->v->pos;
		const Point3d & pos3 = f->he->next.lock()->next.lock()->v->pos;
		Point3d normal = (pos2 - pos1).Cross(pos3 - pos1);
		normal /= normal.L2Norm();
		glColor3d(0.4f, 0.4f, 0.4f);
		glNormal3dv(normal.ToArray());
		glVertex3dv(pos1.ToArray());
		glVertex3dv(pos2.ToArray());
		glVertex3dv(pos3.ToArray());
	}
	glEnd();
	glDisable(GL_LIGHTING);
}

void InitGL()
{
	GLfloat light0Position[] = { 0, 1, 0, 1.0 };

	// initialize GLUT stuffs
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(winWidth, winHeight);
	glutCreateWindow(winName.c_str());

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glPolygonOffset(1.0, 1.0);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glLightfv(GL_LIGHT0, GL_POSITION, light0Position);
	glEnable(GL_LIGHT0);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glutReshapeFunc(ReshapeFunc);
	glutDisplayFunc(DisplayFunc);
	glutKeyboardFunc(KeyboardFunc);
	glutMouseFunc(MouseFunc);
	glutMotionFunc(MotionFunc);
	glutMouseWheelFunc(MouseWheelFunc);
}

void ScreenToNCC(int x, int y, float & nccX, float & nccY)
{
	nccX = (2.0f*x - viewport[2]) / viewport[2];
	nccY = (viewport[3] - 2.0f*y) / viewport[3];
}

void SetBoundaryBox(const Point3d & bmin, const Point3d & bmax)
{
	double radius = bmax.Distance(bmin);
	g_center = 0.5 * (bmin + bmax);
	zNear = 0.2 * radius / sin(0.5 * g_fov * PI / 180.0);
	zFar = zNear + 2.0 * radius;
	g_sdepth = zNear + radius;
	zNear *= 0.01;
	zFar *= 10;
	sdepth = g_sdepth;
	zDelta = sdepth / 40;
}

void ReshapeFunc(int width, int height)
{
	winWidth = width;
	winHeight = height;
	winAspect = (double)width / (double)height;
	glViewport(0, 0, width, height);
	glGetIntegerv(GL_VIEWPORT, viewport);
	glutPostRedisplay();
}


void DisplayFunc()
{
	float rotation[16];
	trackball.m_rotation.GetMatrix(rotation);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(g_fov, winAspect, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	glTranslated(xtrans, ytrans, -sdepth);
	glMultMatrixf(rotation);
	glTranslated(-g_center[0], -g_center[1], -g_center[2]);
	// Draw
	DrawFlatShaded(testMesh);
	glPopMatrix();

	// ÎÄ×Ö
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, winWidth, 0, winHeight);
	glEnable(GL_DEPTH_TEST);
	glColor3f(1.0f, 0.0f, 0.0f);
	glRasterPos2f(10.0f, winHeight - 20.0f);
	drawString("Use keyboard :\n");
	glRasterPos2f(10.0f, winHeight - 40.0f);
	drawString("KEY 'D'  to subdivide mesh\n");
	glRasterPos2f(10.0f, winHeight - 60.0f);
	drawString("KEY 'S'  to save new mesh\n");

	string faceNum = "Face number : ";
	faceNum += to_string(testMesh.Faces().size());
	faceNum += "\n";
	glRasterPos2f(10.0f, winHeight - 100.0f);
	drawString(faceNum.c_str());

	string subTimes = "Subdivition Time : ";
	subTimes += to_string(subdivdeTimes);
	subTimes += "\n";
	glRasterPos2f(10.0f, winHeight - 120.0f);
	drawString(subTimes.c_str());

	char str[40];
	sprintf_s(str, "Total Cost time : %.3f seconds\n", usingTime);
	glRasterPos2f(10.0f, winHeight - 140.0f);
	drawString(str);

	glutSwapBuffers();
}

void KeyboardFunc(unsigned char ch, int x, int y)
{
	int tmp;
	switch (ch)
	{
	case 's':case 'S':
		cout << "Save...";
		newObjName = meshName.substr(0, meshName.find_last_of('.'))
			+ to_string(subdivdeTimes) + ".obj";
		testMesh.saveObj(newObjName);
		cout << "Done at " << newObjName<< endl;
		break;
	case 'd':case 'D':
		start = clock();
		testMesh.LoopSubdivision(1);
		finish = clock();
		subdivdeTimes++;
		usingTime += (double)(finish - start) / CLOCKS_PER_SEC;
		cout << "After " << subdivdeTimes << " subdivition : "
			<< testMesh.Faces().size() << " faces   "
			<< "using " << (double)(finish - start) / CLOCKS_PER_SEC << " seconds" << endl;
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

vector<int> pos;
void MouseFunc(int button, int state, int x, int y)
{
	float nccX, nccY;
	if (button == GLUT_LEFT_BUTTON)
	{
		switch (state)
		{
		case GLUT_DOWN:
			isLeftDown = true;
			ScreenToNCC(x, y, nccX, nccY);
			trackball.Push(nccX, nccY);
			break;
		case GLUT_UP:
			ScreenToNCC(x, y, nccX, nccY);
			isLeftDown = false;
			break;
		}
	}
	if (button == GLUT_MIDDLE_BUTTON)
	{
		switch (state)
		{
		case GLUT_DOWN:
			isMidDown = true;
			ScreenToNCC(x, y, nccX, nccY);
			lastNccX = nccX;
			lastNccY = nccY;
			break;
		case GLUT_UP:
			isMidDown = false;
			break;
		}
	}
}

// GLUT mouse motion callback function
void MotionFunc(int x, int y)
{
	if (isLeftDown)
	{
		float nccX, nccY;
		ScreenToNCC(x, y, nccX, nccY);
		trackball.Move(nccX, nccY);
		glutPostRedisplay();
	}
	else if (isMidDown)
	{
		float nccX, nccY;
		ScreenToNCC(x, y, nccX, nccY);

		xtrans += 0.2*(nccX - lastNccX)*winAspect*sdepth / zNear;
		ytrans += 0.2*(nccY - lastNccY)*sdepth / zNear;
		lastNccX = nccX;
		lastNccY = nccY;
		glutPostRedisplay();
	}
}

void MouseWheelFunc(int button, int dir, int x, int y)
{
	if (sdepth > zNear  && dir>0)
	{
		sdepth -= zDelta;
	}
	if (sdepth < zFar && dir < 0)
	{
		sdepth += zDelta;
	}
	glutPostRedisplay();
}
