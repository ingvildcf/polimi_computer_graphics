float w = 2.0; // Half width
float n = -4.0; // Near plane
float f = 12.0; // Far plane

glm::mat4 I = glm::mat4(1.0f); // Identity matrix
glm::vec3 xAxis = glm::vec3(1,0,0); // x axis
glm::vec3 yAxis = glm::vec3(0,1,0); // y axis
glm::vec3 zAxis = glm::vec3(0,0,1); // z axis

// Function for creating the Parallel Projection Matrix with aspect ratio a
glm::mat4 calculate_PPM(float a) {
	glm::mat4 PPM = I;
	PPM[0][0] = 1/w;
	PPM[1][1] = -a/w;
	PPM[2][2] = 1/(n-f);
	PPM[3][2] = n/(n-f);
	
	return PPM;
}

//ISOMETRIC 
float a_x = glm::radians(35.26f); // x axis rotation
float a_y = glm::radians(45.00f); // y axis rotation

// Create a matrix for Isometric projection with the given aspect ratio a
glm::mat4 PO1(float a) {
	glm::mat4 PPM = calculate_PPM(a);
	glm::mat4 R_x = glm::rotate(I, a_x, xAxis); // always 35.26
	glm::mat4 R_y = glm::rotate(I, a_y, yAxis); // always 45

	glm::mat4 P_iso = PPM*R_x*R_y;

	return P_iso;
}

// Create a matrix for Dimetric projection (alpha = 45 degree)
// with the given aspect ration a
float alpha = glm::radians(45.0f);

glm::mat4 PO2(float a) {
	glm::mat4 PPM = calculate_PPM(a);

	glm::mat4 R_x = glm::rotate(I, alpha, xAxis); // alpha arbitrary
	glm::mat4 R_y = glm::rotate(I, a_y, yAxis); // a_y always 45
	
	glm::mat4 P_dim = PPM*R_x*R_y;

	return P_dim;
}

// Create a matrix for Trimetric projection (alpha = 45 degree, beta = 60 degree)
// with the given aspect ration a
float beta = glm::radians(60.0f);

glm::mat4 PO3(float a) {
	glm::mat4 PPM = calculate_PPM(a);

	glm::mat4 R_x = glm::rotate(I, alpha, xAxis); // alpha arbitrary
	glm::mat4 R_y = glm::rotate(I, beta, yAxis); // beta arbitrary
	
	glm::mat4 P_tri = PPM*R_x*R_y;

	return P_tri;
}

// Create a matrix for Cabinet projection (oblique w length not maintained) 
// alpha = 45 = axis angle
// with the given aspect ration a

float rho = 0.5; // reduction factor

glm::mat4 PO4(float a) {
	glm::mat4 PPM = calculate_PPM(a);
	glm::mat4 S = I;
	S[2][0] = -rho*cos(alpha);
	S[2][1] = -rho*sin(alpha);
	glm::mat4 P_cab = PPM*S;
	return P_cab;
}

