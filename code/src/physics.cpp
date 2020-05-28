#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <vector>
#include <glm\glm.hpp>
#include <iostream>
#include <glm\gtx\intersect.hpp>
#include <math.h>
#include <stdlib.h>
#include <time.h>

//Exemple
extern void Exemple_GUI();
extern void Exemple_PhysicsInit();
extern void Exemple_PhysicsUpdate(float dt);
extern void Exemple_PhysicsCleanup();
bool show_test_window = false;


extern bool renderSphere;
extern bool renderCloth;

glm::vec3 spherePos;
glm::vec3 sphereVel;
glm::vec3 sphereForce;
glm::vec3 g(0, -9.81f, 0);
glm::vec3 sphereLastPos;
glm::vec3 sphereLastVel;
float buoyancyForce;
float rSphere = 1.f;
float mSphere = 0.2f;
glm::vec3 auxito;
int myX;
int myY;

namespace Sphere {
	extern void setupSphere(glm::vec3 pos = glm::vec3(0.f, 1.f, 0.f), float radius = 1.f);
	extern void cleanupSphere();
	extern void updateSphere(glm::vec3 pos, float radius = 1.f);
	extern void drawSphere();
}

namespace ClothMesh {
	extern void setupClothMesh();
	extern void cleanupClothMesh();
	extern void updateClothMesh(float* array_data);
	extern void drawClothMesh();


	int w = 18;
	int h = 14;
	float initialSpaceBetweenPoints = 0.5f;
	double totalTime;

	struct Wave {
		float amplitude = 0.5f; //alçada centre a pic
		float lambda = 2.f; //distancia entre pics
		float frequency = 2; //pics per segon
		glm::vec3 direction = glm::vec3(1,0,0);

		Wave() {}
		Wave(float a, float l, float f, glm::vec3 dir) {
			amplitude = a;
			lambda = l;
			frequency = f;
			direction = dir;
		}
	};

	class Mesh {
	private:
		
		glm::vec3 *positions1D;
		glm::vec3 **positions;
		glm::vec3 **initialPositions;
		std::vector<Wave> *waves;

	public:
		glm::vec3 *getPositions1D() {
			return positions1D;
		}
		glm::vec3 **getPositions() {
			return positions;
		}
		glm::vec3 **getInitialPositions() {
			return initialPositions;
		}
		std::vector<Wave> *getWaves() {
			return waves;
		}

		void setPositions1D() {
			for (int i = 0; i < w; i++) {
				for (int j = 0; j < h; j++)
					positions1D[i * h + j] = glm::vec3(positions[i][j].x, positions[i][j].y, positions[i][j].z);
			}
		}

		void reset() {
			waves->clear();
			for (int i = 0; i < w; i++) {
				for (int j = 0; j < h; j++) {
					initialPositions[i][j].x = i * initialSpaceBetweenPoints - (w-1) * 0.5f * initialSpaceBetweenPoints ;
					initialPositions[i][j].y = 5;
					initialPositions[i][j].z = j * initialSpaceBetweenPoints - (h-1) * 0.5f * initialSpaceBetweenPoints ;
					positions[i][j] = initialPositions[i][j];;
				}
			}
			waves->push_back(Wave());
			totalTime = 0;
		}

		Mesh() {
			positions1D = new glm::vec3[w*h];
			positions = new glm::vec3*[w];
			initialPositions = new glm::vec3*[w];
			for (int i = 0; i < w; i++) {
				positions[i] = new glm::vec3[h];
				initialPositions[i] = new glm::vec3[h];
			}
			waves = new std::vector<Wave>();
			reset();
		}
	};
}
ClothMesh::Mesh *myPM;



void PhysicsInit() {
	srand(time(NULL));
	//renderSphere = true;
	renderCloth = true;
	renderSphere = true;
	myPM = new ClothMesh::Mesh();
	myX = rand() % ClothMesh::w;
	myY = rand() % ClothMesh::h;
	ClothMesh::setupClothMesh();

	auxito = { myPM->getPositions()[myX][myY].x, 8, myPM->getPositions()[myX][myY].z };
	spherePos = auxito;
	sphereLastPos = auxito;
	sphereVel = { 0, 0 ,0 };
	Sphere::updateSphere(spherePos, 1);


}

void PhysicsUpdate(float dt) {

	
	for (int i = 0; i < ClothMesh::w; i++) {
		for (int j = 0; j < ClothMesh::h; j++) {
			glm::vec3 aux = glm::vec3(0, 0, 0);
			float aux2 = 0;
			for (int w = 0; w < myPM->getWaves()->size(); w++) {
				aux += (glm::normalize(myPM->getWaves()->at(w).direction) / (2 * 3.14159f/ myPM->getWaves()->at(w).lambda)) * (float)(myPM->getWaves()->at(w).amplitude
					 * glm::sin(glm::dot(myPM->getWaves()->at(w).direction, myPM->getInitialPositions()[i][j]) - myPM->getWaves()->at(w).frequency * ClothMesh::totalTime));
				aux2 += myPM->getWaves()->at(w).amplitude
					 * glm::cos(glm::dot(myPM->getWaves()->at(w).direction, myPM->getInitialPositions()[i][j]) - myPM->getWaves()->at(w).frequency * ClothMesh::totalTime);
			}
			myPM->getPositions()[i][j] = myPM->getInitialPositions()[i][j] - aux;
			myPM->getPositions()[i][j].y = myPM->getInitialPositions()[i][j].y + aux2;
		}
	}

	if (renderSphere) {
		
		
		float Vs;
		float diff = (spherePos.y-1) - myPM->getPositions()[myX][myY].y;
		float div = 0;
	
		
		if (diff <= 0.2f && diff >= -0.2f) {
			div = 0.5;
		}
		if (diff <= -0.21f) {
			div = 1.f;
		}
		if (diff >= 0.21f) {
			div = 0.f;
		}
		
		//Vs = (mSphere / ((4.f / 3.f) * 3.14159f * rSphere))*div;
		Vs = 0.3f*pow(10,-3)*div;
		buoyancyForce = (1000.f * 9.81f)*Vs;
		
		


		sphereForce = { 0 ,buoyancyForce, 0 };
		sphereLastPos = spherePos;
		sphereLastVel = sphereVel;
		sphereVel = sphereLastVel + dt*((g*mSphere) + sphereForce);
		if (div == 1.f) {
			sphereVel *= 0.81f;
		}

		spherePos = sphereLastPos + sphereVel * dt;

		Sphere::updateSphere(spherePos, 1.f);




		std::cout << sphereVel.y << " / " << sphereForce.y << std::endl;

	}

	ClothMesh::totalTime += dt;

	//update Mesh
	myPM->setPositions1D();
	ClothMesh::updateClothMesh(&(myPM->getPositions1D()[0].x));


}

void PhysicsCleanup() {

	ClothMesh::cleanupClothMesh();


}

void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);

	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate

		for (int i = 0; i < myPM->getWaves()->size(); i++) {
			ImGui::Text("Wave %.0f" , (float)(i+1));
			ImGui::SliderFloat("Amplitude", &myPM->getWaves()->at(i).amplitude, 0, 10);
			ImGui::SliderFloat("Lambda", &myPM->getWaves()->at(i).lambda, 0, 5);
			ImGui::SliderFloat("Frequency", &myPM->getWaves()->at(i).frequency, 0, 10);
		}

		if (ImGui::Button("Add Wave")) {
		
		}

		if (ImGui::Button("Restart")) {
			PhysicsInit();
		}

		ImGui::Checkbox("Sphere", &renderSphere);
	}

	ImGui::End();
}