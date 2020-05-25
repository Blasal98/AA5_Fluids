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

	float PI = 3.1415926f;

	int w = 18;
	int h = 14;
	float initialSpaceBetweenPoints = 0.5f;
	double totalTime;

	struct Wave {
		float amplitude = 0.5f; //alçada centre a pic
		float lambda = 4.f; //distancia entre pics
		float frequency = 2; //pics per segon (s^-1)
		float period = 1/frequency; //quan tarda a fer un cicle (s)
		glm::vec3 direction = glm::vec3(1,0,0);
		float artifact = 2 * PI / lambda;
		bool printSpecsB = false;
		bool artifactSelfAdjust = false;
		bool adjustLambda = false;
		bool adjustAmplitude = false;

		Wave() {}
		Wave(float a, float l, float f, glm::vec3 dir) {
			amplitude = a;
			lambda = l;
			frequency = f;
			period = 1 / f;
			direction = dir;
			artifact = 2 * PI / lambda;
		}

		void printSpecs(int index) {
			std::cout << "-w" << index+1 << "-" << std::endl;
			std::cout << "Total Time: " << totalTime << std::endl;
			std::cout << "Period: " << period << std::endl;
			std::cout << "Frequency: " << frequency << std::endl;
			std::cout << "Amplitude: " << amplitude << std::endl;
			std::cout << "Lambda: " << lambda << std::endl;
			std::cout << "Artifact: " << artifact << std::endl;
			std::cout << "Direction: " << direction.x << " / " << direction.y << " / " << direction.z  << std::endl;
			std::cout << "----" << std::endl;
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

				if (myPM->getWaves()->at(w).adjustLambda) {
					myPM->getWaves()->at(w).lambda = myPM->getWaves()->at(w).amplitude * (2 * ClothMesh::PI) / 0.99f; //ajustem amplitud
					myPM->getWaves()->at(w).adjustLambda = false;
					//std::cout << "Adjusted Lambda ----------------------------------------------------------------------------------------------------" << std::endl;
				}
				else if (myPM->getWaves()->at(w).adjustAmplitude) {
					myPM->getWaves()->at(w).amplitude = myPM->getWaves()->at(w).lambda / (2 * ClothMesh::PI) * 0.99f; //ajustem aplitud
					myPM->getWaves()->at(w).adjustAmplitude = false;
					//std::cout << "Adjusted Amplitude: ------------------------------------------------------------------------------------------------" << std::endl;
				}
				myPM->getWaves()->at(w).artifact = 2 * ClothMesh::PI / myPM->getWaves()->at(w).lambda * myPM->getWaves()->at(w).amplitude; //calculem valor de artfacte

				myPM->getWaves()->at(w).period = 1 / myPM->getWaves()->at(w).frequency; //actualitzem periode amb frequencia
				float auxk = 2 * ClothMesh::PI / myPM->getWaves()->at(w).lambda; //creem k minuscula, magnitud de K majuscula
				myPM->getWaves()->at(w).direction = glm::normalize(myPM->getWaves()->at(w).direction) * auxk; //actualitzem la magnitud la direccio K amb k minuscula
				float phase = 2 * ClothMesh::PI * (ClothMesh::totalTime / myPM->getWaves()->at(w).period - (int)ClothMesh::totalTime / myPM->getWaves()->at(w).period); //calculem la fase
				


				//Calculem increment X i Z
				auxXZ += (glm::normalize(myPM->getWaves()->at(w).direction)) * (float)(myPM->getWaves()->at(w).amplitude
					 * glm::sin(glm::dot(myPM->getWaves()->at(w).direction, myPM->getInitialPositions()[i][j]) - myPM->getWaves()->at(w).frequency * ClothMesh::totalTime/* + phase*/));
				//Calculem increment Y
				auxY += myPM->getWaves()->at(w).amplitude
					 * glm::cos(glm::dot(myPM->getWaves()->at(w).direction, myPM->getInitialPositions()[i][j]) - myPM->getWaves()->at(w).frequency * ClothMesh::totalTime/* + phase*/);
			}
			myPM->getPositions()[i][j] = myPM->getInitialPositions()[i][j] - auxXZ; //actualitzem x i Z
			myPM->getPositions()[i][j].y = myPM->getInitialPositions()[i][j].y + auxY; //actualitzem Y
		}
	}

	ClothMesh::totalTime += dt;

	//update Mesh
	myPM->setPositions1D();
	ClothMesh::updateClothMesh(&(myPM->getPositions1D()[0].x));

	for (int w = 0; w < myPM->getWaves()->size(); w++) {
		if (myPM->getWaves()->at(w).printSpecsB)
			myPM->getWaves()->at(w).printSpecs(w);
	}
}


void PhysicsCleanup() {

	ClothMesh::cleanupClothMesh();


}

void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);

	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate
		if (ImGui::Button("Add Wave")) {

		}

		for (int i = 0; i < myPM->getWaves()->size(); i++) {
			ImGui::Text("Wave %.0f" , (float)(i+1));
			if (ImGui::Button("Increment X direction")) {
				myPM->getWaves()->at(i).direction.x += 0.1f;
			}
			if (ImGui::Button("Increment Z direction")) {
				myPM->getWaves()->at(i).direction.z += 0.1f;
			}
			if (ImGui::SliderFloat("Amplitude", &myPM->getWaves()->at(i).amplitude, 0, 10)) {
				if (myPM->getWaves()->at(i).artifactSelfAdjust) { //esta selfadjust activat
					//if (myPM->getWaves()->at(i).artifact > 1) { //crea Warning
					//	myPM->getWaves()->at(i).lambda = myPM->getWaves()->at(i).amplitude * (2 * ClothMesh::PI) / 0.99f; //ajustem lambda
					//	myPM->getWaves()->at(i).artifact = 2 * ClothMesh::PI / myPM->getWaves()->at(i).lambda * myPM->getWaves()->at(i).amplitude; //calculem valor de artfacte
					//}
					if (myPM->getWaves()->at(i).artifact > 1)
						myPM->getWaves()->at(i).adjustLambda = true;
				}
			}
			if (ImGui::SliderFloat("Lambda", &myPM->getWaves()->at(i).lambda, 1, 60)) {
				if (myPM->getWaves()->at(i).artifactSelfAdjust) { //esta selfadjust activat
					//if (myPM->getWaves()->at(i).artifact > 1) { //crea Warning
					//	myPM->getWaves()->at(i).amplitude = myPM->getWaves()->at(i).lambda / (2 * ClothMesh::PI) * 0.99f; //ajustem aplitud
					//	myPM->getWaves()->at(i).artifact = 2 * ClothMesh::PI / myPM->getWaves()->at(i).lambda * myPM->getWaves()->at(i).amplitude; //calculem valor de artfacte
					//}
					if (myPM->getWaves()->at(i).artifact > 1)
						myPM->getWaves()->at(i).adjustAmplitude = true;
				}
			}

			ImGui::SliderFloat("Frequency", &myPM->getWaves()->at(i).frequency, 0, 10);
			if (myPM->getWaves()->at(i).artifact >= 0 && myPM->getWaves()->at(i).artifact <= 1)
				ImGui::Text("SAFE Artifacts");
			else
				ImGui::Text("WARNING Artifacts !!!");
			ImGui::Checkbox("Artifacts Self Adjust", &myPM->getWaves()->at(i).artifactSelfAdjust);
			ImGui::Checkbox("PrintSpecs", &myPM->getWaves()->at(i).printSpecsB);
		}
		

		
	}

	ImGui::End();
}