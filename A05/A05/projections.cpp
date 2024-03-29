float n = 0.1; // Near plane
float f = 9.9; // Far plane

glm::mat4 I = glm::mat4(1.0f); // Identity matrix
glm::vec3 xAxis = glm::vec3(1,0,0); // x axis
glm::vec3 yAxis = glm::vec3(0,1,0); // y axis
glm::vec3 zAxis = glm::vec3(0,0,1); // z axis

float d = n; // Distance from center of projection to projection plane - change makes a zoom (short d - wide lens, large d - tele lens, d infty - parallel proj)

// Function for creating the Perspective Projection Matrix with aspect ratio a
glm::mat4 calculate_perspective_matrix(float a, float fovy) {
	
	fovy = glm::radians(fovy);
	float r = a*n*glm::tan(fovy/2);
	float l = -a*n*glm::tan(fovy/2);
	float t = n*glm::tan(fovy/2);
	float b = -n*glm::tan(fovy/2);
	glm::mat4 P_persp = glm::transpose(glm::frustum(l, r, b, t, n, f));

	/*
	glm::mat4 P_persp = I;
	P_persp[0][0] = 2*n/(r-l);
	P_persp[1][1] = 2*n/(b-t);
	P_persp[2][2] = f/(n-f);
	P_persp[3][3] = 0.0f;
	P_persp[0][2] = (r+l)/(r-l);
	P_persp[1][2] = (t+b)/(b-t);
	P_persp[2][3] = n*f/(n-f);
	P_persp[3][2] = -1.0f;
	*/
	/*glm::mat4 P_persp = I;
	P_persp[0][0] = 1/(a*glm::tan(glm::radians(fov/2)));
	P_persp[1][1] = 1/glm::tan(glm::radians(fov/2));
	P_persp[2][2] = f/(n-f);
	P_persp[3][3] = 0.0f;
	P_persp[3][2] = -1.0f;
	P_persp[2][3] = n*f/(n-f);*/
	//P_persp[1][1] *= -1;
	return P_persp;
}

// Create a matrix for Perspective projection with the given aspect ratio a,
// and a FovY = 90o
glm::mat4 PO1(float a) {
	//glm::mat4 P_persp = glm::perspective(glm::radians(90.0f), a, n, f);

	float fovy = glm::radians(90.0f);
	float r = a*n*glm::tan(fovy/2);
	float l = -a*n*glm::tan(fovy/2);
	float t = n*glm::tan(fovy/2);
	float b = -n*glm::tan(fovy/2);
	

	glm::mat4 P_persp = I;
	P_persp[0][0] = 2*n/(r-l);
	P_persp[1][1] = 2*n/(b-t);
	P_persp[2][2] = f/(n-f);
	P_persp[3][3] = 0.0f;
	P_persp[2][0] = (r+l)/(r-l);
	P_persp[2][1] = (t+b)/(b-t);
	P_persp[2][3] = -1.0f;
	P_persp[3][2] = n*f/(n-f);

	return P_persp;
}

// Create a matrix for Perspective projection with the given aspect ratio a,
// and a FovY = 120o
glm::mat4 PO2(float a) {

	float fovy = 120.0;
	glm::mat4 P_persp = calculate_perspective_matrix(a, fovy); //glm::perspective(glm::radians(120.0f), a, n, f);
	
	return P_persp;
}

// Create a matrix for Perspective projection with the given aspect ratio a,
// and a FovY = 30o
glm::mat4 PO3(float a) {
	glm::mat4 P_persp = glm::perspective(30.0f, a, n, f);
	return P_persp;
}

// Create a matrix for Perspecrtive projection, with the given aspect ratio a.
// Here the perspective should only show the left side of the view
// with a FovY = 90o. Here r=0, and l, t and b should be computed
// to match both the aspect ratio and the FovY

glm::mat4 PO4(float a) {
	float fov = glm::radians(90.0f);
	float r = 0.0;
	float l = -a*n*glm::tan(fov/2);
	float t = n*glm::tan(fov/2);
	float b = -n*glm::tan(fov/2);
	

	glm::mat4 P_persp = I;
	P_persp[0][0] = 2*n/(r-l);
	P_persp[1][1] = 2*n/(b-t);
	P_persp[2][2] = f/(n-f);
	P_persp[3][3] = 0.0f;
	P_persp[2][0] = (r+l)/(r-l);
	P_persp[2][1] = (t+b)/(b-t);
	P_persp[2][3] = -1.0f;
	P_persp[3][2] = n*f/(n-f);

	return P_persp;
}

