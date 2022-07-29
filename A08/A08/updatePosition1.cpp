// The code in this file allows the robot to move while looking from world frame, 
// it stays looking in the direction it has last moved in 
// Move with W (Forward) S (Backward) A (Left) D (Right) R (Down) F (Up) keys

glm::mat4 I = glm::mat4(1.0f); // Identity matrix
glm::vec3 xAxis = glm::vec3(1,0,0); // x axis
glm::vec3 yAxis = glm::vec3(0,1,0); // y axis
glm::vec3 zAxis = glm::vec3(0,0,1); // z axis

// Initial position
float x = -3, y = 0, z = 2;
glm::vec3 pos = glm::vec3(x, y, z);

// Initial direction
float roll = 0, pitch = 0, yaw = 0;

float move_x = 0.0f, move_y = 0.0f, move_z = 0.0f;

// Rotation angles
float a180 = glm::radians(180.0f);
float a90 = glm::radians(90.0f);
float a0 = glm::radians(0.0f);

// Create the world matrix for the robot
glm::mat4 getRobotWorldMatrix(GLFWwindow* window) {
	
	if(glfwGetKey(window, GLFW_KEY_A)){ // Move west (left)
		roll = a0, pitch = a180, yaw = a0;
		move_x -= 0.1f;
	}

	// Make ifs into else ifs to avoid several directions at once
	
	if(glfwGetKey(window, GLFW_KEY_D)){ // Move east (right)
		roll = a0, pitch = a0, yaw = a0;
		move_x += 0.1f;
	}
	if(glfwGetKey(window, GLFW_KEY_W)){ // Move north (away)
		roll = a0, pitch = a90, yaw = a0;
		move_z -= 0.1f;
	}
	if(glfwGetKey(window, GLFW_KEY_S)){ // Move south (closer)
		roll = a0, pitch = -a90, yaw = a0; // turn right
		move_z += 0.1f;
	}
	if(glfwGetKey(window, GLFW_KEY_F)){ // Move down
		roll = a0, pitch = a0, yaw = -a90;
		move_y -= 0.1f;
	}
	if(glfwGetKey(window, GLFW_KEY_R)){ // Move up
		roll = a0, pitch = a0, yaw = a90;
		move_y += 0.1f;
	}

	glm::mat4 R_x = glm::rotate(I, roll, xAxis);
	glm::mat4 R_y = glm::rotate(I, pitch, yAxis);
	glm::mat4 R_z = glm::rotate(I, yaw, zAxis);

	pos = glm::vec3(move_x, move_y, move_z);

	glm::mat4 T = glm::translate(I, pos);
	
	glm::mat4 WM = T*R_x*R_y*R_z;
	
	return WM;
}

