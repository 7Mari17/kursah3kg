#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}

















void POL(double z1, double x, double y, double z, int numpoly)
{
	// r = sqrt((13 - 9) ^ 2 + (4 - 0) ^ 2);
		//double x = r * cos(alfa);
		//double y = r * sin(alfa);

	x = 11; y = 2; z = 0;

	double k = 0, r = sqrt(32) / 2, kr[2][100], x1, y1;
	double alfa = 0;
	int n = 0, i;
	double pi = 3.1415926535897932384;
	for (i = 0; i < 60; i++)
	{
		alfa = (pi * 12 * i) / 360 + pi / 2;
		x1 = x + r * cos(alfa);
		y1 = y + r * sin(alfa);
		if (y1 <= 4 && x1 >= 9)
		{
			kr[0][n] = x1;
			kr[1][n] = y1;
			n++;
		}
	}
	glColor3f(0.4f, 0.2f, 0.3f);

	for (i = 1; i < n; i++)
	{





	
		glBegin(GL_TRIANGLES);
		 glVertex3d(kr[0][i - 1], kr[1][i - 1], z);
		 glVertex3d(kr[0][i], kr[1][i], z);                     //по оси z полукруг
		 glVertex3d(11, 2, z);
		glEnd();
	}
	glBegin(GL_TRIANGLES);
	glVertex3d(kr[0][0], kr[1][0], z);
	glVertex3d(9, 0, z);
	glVertex3d(11, 2, z);                                  //пустоны треугольники
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3d(kr[0][n - 1], kr[1][n - 1], z);
	glVertex3d(13, 4, z);
	glVertex3d(11, 2, z);
	glEnd();



	glColor4d(0.1, 0.0, 0.266, 0.711111);  // доп прозрачность крышки
	//glColor3f(0.62f, 0.62f, 0.61f);
	for (i = 1; i < n; i++)
	{
		glBegin(GL_TRIANGLES);                                      //по оси z1 полукруг
		glVertex3d(kr[0][i - 1], kr[1][i - 1], z1);
		glVertex3d(kr[0][i], kr[1][i], z1);
		glVertex3d(11, 2, z1);
		glEnd();
	}

	glColor4d(0.1, 0.0, 0.266, 0.711111);
	//glColor3f(0.2f, 0.22f, 0.111f);
	glBegin(GL_TRIANGLES);
	glVertex3d(kr[0][0], kr[1][0], z1);
	glVertex3d(9, 0, z1);
	glVertex3d(11, 2, z1);                                  //пустоны треугольники (соединение)
	glEnd();




	glBegin(GL_TRIANGLES);
	glVertex3d(kr[0][n - 1], kr[1][n - 1], z1);
	glVertex3d(13, 4, z1);
	glVertex3d(11, 2, z1);
	glEnd();


	glColor3f(0.7f, 0.72f, 0.111f);


	for (i = 1; i < n; i++)
	{
		
		glBegin(GL_QUADS);
		 glVertex3d(kr[0][i - 1], kr[1][i - 1], z);
		 glVertex3d(kr[0][i], kr[1][i], z);
		 glVertex3d(kr[0][i], kr[1][i], z1);
		 glVertex3d(kr[0][i - 1], kr[1][i - 1], z1);
		glEnd();
	}
	glColor3f(0.83f, 0.822f, 0.6f);                      //соединение дуги и корабля недостающие кусочки
	glBegin(GL_QUADS);
	glVertex3d(kr[0][0], kr[1][0], z);
	glVertex3d(9, 0, z);
	glVertex3d(9, 0, z1);
	glVertex3d(kr[0][0], kr[1][0], z1);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3d(kr[0][n - 1], kr[1][n - 1], z);
	glVertex3d(13, 4, z);
	glVertex3d(13, 4, z1);
	glVertex3d(kr[0][n - 1], kr[1][n - 1], z1);
	glEnd();


}

void F(int z)
{






	glBegin(GL_TRIANGLES);
	



	//glColor3d(0.75, 0.75, 0.75);
	glVertex3d(0, 0, z);
	//glColor3d(0.11, 0.11, 0.75);
 glVertex3d(9, 0, z);
	//glColor3d(0.40, 0.456, 0.78785);
	 glVertex3d(2, 3, z);
	glEnd();


	glBegin(GL_TRIANGLES);
	//glColor3d(0.11, 0.11, 0.75);
	 glVertex3d(3, 4, z);
	//glColor3d(0.40, 0.456, 0.78785);
 glVertex3d(-1, 4, z);
	//glColor3d(0.75, 0.75, 0.75);
	 glVertex3d(2, 3, z);

	//	glColor3d(0.11, 0.11, 0.75);
	 glVertex3d(3, 4, z);
	//	glColor3d(0.11, 0.11, 0.75);
	 glVertex3d(2, 3, z);
	//	glColor3d(0.40, 0.456, 0.78785);
	 glVertex3d(13, 4, z);

	//	glColor3d(0.44445, 0.456, 0.78785);
	 glVertex3d(9, 0, z);
	//	glColor3d(0.4232, 0.456, 0.7673225);
	 glVertex3d(2, 3, z);
	//	glColor3d(0.40, 0.43323, 0.732335);
	 glVertex3d(13, 4, z);

	glEnd();






	//	glColor3f(0.6f, 0.0f, 0.0f);
	glBegin(GL_QUADS);

	glVertex3d(3, 4, z);
	glVertex3d(5, 4, z);
	glVertex3d(5, 10, z);
	glVertex3d(3, 10, z);

	glVertex3d(7, 8, z);
	glVertex3d(9, 8, z);
	glVertex3d(9, 4, z);
	glVertex3d(7, 4, z);


	glEnd();
}






void F2(int a, int a1)
{


	glColor3d(0.55, 0.75, 0.75);
	glBegin(GL_QUADS);

	glVertex3d(2, 3, a1);

	glVertex3d(2, 3, a);
	
	glVertex3d(0, 0, a);
	
	glVertex3d(0, 0, a1);

	
	glVertex3d(2, 3, a1);

	glVertex3d(2, 3, a);
	
	glVertex3d(-1, 4, a);
	
	glVertex3d(-1, 4, a1);

	/*glVertex3d(-1, 4, a1);     //от носа до кормы
	glVertex3d(-1, 4, a);
	glVertex3d(13, 4, a);
	glVertex3d(13, 4, a1);
	*/

 glVertex3d(13, 4, a);
	 glVertex3d(13, 4, a1);
	 glVertex3d(9, 0, a1);
 glVertex3d(9, 0, a);





	glVertex3d(-1, 4, a);
	 glVertex3d(-1, 4, a1);
	 glVertex3d(3, 4, a1);
	 glVertex3d(3, 4, a);


	/*
	glTexCoord2d(1, 1);
	glTexCoord2d(1, 0);
	glTexCoord2d(0, 0);
	 glTexCoord2d(0, 1);
*/



	 glVertex3d(7, 4, a);
	 glVertex3d(7, 4, a1);
	 glVertex3d(5, 4, a1);
	 glVertex3d(5, 4, a);

	 glVertex3d(13, 4, a);
	 glVertex3d(13, 4, a1);
	glVertex3d(9, 4, a1);
	 glVertex3d(9, 4, a);
	glEnd();


	glColor3d(0.55, 0.75, 0.75);
	glBegin(GL_QUADS);
	 glVertex3d(0, 0, a);
	 glVertex3d(0, 0, a1);
	 glVertex3d(9, 0, a1);
	 glVertex3d(9, 0, a);

	glColor3f(0.6f, 0.0f, 0.0f);
	 glVertex3d(3, 4, a);
	glVertex3d(3, 4, a1);
	 glVertex3d(3, 10, a1);
	 glVertex3d(3, 10, a);

	glVertex3d(5, 10, a);
	 glVertex3d(5, 10, a1);
	 glVertex3d(3, 10, a1);
	 glVertex3d(3, 10, a);

	 glVertex3d(5, 10, a);
	 glVertex3d(5, 10, a1);
	 glVertex3d(5, 4, a1);
	 glVertex3d(5, 4, a);

	glVertex3d(7, 8, a);
	glVertex3d(7, 8, a1);
	 glVertex3d(7, 4, a1);
	 glVertex3d(7, 4, a);

	glVertex3d(7, 8, a);
	 glVertex3d(7, 8, a1);
	 glVertex3d(9, 8, a1);
	 glVertex3d(9, 8, a);

	glVertex3d(9, 4, a);
	 glVertex3d(9, 4, a1);
	 glVertex3d(9, 8, a1);
	 glVertex3d(9, 8, a);




	glEnd();
}














void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  


	

	glColor3d(1, 0, 0);
	glNormal3b(0, 0, -1);
	int z = 0;   // поменять ширину 
	int a = z;

	F(z);

int z1 = 1;       // поменять ширину    
	int a1 = z1;

F2(a, a1);
	glColor4d(0.1, 0.0, 0.266, 0.711111);              //прозрачность крышки
	glNormal3b(0, 0, 1);
	
	z = z1;
	F(z);




	
	


	double k = 0.6;
	double alfa = 180 / k;
	double r = sqrt(((13 - 9) ^ 2) + ((4 - 0) ^ 2));
	//double r = sqrt(32);
	//r = 5, 667;
	POL(a1, -2, 11, 0, 100);   // double r, double x, double y, double z, int numpoly( взятые точки )































   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}