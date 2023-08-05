#include "MeshPainter.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using namespace std;

MeshPainter::MeshPainter()
{
    for (int i = 0; i < 11; i++)
        theta[i] = 0.0;
    matrices = new glm::mat4[100];
    index = 0;
};
MeshPainter::~MeshPainter(){};

std::vector<std::string> MeshPainter::getMeshNames() { return mesh_names; };
std::vector<TriMesh *> MeshPainter::getMeshes() { return meshes; };
std::vector<openGLObject> MeshPainter::getOpenGLObj() { return opengl_objects; };

void MeshPainter::bindObjectAndData(TriMesh *mesh, openGLObject &object, const std::string &texture_image, const std::string &vshader, const std::string &fshader)
{
    // 初始化各种对象

    std::vector<glm::vec3> points = mesh->getPoints();
    std::vector<glm::vec3> normals = mesh->getNormals();
    std::vector<glm::vec3> colors = mesh->getColors();
    std::vector<glm::vec2> textures = mesh->getTextures();

    // 创建顶点数组对象
    glGenVertexArrays(1, &object.vao); // 分配1个顶点数组对象
    glBindVertexArray(object.vao);     // 绑定顶点数组对象

    // 创建并初始化顶点缓存对象
    glGenBuffers(1, &object.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 points.size() * sizeof(glm::vec3) +
                     normals.size() * sizeof(glm::vec3) +
                     colors.size() * sizeof(glm::vec3) +
                     textures.size() * sizeof(glm::vec2),
                 NULL, GL_STATIC_DRAW);

    // 绑定顶点数据
    glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(glm::vec3), points.data());
    // 绑定颜色数据
    glBufferSubData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), colors.size() * sizeof(glm::vec3), colors.data());
    // 绑定法向量数据
    glBufferSubData(GL_ARRAY_BUFFER, (points.size() + colors.size()) * sizeof(glm::vec3), normals.size() * sizeof(glm::vec3), normals.data());
    // 绑定纹理数据
    glBufferSubData(GL_ARRAY_BUFFER, (points.size() + normals.size() + colors.size()) * sizeof(glm::vec3), textures.size() * sizeof(glm::vec2), textures.data());

    object.vshader = vshader;
    object.fshader = fshader;
    object.program = InitShader(object.vshader.c_str(), object.fshader.c_str());

    // 将顶点传入着色器
    object.pLocation = glGetAttribLocation(object.program, "vPosition");
    glEnableVertexAttribArray(object.pLocation);
    glVertexAttribPointer(object.pLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    // 将颜色传入着色器
    object.cLocation = glGetAttribLocation(object.program, "vColor");
    glEnableVertexAttribArray(object.cLocation);
    glVertexAttribPointer(object.cLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(points.size() * sizeof(glm::vec3)));

    // 将法向量传入着色器
    object.nLocation = glGetAttribLocation(object.program, "vNormal");
    glEnableVertexAttribArray(object.nLocation);
    glVertexAttribPointer(object.nLocation, 3,
                          GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET((points.size() + colors.size()) * sizeof(glm::vec3)));

    object.tLocation = glGetAttribLocation(object.program, "vTexture");
    glEnableVertexAttribArray(object.tLocation);
    glVertexAttribPointer(object.tLocation, 2,
                          GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET((points.size() + colors.size() + normals.size()) * sizeof(glm::vec3)));

    // 获得矩阵位置
    object.modelLocation = glGetUniformLocation(object.program, "model");
    object.viewLocation = glGetUniformLocation(object.program, "view");
    object.projectionLocation = glGetUniformLocation(object.program, "projection");

    object.shadowLocation = glGetUniformLocation(object.program, "isShadow");

    // 读取纹理图片数
    object.texture_image = texture_image;
    // 创建纹理的缓存对象
    glGenTextures(1, &object.texture);
    // 调用stb_image生成纹理
    load_texture_STBImage(object.texture_image, object.texture);

    // Clean up
    glUseProgram(0);
    glBindVertexArray(0);
};

void MeshPainter::bindObjectAndData2(TriMesh *mesh, openGLObject &object, const std::string &vshader, const std::string &fshader)
{

    // 创建顶点数组对象
    glGenVertexArrays(1, &object.vao); // 分配1个顶点数组对象
    glBindVertexArray(object.vao);     // 绑定顶点数组对象

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

void MeshPainter::bindLightAndMaterial(TriMesh *mesh, openGLObject &object, Light *light, Camera *camera)
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

    glUniform1f(glGetUniformLocation(object.program, "light.constant"), light->getConstant());
    glUniform1f(glGetUniformLocation(object.program, "light.linear"), light->getLinear());
    glUniform1f(glGetUniformLocation(object.program, "light.quadratic"), light->getQuadratic());
}

// 用于其他物体
void MeshPainter::addMesh(TriMesh *mesh, const std::string &name, const std::string &texture_image, const std::string &vshader, const std::string &fshader)
{
    mesh_names.push_back(name);
    meshes.push_back(mesh);

    openGLObject object;
    // 绑定openGL对象，并传递顶点属性的数据
    bindObjectAndData(mesh, object, texture_image, vshader, fshader);

    opengl_objects.push_back(object);
};

// 用于层级建模robot
void MeshPainter::addMesh2(TriMesh *mesh, const std::string &name, const std::string &vshader, const std::string &fshader)
{
    mesh_names.push_back(name);
    meshes.push_back(mesh);

    openGLObject object;
    // 绑定openGL对象，并传递顶点属性的数据
    bindObjectAndData2(mesh, object, vshader, fshader);

    opengl_objects.push_back(object);
};

void MeshPainter::drawMesh(TriMesh *mesh, openGLObject &object, Light *light, Camera *camera)
{

    // 相机矩阵计算
    camera->updateCamera();
    camera->viewMatrix = camera->getViewMatrix();
    camera->projMatrix = camera->getProjectionMatrix(false);

    glBindVertexArray(object.vao);
    glUseProgram(object.program);

    // 物体的变换矩阵
    glm::mat4 modelMatrix = mesh->getModelMatrix();

    // 传递矩阵
    glUniformMatrix4fv(object.modelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(object.viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
    glUniformMatrix4fv(object.projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
    // 将着色器 isShadow 设置为0，表示正常绘制的颜色，如果是1着表示阴影
    glUniform1i(object.shadowLocation, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, object.texture); // 该语句必须，否则将只使用同一个纹理进行绘制
    // 传递纹理数据 将生成的纹理传给shader
    glUniform1i(glGetUniformLocation(object.program, "texture"), 0);

    // 将材质和光源数据传递给着色器
    bindLightAndMaterial(mesh, object, light, camera);
    // 绘制
    glDrawArrays(GL_TRIANGLES, 0, mesh->getPoints().size());

    glBindVertexArray(0);

    glUseProgram(0);
};

// 让robot行走
void MeshPainter::personwalk(float frame, float &v)
{
    // 人物行走姿态模拟
    if (v > frame)
        v = 0;
    // 判断是否为第一次走动，第一次走动，手臂和脚只摆动半圈，否则需要摆动一圈并且来回摆动
    // 非第一次行走
    if (!firstwalk)
    {
        // 手脚摆动
        if (v < frame / 2)
        {
            theta[5] += 100.0 / frame;
            theta[6] -= 100.0 / frame;
            theta[1] += 100.0 / frame;
            theta[2] -= 100.0 / frame;
        }
        else
        {
            firstwalk = true;
            v = 0;
        }
    }
    else
    {
        // 在上半次和下半次的摆动的区别就是变化角度成相反数
        if (v < frame / 2)
        {
            theta[5] -= 200.0 / frame;
            theta[6] += 200.0 / frame;
            theta[1] -= 200.0 / frame;
            theta[2] += 200.0 / frame;
        }
        else if (v <= frame)
        {
            theta[5] += 200.0 / frame;
            theta[6] -= 200.0 / frame;
            theta[1] += 200.0 / frame;
            theta[2] -= 200.0 / frame;
        }
        else
            v = 0;
    }
    v += 1.0;
}

// 一、三视角切换
void MeshPainter::drawMesh2(Light *light, Camera *camera)
{
    // 相机矩阵计算
    camera->updateCamera();
    camera->viewMatrix = camera->getViewMatrix();
    camera->projMatrix = camera->getProjectionMatrix(false);

    glBindVertexArray(opengl_objects[0].vao);
    glUseProgram(opengl_objects[0].vao);

    // 物体的变换矩阵
    glm::mat4 modelMatrix = glm::mat4(1.0), M = glm::mat4(1.0);
    float leglen = 0.0;

    // 躯干
    // 如果是自由视角的移动则不控制人物的摆动 body_angle记录转换成自由视角时原本的身体朝向 lastview记录人物的位置
    if (freeview)
    {
        modelMatrix = glm::translate(modelMatrix, lastview);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(body_angle), glm::vec3(0.0, 1.0, 0.0));
    }
    // 非自由视角下
    // 人物如果在移动，则发生对应动作
    else
    {
        if (walk)
        {
            personwalk(200, walkv);
        }
        else
        {
            firstwalk = false;
            theta[5] = 0;
            theta[6] = 0;
            theta[1] = 0;
            theta[2] = 0;
            walkv = 0;
        }

        modelMatrix = glm::translate(modelMatrix, glm::vec3(camera->eye.x + 2 * camera->at.x, camera->eye.y - 1.5, camera->eye.z + 2 * camera->at.z));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(theta[0] + 180.0f - camera->rotateAngle), glm::vec3(0.0, 1.0, 0.0));
        lastview = glm::vec3(camera->eye.x + 2 * camera->at.x, camera->eye.y - 1.5, camera->eye.z + 2 * camera->at.z);
        body_angle = theta[0] + 180.0f - camera->rotateAngle;
        leglen = 0.0;
    }

    glm::mat4 instance = glm::mat4(1.0);
    instance = glm::translate(instance, glm::vec3(0.0, 0.18, 0.0));
    instance = glm::scale(instance, glm::vec3(0.7, 0.7, 0.7));
    M = modelMatrix * instance;
    meshes[0]->settModelMatrix(M);
    // 乘以来自父物体的模型变换矩阵，绘制当前物体
    // 传递矩阵
    glUniformMatrix4fv(opengl_objects[0].modelLocation, 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(opengl_objects[0].viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
    glUniformMatrix4fv(opengl_objects[0].projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
    // 将着色器 isShadow 设置为0，表示正常绘制的颜色，如果是1着表示阴影
    glUniform1i(opengl_objects[0].shadowLocation, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, opengl_objects[0].texture); // 该语句必须，否则将只使用同一个纹理进行绘制
    // 传递纹理数据 将生成的纹理传给shader
    glUniform1i(glGetUniformLocation(opengl_objects[0].program, "texture"), 0);
    // 将材质和光源数据传递给着色器
    bindLightAndMaterial(meshes[0], opengl_objects[0], light, camera);
    // 绘制
    glDrawArrays(GL_TRIANGLES, 0, meshes[0]->getPoints().size());
    matrices[index++] = modelMatrix; // 保存矩阵

    // 大腿1
    glBindVertexArray(opengl_objects[1].vao);
    glUseProgram(opengl_objects[1].vao);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.15, -0.15, 0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(theta[1]), glm::vec3(1.0, 0.0, 0.0));

    instance = glm::mat4(1.0);
    instance = glm::scale(instance, glm::vec3(0.4, 0.2, 0.4));
    M = modelMatrix * instance;
    meshes[1]->settModelMatrix(M);
    // 乘以来自父物体的模型变换矩阵，绘制当前物体
    // 传递矩阵
    glUniformMatrix4fv(opengl_objects[1].modelLocation, 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(opengl_objects[1].viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
    glUniformMatrix4fv(opengl_objects[1].projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
    // 将着色器 isShadow 设置为0，表示正常绘制的颜色，如果是1着表示阴影
    glUniform1i(opengl_objects[1].shadowLocation, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, opengl_objects[1].texture); // 该语句必须，否则将只使用同一个纹理进行绘制
    // 传递纹理数据 将生成的纹理传给shader
    glUniform1i(glGetUniformLocation(opengl_objects[1].program, "texture"), 0);
    // 将材质和光源数据传递给着色器
    bindLightAndMaterial(meshes[1], opengl_objects[1], light, camera);
    // 绘制
    glDrawArrays(GL_TRIANGLES, 0, meshes[1]->getPoints().size()); // 保存矩阵

    // 小腿1
    glBindVertexArray(opengl_objects[3].vao);
    glUseProgram(opengl_objects[3].vao);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -0.19 - leglen / 2 + 0.62 * leglen / 1.4, 0.05));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(theta[3]), glm::vec3(0.0, 1.0, 0.0));

    instance = glm::mat4(1.0);
    instance = glm::scale(instance, glm::vec3(0.4, 0.4 + leglen, 0.4));
    M = modelMatrix * instance;
    meshes[3]->settModelMatrix(M);

    glUniformMatrix4fv(opengl_objects[3].modelLocation, 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(opengl_objects[3].viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
    glUniformMatrix4fv(opengl_objects[3].projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
    glUniform1i(opengl_objects[3].shadowLocation, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, opengl_objects[3].texture);
    glUniform1i(glGetUniformLocation(opengl_objects[3].program, "texture"), 0);
    bindLightAndMaterial(meshes[3], opengl_objects[3], light, camera);
    glDrawArrays(GL_TRIANGLES, 0, meshes[3]->getPoints().size());

    // 大腿2
    modelMatrix = matrices[--index];
    matrices[index++] = modelMatrix;
    glBindVertexArray(opengl_objects[2].vao);
    glUseProgram(opengl_objects[2].vao);

    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.15, -0.15, 0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(theta[2]), glm::vec3(1.0, 0.0, 0.0));

    instance = glm::mat4(1.0);
    instance = glm::scale(instance, glm::vec3(0.4, 0.2, 0.4));
    M = modelMatrix * instance;
    meshes[2]->settModelMatrix(M);

    glUniformMatrix4fv(opengl_objects[2].modelLocation, 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(opengl_objects[2].viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
    glUniformMatrix4fv(opengl_objects[2].projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
    glUniform1i(opengl_objects[2].shadowLocation, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, opengl_objects[2].texture);
    glUniform1i(glGetUniformLocation(opengl_objects[2].program, "texture"), 0);
    bindLightAndMaterial(meshes[2], opengl_objects[2], light, camera);
    glDrawArrays(GL_TRIANGLES, 0, meshes[2]->getPoints().size());

    // 小腿2
    glBindVertexArray(opengl_objects[4].vao);
    glUseProgram(opengl_objects[4].vao);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -0.19 - leglen / 2 + 0.62 * leglen / 1.4, 0.05));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(theta[4]), glm::vec3(0.0, 1.0, 0.0));

    instance = glm::mat4(1.0);
    instance = glm::scale(instance, glm::vec3(0.4, 0.4 + leglen, 0.4));
    M = modelMatrix * instance;
    meshes[4]->settModelMatrix(M);

    glUniformMatrix4fv(opengl_objects[4].modelLocation, 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(opengl_objects[4].viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
    glUniformMatrix4fv(opengl_objects[4].projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
    glUniform1i(opengl_objects[4].shadowLocation, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, opengl_objects[4].texture);
    glUniform1i(glGetUniformLocation(opengl_objects[4].program, "texture"), 0);
    bindLightAndMaterial(meshes[4], opengl_objects[4], light, camera);
    glDrawArrays(GL_TRIANGLES, 0, meshes[4]->getPoints().size());

    // 上臂1
    modelMatrix = matrices[--index];
    matrices[index++] = modelMatrix;
    glBindVertexArray(opengl_objects[5].vao);
    glUseProgram(opengl_objects[5].vao);

    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.28, 0.25, 0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(theta[5]), glm::vec3(1.0, 0.0, 0.0));
    instance = glm::mat4(1.0);
    instance = glm::scale(instance, glm::vec3(0.3, 0.3, 0.3));

    M = modelMatrix * instance;
    meshes[5]->settModelMatrix(M);

    glUniformMatrix4fv(opengl_objects[5].modelLocation, 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(opengl_objects[5].viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
    glUniformMatrix4fv(opengl_objects[5].projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
    glUniform1i(opengl_objects[5].shadowLocation, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, opengl_objects[5].texture);
    glUniform1i(glGetUniformLocation(opengl_objects[5].program, "texture"), 0);
    bindLightAndMaterial(meshes[5], opengl_objects[5], light, camera);
    glDrawArrays(GL_TRIANGLES, 0, meshes[5]->getPoints().size());

    // 下臂1
    glBindVertexArray(opengl_objects[7].vao);
    glUseProgram(opengl_objects[7].vao);

    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -0.25, 0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(theta[7]), glm::vec3(1.0, 0.0, 0.0));

    instance = glm::mat4(1.0);
    instance = glm::scale(instance, glm::vec3(0.3, 0.4, 0.3));
    M = modelMatrix * instance;
    meshes[7]->settModelMatrix(M);

    glUniformMatrix4fv(opengl_objects[7].modelLocation, 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(opengl_objects[7].viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
    glUniformMatrix4fv(opengl_objects[7].projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
    glUniform1i(opengl_objects[7].shadowLocation, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, opengl_objects[7].texture);
    glUniform1i(glGetUniformLocation(opengl_objects[7].program, "texture"), 0);
    bindLightAndMaterial(meshes[7], opengl_objects[7], light, camera);
    glDrawArrays(GL_TRIANGLES, 0, meshes[7]->getPoints().size());

    // 上臂2
    modelMatrix = matrices[--index];
    glBindVertexArray(opengl_objects[6].vao);
    glUseProgram(opengl_objects[6].vao);

    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.28, 0.25, 0.0 - armpos));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(theta[6] + armangle), glm::vec3(1.0, 0.0, 0.0));
    instance = glm::mat4(1.0);
    instance = glm::scale(instance, glm::vec3(0.3, 0.3, 0.3));
    M = modelMatrix * instance;
    meshes[6]->settModelMatrix(M);

    glUniformMatrix4fv(opengl_objects[6].modelLocation, 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(opengl_objects[6].viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
    glUniformMatrix4fv(opengl_objects[6].projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
    glUniform1i(opengl_objects[6].shadowLocation, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, opengl_objects[6].texture);
    glUniform1i(glGetUniformLocation(opengl_objects[6].program, "texture"), 0);
    bindLightAndMaterial(meshes[6], opengl_objects[6], light, camera);
    glDrawArrays(GL_TRIANGLES, 0, meshes[6]->getPoints().size());

    // 下臂2

    glBindVertexArray(opengl_objects[8].vao);
    glUseProgram(opengl_objects[8].vao);

    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -0.25, 0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(theta[8]), glm::vec3(1.0, 0.0, 0.0));

    instance = glm::mat4(1.0);
    instance = glm::scale(instance, glm::vec3(0.3, 0.4, 0.3));
    M = modelMatrix * instance;
    meshes[8]->settModelMatrix(M);

    glUniformMatrix4fv(opengl_objects[8].modelLocation, 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(opengl_objects[8].viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
    glUniformMatrix4fv(opengl_objects[8].projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
    glUniform1i(opengl_objects[8].shadowLocation, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, opengl_objects[8].texture);
    glUniform1i(glGetUniformLocation(opengl_objects[8].program, "texture"), 0);
    bindLightAndMaterial(meshes[8], opengl_objects[8], light, camera);
    glDrawArrays(GL_TRIANGLES, 0, meshes[8]->getPoints().size());

    // 头
    modelMatrix = matrices[index];
    glBindVertexArray(opengl_objects[9].vao);
    glUseProgram(opengl_objects[9].vao);

    // 判断左键是否被点击，如果点击，头部绕y轴旋转
    if (buttonleft)
    {
        headangle += headanglew;
        if (headangle >= 360)
            headback = true;
        if (headback)
        {
            headback = false;
            buttonleft = false;
            headangle = 0.0;
        }
    }

    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.52, 0.0 - headpos));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(theta[9] + headangle), glm::vec3(0.0, 1.0, 0.0));

    instance = glm::mat4(1.0);
    instance = glm::scale(instance, glm::vec3(0.4, 0.4, 0.4));
    M = modelMatrix * instance;
    meshes[9]->settModelMatrix(M);

    glUniformMatrix4fv(opengl_objects[9].modelLocation, 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(opengl_objects[9].viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
    glUniformMatrix4fv(opengl_objects[9].projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
    glUniform1i(opengl_objects[9].shadowLocation, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, opengl_objects[9].texture);
    glUniform1i(glGetUniformLocation(opengl_objects[9].program, "texture"), 0);
    bindLightAndMaterial(meshes[9], opengl_objects[9], light, camera);
    glDrawArrays(GL_TRIANGLES, 0, meshes[9]->getPoints().size());

    glBindVertexArray(0);

    glUseProgram(0);
};

void MeshPainter::displayproj(TriMesh *mesh, openGLObject mesh_object, Camera *camera, int isShadow, Light *light)
{

    glBindVertexArray(mesh_object.vao);
    glUseProgram(mesh_object.program);

    // 物体的变换矩阵
    glm::mat4 modelMatrix = light->getShadowProjectionMatrix() * mesh->getModelMatrix();

    // 传递矩阵
    glUniformMatrix4fv(mesh_object.modelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(mesh_object.viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
    glUniformMatrix4fv(mesh_object.projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
    // 将着色器 isShadow 设置为0，表示正常绘制的颜色，如果是1着表示阴影
    glUniform1i(mesh_object.shadowLocation, isShadow);

    // 将材质和光源数据传递给着色器
    bindLightAndMaterial(mesh, mesh_object, light, camera);
    // 绘制
    glDrawArrays(GL_TRIANGLES, 0, mesh->getPoints().size());
}

void MeshPainter::displayproj2(TriMesh *mesh, openGLObject mesh_object, Camera *camera, int isShadow, Light *light)
{

    glBindVertexArray(mesh_object.vao);
    glUseProgram(mesh_object.program);

    // 物体的变换矩阵
    glm::mat4 modelMatrix = light->getShadowProjectionMatrix() * mesh->getModelMatrix2();

    // 传递矩阵
    glUniformMatrix4fv(mesh_object.modelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(mesh_object.viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
    glUniformMatrix4fv(mesh_object.projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
    // 将着色器 isShadow 设置为0，表示正常绘制的颜色，如果是1着表示阴影
    glUniform1i(mesh_object.shadowLocation, isShadow);

    // 将材质和光源数据传递给着色器
    bindLightAndMaterial(mesh, mesh_object, light, camera);
    // 绘制
    glDrawArrays(GL_TRIANGLES, 0, mesh->getPoints().size());
}

void MeshPainter::drawMeshes(Light *light, Camera *camera)
{
    for (int i = 0; i < meshes.size(); i++)
    {
        // 绘制模型
        drawMesh(meshes[i], opengl_objects[i], light, camera);
        if (i == 0 || i == 1)
            continue;
        // 绘制模型阴影 第一个和第二个是地板和天空，不作阴影绘制
        displayproj(meshes[i], opengl_objects[i], camera, 1, light);
    }
};
void MeshPainter::drawMeshes2(Light *light, Camera *camera)
{

    // 绘制模型
    drawMesh2(light, camera);
    for (int i = 0; i < meshes.size(); i++)
        // 绘制模型阴影
        displayproj2(meshes[i], opengl_objects[i], camera, 1, light);
};

void MeshPainter::cleanMeshes()
{
    // 将数据都清空释放
    mesh_names.clear();

    for (int i = 0; i < meshes.size(); i++)
    {
        meshes[i]->cleanData();

        delete meshes[i];
        meshes[i] = NULL;
        glDeleteVertexArrays(1, &opengl_objects[i].vao);
        glDeleteBuffers(1, &opengl_objects[i].vbo);
        glDeleteProgram(opengl_objects[i].program);
    }

    meshes.clear();
    opengl_objects.clear();
};

void MeshPainter::displayleft()
{
}

void MeshPainter::load_texture_STBImage(const std::string &file_name, GLuint &texture)
{
    // 读取纹理图片，并将其传递给着色器

    int width, height, channels = 0;
    unsigned char *pixels = NULL;
    // 读取图片的时候先翻转一下图片，如果不设置的话显示出来是反过来的图片
    stbi_set_flip_vertically_on_load(true);
    // 读取图片数据
    pixels = stbi_load(file_name.c_str(), &width, &height, &channels, 0);

    // 调整行对齐格式
    if (width * channels % 4 != 0)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLenum format = GL_RGB;
    // 设置通道格式
    switch (channels)
    {
    case 1:
        format = GL_RED;
        break;
    case 3:
        format = GL_RGB;
        break;
    case 4:
        format = GL_RGBA;
        break;
    default:
        format = GL_RGB;
        break;
    }

    // 绑定纹理对象
    glBindTexture(GL_TEXTURE_2D, texture);

    // 指定纹理的放大，缩小滤波，使用线性方式，即当图片放大的时候插值方式
    // 将图片的rgb数据上传给opengl
    glTexImage2D(
        GL_TEXTURE_2D,    // 指定目标纹理，这个值必须是GL_TEXTURE_2D
        0,                // 执行细节级别，0是最基本的图像级别，n表示第N级贴图细化级别
        format,           // 纹理数据的颜色格式(GPU显存)
        width,            // 宽度。早期的显卡不支持不规则的纹理，则宽度和高度必须是2^n
        height,           // 高度。早期的显卡不支持不规则的纹理，则宽度和高度必须是2^n
        0,                // 指定边框的宽度。必须为0
        format,           // 像素数据的颜色格式(CPU内存)
        GL_UNSIGNED_BYTE, // 指定像素数据的数据类型
        pixels            // 指定内存中指向图像数据的指针
    );

    // 生成多级渐远纹理，多消耗1/3的显存，较小分辨率时获得更好的效果
    // glGenerateMipmap(GL_TEXTURE_2D);

    // 指定插值方法
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // 恢复初始对齐格式
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    // 释放图形内存
    stbi_image_free(pixels);
};
/*MatrixStack::MatrixStack(int numMatrices = 100) :_index(0), _size(numMatrices)
{
    _matrices = new glm::mat4[numMatrices];
}

MatrixStack::~MatrixStack()
{
    delete[]_matrices;
}*/
