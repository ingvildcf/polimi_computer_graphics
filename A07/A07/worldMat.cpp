// Create a world matrix using position, Euler angles, and size
// Euler angles are passed in YPR parameter:
// YPR.x : Yaw - psi
// YPR.y : Pitch - theta
// YPR.z : Roll - rho

glm::mat4 I = glm::mat4(1.0f); // Identity matrix
glm::vec3 xAxis = glm::vec3(1,0,0); // x axis = east
glm::vec3 yAxis = glm::vec3(0,1,0); // y axis = north
glm::vec3 zAxis = glm::vec3(0,0,1); // z axis = up
 

glm::mat4 MakeWorldMatrixEuler(glm::vec3 pos, glm::vec3 YPR, glm::vec3 size) {
	
	glm::mat4 T = glm::translate(I, pos); // Translation

	// Rotations
	glm::mat4 R_y = glm::rotate(I, glm::radians(YPR.x), yAxis); // Yaw
	glm::mat4 R_x = glm::rotate(I, glm::radians(YPR.y), xAxis); // Pitch
	glm::mat4 R_z = glm::rotate(I, glm::radians(YPR.z), zAxis); // Roll 

	glm::mat4 S = glm::scale(I, size); // Scaling

	// World Matrix
	glm::mat4 M_W = T*R_y*R_x*R_z*S;
	return M_W;
}

// Create a world matrix using position, quaternion angles, and size
glm::mat4 MakeWorldMatrixQuat(glm::vec3 pos, glm::quat rQ, glm::vec3 size) {
	
	glm::mat4 T = glm::translate(I, pos); // Translation
	glm::mat4 R = glm::mat4(rQ); // Rotation
	glm::mat4 S = glm::scale(I, size); // Scaling

	// World Matrix
	glm::mat4 W_M = T*R*S;
	return W_M;
}

