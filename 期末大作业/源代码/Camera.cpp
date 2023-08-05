#include "Camera.h"
using namespace std;
Camera::Camera() { updateCamera(); };
Camera::~Camera() {}

glm::mat4 Camera::getViewMatrix()
{
	return this->lookAt(eye, eye + at, up);
}

glm::mat4 Camera::getProjectionMatrix(bool isOrtho)
{
	if (isOrtho)
	{
		return this->ortho(-scale, scale, -scale, scale, this->near, this->far);
	}
	else
	{
		return this->perspective(fov, aspect, this->near, this->far);
	}
}

glm::mat4 Camera::lookAt(const glm::vec4 &eye, const glm::vec4 &at, const glm::vec4 &up)
{
	// use glm.
	glm::vec3 eye_3 = eye;
	glm::vec3 at_3 = at;
	glm::vec3 up_3 = up;

	glm::mat4 view = glm::lookAt(eye_3, at_3, up_3);

	return view;
}

glm::mat4 Camera::ortho(const GLfloat left, const GLfloat right,
						const GLfloat bottom, const GLfloat top,
						const GLfloat zNear, const GLfloat zFar)
{
	glm::mat4 c = glm::mat4(1.0f);
	c[0][0] = 2.0 / (right - left);
	c[1][1] = 2.0 / (top - bottom);
	c[2][2] = -2.0 / (zFar - zNear);
	c[3][3] = 1.0;
	c[0][3] = -(right + left) / (right - left);
	c[1][3] = -(top + bottom) / (top - bottom);
	c[2][3] = -(zFar + zNear) / (zFar - zNear);

	c = glm::transpose(c);
	return c;
}

glm::mat4 Camera::perspective(const GLfloat fovy, const GLfloat aspect,
							  const GLfloat zNear, const GLfloat zFar)
{
	GLfloat top = tan(fovy * M_PI / 180 / 2) * zNear;
	GLfloat right = top * aspect;

	glm::mat4 c = glm::mat4(1.0f);
	c[0][0] = zNear / right;
	c[1][1] = zNear / top;
	c[2][2] = -(zFar + zNear) / (zFar - zNear);
	c[2][3] = -(2.0 * zFar * zNear) / (zFar - zNear);
	c[3][2] = -1.0;
	c[3][3] = 0.0;

	c = glm::transpose(c);
	return c;
}

glm::mat4 Camera::frustum(const GLfloat left, const GLfloat right,
						  const GLfloat bottom, const GLfloat top,
						  const GLfloat zNear, const GLfloat zFar)
{
	// 任意视锥体矩阵
	glm::mat4 c = glm::mat4(1.0f);
	c[0][0] = 2.0 * zNear / (right - left);
	c[0][2] = (right + left) / (right - left);
	c[1][1] = 2.0 * zNear / (top - bottom);
	c[1][2] = (top + bottom) / (top - bottom);
	c[2][2] = -(zFar + zNear) / (zFar - zNear);
	c[2][3] = -2.0 * zFar * zNear / (zFar - zNear);
	c[3][2] = -1.0;
	c[3][3] = 0.0;

	c = glm::transpose(c);
	return c;
}

// 鼠标控制镜头移动
void Camera::handleMouseMove(GLfloat xoffset, GLfloat yoffset)
{
	xoffset *= this->mouse_sensitivity; // 用鼠标灵敏度调节角度变换
	yoffset *= this->mouse_sensitivity;
	upAngle += yoffset;

	rotateAngle += xoffset;
	// 保证角度在合理范围内
	if (upAngle > 80.0)
		upAngle = 80.0;
	if (upAngle < -80.0)
		upAngle = -80.0;

	if (rotateAngle < 0.0f)
		rotateAngle += 360.0f;
	this->updateCamera(); // 更新相机向量
}

bool Wmove = false, Smove = false, Dmove = false, Amove = false, Zmove = false, Xmove = false; // 键盘控制镜头移动
void Camera::updateCamera()
{
	// 眼睛看向的地方
	float atx = 0.5 * cos(upAngle * M_PI / 180.0) * sin(rotateAngle * M_PI / 180.0);
	float aty = 0.5 * sin(upAngle * M_PI / 180.0);
	float atz = -0.5 * cos(upAngle * M_PI / 180.0) * cos(rotateAngle * M_PI / 180.0);

	at = glm::vec4(atx, aty, atz, 0.0);

	glm::vec4 side = glm::vec4(0, 0, 0, 1.0);
	side.x = -cos(glm::radians(rotateAngle));
	side.y = 0;
	side.z = -sin(glm::radians(rotateAngle));
	this->side = glm::normalize(side);
	up = glm::vec4(0.0, 1.0, 0.0, 0.0);
	if (Wmove == true)
		eye += glm::vec4(atx * 0.001, 0, atz * 0.001, 0.0); // 实现前后左右的移动
	if (Smove == true)
		eye -= glm::vec4(atx * 0.001, 0, atz * 0.001, 0.0);
	if (Amove == true)
		eye += glm::vec4(side.x * 0.001, 0, side.z * 0.001, 0.0);
	if (Dmove == true)
		eye -= glm::vec4(side.x * 0.001, 0, side.z * 0.001, 0.0);
	if (Zmove == true)
		eye += glm::vec4(0.0, aty * 0.002, 0.0, 0.0);
	if (Xmove == true)
		eye -= glm::vec4(0.0, aty * 0.002, 0.0, 0.0);

	// 在跳跃时模拟上抛运动
	if (jump)
	{
		v -= g;
		eye.y += v;
		if (eye.y < 2.0)
		{
			jump = false;
			v = 0.02;
		}
	}
}

// 键盘处理镜头移动
void Camera::keyboard(int key, int action, int mode)
{
	// 键盘事件处理
	// 通过按键改变相机和投影的参数
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		Wmove = true;
	}
	else if (key == GLFW_KEY_W && action == GLFW_RELEASE)
	{
		Wmove = false;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		Smove = true;
	}
	else if (key == GLFW_KEY_S && action == GLFW_RELEASE)
	{
		Smove = false;
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		Amove = true;
	}
	else if (key == GLFW_KEY_A && action == GLFW_RELEASE)
	{
		Amove = false;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		Dmove = true;
	}
	else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
	{
		Dmove = false;
	}
	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		Zmove = true;
	}
	else if (key == GLFW_KEY_Z && action == GLFW_RELEASE)
	{
		Zmove = false;
	}
	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		Xmove = true;
	}
	else if (key == GLFW_KEY_X && action == GLFW_RELEASE)
	{
		Xmove = false;
	}

	// 空格键jump
	if (key == GLFW_KEY_SPACE && mode == 0x0000)
	{
		// 赋予初速度
		jump = true;
		v = 0.0025;
	}
}
