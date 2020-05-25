#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <vector>
#include <glm\glm.hpp>
#include <iostream>
#include <glm\gtx\intersect.hpp>


//Exemple
extern void Exemple_GUI();
extern void Exemple_PhysicsInit();
extern void Exemple_PhysicsUpdate(float dt);
extern void Exemple_PhysicsCleanup();
bool show_test_window = false;


extern bool renderSphere;
extern bool renderCloth;

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
		float frequency = 2; //pics per segon (s^-1)
		float period = 1/frequency; //quan tarda a fer un cicle (s)
		glm::vec3 direction = glm::vec3(1,0,1);

		Wave() {}
		Wave(float a, float l, float f, glm::vec3 dir) {
			amplitude = a;
			lambda = l;
			frequency = f;
			period = 1 / f;
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


void printSpecs() {
	std::cout << "---" << std::endl;
	std::cout << "Total Time: " << ClothMesh::totalTime << std::endl;
	std::cout << "Period: " << myPM->getWaves()->at(0).period << std::endl;
	std::cout << "Frequency: " << myPM->getWaves()->at(0).frequency << std::endl;
	std::cout << "Amplitude: " << myPM->getWaves()->at(0).amplitude << std::endl;
	std::cout << "Lambda: " << myPM->getWaves()->at(0).lambda << std::endl;
	std::cout << "Check: " << 2 * 3.14159f / myPM->getWaves()->at(0).lambda * myPM->getWaves()->at(0).amplitude << std::endl;
	std::cout << "---" << std::endl;
}

void PhysicsInit() {
	//renderSphere = true;
	renderCloth = true;

	myPM = new ClothMesh::Mesh();

	ClothMesh::setupClothMesh();


}

void PhysicsUpdate(float dt) {

	
	for (int i = 0; i < ClothMesh::w; i++) {
		for (int j = 0; j < ClothMesh::h; j++) {
			glm::vec3 auxXZ = glm::vec3(0, 0, 0);
			float auxY = 0;
			

			for (int w = 0; w < myPM->getWaves()->size(); w++) {
				myPM->getWaves()->at(w).period = 1 / myPM->getWaves()->at(w).frequency;

				float auxk = 2 * 3.14159f / myPM->getWaves()->at(w).lambda;
				float phase = 2 * 3.14159f * (ClothMesh::totalTime / myPM->getWaves()->at(w).period - (int)ClothMesh::totalTime / myPM->getWaves()->at(w).period);
				myPM->getWaves()->at(w).direction = glm::normalize(myPM->getWaves()->at(w).direction) * auxk;

				auxXZ += (glm::normalize(myPM->getWaves()->at(w).direction)) * (float)(myPM->getWaves()->at(w).amplitude
					 * glm::sin(glm::dot(myPM->getWaves()->at(w).direction, myPM->getInitialPositions()[i][j]) - myPM->getWaves()->at(w).frequency * ClothMesh::totalTime/* + phase*/));

				auxY += myPM->getWaves()->at(w).amplitude
					 * glm::cos(glm::dot(myPM->getWaves()->at(w).direction, myPM->getInitialPositions()[i][j]) - myPM->getWaves()->at(w).frequency * ClothMesh::totalTime/* + phase*/);
			}
			myPM->getPositions()[i][j] = myPM->getInitialPositions()[i][j] - auxXZ;
			myPM->getPositions()[i][j].y = myPM->getInitialPositions()[i][j].y + auxY;
		}
	}

	ClothMesh::totalTime += dt;

	//update Mesh
	myPM->setPositions1D();
	ClothMesh::updateClothMesh(&(myPM->getPositions1D()[0].x));

	printSpecs();
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
			ImGui::SliderFloat("Lambda", &myPM->getWaves()->at(i).lambda, 0, 20);
			ImGui::SliderFloat("Frequency", &myPM->getWaves()->at(i).frequency, 0, 10);
		}

		if (ImGui::Button("Add Wave")) {
		
		}
	}

	ImGui::End();
}