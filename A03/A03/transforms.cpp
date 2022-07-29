	
	// Globals
	glm::mat4 I(1); // Identity matrix
	glm::vec3 xAxis(1, 0, 0); // x axis
	glm::vec3 yAxis(0, 1, 0); // y axis
	glm::vec3 zAxis(0, 0, 1); // z axis
	
	// Rotate 45 degrees around an arbitrary axis passing through (1,0,-1). The x-axis can be aligned to the arbitrary axis after a rotation of 30 degrees around the z-axis, and then -60 degrees around the y-axis.
	glm::vec3 passingPoint(1,0,-1);
	glm::mat4 T1 = glm::translate(I, passingPoint);
	glm::mat4 Rz = glm::rotate(I, glm::radians(30.0f), zAxis);
	glm::mat4 Ry = glm::rotate(I, glm::radians(-60.0f), yAxis);
	glm::mat4 Rx = glm::rotate(I, glm::radians(45.0f), xAxis);

	glm::mat4 MT1 = T1*Ry*Rz*Rx*glm::inverse(Rz)*glm::inverse(Ry)*glm::inverse(T1);

	// Half the size of an object, using as fixed point (5,0,-2)
	glm::vec3 centerPoint(5,0,-2);

	glm::mat4 T2 = glm::translate(I, centerPoint);
	glm::mat4 S2 = glm::scale(I, glm::vec3(0.5f));
	glm::mat4 MT2 = T2*S2*glm::inverse(T2);
	
	// Mirror the starship along a plane passing through (1,1,1), and obtained rotating 15 degree around the x axis the xz plane
	glm::vec3 centerPoint2(1);

	glm::mat4 T3 = glm::translate(I, centerPoint2);
	glm::mat4 R3 = glm::rotate(I, glm::radians(15.0f), xAxis);
	glm::mat4 S3 = glm::scale(I, glm::vec3(1, -1, 1)); // Mirror
	glm::mat4 MT3 = T3*R3*S3*glm::inverse(R3)*glm::inverse(T3);


	// Apply the inverse of the following sequence of transforms: Translation of (0, 0, 5) then rotation of 30 degree around the Y axis, and finally a uniform scaling of a factor of 3.
	glm::vec3 t(0,0,5);
	glm::mat4 T4 = glm::translate(I, t); // Inverse is -t
	glm::mat4 R4 = glm::rotate(I, glm::radians(30.0f), yAxis);
	glm::mat4 S4 = glm::scale(I, glm::vec3(3));
	glm::mat4 MT4 = glm::inverse(T4)*glm::inverse(R4)*glm::inverse(S4); 

