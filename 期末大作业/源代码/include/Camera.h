﻿#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Angel.h"

class Camera
{
public:
	Camera();
	~Camera();

	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix( bool isOrtho );

	glm::mat4 lookAt(const glm::vec4& eye, const glm::vec4& at, const glm::vec4& up);

	glm::mat4 ortho(const GLfloat left, const GLfloat right,
		const GLfloat bottom, const GLfloat top,
		const GLfloat zNear, const GLfloat zFar);

	glm::mat4 perspective(const GLfloat fovy, const GLfloat aspect,
		const GLfloat zNear, const GLfloat zFar);

	glm::mat4 frustum(const GLfloat left, const GLfloat right,
		const GLfloat bottom, const GLfloat top,
		const GLfloat zNear, const GLfloat zFar);

	void  handleMouseMove(GLfloat xoffset, GLfloat yoffset);

	// 每次更改相机参数后更新一下相关的数值
	void updateCamera();
	// 处理相机的键盘操作
	void keyboard(int key, int action, int mode);

	// 模视矩阵
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;

	//add by myself
	float mouse_sensitivity = 0.5;
	glm::vec4 side;
	bool jump = false;
	float v ,g=0.000003;
	// 相机位置参数
	float radius = 2.0;
	float rotateAngle = 0.0;
	float upAngle = 0.0;
	glm::vec4 eye= glm::vec4(0.0, 2.0, 2.0,1.0);
	glm::vec4 at;
	glm::vec4 up;
	// 投影参数
	#undef near
	#undef far
	float near = 0.1;
	float far = 100.0;
	// 透视投影参数
	float fov = 45.0;
	float aspect = 1.0;
	// 正交投影参数
	float scale = 1.5;

};
#endif