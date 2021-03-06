#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <vector>
#include <glm\glm.hpp>
#include <iostream>
#include <glm\gtx\intersect.hpp>
#include <time.h>

//Exemple
extern void Exemple_GUI();
extern void Exemple_PhysicsInit();
extern void Exemple_PhysicsUpdate(float dt);
extern void Exemple_PhysicsCleanup();
bool show_test_window = false;


extern bool renderSphere;
extern bool renderCloth;

namespace Sphere {
	extern void setupSphere(glm::vec3 pos = glm::vec3(0.f, 1.f, 0.f), float radius = 1.f);
	extern void cleanupSphere();
	extern void updateSphere(glm::vec3 pos, float radius = 1.f);
	extern void drawSphere();
}

glm::vec3 spherePos;
glm::vec3 sphereVel;
glm::vec3 sphereForce;
glm::vec3 g(0, -9.81f, 0);
glm::vec3 sphereLastPos;
glm::vec3 sphereLastVel;
float buoyancyForce;
float rSphere = 1.f;
float mSphere = 4188.79021f;
glm::vec3 auxito;
int myX;
int myY;
float liquidDensity = 1000.f;
float sphereDensity;



namespace ClothMesh {
	extern void setupClothMesh();
	extern void cleanupClothMesh();
	extern void updateClothMesh(float* array_data);
	extern void drawClothMesh();

	float PI = 3.1415926f;
	float timeToRestart = 15;
	bool autoReset = true;

	int w = 18;
	int h = 14;
	float initialSpaceBetweenPoints = 0.5f;
	double totalTime;

	float defaultAmplitude = 0.5f;
	float defaultLambda = 10.f;
	float defaultFrequency = 0.5f;

	float generalAmplitude = defaultAmplitude;
	float generalLambda = defaultLambda;
	float generalFrequency = defaultFrequency;



	struct Wave {
		float amplitude; //al�ada centre a pic
		float lambda; //distancia entre pics
		float frequency; //pics per segon (s^-1)
		float period; //quan tarda a fer un cicle (s)
		glm::vec3 direction;
		float artifact;
		bool printSpecsB = false;
		bool artifactSelfAdjust = false;
		bool adjustLambda = false;
		bool adjustAmplitude = false;

		Wave() {
			direction = glm::vec3(1,0,0);
			amplitude = defaultAmplitude;
			lambda = defaultLambda;
			frequency = defaultFrequency;
			period = 1 / frequency;
			artifact = 2 * PI / lambda;
		}
		Wave(float a, float l, float f, glm::vec3 dir) {
			amplitude = a;
			lambda = l;
			frequency = f;
			period = 1 / f;
			direction = dir;
			artifact = 2 * PI / lambda;
		}
		Wave(glm::vec3 dir) {
			direction = dir;
			amplitude = defaultAmplitude;
			lambda = defaultLambda;
			frequency = defaultFrequency;
			period = 1 / frequency;
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
			generalAmplitude = defaultAmplitude;
			generalLambda = defaultLambda;
			generalFrequency = defaultFrequency;



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

void SphereReset() {
	myX = rand() % ClothMesh::w;
	myY = rand() % ClothMesh::h;
	auxito = { myPM->getPositions()[myX][myY].x, 1, myPM->getPositions()[myX][myY].z };
	spherePos = auxito;
	sphereLastPos = auxito;
	sphereVel = { 0, 0 ,0 };
	Sphere::updateSphere(spherePos, 1);
	liquidDensity = 1000.f;
	mSphere = 4188.79021f;
}


void PhysicsInit() {
	srand(time(NULL));

	renderSphere = true;
	renderCloth = true;

	myPM = new ClothMesh::Mesh();
	SphereReset();

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
				float phase = 2 * ClothMesh::PI * (ClothMesh::totalTime / myPM->getWaves()->at(w).period - (int)(ClothMesh::totalTime / myPM->getWaves()->at(w).period)); //calculem la fase
				//std::cout << phase << std::endl;


				//Calculem increment X i Z
				auxXZ += (glm::normalize(myPM->getWaves()->at(w).direction)) * (float)(myPM->getWaves()->at(w).amplitude
					 * glm::sin(glm::dot(myPM->getWaves()->at(w).direction, myPM->getInitialPositions()[i][j]) - myPM->getWaves()->at(w).frequency * ClothMesh::totalTime + phase));
				//Calculem increment Y
				auxY += myPM->getWaves()->at(w).amplitude
					 * glm::cos(glm::dot(myPM->getWaves()->at(w).direction, myPM->getInitialPositions()[i][j]) - myPM->getWaves()->at(w).frequency * ClothMesh::totalTime + phase);
			}
			myPM->getPositions()[i][j] = myPM->getInitialPositions()[i][j] - auxXZ; //actualitzem x i Z
			myPM->getPositions()[i][j].y = myPM->getInitialPositions()[i][j].y + auxY; //actualitzem Y
		}
	}
	if (renderSphere) {


		float Vs;
		float diff = (spherePos.y - 1) - myPM->getPositions()[myX][myY].y;
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

		Vs = (((4.f / 3.f) * 3.14159f * glm::pow(rSphere,3.f)))*div;
		sphereDensity = mSphere / ((4.f / 3.f) * 3.14159f * glm::pow(rSphere, 3.f));
		buoyancyForce = (liquidDensity) * 9.81f * Vs;

		sphereForce = { 0 ,buoyancyForce, 0 };
		sphereLastPos = spherePos;
		sphereLastVel = sphereVel;
		sphereVel = sphereLastVel + dt * (g * mSphere + sphereForce) / mSphere;
		/*if (div == 1.f) {
			sphereVel *= 0.81f;
		}*/

		spherePos = sphereLastPos + sphereVel * dt;

		Sphere::updateSphere(spherePos, rSphere);
		//std::cout << sphereVel.y << " / " << sphereForce.y << std::endl;

	}

	ClothMesh::totalTime += dt;

	//update Mesh
	myPM->setPositions1D();
	ClothMesh::updateClothMesh(&(myPM->getPositions1D()[0].x));

	for (int w = 0; w < myPM->getWaves()->size(); w++) {
		if (myPM->getWaves()->at(w).printSpecsB)
			myPM->getWaves()->at(w).printSpecs(w);
	}
	if (ClothMesh::totalTime >= ClothMesh::timeToRestart) {
		if(ClothMesh::autoReset)
			myPM->reset();
	}

}


void PhysicsCleanup() {

	ClothMesh::cleanupClothMesh();


}

void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);

	{
		ImGui::Text("AA5 - Francesc Aguilo i Victor Blas");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate
		if (ImGui::Button("Add Random Wave")) {
			myPM->getWaves()->push_back(ClothMesh::Wave(glm::vec3((float)std::rand() / RAND_MAX, (float)std::rand() / RAND_MAX, (float)std::rand() / RAND_MAX)));
		}
		ImGui::Text("Number of waves: %.0f", (float)(myPM->getWaves()->size()));
		//for (int i = 0; i < myPM->getWaves()->size(); i++) {
			//ImGui::Text("---Wave %.0f---" , (float)(1));
		if (ImGui::Button("ResetWaves")) {
			myPM->reset();
		}
		ImGui::Checkbox("Auto Reset", &ClothMesh::autoReset);
		if(ClothMesh::autoReset)
			ImGui::Text("Reseting in: %.3f", ClothMesh::timeToRestart - ClothMesh::totalTime);
		if (ImGui::Button("Reset Sphere")) {
			SphereReset();
		}
		ImGui::SliderFloat("Sphere Mass", &mSphere, 0,5000);
		//ImGui::SliderFloat("Liquid Density", &liquidDensity, 1.f,2000.f);
		ImGui::Text("Liquid - Sphere Density = %.3f", liquidDensity - sphereDensity);
		ImGui::Text("Buoyancy Force = %.3f", buoyancyForce);
		ImGui::Text("Total Force on Sphere = %.3f", buoyancyForce + g.y*mSphere);

		ImGui::Text("-------------- Edit First Wave --------------");
		if (ImGui::Button("Increment X direction")) {
			myPM->getWaves()->at(0).direction.x += 0.1f;
		}
		if (ImGui::Button("Increment Z direction")) {
			myPM->getWaves()->at(0).direction.z += 0.1f;
		}
		if (ImGui::SliderFloat("Amplitude", &myPM->getWaves()->at(0).amplitude, 0, 10)) {
			if (myPM->getWaves()->at(0).artifactSelfAdjust) { //esta selfadjust activat
				//if (myPM->getWaves()->at(0).artifact > 1) { //crea Warning
				//	myPM->getWaves()->at(0).lambda = myPM->getWaves()->at(0).amplitude * (2 * ClothMesh::PI) / 0.99f; //ajustem lambda
				//	myPM->getWaves()->at(0).artifact = 2 * ClothMesh::PI / myPM->getWaves()->at(0).lambda * myPM->getWaves()->at(0).amplitude; //calculem valor de artfacte
				//}
				if (myPM->getWaves()->at(0).artifact > 1)
					myPM->getWaves()->at(0).adjustLambda = true;
			}
		}
		if (ImGui::SliderFloat("Lambda", &myPM->getWaves()->at(0).lambda, 1, 60)) {
			if (myPM->getWaves()->at(0).artifactSelfAdjust) { //esta selfadjust activat
				//if (myPM->getWaves()->at(0).artifact > 1) { //crea Warning
				//	myPM->getWaves()->at(0).amplitude = myPM->getWaves()->at(0).lambda / (2 * ClothMesh::PI) * 0.99f; //ajustem aplitud
				//	myPM->getWaves()->at(0).artifact = 2 * ClothMesh::PI / myPM->getWaves()->at(0).lambda * myPM->getWaves()->at(0).amplitude; //calculem valor de artfacte
				//}
				if (myPM->getWaves()->at(0).artifact > 1)
					myPM->getWaves()->at(0).adjustAmplitude = true;
			}
		}

		ImGui::SliderFloat("Frequency", &myPM->getWaves()->at(0).frequency, 0, 5);
		ImGui::Text("Hi ha l'error d'Artifacts: ");
		if (myPM->getWaves()->at(0).artifact >= 0 && myPM->getWaves()->at(0).artifact <= 1)
			ImGui::Text("NO");
		else
			ImGui::Text("SI!!!");

		ImGui::Checkbox("Artifacts Self Adjust", &myPM->getWaves()->at(0).artifactSelfAdjust);
		ImGui::Checkbox("PrintSpecs", &myPM->getWaves()->at(0).printSpecsB);

		ImGui::Text("-------------- Edit All Waves --------------");
		if (ImGui::SliderFloat("General Amplitude", &ClothMesh::generalAmplitude, 0, 10) ||
			ImGui::SliderFloat("General Lambda", &ClothMesh::generalLambda, 1, 60) ||
			ImGui::SliderFloat("General Frequency", &ClothMesh::generalFrequency, 0, 5)) {
		
			for (int i = 0; i < myPM->getWaves()->size(); i++) {
				myPM->getWaves()->at(i).amplitude = ClothMesh::generalAmplitude;
				myPM->getWaves()->at(i).lambda = ClothMesh::generalLambda;
				myPM->getWaves()->at(i).frequency = ClothMesh::generalFrequency;
			}
		}
	}

	ImGui::End();
}