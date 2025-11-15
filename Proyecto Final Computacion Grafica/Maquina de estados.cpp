#include <iostream>
#include <cmath>
#include <fstream> 
#include <string>  
#include <vector>  

// GLEW
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// otras libs
#include "stb_image.h"
// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//carga Models
#include "SOIL2/SOIL2.h"
// otros .h
#include "Shader.h"
#include "Camera.h"
#include "Model.h"


// --- DEFINICIÓN DE ESTRUCTURA DE FRAME ---
#define MAX_FRAMES 200 
#define MAX_PASOS_INTERPOLACION 190

typedef struct _frame {
	
	float pataPosX, pataPosY, pataPosZ;
	float rotSuperior, rotEnmedio, rotInferior;
	float pataPosXInc, pataPosYInc, pataPosZInc;
	float rotSuperiorInc, rotEnmedioInc, rotInferiorInc;
}FRAME;


// --- --------------------------------- ---
// --- CLASE PARA PATA ANIMADA---
// --- --------------------------------- ---
class PataAnimada
{
	
public:
	// --- Variables de Estado y Pose ---
	glm::vec3 posOffset;
	glm::vec3 rotOffset;
	glm::vec3 ensambleSup;
	glm::vec3 ensambleEnm;
	std::string animFilename;

	// --- Variables de Animación ---
	FRAME KeyFrame[MAX_FRAMES];
	int FrameIndex;
	int playIndex;
	bool play;
	int i_curr_steps;

	// --- Variables de Pose---
	float pataPosX, pataPosY, pataPosZ;
	float rotSuperior, rotEnmedio, rotInferior;

	// --- Modelos (punteros) ---
	Model* modeloS_Sup;
	Model* modeloS_Enm;
	Model* modeloS_Inf;

	// --- Constructor ---
	PataAnimada(Model* sup, Model* enm, Model* inf, const std::string& filename)
	{
		modeloS_Sup = sup;
		modeloS_Enm = enm;
		modeloS_Inf = inf;
		animFilename = filename;

		FrameIndex = 0;
		playIndex = 0;
		play = false;
		i_curr_steps = 0;

		pataPosX = 0.0f; pataPosY = 0.0f; pataPosZ = 0.0f;
		rotSuperior = 0.0f; rotEnmedio = 0.0f; rotInferior = 0.0f;

		posOffset = glm::vec3(0.0f);
		rotOffset = glm::vec3(0.0f);
		ensambleSup = glm::vec3(1.9f, -2.8f, 1.3f);
		ensambleEnm = glm::vec3(2.3f, 1.3f, 1.8f);

		for (int i = 0; i < MAX_FRAMES; i++) {
			KeyFrame[i] = { 0,0,0,0,0,0,0,0,0,0,0,0 };
		}
	}

	// --- Métodos (funciones) de la clase ---
	void setOffsets(glm::vec3 pos, glm::vec3 rot)
	{
		posOffset = pos;
		rotOffset = rot;
	}

	void setEnsamble(glm::vec3 vSup, glm::vec3 vEnm)
	{
		ensambleSup = vSup;
		ensambleEnm = vEnm;
	}

	void startStopAnimation()
	{
		if (play == false && (FrameIndex > 1))
		{
			resetElements();
			interpolation();
			play = true;
			playIndex = 0;
			i_curr_steps = 0;
		}
		else
		{
			play = false;
		}
	}

	void loadAnimationFromFile()
	{
		std::ifstream file(animFilename);
		if (!file.is_open())
		{
			std::cout << "Error: No se pudo abrir el archivo para cargar: " << animFilename << std::endl;
			return;
		}
		file >> FrameIndex;
		if (FrameIndex > MAX_FRAMES)
		{
			std::cout << "Error: El archivo de animación tiene más frames que el máximo permitido." << std::endl;
			FrameIndex = 0;
			file.close();
			return;
		}
		for (int i = 0; i < FrameIndex; i++)
		{
			file >> KeyFrame[i].pataPosX
				>> KeyFrame[i].pataPosY
				>> KeyFrame[i].pataPosZ
				>> KeyFrame[i].rotSuperior
				>> KeyFrame[i].rotEnmedio
				>> KeyFrame[i].rotInferior;
			KeyFrame[i].pataPosXInc = 0;
		}
		file.close();
		std::cout << "Animación cargada" << animFilename << "!" << std::endl;
		if (FrameIndex > 0)
		{
			resetElements();
		}
	}

	// --- Funciones de Animación ---
	void resetElements(void)
	{
		pataPosX = KeyFrame[0].pataPosX;
		pataPosY = KeyFrame[0].pataPosY;
		pataPosZ = KeyFrame[0].pataPosZ;
		rotSuperior = KeyFrame[0].rotSuperior;
		rotEnmedio = KeyFrame[0].rotEnmedio;
		rotInferior = KeyFrame[0].rotInferior;
	}

	void interpolation(void)
	{
		KeyFrame[playIndex].pataPosXInc = (KeyFrame[playIndex + 1].pataPosX - KeyFrame[playIndex].pataPosX) / MAX_PASOS_INTERPOLACION;
		KeyFrame[playIndex].pataPosYInc = (KeyFrame[playIndex + 1].pataPosY - KeyFrame[playIndex].pataPosY) / MAX_PASOS_INTERPOLACION;
		KeyFrame[playIndex].pataPosZInc = (KeyFrame[playIndex + 1].pataPosZ - KeyFrame[playIndex].pataPosZ) / MAX_PASOS_INTERPOLACION;
		KeyFrame[playIndex].rotSuperiorInc = (KeyFrame[playIndex + 1].rotSuperior - KeyFrame[playIndex].rotSuperior) / MAX_PASOS_INTERPOLACION;
		KeyFrame[playIndex].rotEnmedioInc = (KeyFrame[playIndex + 1].rotEnmedio - KeyFrame[playIndex].rotEnmedio) / MAX_PASOS_INTERPOLACION;
		KeyFrame[playIndex].rotInferiorInc = (KeyFrame[playIndex + 1].rotInferior - KeyFrame[playIndex].rotInferior) / MAX_PASOS_INTERPOLACION;
	}

	void Animation()
	{
		if (play)
		{
			if (i_curr_steps >= MAX_PASOS_INTERPOLACION)
			{
				playIndex++;
				if (playIndex > FrameIndex - 2)
				{
					
					playIndex = 0;
					play = false; 
				}
				else
				{
					i_curr_steps = 0;
					interpolation();
				}
			}
			else
			{
				pataPosX += KeyFrame[playIndex].pataPosXInc;
				pataPosY += KeyFrame[playIndex].pataPosYInc;
				pataPosZ += KeyFrame[playIndex].pataPosZInc;
				rotSuperior += KeyFrame[playIndex].rotSuperiorInc;
				rotEnmedio += KeyFrame[playIndex].rotEnmedioInc;
				rotInferior += KeyFrame[playIndex].rotInferiorInc;
				i_curr_steps++;
			}
		}
	}

	void Draw(Shader& lightingShader, GLint modelLoc)
	{
		glm::mat4 modelTemp = glm::mat4(1.0f);
		glm::mat4 model = glm::mat4(1.0f);
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);

		// --- 1. Dibuja PataSuperior (Base) ---
		model = glm::translate(glm::mat4(1.0f), posOffset);
		model = glm::rotate(model, glm::radians(rotOffset.x), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotOffset.y), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotOffset.z), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(pataPosX, pataPosY, pataPosZ));
		model = glm::rotate(model, glm::radians(rotSuperior), glm::vec3(10.0f, 5.0f, 3.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		modeloS_Sup->Draw(lightingShader);
		modelTemp = model;

		// --- 2. Dibuja PataEnmedio ---
		model = modelTemp;
		model = glm::translate(model, ensambleSup);
		model = glm::rotate(model, glm::radians(rotEnmedio), glm::vec3(8.0f, 8.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		modeloS_Enm->Draw(lightingShader);
		modelTemp = model;

		// --- 3. Dibuja PataInferior ---
		model = modelTemp;
		model = glm::translate(model, ensambleEnm);
		model = glm::rotate(model, glm::radians(rotInferior), glm::vec3(8.0f, 8.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		modeloS_Inf->Draw(lightingShader);
	}
};
// --- --------------------------------- ---
// --- FIN DE LA CLASE PataAnimada     ---
// --- --------------------------------- ---


// ---DEFINICIÓN DE FRAME DE CÁMARA ---
typedef struct _cam_frame {
	float posX, posY, posZ;
	float yaw, pitch;
	float posXInc, posYInc, posZInc;
	float yawInc, pitchInc;
}CAM_FRAME;


// --- ------------------------------------ ---
// ---  CLASE REPRODUCTOR DE CÁMARA ---
// --- ------------------------------------ ---
class ReproductorCamara
{
public:
	std::string animFilename;

	CAM_FRAME KeyFrame[MAX_FRAMES];
	int FrameIndex;
	int playIndex;
	bool play;
	int i_curr_steps; 

	ReproductorCamara(const std::string& filename)
	{
		animFilename = filename;
		FrameIndex = 0;
		playIndex = 0;
		play = false;
		i_curr_steps = 0;
	}

	void startStopAnimation(Camera& cam)
	{
		if (play == false && (FrameIndex > 1))
		{
			resetElements(cam);
			interpolation();
			play = true;
			playIndex = 0;
			i_curr_steps = 0;
			printf("Iniciando reproduccion del recorrido...\n");
		}
		else
		{
			play = false;
			printf("Reproduccion pausada. Modo libre.\n");
		}
	}

	void loadAnimationFromFile(Camera& cam)
	{
		std::ifstream file(animFilename);
		if (!file.is_open())
		{
			std::cout << "Error: No se pudo abrir el archivo de recorrido: " << animFilename << std::endl;
			return;
		}
		file >> FrameIndex;
		if (FrameIndex > MAX_FRAMES)
		{
			std::cout << "Error: El archivo de recorrido tiene más frames que el máximo permitido." << std::endl;
			FrameIndex = 0;
			file.close();
			return;
		}
		for (int i = 0; i < FrameIndex; i++)
		{
			file >> KeyFrame[i].posX
				>> KeyFrame[i].posY
				>> KeyFrame[i].posZ
				>> KeyFrame[i].yaw
				>> KeyFrame[i].pitch;

			KeyFrame[i].posXInc = 0; KeyFrame[i].posYInc = 0; KeyFrame[i].posZInc = 0;
			KeyFrame[i].yawInc = 0; KeyFrame[i].pitchInc = 0;
		}
		file.close();
		std::cout << "¡Recorrido de camara cargado" << animFilename << "!" << std::endl;
		if (FrameIndex > 0)
		{
			resetElements(cam); // Pone la cámara en la posición inicial
		}
	}

	void resetElements(Camera& cam)
	{
		
		cam.Position.x = KeyFrame[0].posX;
		cam.Position.y = KeyFrame[0].posY;
		cam.Position.z = KeyFrame[0].posZ;
		cam.Yaw = KeyFrame[0].yaw;
		cam.Pitch = KeyFrame[0].pitch;
		cam.updateCameraVectors();
	}

	void interpolation(void)
	{
		KeyFrame[playIndex].posXInc = (KeyFrame[playIndex + 1].posX - KeyFrame[playIndex].posX) / MAX_PASOS_INTERPOLACION;
		KeyFrame[playIndex].posYInc = (KeyFrame[playIndex + 1].posY - KeyFrame[playIndex].posY) / MAX_PASOS_INTERPOLACION;
		KeyFrame[playIndex].posZInc = (KeyFrame[playIndex + 1].posZ - KeyFrame[playIndex].posZ) / MAX_PASOS_INTERPOLACION;
		KeyFrame[playIndex].yawInc = (KeyFrame[playIndex + 1].yaw - KeyFrame[playIndex].yaw) / MAX_PASOS_INTERPOLACION;
		KeyFrame[playIndex].pitchInc = (KeyFrame[playIndex + 1].pitch - KeyFrame[playIndex].pitch) / MAX_PASOS_INTERPOLACION;
	}

	void Animation(Camera& cam)
	{
		if (play)
		{
			if (i_curr_steps >= MAX_PASOS_INTERPOLACION)
			{
				playIndex++;
				if (playIndex > FrameIndex - 2)
				{
					playIndex = 0;
					play = false;
					printf("Recorrido finalizado. Modo libre.\n");
				}
				else
				{
					i_curr_steps = 0;
					interpolation();
				}
			}
			else
			{
				
				cam.Position.x += KeyFrame[playIndex].posXInc;
				cam.Position.y += KeyFrame[playIndex].posYInc;
				cam.Position.z += KeyFrame[playIndex].posZInc;
				cam.Yaw += KeyFrame[playIndex].yawInc;
				cam.Pitch += KeyFrame[playIndex].pitchInc;
				cam.updateCameraVectors();

				i_curr_steps++; 
			}
		}
	}
};
// --- ------------------------------------ ---
// --- FIN DE LA CLASE ReproductorCamara  ---
// --- ------------------------------------ ---


// --- Variables Globales del Programa ---

const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool active;
glm::vec3 pointLightPositions[] = {
	glm::vec3(0.0f,2.0f, 0.0f),
	glm::vec3(0.0f,0.0f, 0.0f),
	glm::vec3(0.0f,0.0f, 0.0f),
	glm::vec3(0.0f,0.0f, 0.0f)
};
float vertices[] = { //Vértices del cubo
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, -0.5f, 0.5f, 
	-0.5f, 0.0f, 0.0f, -1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 
	1.0f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, -0.5f, 0.5f, 
	-0.5f, -1.0f, 0.0f, 0.0f, -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, -0.5f, 0.5f, 0.5f, -1.0f, 
	0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.5f, -0.5f,
	0.5f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.5f, -0.5f, 0.5f, 0.0f, -1.0f,
	0.0f, 0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 
	-0.5f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f
};
glm::vec3 Light1 = glm::vec3(0);
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Punteros globales a las patas 
PataAnimada* pata1; // Derecha-Delantera
PataAnimada* pata2; // Izquierda-Delantera
PataAnimada* pata3; // Izquierda-Trasera
PataAnimada* pata4; // Derecha-Trasera
// ----------------------------------------

// Puntero global al reproductor de cámara 
ReproductorCamara* recorridoCamara;
// ----------------------------------------------------

// VARIABLES GLOBALES PARA DISH 
Model* modeloDish = nullptr;
float dishRotation = 0.0f;
float dishActivateDistance = 7.0f;
glm::vec3 dishPosition = glm::vec3(0.7f, 9.5f, -3.8f);
float dishScale = 0.18f;
// -----------------------------------------------------------------------------------------
// --- VARIABLES GLOBALES DEL DRON ---
Model* mDronCuerpo = nullptr;
Model* mDronHeliceDer = nullptr;
Model* mDronHeliceIzq = nullptr;
Model* mDronPataDer = nullptr;
Model* mDronPataIzq = nullptr;
float dronAnguloVuelo = 0.0f;
float dronAnguloHelice = 0.0f;
float dronTambaleo = 0.0f;
glm::vec3 offsetHeliceDer = glm::vec3(-0.5f, 0.1f, 0.0f);
glm::vec3 offsetHeliceIzq = glm::vec3(0.5f, 0.15f, 0.0f);
glm::vec3 offsetPataDer = glm::vec3(-0.2f, -0.2f, 0.0f);
glm::vec3 offsetPataIzq = glm::vec3(0.2f, -0.2f, 0.0f);
// -----------------------------------

// --- PARED 2º PISO: variables globales para mostrar/ocultar ---
Model* modeloPared = nullptr;
bool mostrarPared = true;
glm::vec3 paredOffset = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 paredRotDeg = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 paredScale = glm::vec3(1.0f, 1.0f, 1.0f);
// ----------------------------------------------------------------

// --- PUERTA: variables globales 
Model* modeloMarcoPuerta = nullptr;
Model* modeloPuerta = nullptr;
glm::vec3 puertaOffsetLocal = glm::vec3(-1.17f, 5.7f, -4.0f);
glm::vec3 marcoOffsetLocal = glm::vec3(0.0f, -0.001f, 0.0f);
float puertaRotActual = 90.0f;
float puertaRotObjetivo = 180.0f;
float puertaApertura = 95.0f;
float puertaVelocidad = 50.0f;
float puertaDistActiva = 5.5f;
glm::vec3 puertaEjeGiro = glm::vec3(0.0f, 1.0f, 0.0f);
float puertaCerradaOffsetDeg = 0.0f;
// ----------------------------------------------------------------
// --- VARIABLES DEL SKYBOX ---
unsigned int skyboxVAO, skyboxVBO;
unsigned int cubemapTexture;

float skyboxVertices[] = {
	-1.0f,1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
	-1.0f, -1.0f,1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
	 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f
};

unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	return textureID;
}


void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// --- Main ---
int main()
{
	//(Inicialización de GLFW y GLEW) 
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto Final - 319079485", nullptr, nullptr);
	if (nullptr == window) { std::cout << "Failed to create GLFW window" << std::endl; glfwTerminate(); return EXIT_FAILURE; }
	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);
	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit()) { std::cout << "Failed to initialize GLEW" << std::endl; return EXIT_FAILURE; }
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	// ---------------------------------------------


	Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
	Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");


	// Carga de modelos 
	Model* modeloPataSuperior = new Model((char*)"Models/Pata superior.obj");
	Model* modeloPataEnmedio = new Model((char*)"Models/Pata enmedio.obj");
	Model* modeloPataInferior = new Model((char*)"Models/Pata inferior.obj");
	Model* modeloBase = new Model((char*)"Models/base.obj");
	Model* modeloCasaCompleta = new Model((char*)"Models/12casacompleta.obj");
	Model Ball((char*)"Models/todo.obj");
	modeloDish = new Model((char*)"Models/dish.obj");
	mDronCuerpo = new Model((char*)"Models/droncuerpo.obj");
	mDronHeliceDer = new Model((char*)"Models/helicederecha.obj");
	mDronHeliceIzq = new Model((char*)"Models/heliceizquierda.obj");
	mDronPataDer = new Model((char*)"Models/pataderechadron.obj");
	mDronPataIzq = new Model((char*)"Models/pataizquirdadron.obj");
	modeloPared = new Model((char*)"Models/paredfrentesegundopiso.obj");
	modeloMarcoPuerta = new Model((char*)"Models/marco.obj");
	modeloPuerta = new Model((char*)"Models/puerta.obj");
	// ------------------------------------


	// CREACION PATAS METALICAS OXIDADAS
	pata1 = new PataAnimada(modeloPataSuperior, modeloPataEnmedio, modeloPataInferior, "pata1_anim.txt");
	pata1->setOffsets(glm::vec3(-1.65f, 2.8f, -2.1f), glm::vec3(0.0f, 0.0f, 0.0f));
	pata1->setEnsamble(glm::vec3(1.9f, -2.8f, 1.3f), glm::vec3(2.3f, 1.3f, 1.8f));
	pata1->loadAnimationFromFile();

	pata2 = new PataAnimada(modeloPataSuperior, modeloPataEnmedio, modeloPataInferior, "pata2_anim.txt");
	pata2->setOffsets(glm::vec3(-2.2f, 2.8f, -2.1f), glm::vec3(0.0f, 270.0f, 0.0f));
	pata2->setEnsamble(glm::vec3(1.9f, -2.8f, 1.3f), glm::vec3(2.3f, 1.3f, 1.8f));
	pata2->loadAnimationFromFile();

	pata3 = new PataAnimada(modeloPataSuperior, modeloPataEnmedio, modeloPataInferior, "pata3_anim.txt");
	pata3->setOffsets(glm::vec3(-2.2f, 2.8f, -2.6f), glm::vec3(0.0f, 180.0f, 0.0f));
	pata3->setEnsamble(glm::vec3(1.9f, -2.8f, 1.3f), glm::vec3(2.3f, 1.3f, 1.8f));
	pata3->loadAnimationFromFile();

	pata4 = new PataAnimada(modeloPataSuperior, modeloPataEnmedio, modeloPataInferior, "pata4_anim.txt");
	pata4->setOffsets(glm::vec3(-1.65f, 2.8f, -2.7f), glm::vec3(-40.0f, 0.0f, 0.0f));
	pata4->setEnsamble(glm::vec3(1.9f, -2.8f, 1.3f), glm::vec3(2.3f, 1.3f, 1.8f));
	pata4->loadAnimationFromFile();

	printf("Reproductor de 4 patas cargado.\n");
	printf("Presiona 'P' para iniciar/detener la animacion.\n");
	// ---------------------------------

	// inicializar el reproductor de cámara 
	recorridoCamara = new ReproductorCamara("camera_anim.txt");
	recorridoCamara->loadAnimationFromFile(camera); // Carga el recorrido
	printf("Presiona 'L' para iniciar/detener el recorrido de la camara.\n");
	// --------------------------------------------------------

	// 1. Crear Shader de Skybox
	Shader skyboxShader("Shader/skybox.vs", "Shader/skybox.frag");

	// 2. Configurar VAO/VBO del Skybox
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// 3. Cargar las texturas
	std::vector<std::string> faces
	{
		"Models/skybox/right.jpg", "Models/skybox/left.jpg", "Models/skybox/top.jpg",
		"Models/skybox/bottom.jpg", "Models/skybox/back.jpg", "Models/skybox/front.jpg"
	};
	cubemapTexture = loadCubemap(faces);

	// Configuración del shader
	skyboxShader.Use();
	glUniform1i(glGetUniformLocation(skyboxShader.Program, "skybox"), 0);

	// Configuración de VAO/VBO del cubo de luz
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
	glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0); glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);
	// ----------------------------------------


	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents();
		DoMovement();

		//  Actualiza la lógica de animación de las patas 
		pata1->Animation();
		pata2->Animation();
		pata3->Animation();
		pata4->Animation();
		// ---------------------------------------------------

		// Actualiza la animación de la cámara (si está en modo playback) 
		recorridoCamara->Animation(camera);
		// -------------------------------------------------------------------------

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// (Configuración de Luces y Shaders) 
		lightingShader.Use();
		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		glm::mat4 view; view = camera.GetViewMatrix();
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.6f, 0.6f, 0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.6f, 0.6f, 0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.3f, 0.3f, 0.3f);
		glm::vec3 lightColor; lightColor.x = abs(sin(glfwGetTime() * Light1.x)); lightColor.y = abs(sin(glfwGetTime() * Light1.y)); lightColor.z = sin(glfwGetTime() * Light1.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 1.0f, 0.2f, 0.2f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.045f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.075f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.2f, 0.2f, 0.8f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.2f, 0.2f, 0.8f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.3f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.7f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(18.0f)));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 5.0f);
		// ----------------------------------------------------------------

		// DIBUJA LA BASE OXIDADA 
		glm::mat4 model = glm::mat4(1.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		modeloBase->Draw(lightingShader);

		// DIBUJA LA CASA COMPLETA 
		model = glm::mat4(1.0f);
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(-3.5f, 0.1f, -3.5f));
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = translation * rotation;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		modeloCasaCompleta->Draw(lightingShader);
		// ---------------------------------------

		// PUERTA
		{
			glm::mat4 casaTR = translation * rotation;
			glm::vec3 puertaPosMundo = glm::vec3(casaTR * glm::vec4(puertaOffsetLocal, 1.0f));
			float distPuerta = glm::length(camera.GetPosition() - puertaPosMundo);
			puertaRotObjetivo = (distPuerta < puertaDistActiva) ? puertaApertura : 0.0f;
			float diff = puertaRotObjetivo - puertaRotActual;
			if (fabs(diff) > 0.01f) {
				float paso = puertaVelocidad * deltaTime;
				if (fabs(diff) <= paso) puertaRotActual = puertaRotObjetivo;
				else puertaRotActual += paso * (diff > 0.0f ? 1.0f : -1.0f);
			}
			if (modeloMarcoPuerta) {
				glm::mat4 marcoM = casaTR;
				marcoM = glm::translate(marcoM, marcoOffsetLocal);
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(marcoM));
				modeloMarcoPuerta->Draw(lightingShader);
			}
			if (modeloPuerta) {
				glm::mat4 puertaM = casaTR;
				puertaM = glm::translate(puertaM, puertaOffsetLocal);
				puertaM = glm::rotate(puertaM, glm::radians(puertaCerradaOffsetDeg + puertaRotActual), puertaEjeGiro);
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(puertaM));
				modeloPuerta->Draw(lightingShader);
			}
		}
		// ---------------------------------------------------

		//PARED 2º PISO
		if (mostrarPared && modeloPared) {
			glm::mat4 paredModel = glm::mat4(1.0f);
			glm::mat4 tCasa = glm::translate(glm::mat4(1.0f), glm::vec3(-3.5f, 0.1f, -3.45f));
			glm::mat4 rCasa = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			paredModel = tCasa * rCasa;
			paredModel = glm::translate(paredModel, paredOffset);
			if (paredRotDeg.x != 0.0f) paredModel = glm::rotate(paredModel, glm::radians(paredRotDeg.x), glm::vec3(1, 0, 0));
			if (paredRotDeg.y != 0.0f) paredModel = glm::rotate(paredModel, glm::radians(paredRotDeg.y), glm::vec3(0, 1, 0));
			if (paredRotDeg.z != 0.0f) paredModel = glm::rotate(paredModel, glm::radians(paredRotDeg.z), glm::vec3(0, 0, 1));
			paredModel = glm::scale(paredModel, paredScale);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(paredModel));
			modeloPared->Draw(lightingShader);
		}
		// ---------------------------------------------------

		//DIBUJAR EL DISH (PLATO)
		{
			float dist = glm::length(camera.GetPosition() - dishPosition);
			if (dist < dishActivateDistance) {
				dishRotation += 90.0f * deltaTime;
			}
			if (dishRotation >= 360.0f) dishRotation = fmod(dishRotation, 360.0f);
			glm::mat4 dishModel = glm::mat4(1.0f);
			dishModel = glm::translate(dishModel, dishPosition);
			dishModel = glm::rotate(dishModel, glm::radians(dishRotation), glm::vec3(0.0f, 1.0f, 0.0f));
			dishModel = glm::scale(dishModel, glm::vec3(dishScale));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(dishModel));
			if (modeloDish) modeloDish->Draw(lightingShader);
		}
		// ---------------------------------------
		// --- ANIMACIÓN Y DIBUJO DEL DRON ---
		{
			float radioVuelo = 7.0f;
			float velocidadVuelo = 0.7f;
			dronAnguloVuelo += velocidadVuelo * deltaTime;
			dronAnguloHelice += 1000.0f * deltaTime;
			dronTambaleo = sin(glfwGetTime() * 2.0f) * 0.2f;
			float dronX = sin(dronAnguloVuelo) * radioVuelo;
			float dronZ = cos(dronAnguloVuelo) * radioVuelo;
			float dronY = 8.0f + dronTambaleo;
			glm::mat4 modelDron = glm::mat4(1.0f);
			modelDron = glm::translate(modelDron, glm::vec3(dronX, dronY, dronZ));
			modelDron = glm::rotate(modelDron, dronAnguloVuelo + glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			float anguloTambaleo = (float)(cos(glfwGetTime() * 3.0f) * 5.0f);
			modelDron = glm::rotate(modelDron, glm::radians(anguloTambaleo), glm::vec3(0.0f, 0.0f, 1.0f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDron));
			if (mDronCuerpo) mDronCuerpo->Draw(lightingShader);
			glm::mat4 matrizCuerpo = modelDron;
			glm::mat4 modelH = matrizCuerpo;
			modelH = glm::translate(modelH, offsetHeliceDer);
			modelH = glm::rotate(modelH, glm::radians(dronAnguloHelice), glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelH));
			if (mDronHeliceDer) mDronHeliceDer->Draw(lightingShader);
			modelH = matrizCuerpo;
			modelH = glm::translate(modelH, offsetHeliceIzq);
			modelH = glm::rotate(modelH, glm::radians(-dronAnguloHelice), glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelH));
			if (mDronHeliceIzq) mDronHeliceIzq->Draw(lightingShader);
			glm::mat4 modelP = matrizCuerpo;
			modelP = glm::translate(modelP, offsetPataDer);
			float oscilacionPata = sin(glfwGetTime() * 2.5f) * 10.0f;
			modelP = glm::rotate(modelP, glm::radians(oscilacionPata), glm::vec3(1.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelP));
			if (mDronPataDer) mDronPataDer->Draw(lightingShader);
			modelP = matrizCuerpo;
			modelP = glm::translate(modelP, offsetPataIzq);
			modelP = glm::rotate(modelP, glm::radians(oscilacionPata), glm::vec3(1.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelP));
			if (mDronPataIzq) mDronPataIzq->Draw(lightingShader);
		}
		// ---------------------------------------

		// --- Dibujar cada Pata ---
		pata1->Draw(lightingShader, modelLoc);
		pata2->Draw(lightingShader, modelLoc);
		pata3->Draw(lightingShader, modelLoc);
		pata4->Draw(lightingShader, modelLoc);
		// ---------------------------

		// Cubo de Luz
		model = glm::mat4(1);
		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Ball.Draw(lightingShader);
		glDisable(GL_BLEND); glBindVertexArray(0);
		lampShader.Use();
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1); model = glm::translate(model, pointLightPositions[0]);
		model = glm::scale(model, glm::vec3(0.2f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO); glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		// -----------------------------------------------------------
		// --- DIBUJAR SKYBOX ---
		glDepthFunc(GL_LEQUAL);
		skyboxShader.Use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);
		// ----------------------
		glfwSwapBuffers(window);
	}

	// Limpiar memoria
	delete pata1;
	delete pata2;
	delete pata3;
	delete pata4;
	delete modeloPataSuperior;
	delete modeloPataEnmedio;
	delete modeloPataInferior;
	delete modeloBase;
	if (modeloDish) { delete modeloDish; modeloDish = nullptr; }
	if (modeloPared) { delete modeloPared; modeloPared = nullptr; }
	if (modeloPuerta) { delete modeloPuerta; modeloPuerta = nullptr; }
	if (modeloMarcoPuerta) { delete modeloMarcoPuerta; modeloMarcoPuerta = nullptr; }
	if (mDronCuerpo) delete mDronCuerpo;
	if (mDronHeliceDer) delete mDronHeliceDer;
	if (mDronHeliceIzq) delete mDronHeliceIzq;
	if (mDronPataDer) delete mDronPataDer;
	if (mDronPataIzq) delete mDronPataIzq;

	
	delete recorridoCamara;
	// ---------------------------------------------

	glfwTerminate();
	return 0;
}

// DoMovement 
void DoMovement()
{
	// Si el recorrido se está reproduciendo, noprocesar movimiento de la camara
	if (recorridoCamara == nullptr || !recorridoCamara->play)
	{
		if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]) camera.ProcessKeyboard(FORWARD, deltaTime);
		if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]) camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) camera.ProcessKeyboard(LEFT, deltaTime);
		if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);
	}

	
	if (keys[GLFW_KEY_LEFT_BRACKET]) pointLightPositions[0].x += 0.01f; // Tecla '['
	if (keys[GLFW_KEY_RIGHT_BRACKET]) pointLightPositions[0].x -= 0.01f; // Tecla ']'
	if (keys[GLFW_KEY_I]) pointLightPositions[0].y += 0.01f;
	if (keys[GLFW_KEY_O]) pointLightPositions[0].y -= 0.01f;
	if (keys[GLFW_KEY_8]) pointLightPositions[0].z += 0.01f;
	if (keys[GLFW_KEY_9]) pointLightPositions[0].z -= 0.01f;
}
// ---------------------------------------------------------


//KeyCallback 
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// TECLA 'P' PARA REPRODUCIR TODAS LAS PATAS
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		printf("¡Iniciando/Pausando todas las animaciones!\n");
		pata1->startStopAnimation();
		pata2->startStopAnimation();
		pata3->startStopAnimation();
		pata4->startStopAnimation();
	}
	// -------------------------------------------------

	// TECLA 'F' PARA MOSTRAR/OCULTAR PARED 2º PISO 
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		mostrarPared = !mostrarPared;
		printf("Pared segundo piso: %s\n", mostrarPared ? "VISIBLE" : "OCULTA");
	}
	// -----------------------------------------------------

	// NUEVO: TECLA 'L' PARA REPRODUCIR RECORRIDO Y PATAS
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		printf("¡Iniciando/Pausando RECORRIDO y animaciones de PATAS!\n");
		recorridoCamara->startStopAnimation(camera);
		// Sincroniza las patas
		pata1->startStopAnimation();
		pata2->startStopAnimation();
		pata3->startStopAnimation();
		pata4->startStopAnimation();
	}
	// -----------------------------------------------------------

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS) { keys[key] = true; }
		else if (action == GLFW_RELEASE) { keys[key] = false; }
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		active = !active;
		if (active) { Light1 = glm::vec3(0.2f, 0.8f, 1.0f); }
		else { Light1 = glm::vec3(0); }
	}
}
// -----------------------------------------------------

// ---MouseCallback ---
void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	// Bloqueo mouse durante recorrido de cámara
	if (recorridoCamara != nullptr && recorridoCamara->play) {
		firstMouse = true; // activa cuando termine el recorrido
		return;
	}

	if (firstMouse) { lastX = xPos; lastY = yPos; firstMouse = false; }
	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;
	lastX = xPos; lastY = yPos;
	camera.ProcessMouseMovement(xOffset, yOffset);
}
//Cruz Soria Oscar 319079485