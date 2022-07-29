
#include <iostream>
#include <cstdlib>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class testPersp {
	public:
	void run() {
		glm::mat4 M1, M2, M1glm, M2glm;
		glm::vec4 R11, R12, R13;
		glm::vec4 R21, R22, R23;
		
		float l = -1;
		float r = 3;
		float b = -1;
		float t = 2;
		float n = 0.1;
		float f = 9.9;
		
		float a = 4.0/3.0;
		float fovy = glm::radians(45.0f);
		
		M1glm = glm::perspective(fovy, a, n, f);
		M1glm[1][1] *= -1;

		std::cout << "\n";
		for(int i = 0; i < 4; i++) {
		  for(int j = 0; j < 4; j++) {
			std::cout << M1glm[j][i] << "\t";
		  }
		  std::cout << "\n";
		}

		
		M1 = glm::mat4(1.0/(a*tan(fovy/2.0)),0,0,0,
					   0,-1.0/(tan(fovy/2.0)),0,0,
					   0,0,f/(n-f),-1,
					   0,0,n*f/(n-f),0);
		
		std::cout << "\n";
		for(int i = 0; i < 4; i++) {
		  for(int j = 0; j < 4; j++) {
			std::cout << M1[j][i] << "\t";
		  }
		  std::cout << "\n";
		}
		std::cout << "\n";

		R11 = M1 * glm::vec4(-a*tan(fovy/2.0)*n,-tan(fovy/2.0)*n,-n,1.0);
		R12 = M1 * glm::vec4(a*tan(fovy/2.0)*n,tan(fovy/2.0)*n,-n,1.0);
		R13 = M1 * glm::vec4(0,0,-f,1.0);
		R11 /= R11.w;
		R12 /= R12.w;
		R13 /= R13.w;
		for(int j = 0; j < 4; j++) {
			std::cout << R11[j] << "\t";
		}
		std::cout << "\n";
		for(int j = 0; j < 4; j++) {
			std::cout << R12[j] << "\t";
		}
		std::cout << "\n";
		for(int j = 0; j < 4; j++) {
			std::cout << R13[j] << "\t";
		}
		std::cout << "\n";
		
		
		M2glm = glm::scale(glm::mat4(1.0), glm::vec3(1,-1,1)) *
				glm::frustum(l, r, b, t, n, f);
		std::cout << "\n";
		for(int i = 0; i < 4; i++) {
		  for(int j = 0; j < 4; j++) {
			std::cout << M2glm[j][i] << "\t";
		  }
		  std::cout << "\n";
		}

		M2 = glm::mat4(2.0*n/(r-l),0,0,0,  
					   0,2.0*n/(b-t),0,0, 
					   (r+l)/(r-l),(t+b)/(b-t),f/(n-f),-1,
					   0,0,n*f/(n-f),0);
		
		std::cout << "\n";
		for(int i = 0; i < 4; i++) {
		  for(int j = 0; j < 4; j++) {
			std::cout << M2[j][i] << "\t";
		  }
		  std::cout << "\n";
		}
		std::cout << "\n";

		R21 = M2 * glm::vec4(l,b,-n,1.0);
		R22 = M2 * glm::vec4(r,t,-n,1.0);
		R23 = M2 * glm::vec4((l+r)/2.0,(t+b)/2.0,-f,1.0);
		R21 /= R21.w;
		R22 /= R22.w;
		R23 /= R23.w;
		for(int j = 0; j < 4; j++) {
			std::cout << R21[j] << "\t";
		}
		std::cout << "\n";
		for(int j = 0; j < 4; j++) {
			std::cout << R22[j] << "\t";
		}
		std::cout << "\n";
		for(int j = 0; j < 4; j++) {
			std::cout << R23[j] << "\t";
		}
		std::cout << "\n";
	}
};

int main() {
    testPersp app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}