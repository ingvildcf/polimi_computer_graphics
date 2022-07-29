// This has been adapted from the Vulkan tutorial

#include "MyProject.hpp"

const std::string MODEL_PATH = "models/Terrain.obj";
const std::string TEXTURE_PATH = "textures/Terrain.png";

/*
glm::mat4 I = glm::mat4(1.0f); // Identity matrix
glm::vec3 xAxis = glm::vec3(1,0,0); // x axis
glm::vec3 yAxis = glm::vec3(0,1,0); // y axis
glm::vec3 zAxis = glm::vec3(0,0,1); // z axis

// Create the world matrix for the robot
glm::mat4 getRobotWorldMatrix(GLFWwindow* window) {
	// Initial position
	float x = -3, y = 0, z = 2;
	static glm::vec3 pos = glm::vec3(x, y, z);

	// Initial direction
	float roll = 0, pitch = 0, yaw = 0;
	static glm::vec3 dir = glm::vec3(roll, pitch, yaw);

	float move_x = 0, move_y = 0, move_z = 0;
	float a180 = glm::radians(180.0f);
	float a90 = glm::radians(90.0f);
	float a0 = glm::radians(0.0f);
	
	if(glfwGetKey(window, GLFW_KEY_S)){ // Move backwards
		pitch = a180, yaw = a0;
		move_x -= 0.1f;
	}
	if(glfwGetKey(window, GLFW_KEY_W)){ // Move forwards
		pitch = a0, yaw = a0;
		move_x += 0.1f;
	}
	if(glfwGetKey(window, GLFW_KEY_A)){ // Move to the left
		pitch = a90, yaw = a0; // turn left
		move_z -= 0.1f;
	}
	if(glfwGetKey(window, GLFW_KEY_D)){ // Move to the right
		pitch = -a90, yaw = a0; // turn right
		move_z += 0.1f;
	}
	if(glfwGetKey(window, GLFW_KEY_F)){ // Move down
		roll = a0, pitch = a0, yaw = -a90;
		move_y -= 0.1f;
	}
	if(glfwGetKey(window, GLFW_KEY_R)){ // Move up
		pitch = a0, yaw = a90;
		move_y += 0.1f;
	}
	pos += glm::vec3(move_x, move_y, move_z);
	//dir = glm::vec3(roll, pitch, yaw);

	glm::mat4 R_x = glm::rotate(I, roll, xAxis);
	glm::mat4 R_y = glm::rotate(I, pitch, yAxis);
	glm::mat4 R_z = glm::rotate(I, yaw, zAxis);

	glm::mat4 T = glm::translate(I, pos);
	
	glm::mat4 WM = T*R_x*R_y*R_z;
	
	return WM;
}
*/

// The uniform buffer object used in this example
struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};


// MAIN ! 
class MyProject : public BaseProject {
	protected:
	// Here you list all the Vulkan objects you need:
	
	// Descriptor Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSL1;

	// Pipelines [Shader couples]
	Pipeline P1;

	// Models, textures and Descriptors (values assigned to the uniforms)
	Model M1;
	Texture T1;
	DescriptorSet DS1;
	
	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "My Project";
		initialBackgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 1;
		texturesInPool = 1;
		setsInPool = 1;
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
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		P1.init(this, "shaders/vert.spv", "shaders/frag.spv", {&DSL1});

		// Models, textures and Descriptors (values assigned to the uniforms)
		M1.init(this, MODEL_PATH);
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
	}

	// Here you destroy all the objects you created!		
	void localCleanup() {
		DS1.cleanup();
		T1.cleanup();
		M1.cleanup();
		P1.cleanup();
		DSL1.cleanup();
	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
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
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>
					(currentTime - startTime).count();
					
					
		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f),
								time * glm::radians(90.0f),
								glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
							   glm::vec3(0.0f, 0.0f, 0.0f),
							   glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f),
						swapChainExtent.width / (float) swapChainExtent.height,
						0.1f, 10.0f);
		ubo.proj[1][1] *= -1;
		
		void* data;

		// Here is where you actually update your uniforms
		vkMapMemory(device, DS1.uniformBuffersMemory[0][currentImage], 0,
							sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, DS1.uniformBuffersMemory[0][currentImage]);
	}	
};

// This is the main: probably you do not need to touch this!
int main() {
    MyProject app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}