
#include <iostream>
#include <cstdlib>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>


class testQuaternions {
	public:
	void compareEuelerAngles(float yaw, float pitch, float roll) {
		// this simple procedure compares the rotation matrix seen in the lessons
		// with the one obtained from the quaternions
		
		std::cout << "Compare Eueler Angles with Quaternion: " <<
					 yaw << " " << pitch << " " << roll << "\n";
		
		glm::mat4 Mx = glm::rotate(glm::mat4(1), glm::radians(pitch), glm::vec3(1,0,0));
		glm::mat4 My = glm::rotate(glm::mat4(1), glm::radians(yaw),   glm::vec3(0,1,0));
		glm::mat4 Mz = glm::rotate(glm::mat4(1), glm::radians(roll),  glm::vec3(0,0,1));
		glm::mat4 ME = My * Mx * Mz;
		glm::quat qe = glm::quat(glm::vec3(0, glm::radians(yaw), 0)) *
					   glm::quat(glm::vec3(glm::radians(pitch), 0, 0)) *
					   glm::quat(glm::vec3(0, 0, glm::radians(roll)));

		glm::mat4 MQ = glm::mat4(qe);

		
		glm::mat4 MEa = glm::eulerAngleYXZ(glm::radians(yaw),
										   glm::radians(pitch),
										   glm::radians(roll));
		
		std::cout << "\nWith matrix products\n";
		for(int i = 0; i < 4; i++) {
		  for(int j = 0; j < 4; j++) {
			std::cout << ME[j][i] << "\t";
		  }
		  std::cout << "\n";
		}
		std::cout << "\nWith basic quaternion products\n";
		for(int i = 0; i < 4; i++) {
		  for(int j = 0; j < 4; j++) {
			std::cout << MQ[j][i] << "\t";
		  }
		  std::cout << "\n";
		}
		std::cout << "\nWith the glm::eulerAngleYXZ function\n";
		for(int i = 0; i < 4; i++) {
		  for(int j = 0; j < 4; j++) {
			std::cout << MEa[j][i] << "\t";
		  }
		  std::cout << "\n";
		}
		std::cout << "\n";
		
	}
	
	void run() {
		// create quaterion from Euler angles
		glm::quat qe = glm::quat(glm::vec3(0, glm::radians(45.0f), 0));

		// create quaterion from scalar, i, j and k components
		glm::quat qv = glm::quat(cos(glm::radians(22.5f)), 0, sin(glm::radians(22.5f)), 0);

		// create quaterion from axis and angle
		glm::quat qa = glm::rotate(glm::quat(1,0,0,0), glm::radians(45.0f), glm::vec3(0,1,0));
		
		std::cout << "Creating quaternions in different ways\n";
		std::cout << qe.x << "\t" << qe.y << "\t" << qe.z << "\t" << qe.w << "\n" ;
		std::cout << qv.x << "\t" << qv.y << "\t" << qv.z << "\t" << qv.w << "\n" ;
		std::cout << qa.x << "\t" << qa.y << "\t" << qa.z << "\t" << qa.w << "\n" ;
		
		// create rotation matrix from quaternion
		std::cout << "Corresponding rotation matrix\n";
		glm::mat4 R = glm::mat4(qe);
		for(int i = 0; i < 4; i++) {
		  for(int j = 0; j < 4; j++) {
			std::cout << R[j][i] << "\t";
		  }
		  std::cout << "\n";
		}
		std::cout << "\n";
		
		glm::quat qb = glm::quat(glm::vec3(glm::radians(30.0f), 0, 0));
		
		// multiply quaternios
		glm::quat qp = qb * qa;
		std::cout << "Quaternions product results\n";
		std::cout << qp.x << "\t" << qp.y << "\t" << qp.z << "\t" << qp.w << "\n" ;
		
		glm::mat4 R2 = glm::mat4(qp);
		std::cout << "Corresponding rotation matrix\n";
		for(int i = 0; i < 4; i++) {
		  for(int j = 0; j < 4; j++) {
			std::cout << R2[j][i] << "\t";
		  }
		  std::cout << "\n";
		}
		std::cout << "\n";

		std::cout << "Quaternion normalization\n";
		glm::quat qn = glm::quat(-1, 0, 2, 3);
		std::cout << qn.x << "\t" << qn.y << "\t" << qn.z << "\t" << qn.w << "\n" ;
		glm::quat qnn = glm::normalize(qn);
		std::cout << qnn.x << "\t" << qnn.y << "\t" << qnn.z << "\t" << qnn.w << "\n" ;
		
		compareEuelerAngles(30,0,0);
		compareEuelerAngles(0,30,0);
		compareEuelerAngles(0,0,30);
		compareEuelerAngles(50,40,30);
		std::cout << "\n";
	}
};

int main() {
    testQuaternions app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}