//Author Matthew Reilly

#pragma comment(linker, "/NODEFAULTLIB:MSVCRT")

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void errorCallbackGLFW(int error, const char* description);
void hintsGLFW();
void endProgram();
void update(GLfloat currentTime);
void setupRender();
void onResizeCallback(GLFWwindow* window, int w, int h);
void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void onMouseMoveCallback(GLFWwindow* window, double x, double y);
void onMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset);
void debugGL();
static void APIENTRY openGLDebugCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam);
string readShader(string name);
void checkErrorShader(GLuint shader);
void readObj(string name, struct modelObject *obj);
void readTexture(string name, GLuint texture);


// VARIABLES
GLFWwindow*		window;
int				windowWidth = 1280;
int				windowHeight = 720;
bool			running = true;
glm::mat4		proj_matrix;
glm::vec3		modelAngle = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3		modelDisp = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3		cameraPosition = glm::vec3(0.0f, 0.5f, 2.0f);
float			cameraAdd = 2;
glm::vec3		cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float           aspect = (float)windowWidth / (float)windowHeight;
float			fovy = 45.0f;
bool			keyStatus[1024];
GLfloat			deltaTime = 0.0f;
GLfloat			lastTime = 0.0f;
GLuint			fullTransformUniformLocation;
// FPS camera variables
GLfloat			yaw = -90.0f;	// init pointing to inside
GLfloat			pitch = 0.0f;	// start centered
GLfloat			lastX = (GLfloat)windowWidth / 2.0f;	// start middle screen
GLfloat			lastY = (GLfloat)windowHeight / 2.0f;	// start middle screen
bool			firstMouse = true;
bool cameraBack = false;
double previousTime = 0;
bool end = false;
float translateAdd = -2.5;
float rotateAdd = 0;
bool click = false;
// OBJ Variables
struct modelObject {
	std::vector < glm::vec3 > out_vertices;
	std::vector < glm::vec2 > out_uvs;
	std::vector < glm::vec3 > out_normals;
	GLuint*		texture;
	GLuint      program;
	GLuint      vao;
	GLuint      buffer[2];
	GLint       mv_location;
	GLint       proj_location;
	GLint		tex_location;

	// extra variables for this example
	GLuint		matColor_location;
	GLuint		lightColor_location;

} object1, object2, object3, object4, object5, object6, object7, object8, object9,object10;

glm::vec3		*modelPositions;
glm::vec3		*modelRotations;


// Light
bool			movingLight = true;
glm::vec3		lightDisp = glm::vec3(-1.0f, -1.0f, 0.0f);
glm::vec3		lightFront = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3		ia = glm::vec3(0.0f, 0.0f, 0.8f);
GLfloat			ka = 0.05f;
glm::vec3		id = glm::vec3(0.93f, 0.75f, 0.32f);
GLfloat			kd = 1.0f;
glm::vec3		is = glm::vec3(1.00f, 1.00f, 1.0f);
GLfloat			ks = 0.01f;
bool lightOn = true;
glm::vec3		*lightPositions;
glm::vec3		*lightRotations;
void startup(modelObject* obj, std::string filename);
void render(GLfloat currentTime, modelObject* obj, bool animation);
int main()
{
	if (!glfwInit()) {							// Checking for GLFW
		cout << "Could not initialise GLFW...";
		return 0;
	}

	glfwSetErrorCallback(errorCallbackGLFW);	// Setup a function to catch and display all GLFW errors.

	hintsGLFW();								// Setup glfw with various hints.		

	// Start a window using GLFW
	string title = "My OpenGL Application";

	// Fullscreen
	//const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	//windowWidth = mode->width; windowHeight = mode->height;
	//window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), glfwGetPrimaryMonitor(), NULL);

	// Window
	window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), NULL, NULL);
	if (!window) {								// Window or OpenGL context creation failed
		cout << "Could not initialise GLFW...";
		endProgram();
		return 0;
	}

	glfwMakeContextCurrent(window);				// making the OpenGL context current

												// Start GLEW (note: always initialise GLEW after creating your window context.)
	glewExperimental = GL_TRUE;					// hack: catching them all - forcing newest debug callback (glDebugMessageCallback)
	GLenum errGLEW = glewInit();
	if (GLEW_OK != errGLEW) {					// Problems starting GLEW?
		cout << "Could not initialise GLEW...";
		endProgram();
		return 0;
	}

	debugGL();									// Setup callback to catch openGL errors.	

	// Setup all the message loop callbacks.
	glfwSetWindowSizeCallback(window, onResizeCallback);		// Set callback for resize
	glfwSetKeyCallback(window, onKeyCallback);					// Set Callback for keys
	glfwSetMouseButtonCallback(window, onMouseButtonCallback);	// Set callback for mouse click
	glfwSetCursorPosCallback(window, onMouseMoveCallback);		// Set callback for mouse move
	glfwSetScrollCallback(window, onMouseWheelCallback);		// Set callback for mouse wheel.
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);	// Set mouse cursor. Fullscreen
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	// Set mouse cursor FPS.

	//startup each object.
	setupRender();								// setup some render variables.
	startup(&object1, "pavementRight.obj");// Setup all necessary information for startup (aka. load texture, shaders, models, etc).
	startup(&object2, "wallRight.obj");
	startup(&object3, "road.obj");
	startup(&object4, "wallLeft.obj");
	startup(&object5, "pavementLeft.obj");
	startup(&object6, "lamp1.obj");
	startup(&object7, "poster.obj");
	startup(&object8, "tumbleweed.obj");
	startup(&object9, "wallBack.obj");


	do {										// run until the window is closed
		GLfloat currentTime = (GLfloat)glfwGetTime();		// retrieve timelapse
		deltaTime = currentTime - lastTime;		// Calculate delta time
		lastTime = currentTime;					// Save for next frame calculations.
		glfwPollEvents();						// poll callbacks
		update(currentTime);
		glViewport(0, 0, windowWidth, windowHeight);

		// Clear colour buffer
		glm::vec4 backgroundColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); glClearBufferfv(GL_COLOR, 0, &backgroundColor[0]);

		// Clear Deep buffer
		static const GLfloat one = 1.0f; glClearBufferfv(GL_DEPTH, 0, &one);

		// Enable blend
		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		//render each object
		render(currentTime, &object1,false);
		render(currentTime, &object2, false);
		render(currentTime, &object3, false);
		render(currentTime, &object4, false);
		render(currentTime, &object5, false);
		render(currentTime, &object6, false);
		render(currentTime, &object7, false);
		render(currentTime, &object8, true);
		render(currentTime, &object9, false);

		// update (physics, animation, structures, etc)
							// call render function.

		glfwSwapBuffers(window);				// swap buffers (avoid flickering and tearing)

		running &= (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE);	// exit if escape key pressed
		running &= (glfwWindowShouldClose(window) != GL_TRUE);
	} while (running);

	endProgram();			// Close and clean everything up...

	cout << "\nPress any key to continue...\n";
	cin.ignore(); cin.get(); // delay closing console to read debugging errors.

	return 0;
}

void errorCallbackGLFW(int error, const char* description) {
	cout << "Error GLFW: " << description << "\n";
}

void hintsGLFW() {
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);			// Create context in debug mode - for debug message callback
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
}

void endProgram() {
	glfwMakeContextCurrent(window);		// destroys window handler
	glfwTerminate();	// destroys all windows and releases resources.

	// tidy heap memory
	delete[] object1.texture;
	delete[] object2.texture;
	delete[] object3.texture;
	delete[] object4.texture;
	delete[] object5.texture;
	delete[] object6.texture;
	delete[] object7.texture;
	delete[] object8.texture;
	delete[] object9.texture;
	delete[] object10.texture;
	delete[] modelPositions;
	delete[] modelRotations;
}

void setupRender() {
	glfwSwapInterval(1);	// Ony render when synced (V SYNC)

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 0);
	glfwWindowHint(GLFW_STEREO, GL_FALSE);
}

void startup(modelObject *obj, string filename) {

	// Load main object model and shaders

	// --------------Main Model---------------------
	(*obj).program = glCreateProgram();

	string vs_text = readShader("vs_model.glsl");  const char* vs_source = vs_text.c_str();
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_source, NULL);
	glCompileShader(vs);
	checkErrorShader(vs);
	glAttachShader((*obj).program, vs);

	string fs_text = readShader("fs_model.glsl");  const char* fs_source = fs_text.c_str();
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_source, NULL);
	glCompileShader(fs);
	checkErrorShader(fs);
	glAttachShader((*obj).program, fs);

	glLinkProgram((*obj).program);
	glUseProgram((*obj).program);


	readObj(filename, obj);



	glGenVertexArrays(1, &(*obj).vao);			// Create Vertex Array Object
	glBindVertexArray((*obj).vao);				// Bind VertexArrayObject

	glGenBuffers(3, (*obj).buffer);			// Create new buffers (Vertices, Texture Coordinates, Normals)
	glBindBuffer(GL_ARRAY_BUFFER, (*obj).buffer[0]);	// Bind Buffer Vertex
	glBufferStorage(GL_ARRAY_BUFFER, (*obj).out_vertices.size() * sizeof(glm::vec3), &(*obj).out_vertices[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, (*obj).buffer[1]);	// Bind Buffer UV
	glBufferStorage(GL_ARRAY_BUFFER, (*obj).out_uvs.size() * sizeof(glm::vec2), &(*obj).out_uvs[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, (*obj).buffer[2]);	// Bind Buffer Normals
	glBufferStorage(GL_ARRAY_BUFFER, (*obj).out_normals.size() * sizeof(glm::vec3), &(*obj).out_normals[0], GL_DYNAMIC_STORAGE_BIT);

	glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);		// Vertices									
	glBindVertexBuffer(0, (*obj).buffer[0], 0, sizeof(GLfloat) * 3);
	glVertexAttribBinding(0, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 0);			// UV									
	glBindVertexBuffer(1, (*obj).buffer[1], 0, sizeof(GLfloat) * 2);
	glVertexAttribBinding(1, 1);
	glEnableVertexAttribArray(1);

	glVertexAttribFormat(2, 3, GL_FLOAT, GL_FALSE, 0);			// Normals									
	glBindVertexBuffer(2, (*obj).buffer[2], 0, sizeof(GLfloat) * 3);
	glVertexAttribBinding(2, 2);
	glEnableVertexAttribArray(2);

	glBindVertexArray((*obj).vao);				// Bind VertexArrayObject

	(*obj).mv_location = glGetUniformLocation((*obj).program, "mv_matrix");
	(*obj).proj_location = glGetUniformLocation((*obj).program, "proj_matrix");
	(*obj).tex_location = glGetUniformLocation((*obj).program, "tex");
	(*obj).lightColor_location = glGetUniformLocation((*obj).program, "ia");
	(*obj).lightColor_location = glGetUniformLocation((*obj).program, "ka");





	modelPositions = new glm::vec3[3];
		modelPositions[0] = glm::vec3(0.0f, 0.0f, 0.0f);
		modelPositions[1] = glm::vec3(0.0f, 0.05f, -4.0f);
		modelPositions[2] = glm::vec3(-0.15f, 0.05f, -4.0f);



	modelRotations = new glm::vec3[3];
	modelRotations[0] = glm::vec3(0.0f, 30.0f, 0.0f);
	modelRotations[1] = glm::vec3(0.0f, 30.0f, 0.0f);
	modelRotations[2] = glm::vec3(0.0f, 30.0f, 0.0f);


	lightPositions = new glm::vec3[2];
	lightPositions[0] = glm::vec3(0.0f, 3.50f, 0.0f);
	lightPositions[1] = glm::vec3(0.25f, 1.5f, -0.75f);


	lightRotations = new glm::vec3[2];
	lightRotations[0] = glm::vec3(0.0f, 30.0f, 0.0f);
	lightRotations[1] = glm::vec3(0.0f, 30.0f, 0.0f);




	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Calculate proj_matrix for the first time.
	aspect = (float)windowWidth / (float)windowHeight;
	proj_matrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
}

void update(GLfloat currentTime) {

	// calculate movement
	GLfloat cameraSpeed = 1.0f * deltaTime;
	if (keyStatus[GLFW_KEY_W]) cameraPosition += cameraSpeed * cameraFront;
	if (keyStatus[GLFW_KEY_S]) cameraPosition -= cameraSpeed * cameraFront;
	if (keyStatus[GLFW_KEY_A]) cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (keyStatus[GLFW_KEY_D]) cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

	if (keyStatus[GLFW_KEY_L] && (movingLight == false)) {
		cout << "Change mode to moving light...\n";
		movingLight = true;
	}
	if (keyStatus[GLFW_KEY_M] && (movingLight == true)) {
		cout << "Change mode to moving object...\n";
		movingLight = false;
	}

	if (movingLight == false) {		// moving object rotation and z displacement
		if (keyStatus[GLFW_KEY_LEFT])			modelAngle.y += 0.05f;
		if (keyStatus[GLFW_KEY_RIGHT])			modelAngle.y -= 0.05f;
		if (keyStatus[GLFW_KEY_UP])				modelAngle.x += 0.05f;
		if (keyStatus[GLFW_KEY_DOWN])			modelAngle.x -= 0.05f;
		if (keyStatus[GLFW_KEY_KP_ADD])			modelDisp.z += 0.10f;
		if (keyStatus[GLFW_KEY_KP_SUBTRACT])	modelDisp.z -= 0.10f;
	}
	else {							// moving light displacement x y z
		if (keyStatus[GLFW_KEY_LEFT])			lightDisp.x -= 0.05f;
		if (keyStatus[GLFW_KEY_RIGHT])			lightDisp.x += 0.05f;
		if (keyStatus[GLFW_KEY_UP])				lightDisp.y += 0.05f;
		if (keyStatus[GLFW_KEY_DOWN])			lightDisp.y -= 0.05f;
		if (keyStatus[GLFW_KEY_KP_ADD])			lightDisp.z += 0.05f;
		if (keyStatus[GLFW_KEY_KP_SUBTRACT])	lightDisp.z -= 0.05f;
	}

}

void render(GLfloat currentTime, modelObject *obj, bool animation) {

	//if an animated object eg. tumbleweed.
	if (animation == true) {
	//if the tumbleweed rolls off screen reset to orginal position
			if (translateAdd > 4) {
				modelPositions[1] = glm::vec3(0.0f, 0.05f, -4.0f);
				modelPositions[2] = glm::vec3(-0.15f, 0.05f, -4.0f);
				translateAdd = -2.5;
				rotateAdd = 0;
			};
			//tranform the tumbleweed each time it is rendered.
			translateAdd += 0.004;
			rotateAdd += 0.08;
			modelPositions[1] = glm::vec3(0.0f, 0.05f, translateAdd);
			modelRotations[1] = glm::vec3(rotateAdd, 0.0f, 30.0f);
			modelPositions[2] = glm::vec3(-0.15f, 0.05f, (translateAdd-1));
			modelRotations[2] = glm::vec3(rotateAdd, 0.05f, 30.0f);
	
			
	};
	//movement of the camera.
	if (cameraBack == false) {
		cameraAdd -= 0.0001;
	}
	if(cameraBack == true){
		cameraAdd += 0.0001;
	}
	if (cameraAdd < -2) {
		cameraBack = true;
	}
	if (cameraAdd > 2) {
		cameraBack = false;
	}

	glm::vec3		cameraPosition = glm::vec3(0.0f, 0.5f, cameraAdd);
	// ----------draw main model------------
	
		glUseProgram((*obj).program);
		glBindVertexArray((*obj).vao);
		glUniformMatrix4fv((*obj).proj_location, 1, GL_FALSE, &proj_matrix[0][0]);

		glUniform4f(glGetUniformLocation((*obj).program, "viewPosition"), cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0);
		//Lighting of each object.
			
				glUniform4f(glGetUniformLocation((*obj).program, "ia"), ia.r, ia.g, ia.b, 2.0);
				glUniform1f(glGetUniformLocation((*obj).program, "ka"), ka);
				glUniform4f(glGetUniformLocation((*obj).program, "id"), id.r, id.g, id.b, 1.0);
				glUniform1f(glGetUniformLocation((*obj).program, "kd"), 1.0f);
				glUniform4f(glGetUniformLocation((*obj).program, "is"), is.r, is.g, is.b, 1.0);
				glUniform1f(glGetUniformLocation((*obj).program, "ks"), 1.0f);
				glUniform1f(glGetUniformLocation((*obj).program, "shininess"), 100.0f);

				glUniform1f(glGetUniformLocation((*obj).program, "lightConstant"), 0.50f);
				glUniform1f(glGetUniformLocation((*obj).program, "lightLinear"), 0.022f);
				glUniform1f(glGetUniformLocation((*obj).program, "lightQuadratic"), 0.0019f);


				glUniform4f(glGetUniformLocation((*obj).program, "lightPosition"), lightPositions[1].x, lightPositions[1].y, lightPositions[1].z, 1.0);
				glUniform4f(glGetUniformLocation((*obj).program, "lightSpotDirection"), lightFront.x, lightFront.y, lightFront.z, 0.0);
				glUniform1f(glGetUniformLocation((*obj).program, "lightSpotCutOff"), glm::cos(glm::radians(45.5f)));
				glUniform1f(glGetUniformLocation((*obj).program, "lightSpotOuterCutOff"), glm::cos(glm::radians(60.0f)));


			
			// Bind textures and samplers - using 0 as I know there is only one texture - need to extend.
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, (*obj).texture[0]);
			glUniform1i((*obj).tex_location, 0);
		
	glm::mat4 viewMatrix = glm::lookAt(cameraPosition,					// eye
		cameraPosition + cameraFront,	// centre
		cameraUp);						// up

	//draw each object.
	glm::mat4 modelMatrix;
		if (animation == true) {
			//render tumberweeds
			for (int i = 1; i < 3; i++) {
				modelMatrix = glm::translate(glm::mat4(1.0f), modelPositions[i]);// modelDisp.z));
				modelMatrix = glm::translate(modelMatrix, modelPositions[i]);
				modelMatrix = glm::rotate(modelMatrix, modelAngle.x + modelRotations[i].x, glm::vec3(1.0f, 0.0f, 0.0f));
				modelMatrix = glm::rotate(modelMatrix, modelAngle.y + modelRotations[i].y, glm::vec3(0.0f, 1.0f, 0.0f));


				modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));

				glm::mat4 mv_matrix = viewMatrix * modelMatrix;

				glUniformMatrix4fv(glGetUniformLocation((*obj).program, "model_matrix"), 1, GL_FALSE, &modelMatrix[0][0]);
				glUniformMatrix4fv(glGetUniformLocation((*obj).program, "view_matrix"), 1, GL_FALSE, &viewMatrix[0][0]);

				glDrawArrays(GL_TRIANGLES, 0, (*obj).out_vertices.size());
			}
		}
		//render other objects
		else {
			modelMatrix = glm::translate(glm::mat4(1.0f), modelPositions[0]);// modelDisp.z));
			modelMatrix = glm::translate(modelMatrix, modelPositions[0]);
			modelMatrix = glm::rotate(modelMatrix, modelAngle.x + modelRotations[0].x, glm::vec3(1.0f, 0.0f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, modelAngle.y + modelRotations[0].y, glm::vec3(0.0f, 1.0f, 0.0f));


			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));

			glm::mat4 mv_matrix = viewMatrix * modelMatrix;

			glUniformMatrix4fv(glGetUniformLocation((*obj).program, "model_matrix"), 1, GL_FALSE, &modelMatrix[0][0]);
			glUniformMatrix4fv(glGetUniformLocation((*obj).program, "view_matrix"), 1, GL_FALSE, &viewMatrix[0][0]);

			glDrawArrays(GL_TRIANGLES, 0, (*obj).out_vertices.size());

		}
	


}


void onResizeCallback(GLFWwindow* window, int w, int h) {
	windowWidth = w;
	windowHeight = h;

	aspect = (float)w / (float)h;
	proj_matrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
}

void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) keyStatus[key] = true;
	else if (action == GLFW_RELEASE) keyStatus[key] = false;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	
		if (lightOn == true) {
			lightOn = false;
			lightPositions[1] = glm::vec3(0.25f, -10.0f, -0.75f);
		}
		else {
			lightOn = true;
			lightPositions[1] = glm::vec3(0.25f, 1.5f, -0.75f);
		}
	
}

void onMouseMoveCallback(GLFWwindow* window, double x, double y) {
	int mouseX = static_cast<int>(x);
	int mouseY = static_cast<int>(y);

	if (firstMouse) {
		lastX = (GLfloat)mouseX; lastY = (GLfloat)mouseY; firstMouse = false;
	}

	GLfloat xoffset = mouseX - lastX;
	GLfloat yoffset = lastY - mouseY; // Reversed
	lastX = (GLfloat)mouseX; lastY = (GLfloat)mouseY;

	GLfloat sensitivity = 0.05f;
	xoffset *= sensitivity; yoffset *= sensitivity;

	yaw += xoffset; pitch += yoffset;

	// check for pitch out of bounds otherwise screen gets flipped
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	cameraFront = glm::normalize(front);

}

static void onMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset) {
	int yoffsetInt = static_cast<int>(yoffset);

	fovy += yoffsetInt;
	if (fovy >= 1.0f && fovy <= 45.0f) fovy -= (GLfloat)yoffset;
	if (fovy <= 1.0f) fovy = 1.0f;
	if (fovy >= 45.0f) fovy = 45.0f;
	proj_matrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
}

void debugGL() {
	//Output some debugging information
	cout << "VENDOR: " << (char *)glGetString(GL_VENDOR) << endl;
	cout << "VERSION: " << (char *)glGetString(GL_VERSION) << endl;
	cout << "RENDERER: " << (char *)glGetString(GL_RENDERER) << endl;

	// Enable Opengl Debug
	//glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback((GLDEBUGPROC)openGLDebugCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
}

static void APIENTRY openGLDebugCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam) {

	cout << "---------------------opengl-callback------------" << endl;
	cout << "Message: " << message << endl;
	cout << "type: ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		cout << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		cout << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		cout << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		cout << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		cout << "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER:
		cout << "OTHER";
		break;
	}
	cout << " --- ";

	cout << "id: " << id << " --- ";
	cout << "severity: ";
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:
		cout << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		cout << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		cout << "HIGH";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		cout << "NOTIFICATION";
	}
	cout << endl;
	cout << "-----------------------------------------" << endl;
}

string readShader(string name) {
	string vs_text;
	std::ifstream vs_file(name);

	string vs_line;
	if (vs_file.is_open()) {

		while (getline(vs_file, vs_line)) {
			vs_text += vs_line;
			vs_text += '\n';
		}
		vs_file.close();
	}
	return vs_text;
}

void  checkErrorShader(GLuint shader) {
	// Get log lenght
	GLint maxLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

	// Init a string for it
	std::vector<GLchar> errorLog(maxLength);

	if (maxLength > 1) {
		// Get the log file
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		cout << "--------------Shader compilation error-------------\n";
		cout << errorLog.data();
	}

}

void readObj(string name, struct modelObject *obj) {
	cout << "Loading model " << name << "\n";

	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	std::vector< string > materials, textures;
	std::vector< glm::vec3 > obj_vertices;
	std::vector< glm::vec2 > obj_uvs;
	std::vector< glm::vec3 > obj_normals;

	std::ifstream dataFile(name);

	string rawData;		// store the raw data.
	int countLines = 0;
	if (dataFile.is_open()) {
		string dataLineRaw;
		while (getline(dataFile, dataLineRaw)) {
			rawData += dataLineRaw;
			rawData += "\n";
			countLines++;
		}
		dataFile.close();
	}

	cout << "Finished reading model file " << name << "\n";

	istringstream rawDataStream(rawData);
	string dataLine;
	int linesDone = 0;
	while (std::getline(rawDataStream, dataLine)) {
		if (dataLine.find("v ") != string::npos) {	// does this line have a vector?
			glm::vec3 vertex;

			int foundStart = dataLine.find(" ");  int foundEnd = dataLine.find(" ", foundStart + 1);
			vertex.x = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			vertex.y = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			vertex.z = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			obj_vertices.push_back(vertex);
		}
		else if (dataLine.find("vt ") != string::npos) {	// does this line have a uv coordinates?
			glm::vec2 uv;

			int foundStart = dataLine.find(" ");  int foundEnd = dataLine.find(" ", foundStart + 1);
			uv.x = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			uv.y = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			obj_uvs.push_back(uv);
		}
		else if (dataLine.find("vn ") != string::npos) { // does this line have a normal coordinates?
			glm::vec3 normal;

			int foundStart = dataLine.find(" ");  int foundEnd = dataLine.find(" ", foundStart + 1);
			normal.x = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			normal.y = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			normal.z = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			obj_normals.push_back(normal);
		}
		else if (dataLine.find("f ") != string::npos) { // does this line defines a triangle face?
			string parts[3];

			int foundStart = dataLine.find(" ");  int foundEnd = dataLine.find(" ", foundStart + 1);
			parts[0] = dataLine.substr(foundStart + 1, foundEnd - foundStart - 1);

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			parts[1] = dataLine.substr(foundStart + 1, foundEnd - foundStart - 1);

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			parts[2] = dataLine.substr(foundStart + 1, foundEnd - foundStart - 1);

			for (int i = 0; i < 3; i++) {		// for each part

				vertexIndices.push_back(stoul(parts[i].substr(0, parts[i].find("/"))));

				int firstSlash = parts[i].find("/"); int secondSlash = parts[i].find("/", firstSlash + 1);

				if ((firstSlash + 1) != (secondSlash)) {	// there are texture coordinates.
					uvIndices.push_back(stoul(parts[i].substr(firstSlash + 1, secondSlash - firstSlash + 1)));
				}


				normalIndices.push_back(stoul(parts[i].substr(secondSlash + 1)));

			}
		}
		else if (dataLine.find("mtllib ") != string::npos) { // does this object have a material?
			materials.push_back(dataLine.substr(dataLine.find(" ") + 1));
		}

		linesDone++;

		if (linesDone % 50000 == 0) {
			cout << "Parsed " << linesDone << " of " << countLines << " from model...\n";
		}

	}

	// Double check here in which coordinate system you exported your models - and flip or not the vertices...
	/*
	for (unsigned int i = 0; i < vertexIndices.size(); i += 3) {
		(*obj).out_vertices.push_back(obj_vertices[vertexIndices[i+2] - 1]);
		(*obj).out_normals.push_back(obj_normals[normalIndices[i+2] - 1]);
		(*obj).out_uvs.push_back(obj_uvs[uvIndices[i+2] - 1]);

		(*obj).out_vertices.push_back(obj_vertices[vertexIndices[i+1] - 1]);
		(*obj).out_normals.push_back(obj_normals[normalIndices[i+1] - 1]);
		(*obj).out_uvs.push_back(obj_uvs[uvIndices[i + 1] - 1]);

		(*obj).out_vertices.push_back(obj_vertices[vertexIndices[i] - 1]);
		(*obj).out_normals.push_back(obj_normals[normalIndices[i] - 1]);
		(*obj).out_uvs.push_back(obj_uvs[uvIndices[i + 0] - 1]);
	}
	*/

	for (unsigned int i = 0; i < vertexIndices.size(); i++) {
		(*obj).out_vertices.push_back(obj_vertices[vertexIndices[i] - 1]);
		(*obj).out_normals.push_back(obj_normals[normalIndices[i] - 1]);
		(*obj).out_uvs.push_back(obj_uvs[uvIndices[i] - 1]);
	}


	// Load Materials
	for (unsigned int i = 0; i < materials.size(); i++) {

		std::ifstream dataFileMat(materials[i]);

		string rawDataMat;		// store the raw data.
		int countLinesMat = 0;
		if (dataFileMat.is_open()) {
			string dataLineRawMat;
			while (getline(dataFileMat, dataLineRawMat)) {
				rawDataMat += dataLineRawMat;
				rawDataMat += "\n";
			}
			dataFileMat.close();
		}

		istringstream rawDataStreamMat(rawDataMat);
		string dataLineMat;
		while (std::getline(rawDataStreamMat, dataLineMat)) {
			if (dataLineMat.find("map_Kd ") != string::npos) {	// does this line have a texture map?
				textures.push_back(dataLineMat.substr(dataLineMat.find(" ") + 1));
			}
		}
	}

	(*obj).texture = new GLuint[textures.size()];		// Warning possible memory leak here - there is a new here, where is your delete?
	glGenTextures(textures.size(), (*obj).texture);

	for (unsigned int i = 0; i < textures.size(); i++) {
		readTexture(textures[i], (*obj).texture[i]);
	}

	cout << "done";
}

void readTexture(string name, GLuint textureName) {

	// Flip images as OpenGL expects 0.0 coordinates on the y-axis to be at the bottom of the image.
	stbi_set_flip_vertically_on_load(true);

	// Load image Information.
	int iWidth, iHeight, iChannels;
	unsigned char *iData = stbi_load(name.c_str(), &iWidth, &iHeight, &iChannels, 0);

	// Load and create a texture 
	glBindTexture(GL_TEXTURE_2D, textureName); // All upcoming operations now have effect on this texture object

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, iWidth, iHeight);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iWidth, iHeight, GL_RGBA, GL_UNSIGNED_BYTE, iData);


	// This only works for 2D Textures...
	// Set the texture wrapping parameters (next lecture)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering parameters (next lecture)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Generate mipmaps (next lecture)
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);// Unbind texture when done, so we won't accidentily mess up our texture.


}

