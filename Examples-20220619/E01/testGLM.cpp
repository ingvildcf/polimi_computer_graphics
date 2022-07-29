/********************************************************
**** Tutorial chapter starting at page  249          ****
**** Loading model                                   ****
****                                                 ****
**** This part requires that corresponding shaders   ****
**** have been compiled                              ****
********************************************************/

#include <iostream>
#include <cstdlib>

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class testGLM {
	public:
	void run() {
		glm::mat4 M1, M2, M;
		M1 = translate(glm::mat4(1), glm::vec3(1, 2, 3));
		M2 = glm::mat4(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
		M = M1 * M2;
		for(int i = 0; i < 4; i++) {
		  for(int j = 0; j < 4; j++) {
			std::cout << M1[j][i] << "\t";
		  }
		  std::cout << "\t";
		  for(int j = 0; j < 4; j++) {
			std::cout << M2[j][i] << "\t";
		  }
		  std::cout << "\n";
		}
		std::cout << "\n";
		for(int i = 0; i < 4; i++) {
		  for(int j = 0; j < 4; j++) {
			std::cout << M[j][i] << "\t";
		  }
		  std::cout << "\n";
		}
		std::cout << "\n";
		
		
		
		
		
		
		
		glm::mat4 M3, M4;
		M3 = inverse(M1);
		std::cout << "\n";
		for(int i = 0; i < 4; i++) {
		  for(int j = 0; j < 4; j++) {
			std::cout << M3[j][i] << "\t";
		  }
		  std::cout << "\n";
		}
		std::cout << "\n";

		M4 = transpose(M1);
		std::cout << "\n";
		for(int i = 0; i < 4; i++) {
		  for(int j = 0; j < 4; j++) {
			std::cout << M4[j][i] << "\t";
		  }
		  std::cout << "\n";
		}
		std::cout << "\n";
		
		
		
		
		
		
		
		
		glm::vec4 V = glm::vec4(5,2,1,1);
		glm::vec4 x;
		
		x = M1 * V;
		for(int j = 0; j < 4; j++) {
			std::cout << x[j] << "\t";
		}
		std::cout << "\n\n";
		
		glm::mat4 MT = M1 + M2 - M3;
		for(int i = 0; i < 4; i++) {
		  for(int j = 0; j < 4; j++) {
			std::cout << MT[j][i] << "\t";
		  }
		  std::cout << "\n";
		}
		std::cout << "\n";
		
		
	}
};

int main() {
    testGLM app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}