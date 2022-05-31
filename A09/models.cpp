glm::mat4 I = glm::mat4(1.0f);
glm::vec3 yAxis = glm::vec3(0,1,0);

std::vector<tinyobj::real_t> nodeCircle(int n, float r, glm::vec4 center){
    std::vector<tinyobj::real_t> vertices(3*n);
    glm::vec3 vertex; 
    glm::mat4 R;
    float angle = glm::radians(360.0f/n);

    for(int i = 0; i < n; i++) {
        R = glm::rotate(I, angle, yAxis);
        vertex = center + R*glm::vec4(r,0,0,1);
        vertices.push_back(vertex);
    }
    return vertices;
}

// this function creates the geometries to be shown, and output thems
// in global variables M1_vertices and M1_indices to M4_vertices and M4_indices
void makeModels() {
//// M1 : Cube
// Replace the code below, that creates a simple square, with the one to create a cube.

// Resizes the vertices array. Repalce the values with the correct number of
// vertices components (3 * number of vertices)
M1_vertices.resize(3 * 8);
M1_vertices = {0,0,0, 0,0,1, 0,1,1, 0,1,0, 1,0,0, 1,0,1, 1,1,1, 1,1,0};


//for (int i = 0; i < 8; i++){
//    for (int j = 0; j < 3; j++){
//        if (j == 0){
//            if (i < 4){
//                M1_vertices[3*i+j] = 0;
//            }
//        }
//        else if (j == 1){
//            if (i == 2 || i == 3 || i == 6 || i == 7){
//                M1_vertices[3*i+j] = 0;
//            }
//        }
//        else if (j == 2){
//            if (i == 0 || i == 3 || i == 4 || i == 7){
//                M1_vertices[3*i+j] = 0;
//            }
//        }
//        else M1_vertices[3*i+j] = 1;
//    }
//}


// Resizes the indices array. Repalce the values with the correct number of
// indices (3 * number of triangles)
M1_indices.resize(3 * 12);
M1_indices = {0,1,2, 2,3,0, 0,3,4, 4,3,7, 5,6,7, 7,4,5, 1,6,5, 1,2,6, 7,6,2, 3,2,7, 4,5,0, 1,0,5};



//// M2 : Cylinder
// Replace the code below, that creates a simple rotated square, with the one to create a cylinder.

// Resizes the vertices array. Repalce the values with the correct number of
// vertices components (3 * number of vertices)
M2_vertices.resize(12);

glm::mat4 I = glm::mat4(1.0f);
glm::vec3 yAxis = glm::vec3(0, 1, 0);

int n = 32;
float r = 0.3;
glm::vec4 c = glm::vec4(0,0,0,1);
M2_vertices = nodeCircle(n, r, c);

//M2_square = {0,0,0, 0,0,1, 0,1,1, 0,1,0};
//M2_square_prev = M2_square;
glm::mat4 R = glm::rotate(I, glm::radians(10.0f), yAxis);
//M2_square = R * M2_square_prev;

// Vertices definitions
M2_vertices[0]  =  0.0;
M2_vertices[1]  = -1.0;
M2_vertices[2]  = -1.1;
M2_vertices[3]  =  1.0;
M2_vertices[4]  =  0.0;
M2_vertices[5]  = -1.1;
M2_vertices[6]  =  0.0;
M2_vertices[7]  =  1.0;
M2_vertices[8]  = -1.1;
M2_vertices[9]  = -1.0;
M2_vertices[10] =  0.0;
M2_vertices[11] = -1.1;


// Resizes the indices array. Repalce the values with the correct number of
// indices (3 * number of triangles)
M2_indices.resize(6);

// indices definitions
M2_indices[0] = 0;
M2_indices[1] = 1;
M2_indices[2] = 2;
M2_indices[3] = 2;
M2_indices[4] = 3;
M2_indices[5] = 0;











//// M3 : Sphere
// Replace the code below, that creates a simple triangle, with the one to create a sphere.

// Resizes the vertices array. Repalce the values with the correct number of
// vertices components (3 * number of vertices)
M3_vertices.resize(9);

// Vertices definitions
M3_vertices[0]  =  0.0;
M3_vertices[1]  =  1.0;
M3_vertices[2]  = -1.2;
M3_vertices[3]  = -0.866;
M3_vertices[4]  = -0.5;
M3_vertices[5]  = -1.2;
M3_vertices[6]  =  0.866;
M3_vertices[7]  = -0.5;
M3_vertices[8]  = -1.2;


// Resizes the indices array. Repalce the values with the correct number of
// indices (3 * number of triangles)
M3_indices.resize(3);

// indices definitions
M3_indices[0] = 0;
M3_indices[1] = 1;
M3_indices[2] = 2;









//// M4 : Spring
// Replace the code below, that creates a simple octahedron, with the one to create a spring.
M4_vertices.resize(3 * 6);

// Vertices definitions
M4_vertices[0]  =  0.0;
M4_vertices[1]  =  1.414;
M4_vertices[2]  = -1.0;
M4_vertices[3]  =  0.0;
M4_vertices[4]  = -1.414;
M4_vertices[5]  = -1.0;
M4_vertices[6]  = -1.0;
M4_vertices[7]  =  0.0;
M4_vertices[8]  = -2.0;
M4_vertices[9]  = -1.0;
M4_vertices[10] =  0.0;
M4_vertices[11] =  0.0;
M4_vertices[12] =  1.0;
M4_vertices[13] =  0.0;
M4_vertices[14] =  0.0;
M4_vertices[15] =  1.0;
M4_vertices[16] =  0.0;
M4_vertices[17] = -2.0;


// Resizes the indices array. Repalce the values with the correct number of
// indices (3 * number of triangles)
M4_indices.resize(3 * 8);

// indices definitions
M4_indices[0]  = 0;
M4_indices[1]  = 2;
M4_indices[2]  = 3;
M4_indices[3]  = 1;
M4_indices[4]  = 3;
M4_indices[5]  = 2;
M4_indices[6]  = 0;
M4_indices[7]  = 3;
M4_indices[8]  = 4;
M4_indices[9]  = 1;
M4_indices[10] = 4;
M4_indices[11] = 3;
M4_indices[12] = 0;
M4_indices[13] = 4;
M4_indices[14] = 5;
M4_indices[15] = 1;
M4_indices[16] = 5;
M4_indices[17] = 4;
M4_indices[18] = 0;
M4_indices[19] = 5;
M4_indices[20] = 2;
M4_indices[21] = 1;
M4_indices[22] = 2;
M4_indices[23] = 5;
}