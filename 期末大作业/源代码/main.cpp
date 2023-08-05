#include "Angel.h"
#include "TriMesh.h"
#include "Camera.h"
#include "MeshPainter.h"

#include <vector>
#include <string>
using namespace std;
int WIDTH = 1200;
int HEIGHT = 1000;

int mainWindow;

Camera *camera = new Camera();
Light *light = new Light();
MeshPainter *painter = new MeshPainter();
MeshPainter *painter2 = new MeshPainter();

TriMesh *plane = new TriMesh(); // 投影平面
bool firstMouseMove = true;
double lastX, lastY;
glm::vec4 lasteyepos;
bool isMove = false;
float moveDelta = 0.0001;
bool mousecontrol = true, slmove = true;
float slmvx = 0.001, slmvy = 0.0012, slmvz = 0.001, g = -0.000003, slmx = 0.0, slmy = 0.0, slmz = 0.0;

// 这个用来回收和删除我们创建的物体对象
std::vector<TriMesh *> meshList;
std::vector<TriMesh *> meshList2;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void bindObjectAndData(TriMesh *mesh, openGLObject &object, const std::string &vshader, const std::string &fshader)
{

	// 创建顶点数组对象
	glGenVertexArrays(1, &object.vao); // 分配1个顶点数组对象
	glBindVertexArray(object.vao);	   // 绑定顶点数组对象

	// 创建并初始化顶点缓存对象
	glGenBuffers(1, &object.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
	glBufferData(GL_ARRAY_BUFFER,
				 (mesh->getPoints().size() + mesh->getColors().size() + mesh->getNormals().size()) * sizeof(glm::vec3),
				 NULL,
				 GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, mesh->getPoints().size() * sizeof(glm::vec3), &mesh->getPoints()[0]);
	glBufferSubData(GL_ARRAY_BUFFER, mesh->getPoints().size() * sizeof(glm::vec3), mesh->getColors().size() * sizeof(glm::vec3), &mesh->getColors()[0]);
	glBufferSubData(GL_ARRAY_BUFFER, (mesh->getPoints().size() + mesh->getColors().size()) * sizeof(glm::vec3), mesh->getNormals().size() * sizeof(glm::vec3), &mesh->getNormals()[0]);

	object.vshader = vshader;
	object.fshader = fshader;
	object.program = InitShader(object.vshader.c_str(), object.fshader.c_str());

	// 从顶点着色器中初始化顶点的坐标
	object.pLocation = glGetAttribLocation(object.program, "vPosition");
	glEnableVertexAttribArray(object.pLocation);
	glVertexAttribPointer(object.pLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	// 从顶点着色器中初始化顶点的颜色
	object.cLocation = glGetAttribLocation(object.program, "vColor");
	glEnableVertexAttribArray(object.cLocation);
	glVertexAttribPointer(object.cLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(mesh->getPoints().size() * sizeof(glm::vec3)));

	// 从顶点着色器中初始化顶点的法向量
	object.nLocation = glGetAttribLocation(object.program, "vNormal");
	glEnableVertexAttribArray(object.nLocation);
	glVertexAttribPointer(object.nLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET((mesh->getPoints().size() + mesh->getColors().size()) * sizeof(glm::vec3)));

	// 获得矩阵位置
	object.modelLocation = glGetUniformLocation(object.program, "model");
	object.viewLocation = glGetUniformLocation(object.program, "view");
	object.projectionLocation = glGetUniformLocation(object.program, "projection");
	object.shadowLocation = glGetUniformLocation(object.program, "isShadow");
}

void init()
{
	std::string vshader, fshader, fshader1;
	// 读取着色器并使用
	vshader = "shaders/vshader.glsl";
	fshader = "shaders/fshader.glsl";
	fshader1 = "shaders/fshader1.glsl";

	// 设置光源位置
	light->setTranslation(glm::vec3(-10.0, 20.0, 5.0));
	light->setAmbient(glm::vec4(1.0, 1.0, 1.0, 1.0));  // 环境光
	light->setDiffuse(glm::vec4(1.0, 1.0, 1.0, 1.0));  // 漫反射
	light->setSpecular(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 镜面反射
	light->setAttenuation(1.0, 0.045, 0.0075);		   // 衰减系数

	// ——————————————————————————————————————————————————天空地面————————————————————————————————————————————————————
	// 地面 meshes[0]
	plane->generateSquare(glm::vec3(0.3, 0.3, 0.3));
	plane->setNormalize(true);
	// 设置物体的旋转位移
	plane->setRotation(glm::vec3(90.0, 0.0, 0.0));
	plane->setTranslation(glm::vec3(0, -0.003, 0));
	plane->setScale(glm::vec3(200, 200, 200));
	plane->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	plane->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	plane->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	plane->setShininess(27.8974f);										  // 高光系数
	painter->addMesh(plane, "mesh_c", "./assets/ground_sky/sand1.jpg", vshader, fshader);

	// 天空 meshes[1]
	TriMesh *skybox = new TriMesh();
	skybox->setNormalize(true);
	skybox->readObj("./assets/ground_sky/skybox.obj");
	// 设置物体的旋转位移
	skybox->setTranslation(glm::vec3(1.0, 0.25, -5.0));
	skybox->setRotation(glm::vec3(0.0, -90.0, 0.0));
	skybox->setScale(glm::vec3(250, 250, 250));
	skybox->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	skybox->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	skybox->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	skybox->setShininess(27.8974f);										   // 高光系数
	// 加到painter中
	painter->addMesh(skybox, "mesh_a", "./assets/ground_sky/skybox.png", vshader, fshader1);
	meshList.push_back(skybox);

	// ————————————————————————————————————————————————虚拟物体————————————————————————————————————————————————————
	// castle旁上下跳动骑士1 meshes[2]
	TriMesh *castle_knight1 = new TriMesh();
	castle_knight1->setNormalize(true);
	castle_knight1->readObj("./assets/others/chr_knight.obj");
	// 设置物体的旋转位移
	castle_knight1->setTranslation(glm::vec3(2, 0.4, -12.5));
	castle_knight1->setRotation(glm::vec3(0.0, 0.0, 0.0));
	castle_knight1->setScale(glm::vec3(1.0, 1.0, 1.0));
	castle_knight1->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	castle_knight1->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	castle_knight1->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	castle_knight1->setShininess(27.8974f);										   // 高光系数
	// 加到painter中
	painter->addMesh(castle_knight1, "mesh_a", "./assets/others/chr_knight.png", vshader, fshader);
	meshList.push_back(castle_knight1);

	//  空中上下浮动骑士 meshes[3]
	TriMesh *preLaserMan = new TriMesh();
	preLaserMan->setNormalize(true);
	preLaserMan->readObj("./assets/others/chr_knight.obj");
	// 设置物体的旋转位移
	preLaserMan->setTranslation(glm::vec3(0.0, 10, -20));
	preLaserMan->setRotation(glm::vec3(0.0, 0.0, 0.0));
	preLaserMan->setScale(glm::vec3(5.0, 5.0, 5.0));
	preLaserMan->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));	// 环境光
	preLaserMan->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));	// 漫反射
	preLaserMan->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	preLaserMan->setShininess(27.8974f);										// 高光系数
	// 加到painter中preLaserMan
	painter->addMesh(preLaserMan, "mesh_c", "./assets/others/chr_knight.png", vshader, fshader);
	meshList.push_back(preLaserMan);

	// 左边城堡monu3 meshes[4]
	TriMesh *monu3 = new TriMesh();
	monu3->setNormalize(true);
	monu3->readObj("./assets/castle_monu/monu3.obj");
	// 设置物体的旋转位移
	monu3->setTranslation(glm::vec3(-10.0, 5, -10.0));
	monu3->setRotation(glm::vec3(0.0, 0.0, 0.0));
	monu3->setScale(glm::vec3(20.0, 20.0, 20.0));
	monu3->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	monu3->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	monu3->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	monu3->setShininess(27.8974f);										  // 高光系数
	// 加到painter中preLaserMan
	painter->addMesh(monu3, "mesh_c", "./assets/castle_monu/monu3.png", vshader, fshader);
	meshList.push_back(monu3);

	// monu3_knight1 meshes[5]
	TriMesh *monu3_knight1 = new TriMesh();
	monu3_knight1->setNormalize(true);
	monu3_knight1->readObj("./assets/others/chr_knight.obj");
	// 设置物体的旋转位移
	monu3_knight1->setTranslation(glm::vec3(-10.0, 6.4, -10));
	monu3_knight1->setRotation(glm::vec3(0.0, 180.0, 0.0));
	monu3_knight1->setScale(glm::vec3(1.0, 1.0, 1.0));
	monu3_knight1->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	monu3_knight1->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	monu3_knight1->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	monu3_knight1->setShininess(27.8974f);										  // 高光系数
	// 加到painter中preLaserMan
	painter->addMesh(monu3_knight1, "mesh_c", "./assets/others/chr_knight.png", vshader, fshader);
	meshList.push_back(monu3_knight1);

	// monu3_knight2 meshes[6]
	TriMesh *monu3_knight2 = new TriMesh();
	monu3_knight2->setNormalize(true);
	monu3_knight2->readObj("./assets/others/chr_knight.obj");
	// 设置物体的旋转位移
	monu3_knight2->setTranslation(glm::vec3(-10.0, 7.8, -14.8));
	monu3_knight2->setRotation(glm::vec3(0.0, 90.0, 0.0));
	monu3_knight2->setScale(glm::vec3(1.0, 1.0, 1.0));
	monu3_knight2->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	monu3_knight2->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	monu3_knight2->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	monu3_knight2->setShininess(27.8974f);										  // 高光系数
	// 加到painter中preLaserMan
	painter->addMesh(monu3_knight2, "mesh_c", "./assets/others/chr_knight.png", vshader, fshader);
	meshList.push_back(monu3_knight2);

	// monu3_knight3 meshes[7]
	TriMesh *monu3_knight3 = new TriMesh();
	monu3_knight3->setNormalize(true);
	monu3_knight3->readObj("./assets/others/chr_knight.obj");
	// 设置物体的旋转位移
	monu3_knight3->setTranslation(glm::vec3(-7.0, 3.2, -8));
	monu3_knight3->setRotation(glm::vec3(0.0, 0.0, 0.0));
	monu3_knight3->setScale(glm::vec3(1.0, 1.0, 1.0));
	monu3_knight3->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	monu3_knight3->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	monu3_knight3->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	monu3_knight3->setShininess(27.8974f);										  // 高光系数
	// 加到painter中
	painter->addMesh(monu3_knight3, "mesh_c", "./assets/others/chr_knight.png", vshader, fshader);
	meshList.push_back(monu3_knight3);

	// monu3_knight4 meshes[8]
	TriMesh *monu3_knight4 = new TriMesh();
	monu3_knight4->setNormalize(true);
	monu3_knight4->readObj("./assets/others/chr_knight.obj");
	// 设置物体的旋转位移
	monu3_knight4->setTranslation(glm::vec3(-3.5, 0.4, -10));
	monu3_knight4->setRotation(glm::vec3(0.0, 180.0, 0.0));
	monu3_knight4->setScale(glm::vec3(1.0, 1.0, 1.0));
	monu3_knight4->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	monu3_knight4->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	monu3_knight4->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	monu3_knight4->setShininess(27.8974f);										  // 高光系数
	// 加到painter中
	painter->addMesh(monu3_knight4, "mesh_c", "./assets/others/chr_knight.png", vshader, fshader);
	meshList.push_back(monu3_knight4);

	// monu3_knight5 meshes[9]
	TriMesh *monu3_knight5 = new TriMesh();
	monu3_knight5->setNormalize(true);
	monu3_knight5->readObj("./assets/others/chr_knight.obj");
	// 设置物体的旋转位移
	monu3_knight5->setTranslation(glm::vec3(-13.0, 0.4, -3.5));
	monu3_knight5->setRotation(glm::vec3(0.0, 90.0, 0.0));
	monu3_knight5->setScale(glm::vec3(1.0, 1.0, 1.0));
	monu3_knight5->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	monu3_knight5->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	monu3_knight5->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	monu3_knight5->setShininess(27.8974f);										  // 高光系数
	// 加到painter中preLaserMan
	painter->addMesh(monu3_knight5, "mesh_c", "./assets/others/chr_knight.png", vshader, fshader);
	meshList.push_back(monu3_knight5);

	// castle meshes[10]
	TriMesh *castle = new TriMesh();
	castle->setNormalize(true);
	castle->readObj("./assets/castle_monu/castle.obj");
	// 设置物体的旋转位移
	castle->setTranslation(glm::vec3(5.0, 2.9, -10));
	castle->setRotation(glm::vec3(0.0, 90.0, 0.0));
	castle->setScale(glm::vec3(10, 10, 10));
	castle->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	castle->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	castle->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	castle->setShininess(27.8974f);										   // 高光系数
	// 加到painter中
	painter->addMesh(castle, "mesh_c", "./assets/castle_monu/castle.png", vshader, fshader);
	meshList.push_back(castle);

	// monu1_knight1  meshes[11]
	TriMesh *monu1_knight1 = new TriMesh();
	monu1_knight1->setNormalize(true);
	monu1_knight1->readObj("./assets/others/chr_knight.obj");
	// 设置物体的旋转位移
	monu1_knight1->setTranslation(glm::vec3(4.2, 1.0, 0.7));
	monu1_knight1->setRotation(glm::vec3(0.0, -90.0, 0.0));
	monu1_knight1->setScale(glm::vec3(1.0, 1.0, 1.0));
	monu1_knight1->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	monu1_knight1->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	monu1_knight1->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	monu1_knight1->setShininess(27.8974f);										  // 高光系数
	// 加到painter中preLaserMan
	painter->addMesh(monu1_knight1, "mesh_c", "./assets/others/chr_knight.png", vshader, fshader);
	meshList.push_back(monu1_knight1);

	// cars0 meshes[12]
	TriMesh *cars0 = new TriMesh();
	cars0->setNormalize(true);
	cars0->readObj("./assets/others/cars-0.obj");
	// 设置物体的旋转位移
	cars0->setTranslation(glm::vec3(-6.0, 0.3, -2));
	cars0->setRotation(glm::vec3(0.0, 0.0, 0.0));
	cars0->setScale(glm::vec3(2.0, 2.0, 2.0));
	cars0->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	cars0->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	cars0->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	cars0->setShininess(27.8974f);										  // 高光系数
	// 加到painter中
	painter->addMesh(cars0, "mesh_c", "./assets/others/cars-0.png", vshader, fshader);
	meshList.push_back(cars0);

	// cars1 meshes[13]
	TriMesh *cars1 = new TriMesh();
	cars1->setNormalize(true);
	cars1->readObj("./assets/others/cars-1.obj");
	// 设置物体的旋转位移
	cars1->setTranslation(glm::vec3(3.0, 0.38, -2.0));
	cars1->setRotation(glm::vec3(0.0, 0.0, 0.0));
	cars1->setScale(glm::vec3(2.0, 2.0, 2.0));
	cars1->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	cars1->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	cars1->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	cars1->setShininess(27.8974f);										  // 高光系数
	// 加到painter中
	painter->addMesh(cars1, "mesh_c", "./assets/others/cars-1.png", vshader, fshader);
	meshList.push_back(cars1);

	// castle旁上下跳动骑士2 meshes[14]
	TriMesh *FireSlam = new TriMesh();
	FireSlam->setNormalize(true);
	FireSlam->readObj("./assets/others/chr_knight.obj");
	// 设置物体的旋转位移
	FireSlam->setTranslation(glm::vec3(2.6, 0.45, -7));
	FireSlam->setRotation(glm::vec3(0.0, 90.0, 0.0));
	FireSlam->setScale(glm::vec3(1.0, 1.0, 1.0));
	FireSlam->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));	 // 环境光
	FireSlam->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));	 // 漫反射
	FireSlam->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	FireSlam->setShininess(27.8974f);										 // 高光系数
	// 加到painter中
	painter->addMesh(FireSlam, "mesh_c", "./assets/others/chr_knight.png", vshader, fshader);
	meshList.push_back(FireSlam);

	// 右边粉色城堡monu1  meshes[15]
	TriMesh *monu1 = new TriMesh();
	monu1->setNormalize(true);
	monu1->readObj("./assets/castle_monu/monu1.obj");
	// 设置物体的旋转位移
	monu1->setTranslation(glm::vec3(5, 3.57, 0));
	monu1->setRotation(glm::vec3(0.0, -180.0, 0.0));
	monu1->setScale(glm::vec3(10, 10, 10));
	monu1->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	monu1->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	monu1->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	monu1->setShininess(27.8974f);										  // 高光系数
	// 加到painter中
	painter->addMesh(monu1, "mesh_c", "./assets/castle_monu/monu1.png", vshader, fshader);
	meshList.push_back(monu1);

	// castle顶部骑士 meshes[16]
	TriMesh *castle_knight2 = new TriMesh();
	castle_knight2->setNormalize(true);
	castle_knight2->readObj("./assets/others/chr_knight.obj");
	// 设置物体的旋转位移
	castle_knight2->setTranslation(glm::vec3(2.8, 6.2, -7.7));
	castle_knight2->setRotation(glm::vec3(0.0, -90.0, 0.0));
	castle_knight2->setScale(glm::vec3(1.0, 1.0, 1.0));
	castle_knight2->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	castle_knight2->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	castle_knight2->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	castle_knight2->setShininess(27.8974f);										   // 高光系数
	// 加到painter中preLaserMan
	painter->addMesh(castle_knight2, "mesh_c", "./assets/others/chr_knight.png", vshader, fshader);
	meshList.push_back(castle_knight2);

	// 空中浮动骑士下面的球 meshes[17]
	TriMesh *ball = new TriMesh();
	ball->setNormalize(true);
	ball->readObj("./assets/others/ball.obj");
	// 设置物体的旋转位移
	ball->setTranslation(glm::vec3(0.25, 7.6, -20));
	ball->setRotation(glm::vec3(0.0, 0.0, 0.0));
	ball->setScale(glm::vec3(3.0, 3.0, 3.0));
	ball->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));	 // 环境光
	ball->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));	 // 漫反射
	ball->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	ball->setShininess(27.8974f);										 // 高光系数
	// 加到painter中preLaserMan
	painter->addMesh(ball, "mesh_c", "./assets/others/ball.png", vshader, fshader);
	meshList.push_back(ball);

	// ————————————————————————————————————————————层级建模——————————————————————————————————————————————
	// 躯干
	TriMesh *tarso = new TriMesh();
	// 读取模型
	tarso->setNormalize(true);
	tarso->readObj("./assets/robot/tarso.obj");
	tarso->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	tarso->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	tarso->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	tarso->setShininess(27.8974f);										  // 高光系数
	// 加到painter2中
	painter2->addMesh(tarso, "mesh_b", "./assets/robot/tarso.png", vshader, fshader);
	meshList2.push_back(tarso);

	// 大腿1
	TriMesh *upleg = new TriMesh();
	// 读取模型
	upleg->setNormalize(true);
	upleg->readObj("./assets/robot/upleg.obj");
	upleg->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	upleg->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	upleg->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	upleg->setShininess(27.8974f);										  // 高光系数
	// 设置物体的旋转位移
	// 加到painter2中
	painter2->addMesh(upleg, "mesh_b", "./assets/robot/upleg.png", vshader, fshader);
	meshList2.push_back(upleg);

	// 大腿2
	TriMesh *upleg2 = new TriMesh();
	// 读取模型
	upleg2->setNormalize(true);
	upleg2->readObj("./assets/robot/upleg.obj");
	upleg2->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	upleg2->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	upleg2->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	upleg2->setShininess(27.8974f);										   // 高光系数
	// 加到painter2中
	painter2->addMesh(upleg2, "mesh_b", "./assets/robot/upleg.png", vshader, fshader);
	meshList2.push_back(upleg2);

	// 小腿1
	TriMesh *lowleg1 = new TriMesh();
	// 读取模型
	lowleg1->setNormalize(true);
	lowleg1->readObj("./assets/robot/lowleg.obj");
	lowleg1->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));	// 环境光
	lowleg1->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));	// 漫反射
	lowleg1->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	lowleg1->setShininess(27.8974f);										// 高光系数
	// 加到painter2中
	painter2->addMesh(lowleg1, "mesh_b", "./assets/robot/lowleg.png", vshader, fshader);
	meshList2.push_back(lowleg1);

	// 小腿2
	TriMesh *lowleg2 = new TriMesh();
	// 读取模型
	lowleg2->setNormalize(true);
	lowleg2->readObj("./assets/robot/lowleg.obj");
	lowleg2->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));	// 环境光
	lowleg2->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));	// 漫反射
	lowleg2->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	lowleg2->setShininess(27.8974f);										// 高光系数
	// 加到painter2中
	painter2->addMesh(lowleg2, "mesh_b", "./assets/robot/lowleg.png", vshader, fshader);
	meshList2.push_back(lowleg2);

	// 上臂1
	TriMesh *uparm1 = new TriMesh();
	// 读取模型
	uparm1->setNormalize(true);
	uparm1->readObj("./assets/robot/uparm.obj");
	uparm1->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	uparm1->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	uparm1->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	uparm1->setShininess(27.8974f);										   // 高光系数
	// 加到painter2中
	painter2->addMesh(uparm1, "mesh_b", "./assets/robot/uparm.png", vshader, fshader);
	meshList2.push_back(uparm1);

	// 上臂2
	TriMesh *uparm2 = new TriMesh();
	// 读取模型
	uparm2->setNormalize(true);
	uparm2->readObj("./assets/robot/uparm.obj");
	uparm2->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));  // 环境光
	uparm2->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));  // 漫反射
	uparm2->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	uparm2->setShininess(27.8974f);										   // 高光系数
	// 加到painter2中
	painter2->addMesh(uparm2, "mesh_b", "./assets/robot/uparm.png", vshader, fshader);
	meshList2.push_back(uparm2);

	// 下臂1
	TriMesh *lowarm1 = new TriMesh();
	// 读取模型
	lowarm1->setNormalize(true);
	lowarm1->readObj("./assets/robot/lowarm.obj");
	lowarm1->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));	// 环境光
	lowarm1->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));	// 漫反射
	lowarm1->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	lowarm1->setShininess(27.8974f);										// 高光系数
	// 加到painter2中
	painter2->addMesh(lowarm1, "mesh_b", "./assets/robot/lowarm.png", vshader, fshader);
	meshList2.push_back(lowarm1);

	// 下臂2
	TriMesh *lowarm2 = new TriMesh();
	// 读取模型
	lowarm2->setNormalize(true);
	lowarm2->readObj("./assets/robot/lowarm.obj");
	lowarm2->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));	// 环境光
	lowarm2->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));	// 漫反射
	lowarm2->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	lowarm2->setShininess(27.8974f);										// 高光系数
	// 加到painter2中
	painter2->addMesh(lowarm2, "mesh_b", "./assets/robot/lowarm.png", vshader, fshader);
	meshList2.push_back(lowarm2);

	// 头
	TriMesh *head = new TriMesh();
	// 读取模型
	head->setNormalize(true);
	head->readObj("./assets/robot/head.obj");
	head->setAmbient(glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f));	 // 环境光
	head->setDiffuse(glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f));	 // 漫反射
	head->setSpecular(glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f)); // 镜面反射
	head->setShininess(27.8974f);										 // 高光系数
	// 加到painter2中
	painter2->addMesh(head, "mesh_b", "./assets/robot/head.png", vshader, fshader);
	meshList2.push_back(head);

	glClearColor(41 / 256.0, 105.0 / 256.0, 152.0 / 256.0, 0.0);
}

void bindLightAndMaterial(TriMesh *mesh, openGLObject &object, Light *light, Camera *camera)
{
	// 传递相机的位置
	glUniform3fv(glGetUniformLocation(object.program, "eye_position"), 1, &camera->eye[0]);

	// 传递物体的材质
	glm::vec4 meshAmbient = mesh->getAmbient();
	glm::vec4 meshDiffuse = mesh->getDiffuse();
	glm::vec4 meshSpecular = mesh->getSpecular();
	float meshShininess = mesh->getShininess();

	glUniform4fv(glGetUniformLocation(object.program, "material.ambient"), 1, &meshAmbient[0]);
	glUniform4fv(glGetUniformLocation(object.program, "material.diffuse"), 1, &meshDiffuse[0]);
	glUniform4fv(glGetUniformLocation(object.program, "material.specular"), 1, &meshSpecular[0]);
	glUniform1f(glGetUniformLocation(object.program, "material.shininess"), meshShininess);

	// 传递光源信息
	glm::vec4 lightAmbient = light->getAmbient();
	glm::vec4 lightDiffuse = light->getDiffuse();
	glm::vec4 lightSpecular = light->getSpecular();
	glm::vec3 lightPosition = light->getTranslation();
	glUniform4fv(glGetUniformLocation(object.program, "light.ambient"), 1, &lightAmbient[0]);
	glUniform4fv(glGetUniformLocation(object.program, "light.diffuse"), 1, &lightDiffuse[0]);
	glUniform4fv(glGetUniformLocation(object.program, "light.specular"), 1, &lightSpecular[0]);
	glUniform3fv(glGetUniformLocation(object.program, "light.position"), 1, &lightPosition[0]);
}
void display()
{
	// 控制自动来回移动
	if (!isMove)
	{
		painter->meshes[6]->setRotation(glm::vec3(0.0, 90.0, 0.0));
		painter->meshes[8]->setRotation(glm::vec3(0.0, 0.0, 0.0));
		painter->meshes[9]->setRotation(glm::vec3(0.0, 90.0, 0.0));
		moveDelta += 0.004; // 来回移动，添加相反数
	}
	else
	{
		painter->meshes[6]->setRotation(glm::vec3(0.0, -90.0, 0.0));
		painter->meshes[8]->setRotation(glm::vec3(0.0, 180.0, 0.0));
		painter->meshes[9]->setRotation(glm::vec3(0.0, -90.0, 0.0));
		moveDelta -= 0.004; // 来回移动，添加相反数
	}
	if (moveDelta > 5)
		isMove = true; // 控制在一定的范围内移动
	if (moveDelta < 0)
		isMove = false;
	painter->meshes[6]->setTranslation(glm::vec3(-10.0 + moveDelta, 7.8, -14.8));
	painter->meshes[8]->setTranslation(glm::vec3(-3.5, 0.4, -10 + moveDelta));	 // knight4
	painter->meshes[9]->setTranslation(glm::vec3(-13.0 + moveDelta, 0.4, -3.5)); // knight5

	// 控制自动来回上下跳动
	if (slmy < 0)
		slmvy = 0.0012; // 刷新初速度
	slmvy += g;
	slmy += slmvy;

	if (slmove)
	{
		// 判断向什么方向移动 前或者后
		slmx += slmvx;
		slmz += slmvz;
		painter->meshes[2]->setTranslation(glm::vec3(1.8, 0.4 + slmy, -12.5 + slmz));
		painter->meshes[2]->setRotation(glm::vec3(0.0, 0.0, 0.0));
		painter->meshes[3]->setTranslation(glm::vec3(0.0, 10 + slmy, -20));
		painter->meshes[3]->setRotation(glm::vec3(0.0, 0.0, 0.0));
		painter->meshes[17]->setTranslation(glm::vec3(0.25, 7.6 + slmy, -20));
		painter->meshes[17]->setRotation(glm::vec3(0.0, 0.0, 0.0));
		painter->meshes[14]->setTranslation(glm::vec3(2.6 + slmx, 0.45 + slmy, -7));
		painter->meshes[14]->setRotation(glm::vec3(0.0, 90.0, 0.0));
	}
	else
	{
		slmx -= slmvx;
		slmz -= slmvz;
		painter->meshes[2]->setTranslation(glm::vec3(1.8, 0.4 + slmy, -12.5 + slmz));
		painter->meshes[2]->setRotation(glm::vec3(0.0, 180.0, 0.0));
		painter->meshes[3]->setTranslation(glm::vec3(0.0, 10 + slmy, -20));
		painter->meshes[17]->setTranslation(glm::vec3(0.25, 7.6 + slmy, -20));
		painter->meshes[14]->setTranslation(glm::vec3(2.6 + slmx, 0.45 + slmy, -7));
		painter->meshes[14]->setRotation(glm::vec3(0.0, -90.0, 0.0));
	}
	if (slmx > 4.0)
		slmove = false; // 控制在一定范围内移动
	if (slmx < 0)
		slmove = true;

	if (slmz > 4.0)
		slmove = false; // 控制在一定范围内移动
	if (slmz < 0)
		slmove = true;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	painter->drawMeshes(light, camera);
	painter2->drawMeshes2(light, camera);
}

void printHelp()
{
	std::cout << "================================================" << std::endl;

	std::cout << "Keyboard Usage" << std::endl
			  << "ESC/Q:		Exit" << std::endl
			  << "3:            Change camera" << std::endl
			  << "W/S/A/D/Z/X:  Front/rear/left/right/up/down" << std::endl
			  << "SPACE:        Jump" << std::endl
			  <<

		std::endl
			  << "Mouse Usage" << std::endl
			  << "move:		    Free browsing" << std::endl
			  << "left_buton:	Head rotation" << std::endl
			  << std::endl;
}

// 键盘交互
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	float tmp;
	glm::vec4 ambient;
	switch (key)
	{
	// 按ESC或Q建退出程序
	case GLFW_KEY_Q:;
	case GLFW_KEY_ESCAPE:
		exit(EXIT_SUCCESS);
		break; // 结束程序

	// 按3键实现一、三人称视角的切换
	case GLFW_KEY_3:
		if (action == GLFW_PRESS)
		{
			if (painter2->freeview)
			{
				painter2->freeview = false;
				camera->eye = lasteyepos;
			}
			else
			{
				painter2->freeview = true;
				lasteyepos = camera->eye;
			}
		}
		break;
	default:
		// 前后左右上下移动
		if (key == GLFW_KEY_S || key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_D || key == GLFW_KEY_Z || key == GLFW_KEY_X || key == GLFW_KEY_SPACE)
		{
			if (action == GLFW_PRESS)
			{
				painter2->walk = true;
			}
			if (action == GLFW_RELEASE)
			{
				painter2->walk = false;
			}
		}

		camera->keyboard(key, action, mode);
		break;
	}
}

void cleanData()
{
	delete camera;
	camera = NULL;

	delete light;
	light = NULL;

	painter->cleanMeshes();

	delete painter;
	painter = NULL;

	for (int i = 0; i < meshList.size(); i++)
	{
		delete meshList[i];
	}
	meshList.clear();
}

// 鼠标移动交互
void mouse_move_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (firstMouseMove) // 首次鼠标移动
	{
		lastX = xpos;
		lastY = ypos;
		firstMouseMove = false;
	}

	GLfloat xoffset = xpos - lastX; // 记录x y上的偏移值
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;
	// 在未暂停的情况下鼠标的操作才有效
	if (mousecontrol)
	{
		camera->handleMouseMove(xoffset, yoffset);
	}
}

// 鼠标按键交互
void mouse_button_callback(GLFWwindow *window, int button, int action, int mode)
{
	// 按鼠标左键头部旋转
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		painter2->buttonleft = true;
	}
}

// 修改aspect适应窗口变化，改变窗口大小也不改变物体实际比例
void update_camera_aspect()
{
	float w = WIDTH, h = HEIGHT;
	camera->aspect = w / h;
}

int main(int argc, char **argv)
{
	// 初始化GLFW库，必须是应用程序调用的第一个GLFW函数
	glfwInit();

	// 配置GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// 配置窗口属性
	GLFWwindow *window = glfwCreateWindow(1200, 1000, "2020152051_Wang Xiaoli_Final Assignment", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_move_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Init mesh, shaders, buffer
	init();
	// 输出帮助信息
	printHelp();
	// 启用深度测试
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwGetWindowSize(window, &WIDTH, &HEIGHT); // 窗口大小变化不改变物体形状
		// 改变透视投影参数aspect
		update_camera_aspect();

		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	cleanData();

	return 0;
}