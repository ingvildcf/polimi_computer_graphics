// This has been adapted from the Vulkan tutorial

#include "MyProject.hpp"

const std::string MODEL_PATH = "models/Terrain.obj";
const std::string TEXTURE_PATH = "textures/mapa.jpg";

const std::string MODEL_PATH_2 = "models/drone.obj";
const std::string TEXTURE_PATH_2 = "textures/espana.jpg";

// Position limits
static const float field_width = 30;
static const float field_length = 30;
static const float border_left = field_width / 2;
static const float border_right = (-1) * (field_width / 2);
static const float border_top = (-1) * (field_length / 2);
static const float border_bottom = field_length / 2;
static const float border_floor = 0;
static const float border_ceil = 8;

// Orientation Limits
static const float max_roll = glm::radians(45.0f);
static const float max_pitch = glm::radians(45.0f);
static const float max_yaw = glm::radians(45.0f);


static const float radius_drone = 2.5;

// Drone position and orientation
glm::vec3 global_pos_drone = glm::vec3(0, 0, 0);
float droneYaw = 0.0;
float dronePitch = 0.0;
float droneRoll = 0.0;
//glm::vec3 drone_orientation = glm::vec3(droneYaw, dronePitch, droneYaw);

// Top view
bool seenCenter = true;
float deltaHeight = 4.0f;


// Follow drone
bool seenFollow = false;
float followerDist = 0.10;
static glm::vec3 FollowerPos = global_pos_drone;

// Drone camera view
bool seenDrone = false;

//Extra view
bool seenExtra = false;

// Camera position
glm::vec3 cameraPos = global_pos_drone;

glm::mat3 CamDir = glm::mat3(1.0f);
glm::vec3 CamPos = glm::vec3(0.0f, 0.0f, 0.0f);

// The uniform buffer object used in this example
struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct GlobalUniformBufferObject {
	alignas(16) glm::vec3 lightDir;
	alignas(16) glm::vec4 lightColor;
	alignas(16) glm::vec3 eyePos;
};


// MAIN ! 
class droneSimulator : public BaseProject {
	protected:
	// Here you list all the Vulkan objects you need:
	
	// Descriptor Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSL1;
	DescriptorSetLayout DSL2;

	// Pipelines [Shader couples]
	Pipeline P1;
	Pipeline P2;

	// Models, textures and Descriptors (values assigned to the uniforms)
	Model M1;
	Texture T1;
	DescriptorSet DS1;

	Model M2;
	Texture T2;
	DescriptorSet DS2;
	
	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "Drone Simulator";
		initialBackgroundColor = {0.529f, 0.808f, 0.922f, 1.0f};
		
		// Descriptor pool sizes - 2 models loaded (terrain and drone)
		uniformBlocksInPool = 2;
		texturesInPool = 2;
		setsInPool = 2;
	}
	
	// Here you load and setup all your Vulkan objects
	void localInit() {
		// Descriptor Layouts [what will be passed to the shaders]
		DSL1.init(this, {
					// this array contains the binding:
					// first  element : the binding number
					// second element : the time of element (buffer or texture)
					// third  element : the pipeline stage where it will be used
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
				  });

		// Pipelines [Shader couples]
		P1.init(this, "shaders/vert.spv", "shaders/frag.spv", {&DSL1});

		// Models, textures and Descriptors (values assigned to the uniforms)
		M1.init(this, MODEL_PATH, 1);
		T1.init(this, TEXTURE_PATH);
		DS1.init(this, &DSL1, {
		// the second parameter, is a pointer to the Uniform Set Layout of this set
		// the last parameter is an array, with one element per binding of the set.
		// first  elmenet : the binding number
		// second element : UNIFORM or TEXTURE (an enum) depending on the type
		// third  element : only for UNIFORMs, the size of the corresponding C++ object
		// fourth element : only for TEXTUREs, the pointer to the corresponding texture object
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T1}
				});
		
		// MODEL 2
		DSL2.init(this, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
						 {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}});

		P2.init(this, "shaders/vert.spv", "shaders/frag.spv", {&DSL2});

		M2.init(this, MODEL_PATH_2, 2);
		T2.init(this, TEXTURE_PATH_2);
		DS2.init(this, &DSL2, {
			{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, 
			{1, TEXTURE, 0, &T2}
		});

		
	}

	// Here you destroy all the objects you created!		
	void localCleanup() {
		DS1.cleanup();
		T1.cleanup();
		M1.cleanup();
		P1.cleanup();
		DSL1.cleanup();

		// MODEL 2
		DS2.cleanup();
		T2.cleanup();
		M2.cleanup();
		P2.cleanup();
		DSL2.cleanup();
	}
	
	//Creation of the command buffer:
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
				
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				P1.graphicsPipeline);
				
		VkBuffer vertexBuffers[] = {M1.vertexBuffer};
		// property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		// property .indexBuffer of models, contains the VkBuffer handle to its index buffer
		vkCmdBindIndexBuffer(commandBuffer, M1.indexBuffer, 0,
								VK_INDEX_TYPE_UINT32);

		// property .pipelineLayout of a pipeline contains its layout.
		// property .descriptorSets of a descriptor set contains its elements.
		vkCmdBindDescriptorSets(commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						P1.pipelineLayout, 0, 1, &DS1.descriptorSets[currentImage],
						0, nullptr);
						
		// property .indices.size() of models, contains the number of triangles * 3 of the mesh.
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(M1.indices.size()), 1, 0, 0, 0);


		// MODELO 2
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, P2.graphicsPipeline);

		VkBuffer vertexBuffers2[] = {M2.vertexBuffer};

		VkDeviceSize offsets2[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers2, offsets2);
		vkCmdBindIndexBuffer(commandBuffer, M2.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, P2.pipelineLayout, 0, 1, &DS2.descriptorSets[currentImage], 0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M2.indices.size()), 1, 0, 0, 0);
	}

	// Update the uniforms
	void updateUniformBuffer(uint32_t currentImage) {
		
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		float lastTime = 0.0f;
		float deltaT = time - lastTime;
		lastTime = time;			

		// updates global uniforms
		GlobalUniformBufferObject gubo{};
		gubo.lightDir = glm::vec3(cos(glm::radians(135.0f)), sin(glm::radians(135.0f)), 0.0f);
		gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		gubo.eyePos = CamPos;

		UniformBufferObject ubo_terrain{};
		ubo_terrain.model =glm::rotate(glm::mat4(1.0f),
							glm::radians(90.0f),
							glm::vec3(0.0f, 0.0f, 1.0f));
		ubo_terrain.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
							   glm::vec3(0.0f, 0.0f, 0.0f),
							   glm::vec3(0.0f, 0.0f, 1.0f));
		ubo_terrain.proj = glm::perspective(glm::radians(45.0f),
						swapChainExtent.width / (float) swapChainExtent.height,
						0.1f, 10.0f);
		ubo_terrain.proj[1][1] *= -1;

		// Drone
		UniformBufferObject ubo_drone{};
		ubo_drone.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.1, 0.1, 0.1)) * glm::translate(glm::mat4(1.0f), global_pos_drone) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		ubo_drone.view = glm::lookAt(glm::vec3(0.0f, 2.0f, 3.0f),
										   glm::vec3(0.0f, 0.0f, 0.0f),
										   glm::vec3(0.0f, 0.0f, 1.0f));
		ubo_drone.proj = glm::perspective(glm::radians(45.0f),
												swapChainExtent.width / (float)swapChainExtent.height,
												0.1f, 10.0f);
		ubo_drone.proj[1][1] *= -1;
		
		void* data;


		// CAMERA UPDATE
		setCameraMode(window, seenCenter, seenDrone, seenFollow, seenExtra);

		// Update drone position and orientation
		getDroneInput(window);


		// Top view
		if (seenCenter) //GLFW_KEY_I
		{
			cameraPos = global_pos_drone + glm::vec3(0.0f, 0.0f, deltaHeight);
			
			ubo_terrain.view = LookAtMat(cameraPos, global_pos_drone , 0.0f);
			ubo_drone.model *= LookInDirMat(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(droneRoll, dronePitch, droneYaw));
			ubo_drone.view = LookAtMat(glm::vec3(0.0f, 0.0f, deltaHeight), glm::vec3(0.0f) , 0.0f); // Works quite well

		}
		// Follow drone
		else if (seenFollow) //GLFW_KEY_0
		{
			cameraPos = global_pos_drone + glm::vec3(0.0f, deltaHeight, deltaHeight/2);

			ubo_terrain.view = LookInDirMat(cameraPos, glm::vec3(droneRoll, dronePitch, droneYaw));

			ubo_drone.view = LookAtMat(global_pos_drone, global_pos_drone, 0.0f);

		}
		// Drone camera
		else if (seenDrone) //GLFW_KEY_P
		{	
			cameraPos = global_pos_drone;
		
			ubo_terrain.view = LookInDirMat(cameraPos, glm::vec3(droneRoll, dronePitch, droneYaw));

			ubo_drone.view = LookAtMat(cameraPos, global_pos_drone, 0.0f);
						
		}
		//Extra view
		else if (seenExtra) //GLFW_KEY_L
		{	
			cameraPos = global_pos_drone + glm::vec3(0.0f, 0.0f, deltaHeight);
			ubo_terrain.view = LookAtMat(cameraPos, global_pos_drone , 0.0f);
			ubo_drone.model *= LookInDirMat(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(droneRoll, dronePitch, droneYaw));
			ubo_drone.view = glm::lookAt(glm::vec3(2.0f, 0.0f, 1.0f),
											  glm::vec3(0.0f, 0.0f, 0.0f),
											  glm::vec3(0.0f, 0.0f, 1.0f));
						
		}

		if(glfwGetKey(window, GLFW_KEY_ESCAPE)){ cleanup(); }

		// Update uniforms
		//MODEL 1
		vkMapMemory(device, DS1.uniformBuffersMemory[0][currentImage], 0,
							sizeof(ubo_terrain), 0, &data);
		memcpy(data, &ubo_terrain, sizeof(ubo_terrain));
		vkUnmapMemory(device, DS1.uniformBuffersMemory[0][currentImage]);
		
		//MODEL 2
		vkMapMemory(device, DS2.uniformBuffersMemory[0][currentImage], 0,
							sizeof(ubo_drone), 0, &data);
		memcpy(data, &ubo_drone, sizeof(ubo_drone));
		vkUnmapMemory(device, DS2.uniformBuffersMemory[0][currentImage]);
	
	}	

	void getDroneInput(GLFWwindow *window){
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		float lastTime = 0.0f;
		float deltaT = time - lastTime;
		lastTime = time;

		const float ROT_SPEED = glm::radians(.009f);
		const float MOVE_SPEED = 0.01;

		if(glfwGetKey(window, GLFW_KEY_LEFT)) {
			droneYaw += deltaT * ROT_SPEED;
		}
		if(glfwGetKey(window, GLFW_KEY_RIGHT)) {
			droneYaw -= deltaT * ROT_SPEED;
		}
		if(glfwGetKey(window, GLFW_KEY_UP)) {
			dronePitch += deltaT * ROT_SPEED;
		}
		if(glfwGetKey(window, GLFW_KEY_DOWN)) {
			dronePitch -= deltaT * ROT_SPEED;
		}
		if(glfwGetKey(window, GLFW_KEY_Q)) {
			droneRoll -= deltaT * ROT_SPEED;
		}
		if(glfwGetKey(window, GLFW_KEY_E)) {
			droneRoll += deltaT * ROT_SPEED;
		}
		
		ensureOrientationLimits();



		if(glfwGetKey(window, GLFW_KEY_A)) {
			global_pos_drone -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), 0.0f,
									glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1,0,0,1)) * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_D)) {
			global_pos_drone += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), 0.0f,
									glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1,0,0,1)) * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_S)) {
			global_pos_drone -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), 0.0f,
									glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0,1,0,1)) * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_W)) {
			global_pos_drone += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), 0.0f,
									glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0,1,0,1)) * deltaT;
		}

		if(glfwGetKey(window, GLFW_KEY_F)){ 
        	global_pos_drone -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), 0.0f,
									glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0,0,1,1)) * deltaT;
    	}

    	if(glfwGetKey(window, GLFW_KEY_R)){         	
        	global_pos_drone += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), droneYaw,
									glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0,0,1,1)) * deltaT;
    	}
		
		ensurePosLimits();
	}

	void ensurePosLimits(){
		if (global_pos_drone.x + radius_drone > border_left)
		{
			global_pos_drone.x = border_left - radius_drone;
		}
		else if (global_pos_drone.y + radius_drone > border_bottom)
		{
			global_pos_drone.y = border_bottom - radius_drone;
		}
		else if (global_pos_drone.x - radius_drone < border_right)
		{
			global_pos_drone.x = border_right + radius_drone;
		}
		else if (global_pos_drone.y - radius_drone < border_top)
		{
			global_pos_drone.y = border_top + radius_drone;
		}
		else if (global_pos_drone.z < border_floor)
		{
			global_pos_drone.z = border_floor;
		}
		else if (global_pos_drone.z > border_ceil)
		{
			global_pos_drone.z = border_ceil;
		}
		else
		{
			return;
		}
	}

	void ensureOrientationLimits(){
		if (droneRoll > max_roll)
		{
			droneRoll = max_roll;
		}
		else if (droneRoll < -max_roll)
		{
			droneRoll = -max_roll;
		}
		else if (droneYaw > max_yaw)
		{
			droneYaw = max_yaw;
		}
		else if (droneYaw < -max_yaw)
		{
			droneYaw = -max_yaw;
		}
		else if (dronePitch > max_pitch)
		{
			dronePitch = max_pitch;
		}
		else if (dronePitch < -max_pitch)
		{
			dronePitch = -max_pitch;
		}
		else
		{
			return;
		}
	}

	void setCameraMode(GLFWwindow *window, bool &seenCenter, bool &seenDrone, bool &seenFollow, bool &seenExtra){
		if (glfwGetKey(window, GLFW_KEY_I))
		{ // Top view
			seenCenter = true;
			seenFollow = false;
			seenDrone = false;
			seenExtra = false;
		}
		if (glfwGetKey(window, GLFW_KEY_O))
		{ // follow drone
			seenCenter = false;
			seenFollow = true;
			seenDrone = false;
			seenExtra = false;
		}
		if (glfwGetKey(window, GLFW_KEY_P))
		{ // camera drone
			seenCenter = false;
			seenFollow = false;
			seenDrone = true;
			seenExtra = false;
		}
		if (glfwGetKey(window, GLFW_KEY_L))
		{ // camera drone
			seenCenter = false;
			seenFollow = false;
			seenDrone = false;
			seenExtra = true;
		}
	}

	glm::mat4 I = glm::mat4(1.0f); // Identity matrix
	glm::vec3 xAxis = glm::vec3(1,0,0); // x axis
	glm::vec3 yAxis = glm::vec3(0,1,0); // y axis
	glm::vec3 zAxis = glm::vec3(0,0,1); // z axis

	glm::mat4 LookInDirMat(glm::vec3 Pos, glm::vec3 Angs) {
		glm::mat4 T = glm::translate(I, -Pos);
		glm::mat4 R_y = glm::rotate(I, -Angs.x, yAxis);
		glm::mat4 R_x = glm::rotate(I, -Angs.y, xAxis);
		glm::mat4 R_z = glm::rotate(I, -Angs.z, zAxis);

		glm::mat4 M_V = R_z*R_x*R_y*T;
		return M_V;
	}


	glm::mat4 LookAtMat(glm::vec3 Pos, glm::vec3 aim, float Roll) {
		glm::mat4 R = glm::rotate(I, glm::radians(Roll), yAxis);
		glm::vec3 u = yAxis;
		glm::mat4 M_V = R*glm::lookAt(Pos, aim, u);
		return M_V;
	}
};


// Main
int main() {
    droneSimulator app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}