#ifndef _MESH_PAINTER_H_
#define _MESH_PAINTER_H_

#include "TriMesh.h"
#include "Angel.h"

#include "Camera.h"

#include <vector>

struct openGLObject
{
	// 顶点数组对象
	GLuint vao;
	// 顶点缓存对象
	GLuint vbo;

	// 着色器程序
	GLuint program;
	// 着色器文件
	std::string vshader;
	std::string fshader;
	// 着色器变量
	GLuint pLocation;
	GLuint cLocation;
	GLuint nLocation;
	GLuint tLocation;

	// 纹理
	std::string texture_image;
	GLuint texture;

	// 投影变换变量
	GLuint modelLocation;
	GLuint viewLocation;
	GLuint projectionLocation;

	// 阴影变量
	GLuint shadowLocation;
};

class MatrixStack
{
public:
	int _index;
	int _size;
	glm::mat4 *_matrices;

	// MatrixStack(int numMatrices = 100) :_index(0), _size(numMatrices);

	//~MatrixStack();

	void push(const glm::mat4 &m);

	glm::mat4 &pop();
};

class MeshPainter
{

public:
	MeshPainter();
	~MeshPainter();

	std::vector<std::string> getMeshNames();

	std::vector<TriMesh *> getMeshes();
	std::vector<openGLObject> getOpenGLObj();

	// 读取纹理文件
	void load_texture_STBImage(const std::string &file_name, GLuint &texture);

	// 传递光线材质数据的
	// void bindLightAndMaterial( int mesh_id, int light_id, Camera* camera );
	void bindLightAndMaterial(TriMesh *mesh, openGLObject &object, Light *light, Camera *camera);

	void bindObjectAndData(TriMesh *mesh, openGLObject &object, const std::string &texture_image, const std::string &vshader, const std::string &fshader);
	void bindObjectAndData2(TriMesh *mesh, openGLObject &object, const std::string &vshader, const std::string &fshader);
	// 添加物体
	void addMesh(TriMesh *mesh, const std::string &name, const std::string &texture_image, const std::string &vshader, const std::string &fshader);
	void addMesh2(TriMesh *mesh, const std::string &name, const std::string &vshader, const std::string &fshader);
	// 绘制物体
	void drawMesh(TriMesh *mesh, openGLObject &object, Light *light, Camera *camera);
	void drawMesh2(Light *light, Camera *camera);
	void displayproj(TriMesh *mesh, openGLObject mesh_object, Camera *camera, int isShadow, Light *light);
	void displayproj2(TriMesh *mesh, openGLObject mesh_object, Camera *camera, int isShadow, Light *light);
	// 绘制多个物体
	void drawMeshes(Light *light, Camera *camera);
	void drawMeshes2(Light *light, Camera *camera);

	void personwalk(float frame, float &v); // 人物移动
	void displayleft();
	// 清空数据
	void cleanMeshes();
	bool walk = false, firstwalk = false, freeview = false;
	bool buttonleft = false, headback = false, buttonright = false, armback = false;
	float headangle = 0.0, headmove = 0.0, headpos = 0.0, headanglew = 1.0;
	float armangle = 0.0, armmove = 0.0, armpos = 0.0, armanglew = 1.0;
	std::vector<TriMesh *> meshes;

private:
	std::vector<std::string> mesh_names;
	std::vector<openGLObject> opengl_objects;
	glm::mat4 *matrices;
	int index;
	GLfloat theta[11];
	glm::vec3 lastview;
	float body_angle;
	float walkv = 0.0;
};

#endif