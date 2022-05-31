glm::mat4 I = glm::mat4(1.0f); // Identity matrix
glm::vec3 xAxis = glm::vec3(1,0,0); // x axis
glm::vec3 yAxis = glm::vec3(0,1,0); // y axis
glm::vec3 zAxis = glm::vec3(0,0,1); // z axis

// Create a world matrix using position, Euler angles, and size
// Euler angles are passed in YPR parameter:
// YPR.x : Yaw
// YPR.y : Pitch
// YPR.z : Roll
glm::mat4 MakeWorldMatrixEuler(glm::vec3 pos, glm::vec3 YPR, glm::vec3 size) {
	glm::mat4 T = glm::translate(I, pos);
	glm::mat4 RY = glm::rotate(I, YPR.x, yAxis);
	glm::mat4 RP = glm::rotate(I, YPR.y, xAxis);
	glm::mat4 RR = glm::rotate(I, YPR.z, zAxis);
	glm::mat4 S = glm::scale(I, size);
	glm::mat4 WM_e = T*RY*RP*RR*S;
	return WM_e;
}

// Create a world matrix using position, quaternion angles, and size
glm::mat4 MakeWorldMatrixQuat(glm::vec3 pos, glm::quat rQ, glm::vec3 size) {
	glm::mat4 T = glm::translate(I, pos);
	glm::mat4 R_q = glm::rotate(I, rQ, yAxis);
	glm::mat4 S = glm::scale(I, size);
	glm::mat4 WM_q = T*R*S;
	return WM_q;
}

