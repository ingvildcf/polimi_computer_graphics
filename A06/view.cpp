glm::mat4 I = glm::mat4(1.0f); // Identity matrix
glm::vec3 xAxis = glm::vec3(1,0,0); // x axis
glm::vec3 yAxis = glm::vec3(0,1,0); // y axis
glm::vec3 zAxis = glm::vec3(0,0,1); // z axis
 
 // Create a look in direction matrix
 // Pos    -> Position of the camera
 // Angs.x -> direction (alpha)
 // Angs.y -> elevation (beta)
 // Angs.z -> roll (rho)
glm::mat4 LookInDirMat(glm::vec3 Pos, glm::vec3 Angs) {
	glm::mat4 T = glm::translate(I, -Pos);
	glm::mat4 R_y = glm::rotate(I, -Angs.x, yAxis);
	glm::mat4 R_x = glm::rotate(I, -Angs.y, xAxis);
	glm::mat4 R_z = glm::rotate(I, -Angs.z, zAxis);

	glm::mat4 M_V = R_z*R_x*R_y*T;
	return M_V;
}

 // Create a look at matrix
 // Pos    -> Position of the camera (c)
 // aim    -> Posizion of the target (a)
 // Roll   -> roll (rho)
 glm::vec3 u = yAxis;

glm::mat4 LookAtMat(glm::vec3 Pos, glm::vec3 aim, float Roll) {
	glm::mat4 R = glm::rotate(I, glm::radians(Roll), yAxis);
	glm::mat4 M_V = R*glm::lookAt(Pos, aim, u);
	return M_V;
}



