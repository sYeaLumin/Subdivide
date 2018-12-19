#pragma once

#include <iostream>
#include <string>
#include <GL/freeglut.h>
#include "TrackBall.h"
#include "HalfEdge.h"

using namespace std;
using namespace HE;

void InitGL();
void InitMenu();
void ScreenToNCC(int x, int y, float & nccX, float & nccY);
void SetBoundaryBox(const Point3d & bmin, const Point3d & bmax);
void LoadMesh(HalfedgeMesh & mesh, string & modelName);

HalfedgeMesh testMesh;
int main(int argc, char *argv[]) {

	//string name = "obj\\spoon.obj";
	string name = "obj\\bunny.obj";
	LoadMesh(testMesh, name);
	cout << "Face number : " << testMesh.Faces().size() << endl;
	testMesh.LoopSubdivision(1);
	cout << "Face number : " << testMesh.Faces().size() << endl;

	glutInit(&argc, argv);
	InitGL();
	InitMenu();

	glutMainLoop();
	system("pause");
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

/**********************************************************************************/
/****************************** OpenGL Part **************************************/
/**********************************************************************************/
int mainMenu, displayMenu;						// glut menu handlers
int winWidth = 1400, winHeight = 800;
int between = 35;
double winAspect;
string winName = "Test";

double zNear = 1.0, zFar = 100.0;				// clipping
double g_fov = 45.0;
Point3d g_center;
double g_sdepth, sdepth = 10, lastNccX = 0.0, lastNccY = 0.0;
double zDelta;									// zoom in, zoom out
int viewport[4];
bool isLeftDown = false, isRightDown = false, isMidDown = false;
TrackBall trackball;

void MenuCallback(int value);
void DisplayFunc();
void KeyboardFunc(unsigned char ch, int x, int y);
void ReshapeFunc(int width, int height);
void MotionFunc(int x, int y);
void MouseWheelFunc(int button, int dir, int x, int y);
void MouseFunc(int button, int state, int x, int y);
void DrawFlatShaded(HalfedgeMesh & mesh);

// GLUT menu callback function
void MenuCallback(int value)
{
	vector<int> newFList;
	switch (value)
	{
	case 99:
		exit(0);
		break;
	default:
		break;
	}
	glutPostRedisplay();
}


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

void DrawMeshWithDifferentColor(HalfedgeMesh & mesh)
{
	float rotation[16];
	trackball.m_rotation.GetMatrix(rotation);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(g_fov, winAspect, zNear, zFar);
	//glOrtho(-2.0, 2.0, -2.0, 2.0, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(xtrans, ytrans, -sdepth);
	glMultMatrixf(rotation);
	glTranslated(-g_center[0], -g_center[1], -g_center[2]);

	FaceList fList = mesh.Faces();
	glShadeModel(GL_FLAT);
	glDisable(GL_LIGHTING);
	glBegin(GL_TRIANGLES);
	for (size_t i = 0; i<fList.size(); i++) {
		shared_ptr<Face> f = fList[i];
		const Point3d & pos1 = f->he->v->pos;
		const Point3d & pos2 = f->he->next.lock()->v->pos;
		const Point3d & pos3 = f->he->next.lock()->next.lock()->v->pos;

		int r = (i & 0x000000FF) >> 0;
		int g = (i & 0x0000FF00) >> 8;
		int b = (i & 0x00FF0000) >> 16;
		glColor3f(r / 255.0f, g / 255.0f, b / 255.0f);

		Point3d normal = (pos2 - pos1).Cross(pos3 - pos1);
		normal /= normal.L2Norm();
		glNormal3dv(normal.ToArray());
		glVertex3dv(pos1.ToArray());
		glVertex3dv(pos2.ToArray());
		glVertex3dv(pos3.ToArray());
	}
	glEnd();

	glFlush();
	glFinish();
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
	//glutSpecialFunc(SpecialKeyFcn);
	glutMouseFunc(MouseFunc);
	glutMotionFunc(MotionFunc);
	glutMouseWheelFunc(MouseWheelFunc);
}

int modelMune;
void InitMenu()
{
	modelMune = glutCreateMenu(MenuCallback);
	//glutAddMenuEntry("Mdoel 1", MODEL_ONE);
	mainMenu = glutCreateMenu(MenuCallback);
	//glutAddMenuEntry("Display", FLATSHADED);
	glutAddSubMenu("Change Model", modelMune);
	glutAddMenuEntry("Exit", 99);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
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

// GLUT reshape callback function
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
	
	DrawFlatShaded(testMesh);

	glPopMatrix();

	glutSwapBuffers();
}

void KeyboardFunc(unsigned char ch, int x, int y)
{
	int tmp;
	switch (ch)
	{
	case 'z':case 'Z':
		break;
	case 'c':case 'C':
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
