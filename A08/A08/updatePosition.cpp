// The code in this file allows the robot to move while looking from world frame, 
// it always turns back to look in the direction of positive x axis after each move
// Move with W (Forward) S (Backward) A (Left) D (Right) R (Down) F (Up) keys

glm::mat4 I = glm::mat4(1.0f); // Identity matrix
glm::vec3 xAxis = glm::vec3(1,0,0); // x axis
glm::vec3 yAxis = glm::vec3(0,1,0); // y axis
glm::vec3 zAxis = glm::vec3(0,0,1); // z axis

// Create the world matrix for the robot
glm::mat4 getRobotWorldMatrix(GLFWwindow* window) {
	// Initial position
	float x = -3, y = 0, z = 2;
	static glm::vec3 pos = glm::vec3(x, y, z);

	// Initial direction
	float roll = 0, pitch = 0, yaw = 0;
	static glm::vec3 dir = glm::vec3(roll, pitch, yaw);

	float move_x = 0, move_y = 0, move_z = 0;
	float a180 = glm::radians(180.0f);
	float a90 = glm::radians(90.0f);
	float a0 = glm::radians(0.0f);
	
	if(glfwGetKey(window, GLFW_KEY_S)){ // Move backwards
		pitch = a180, yaw = a0;
		move_x -= 0.1f;
	}
	if(glfwGetKey(window, GLFW_KEY_W)){ // Move forwards
		pitch = a0, yaw = a0;
		move_x += 0.1f;
	}
	if(glfwGetKey(window, GLFW_KEY_A)){ // Move to the left
		pitch = a90, yaw = a0; // turn left
		move_z -= 0.1f;
	}
	if(glfwGetKey(window, GLFW_KEY_D)){ // Move to the right
		pitch = -a90, yaw = a0; // turn right
		move_z += 0.1f;
	}
	if(glfwGetKey(window, GLFW_KEY_F)){ // Move down
		roll = a0, pitch = a0, yaw = -a90;
		move_y -= 0.1f;
	}
	if(glfwGetKey(window, GLFW_KEY_R)){ // Move up
		pitch = a0, yaw = a90;
		move_y += 0.1f;
	}
	pos += glm::vec3(move_x, move_y, move_z);
	//dir = glm::vec3(roll, pitch, yaw);

	glm::mat4 R_x = glm::rotate(I, roll, xAxis);
	glm::mat4 R_y = glm::rotate(I, pitch, yAxis);
	glm::mat4 R_z = glm::rotate(I, yaw, zAxis);

	glm::mat4 T = glm::translate(I, pos);
	
	glm::mat4 WM = T*R_x*R_y*R_z;
	
	return WM;
}

