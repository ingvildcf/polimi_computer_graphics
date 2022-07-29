
#include <iostream>
#include <cstdlib>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class testOrtho {
	public:
	void run() {
		glm::mat4 M1, Port;
		glm::vec4 LBN, RTF, R11, R12, R21, R22;
		
		float l = -1;
		float r = 3;
		float b = -1;
		float t = 2;
		float n = 0.1;
		float f = 10;
		
		LBN = glm::vec4(l,b,-n,1.0);
		RTF = glm::vec4(r,t,-f,1.0);
		
		M1 = glm::mat4(2.0/(r-l),0,0,0,   0,2.0/(b-t),0,0,  0,0,1.0/(n-f),0,
					  (r+l)/(l-r),(t+b)/(t-b),n/(n-f),1);
		
		std::cout << "\n";
		for(int i = 0; i < 4; i++) {
		  for(int j = 0; j < 4; j++) {
			std::cout << M1[j][i] << "\t";
		  }
		  std::cout << "\n";
		}
		std::cout << "\n";

		R11 = M1 * LBN;
		R12 = M1 * RTF;
		for(int j = 0; j < 4; j++) {
			std::cout << R11[j] << "\t";
		}
		std::cout << "\n";
		for(int j = 0; j < 4; j++) {
			std::cout << R12[j] << "\t";
		}
		std::cout << "\n";




		Port = glm::scale(glm::mat4(1.0), glm::vec3(1,-1,1)) *
		       glm::ortho(l, r, b, t, n, f);

		std::cout << "\n";
		for(int i = 0; i < 4; i++) {
		  for(int j = 0; j < 4; j++) {
			std::cout << Port[j][i] << "\t";
		  }
		  std::cout << "\n";
		}
		std::cout << "\n";
		R21 = Port * LBN;
		R22 = Port * RTF;
		for(int j = 0; j < 4; j++) {
			std::cout << R21[j] << "\t";
		}
		std::cout << "\n";
		for(int j = 0; j < 4; j++) {
			std::cout << R22[j] << "\t";
		}
		std::cout << "\n";
	}
};

int main() {
    testOrtho app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}