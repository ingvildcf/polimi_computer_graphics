// this function creates the geometries to be shown, and output thems
// in global variables M1_vertices and M1_indices to M4_vertices and M4_indices

struct Vertex {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

std::vector<Vertex> M1_vertices;
std::vector<Vertex> M2_vertices;


void makeModels() {
//// M1 : Cube
M1_vertices.resize(4);

// first vertex of M1
M1_vertices[0].pos  = glm::vec3(-1.0,-1.0,-1.0);
M1_vertices[0].norm = glm::vec3(0.0,0.0,1.0);
M1_vertices[0].UV   = glm::vec2(0.0,1.0);

// second vertex of M1
M1_vertices[1].pos  = glm::vec3(1.0,-1.0,-1.0);
M1_vertices[1].norm = glm::vec3(0.0,0.0,1.0);
M1_vertices[1].UV   = glm::vec2(1.0,1.0);

// third vertex of M1
M1_vertices[2].pos  = glm::vec3(1.0,1.0,-1.0);
M1_vertices[2].norm = glm::vec3(0.0,0.0,1.0);
M1_vertices[2].UV   = glm::vec2(1.0,0.0);

// fourth vertex of M1
M1_vertices[3].pos  = glm::vec3(-1.0,1.0,-1.0);
M1_vertices[3].norm = glm::vec3(0.0,0.0,1.0);
M1_vertices[3].UV   = glm::vec2(0.0,0.0);


// Resizes the indices array. Repalce the values with the correct number of
// indices (3 * number of triangles)
M1_indices.resize(3 * 2);

// first triangle
M1_indices[0] = 0;
M1_indices[1] = 1;
M1_indices[2] = 2;

// second triangle
M1_indices[3] = 2;
M1_indices[4] = 3;
M1_indices[5] = 0;


//// M2 : Cylinder
// Replace the code below, that creates a simple rotated square, with the one to create a cylinder.

// Resizes the vertices array. Repalce the values with the correct number of
// vertices
M2_vertices.resize(4);

// Vertices definitions
M2_vertices[0].pos  = glm::vec3(0.0,-1.0,-1.1);
M2_vertices[0].norm = glm::vec3(0.0,0.0,1.0);
M2_vertices[0].UV   = glm::vec2(0.0,1.0);

M2_vertices[1].pos  = glm::vec3(1.0,0.0,-1.1);
M2_vertices[1].norm = glm::vec3(0.0,0.0,1.0);
M2_vertices[1].UV   = glm::vec2(1.0,1.0);

M2_vertices[2].pos  = glm::vec3(0.0,1.0,-1.1);
M2_vertices[2].norm = glm::vec3(0.0,0.0,1.0);
M2_vertices[2].UV   = glm::vec2(1.0,0.0);

M2_vertices[3].pos  = glm::vec3(-1.0,0.0,-1.1);
M2_vertices[3].norm = glm::vec3(0.0,0.0,1.0);
M2_vertices[3].UV   = glm::vec2(0.0,0.0);


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

}