#include <math.h>

glm::mat4 I = glm::mat4(1.0f);
glm::vec3 yAxis = glm::vec3(0,1,0);

// this function creates the geometries to be shown, and output thems
// in global variables M1_vertices and M1_indices to M4_vertices and M4_indices
void makeModels() {
//// M1 : Cube
// Replace the code below, that creates a simple square, with the one to create a cube.

// Resizes the vertices array. Repalce the values with the correct number of
// vertices components (3 * number of vertices)
    M1_vertices.resize(3 * 8);
    M1_vertices = {0,0,0, 0,0,1, 0,1,1, 0,1,0, 1,0,0, 1,0,1, 1,1,1, 1,1,0};





// Resizes the indices array. Repalce the values with the correct number of
// indices (3 * number of triangles)
M1_indices.resize(3 * 12);
M1_indices = {0,1,2, 2,3,0, 0,3,4, 4,3,7, 5,6,7, 7,4,5, 1,6,5, 1,2,6, 7,6,2, 3,2,7, 4,5,0, 1,0,5};



//// M2 : Cylinder
// Replace the code below, that creates a simple rotated square, with the one to create a cylinder.

// Resizes the vertices array. Repalce the values with the correct number of
// vertices components (3 * number of vertices)

int n = 72;
float r = 0.5;
float h = 6.0;

float alpha = glm::radians(360.0/n);

M2_vertices.resize((n + 1)*3*2);

// Vertices definitions 
int v = 0; 

// Triangle fan 1
for (int i = 0; i < n + 1; i++){
    M2_vertices[v] = r*glm::cos(alpha*i);
    M2_vertices[v + 1] = 0.0;
    M2_vertices[v + 2] = r*glm::sin(alpha*i);
    v += 3;
}

// Triangle fan 2
//t = 3*(n + 1);
for (int i = 0; i < n + 1; i++){
    M2_vertices[v] = r*glm::cos(alpha*i);
    M2_vertices[v + 1] = 0.0;
    M2_vertices[v + 2] = r*glm::sin(alpha*i);
    v += 3;
}
// Center roof
M2_vertices[0] = 0.0;
M2_vertices[1] = 0.0;
M2_vertices[2] = 0.0;

// Center bottom
M2_vertices[v + 1] = 0.0;
M2_vertices[v + 2] = 1.0;
M2_vertices[v + 3] = 0.0;


// Resizes the indices array. Repalce the values with the correct number of
// indices (3 * number of triangles)
M2_indices.resize(3*(n + 1)*5);

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

// TRY ME http://www.songho.ca/opengl/gl_sphere.html


M3_vertices.resize(9); //9

for (int sphere_row = 0; sphere_row < 10; sphere_row++){
    for (int sphere_col = 0; sphere_col < 10; sphere_col++){
        M3_vertices[sphere_row][sphere_col] = 
    }
    
}

/* Function to return a circle with centrum c, radius r of n vertices */
std::vector<tinyobj::real_t> vertexCircle(float r, std::vec3 c, int n){
    std::vector<tinyobj::real_t> vertex_circle;
    float angle = glm::rad(360.0f/n);

    for(int i = 0; i < 3*n; i++) {
        if ((i+1) % 3 == 0){ // z coordinate
            vertex_circle[i] = c.z;
        }
        else if (i == 0 || i % 3 == 0){ // x coordinate
            vertex_circle[i] = c.x +  + r * cos(i*angle);
        }
        else{ // y coordinate
            vertex_circle[i] = c.y + r * sin(i*angle); 
        }
    }
    return vertex_circle;
}

int n = 10;
float r = 1;
std::vec3 c = {0,0,0};

std::vector<tinyobj> top_circle = vertexCircle(r, c, n);



// Vertices definitions
/*
M3_vertices[0]  =  0.0;
M3_vertices[1]  =  1.0;
M3_vertices[2]  = -1.2;
M3_vertices[3]  = -0.866;
M3_vertices[4]  = -0.5;
M3_vertices[5]  = -1.2;
M3_vertices[6]  =  0.866;
M3_vertices[7]  = -0.5;
M3_vertices[8]  = -1.2;
*/


M3_vertices.resize(3*200);

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