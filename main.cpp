#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<stb/stb_image.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdlib>
#include<ctime>
#include<vector>
#include<math.h>
#include <chrono>
#include <thread>
#include <array>

#include"Texture.h"
#include"shaderClass.h"
#include"VAO.h"
#include"VBO.h"
#include"EBO.h"
#include"Camera.h"
#include "FrustumCulling.h"



const unsigned int width = 1920;
const unsigned int height = 1080;
const float M_PI = 3.14159265359f;

GLfloat lightVertices[] =
{ //     COORDINATES     //
	-0.1f, -0.1f,  0.1f,
	-0.1f, -0.1f, -0.1f,
	 0.1f, -0.1f, -0.1f,
	 0.1f, -0.1f,  0.1f,
	-0.1f,  0.1f,  0.1f,
	-0.1f,  0.1f, -0.1f,
	 0.1f,  0.1f, -0.1f,
	 0.1f,  0.1f,  0.1f
};

GLuint lightIndices[] =
{
	0, 1, 2,
	0, 2, 3,
	0, 4, 7,
	0, 7, 3,
	3, 7, 6,
	3, 6, 2,
	2, 6, 5,
	2, 5, 1,
	1, 5, 4,
	1, 4, 0,
	4, 5, 6,
	4, 6, 7
};


int main()
{
	srand(static_cast<unsigned int>(time(0)));
	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object of 800 by 800 pixels, naming it "YoutubeOpenGL"
	GLFWwindow* window = glfwCreateWindow(width, height, "YoutubeOpenGL", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
	glViewport(0, 0, width, height);


	// Generates Shader object using shaders default.vert and default.frag
	Shader shaderProgram("default.vert", "default.frag");
								
	// --- Sphere mesh generation: only generate one sphere mesh ---
	// This creates the geometry for a single sphere, which will be reused for all 1000 spheres
	// The Vertex struct is defined above
	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 texCoord;
		glm::vec3 normal;
	};
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	int sectorCount = 15;
	int stackCount = 15;
	float radius = 1.0f;
	float x, y, z, xy;
	float nx, ny, nz, lengthInv = 1.0f / radius;
	float s, t;
	float sectorStep = 2 * M_PI / sectorCount;
	float stackStep = M_PI / stackCount;
	float sectorAngle, stackAngle;
	for (int i = 0; i <= stackCount; ++i) {
		stackAngle = M_PI / 2 - i * stackStep;
		xy = radius * cosf(stackAngle);
		z = radius * sinf(stackAngle);
		for (int j = 0; j <= sectorCount; ++j) {
			sectorAngle = (j * sectorStep) / 2;
			x = xy * cosf(sectorAngle);
			y = xy * sinf(sectorAngle);
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;
			s = (float)j / sectorCount;
			t = (float)i / stackCount;
			vertices.push_back(Vertex{
				glm::vec3(x, y, z),
				glm::vec3(1.0f, 1.0f, 1.0f),
				glm::vec2(s, t),
				glm::vec3(nx, ny, nz)
			});
		}
	}
	for (int i = 0; i < stackCount; ++i) {
		int k1 = i * (sectorCount + 1);
		int k2 = k1 + sectorCount + 1;
		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
			if (i != 0) {
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}
			if (i != (stackCount - 1)) {
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}

	// --- Generate random positions for spheres (instance data) ---
	std::vector<glm::vec3> spherePositions;
	int numSpheres = 500000; // You can adjust this for performance
	for (int i = 0; i < numSpheres; ++i) {
		// Random position in a cube region [-10, 10] for x, y, z
		/*float x = static_cast<float>(tan(rand() % 500 - 100));
		float y = static_cast<float>(tan(rand() % 500 - 100));
		float z = static_cast<float>(tan(rand() % 500 - 100));*/
		float x = static_cast<float>((rand() % 200 - 100) * tan(rand() % 100));
		float y = static_cast<float>((rand() % 200 - 100) * tan(rand() % 100));
		float z = static_cast<float>((rand() % 200 - 100) * tan(rand() % 100));
		spherePositions.push_back(glm::vec3(x, y, z));
	}

	VBO sphereVBO(vertices.data(), vertices.size() * sizeof(Vertex));
	VAO circleVAO;
	EBO sphereEBO(indices.data(), indices.size() * sizeof(unsigned int));
	circleVAO.Bind();
	sphereVBO.Bind();
	sphereEBO.Bind();


	GLsizei stride = sizeof(Vertex);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	// aColor (location = 1)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// aTex (location = 2)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// aNormal (location = 3)
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// Unbind all to prevent accidentally modifying them

	circleVAO.Unbind();
	sphereVBO.Unbind();
	sphereEBO.Unbind();

	// Shader for light cube
	Shader lightShader("light.vert", "light.frag");
	// Generates Vertex Array Object and binds it
	VAO lightVAO;
	lightVAO.Bind();
	// Generates Vertex Buffer Object and links it to vertices
	VBO lightVBO(lightVertices, sizeof(lightVertices));
	// Generates Element Buffer Object and links it to indices
	EBO lightEBO(lightIndices, sizeof(lightIndices));
	// Links VBO attributes such as coordinates and colors to VAO
	lightVAO.LinkAttrib(lightVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
	// Unbind all to prevent accidentally modifying them
	lightVAO.Unbind();
	lightVBO.Unbind();
	lightEBO.Unbind();



	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	// Move the light to the center
	glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 lightModel = glm::mat4(1.0f);
	lightModel = glm::translate(lightModel, lightPos);

	//glm::vec3 pyramidPos = glm::vec3(0.0f, 0.0f, 0.0f);
	//glm::mat4 pyramidModel = glm::mat4(1.0f);
	//pyramidModel = glm::translate(pyramidModel, pyramidPos);


	lightShader.Activate();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
	glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	shaderProgram.Activate();
	//glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(pyramidModel));
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);


	/*Texture brickTex("brick.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
	brickTex.texUnit(shaderProgram, "tex0", 0);*/

	// Original code from the tutorial
	Texture brickTex("obama_512.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
	brickTex.Bind();
	brickTex.texUnit(shaderProgram, "tex0", 0);
	brickTex.Unbind();



	// Enables the Depth Buffer
	glEnable(GL_DEPTH_TEST);

	// Creates camera object
	Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));

	// --- Set up instance VBO for sphere positions (vec3) ---
	GLuint instanceVBO;
	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	// Initialize with all sphere positions
	// Note: Using GL_DYNAMIC_DRAW for frequent updates
	// This allows us to update the instance data each frame
	glBufferData(GL_ARRAY_BUFFER, numSpheres * sizeof(glm::vec3), spherePositions.data(), GL_DYNAMIC_DRAW); // changed to DYNAMIC

    circleVAO.Bind();
    glEnableVertexAttribArray(4); // location 4 for instance position
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glVertexAttribDivisor(4, 1); // Advance per instance
    circleVAO.Unbind();
    glBindBuffer(GL_ARRAY_BUFFER, 0);


	double prevTime = 0.0;
	double crntTime = 0.0;
	double timeDiff = 0.0;
	unsigned int counter = 0;
	// Main while loop
	while (!glfwWindowShouldClose(window))
	{
		crntTime = glfwGetTime();
		timeDiff = crntTime - prevTime;
		counter++;
		if (timeDiff >= 1.0 / 30.0)
		{
			// Create new title
			std::string FPS = std::to_string((1.0 / timeDiff) * counter);
			std::string ms = std::to_string((timeDiff / counter) * 1000);
			std::string newTitle = "1 Million spheres - " + FPS + "FPS / " + ms + "ms";
			glfwSetWindowTitle(window, newTitle.c_str());

			// Resets times and counter
			prevTime = crntTime;
			counter = 0;
		}
		// Specify the color of the background
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		// Clean the back buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Handles camera inputs
		camera.Inputs(window);
		// Updates and exports the camera matrix to the Vertex Shader
		camera.updateMatrix(45.0f, 1.0f, 500.0f);

		// --- Frustum culling: build visible sphere list ---
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 500.0f);
        std::array<FrustumPlane, 6> frustum;
        ExtractFrustumPlanes(proj * view, frustum);
        std::vector<glm::vec3> visibleSpheres;
        visibleSpheres.reserve(numSpheres / 10); // heuristic
        for (const auto& pos : spherePositions) {
            if (SphereInFrustum(frustum, pos, radius)) {
                visibleSpheres.push_back(pos);
            }
        }
        int numVisible = static_cast<int>(visibleSpheres.size());
        // Update instance VBO with only visible spheres
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, numVisible * sizeof(glm::vec3), visibleSpheres.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);


		// Tell OpenGL which Shader Program we want to use
		shaderProgram.Activate();
		// Updates and exports the camera matrix to the Vertex Shader
		glUniform3f(glGetUniformLocation(shaderProgram.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		camera.Matrix(shaderProgram, "camMatrix");

		// --- Pass time as a uniform for animation in the vertex shader ---
		//glUniform1f(glGetUniformLocation(shaderProgram.ID, "uTime"), static_cast<float>(glfwGetTime()));


		glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

		brickTex.Bind();
		circleVAO.Bind();
		// --- Instanced rendering: draw only visible spheres ---
		glDrawElementsInstanced(
			GL_TRIANGLES,
			static_cast<GLsizei>(indices.size()),
			GL_UNSIGNED_INT,
			0,
			numVisible // only visible
		);


		// Tells OpenGL which Shader Program we want to use
		lightShader.Activate();
		// Export the camMatrix to the Vertex Shader of the light cube
		camera.Matrix(lightShader, "camMatrix");
		// Bind the VAO so OpenGL knows to use it
		lightVAO.Bind();
		// Draw primitives, number of indices, datatype of indices, index of indices
		glDrawElements(GL_TRIANGLES, sizeof(lightIndices) / sizeof(int), GL_UNSIGNED_INT, 0);


		// --- Animate the light source (cube) in a circle ---
		float lightAngle = static_cast<float>(glfwGetTime()) * 0.5f; // Light rotation speed
		float lightRadius = 2.0f; // Distance from center
		// Light moves in XZ plane
		lightPos = glm::vec3(
			cos(lightAngle) * lightRadius,
			0.0f,
			sin(lightAngle) * lightRadius
		);
		lightModel = glm::mat4(1.0f);
		lightModel = glm::translate(lightModel, lightPos);
		// Update uniforms for light position
		lightShader.Activate();
		glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
		glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		shaderProgram.Activate();
		glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);


		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();
	}

	shaderProgram.Delete();
	lightVAO.Delete();
	lightVBO.Delete();
	lightEBO.Delete();
	lightShader.Delete();
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}