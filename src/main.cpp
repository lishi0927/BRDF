// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <vector>
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "ground.h"
#include "shader.hpp"
#include "texture.hpp"
#include "controls.hpp"
#include "objloader.hpp"

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

#define COOK_Torrance

#ifdef DIFFUSE_LIGHT
#include "diffuselight.h"

#endif

#ifdef SPECULAR_LIGHT
#include "specularlight.h"
#endif

#ifdef COOK_Torrance
#include "brdf_cooktorrance.h"
float m_scale;
#endif

Texture* m_pTexture;

GLuint VertexArrayID;
GLuint vertexbuffer;
GLuint uvbuffer;
GLuint normalbuffer;
int verticenumber;


static const float FieldDepth = 20.0f;
static const float FieldWidth = 10.0f;

GLuint m_VBO;

void CreateVertexBuffer()
{
	const glm::vec3 Normal = glm::vec3(0.0, 1.0f, 0.0f);

	Vertex Vertices[6] = {
		Vertex(glm::vec3(0.0f, 0.0f, 0.0f),             glm::vec2(0.0f, 0.0f), Normal),
		Vertex(glm::vec3(0.0f, 0.0f, FieldDepth),       glm::vec2(0.0f, 1.0f), Normal),
		Vertex(glm::vec3(FieldWidth, 0.0f, 0.0f),       glm::vec2(1.0f, 0.0f), Normal),

		Vertex(glm::vec3(FieldWidth, 0.0f, 0.0f),       glm::vec2(1.0f, 0.0f), Normal),
		Vertex(glm::vec3(0.0f, 0.0f, FieldDepth),       glm::vec2(0.0f, 1.0f), Normal),
		Vertex(glm::vec3(FieldWidth, 0.0f, FieldDepth), glm::vec2(1.0f, 1.0f), Normal)
	};

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

void init()
{
	LightInit();
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	CreateVertexBuffer();


	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	/*

	// Load the texture
	// TextureI = loadDDS("uvmap.DDS");
	// Read our .obj file
	 std::vector<glm::vec3> vertices;
	 std::vector<glm::vec2> uvs;
	 std::vector<glm::vec3> normals;
	bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);

	// Load it into a VBO
	verticenumber = vertices.size();
	
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	vertices.clear();
	uvs.clear();
	normals.clear();
	*/
#ifdef DIFFUSE_LIGHT
	Diffuseshader.shader = LoadShaders("shader/diffuse_vert.glsl", "shader/diffuse_fragment.glsl");

	if (!Diffuseshader.Init())
	{
		std::cout << "DiffuseShader ERROR!" << std::endl;
		exit(-1);
	}

	Diffuseshader.Enable();

	Diffuseshader.SetTextureUnit(0);
#endif

#ifdef SPECULAR_LIGHT
	SpecularShader.shader = LoadShaders("shader/specular_vert.glsl", "shader/specular_fragment.glsl");
	if (!SpecularShader.Init())
	{
		std::cout << "SpecularShader ERROR!" << std::endl;
		exit(-1);
	}

	SpecularShader.Enable();

	SpecularShader.SetTextureUnit(0);
#endif

#ifdef COOK_Torrance
	m_scale = 0;

	CookShader.shader = LoadShaders("shader/cooktorrance_vert.glsl", "shader/cooktorrance_fragment.glsl");
	if (!CookShader.Init())
	{
		std::cout << "CookTorrance Error!" << std::endl;
		exit(-1);
	}

	CookShader.Enable();
	CookShader.SetTextureUnit(0);

#endif



	m_pTexture = new Texture(GL_TEXTURE_2D, "test.png");

	if (!m_pTexture->Load()) {
		std::cout << "Texture ERROR!" << std::endl;
		exit(-1);
	}
	
}


void render()
{
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	

	// Compute the MVP matrix from keyboard and mouse input
	computeMatricesFromInputs();
	glm::mat4 ProjectionMatrix = getProjectionMatrix();
	glm::mat4 ViewMatrix = getViewMatrix();
	glm::mat4 ModelMatrix = glm::mat4(1.0);
	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
	glm::vec3 camerapos = getCameraPosition();

#ifdef DIFFUSE_LIGHT
	Diffuseshader.Enable();
	Diffuseshader.SetWVP(MVP);
	Diffuseshader.SetWorldMatrix(ModelMatrix);
	Diffuseshader.SetDirectionalLight(m_directionalLight);
#endif
#ifdef SPECULAR_LIGHT
	SpecularShader.Enable();
	SpecularShader.SetWVP(MVP);
	SpecularShader.SetWorldMatrix(ModelMatrix);
	SpecularShader.SetDirectionalLight(m_directionalLight);
	SpecularShader.SetEyeWorldPos(camerapos);
	SpecularShader.SetMatSpecularIntensity(1.0f);
	SpecularShader.SetMatSpecularPower(32);
#endif

#ifdef COOK_Torrance
	m_scale += 0.0057f;
	PointLight pl[2];
	pl[0].DiffuseIntensity = 0.5f;
	pl[0].Color = glm::vec3(1.0f, 0.5f, 0.0f);
	pl[0].Position = glm::vec3(3.0f, 1.0f, 20 * (cosf(m_scale) + 1.0f) / 2.0f);
	pl[0].Attenuation.Linear = 0.1f;
	pl[1].DiffuseIntensity = 0.5f;
	pl[1].Color = glm::vec3(0.0f, 0.5f, 1.0f);
	pl[1].Position = glm::vec3(7.0f, 1.0f, 20 * (sinf(m_scale) + 1.0f) / 2.0f);
	pl[1].Attenuation.Linear = 0.1f;


//	CookShader.Enable();
	CookShader.SetPointLights(2, pl);
	CookShader.SetWVP(MVP);
	CookShader.SetWorldMatrix(ModelMatrix);
	CookShader.SetDirectionalLight(m_directionalLight);
	CookShader.SetEyeWorldPos(camerapos);
	CookShader.SetRoughness(0.3);
	CookShader.SetFresnel(0.8);
	CookShader.SetGK(0.2);
#endif

//	m_pTexture->Bind(GL_TEXTURE0);


/*
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
		1,                                // attribute
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(
		2,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// Draw the triangles !
	glDrawArrays(GL_TRIANGLES, 0, verticenumber);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
*/
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);
	m_pTexture->Bind(GL_TEXTURE0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS) return;

	switch (key) {
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GL_TRUE);
		break;
	case GLFW_KEY_A:
		m_directionalLight.AmbientIntensity += 0.05f;
		break;
	case GLFW_KEY_S:
		m_directionalLight.AmbientIntensity -= 0.05f;
		break;
	case GLFW_KEY_Z:
		m_directionalLight.DiffuseIntensity += 0.05f;
		break;
	case GLFW_KEY_X:
		m_directionalLight.DiffuseIntensity -= 0.05f;
		break;
	}
}

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial 01", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}


	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, keyCallback);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	init();

	do {
		// Clear the screen. It's not mentioned before Tutorial 02, but it can cause flickering, so it's there nonetheless.
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw nothing, see you in tutorial 2 !

		render();

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwWindowShouldClose(window) == 0);

	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	delete m_pTexture;

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

