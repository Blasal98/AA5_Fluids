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
	//renderSphere = true;
	renderCloth = true;

	myPM = new ClothMesh::Mesh();

	ClothMesh::setupClothMesh();


}

void PhysicsUpdate(float dt) {

	for (int w = 0; w < myPM->getWaves()->size(); w++) {
		for (int i = 0; i < ClothMesh::w; i++) {
			for (int j = 0; j < ClothMesh::h; j++) {

			}
		}
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
		Exemple_GUI();
	}

	ImGui::End();
}