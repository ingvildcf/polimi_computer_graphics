#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <optional>
#include <set>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <array>
#include <unordered_map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "models/";
const std::string TEXTURE_PATH = "textures/";
const std::string SHADER_PATH = "shaders/";

const int MAX_FRAMES_IN_FLIGHT = 2;

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
	const bool Verbose = false;
#else
	const bool enableValidationLayers = true;
	const bool Verbose = true;
#endif

enum PipelineType {Flat, Wire};

struct Model {
	const char *ObjFile;
	const char *TextureFile;
	const glm::vec3 pos;
	const float scale;
	const PipelineType pt;
};

const std::vector<Model> SceneToLoad = {
	{"X-WING-baker.obj", "XwingColors.png", {0,0,0}, 0.1, Flat},
	{"X-WING-baker.obj", "XwingColors.png", {0,0,0}, 0.1, Wire}
};

struct SkyBoxModel {
	const char *ObjFile;
	const char *TextureFile[6];
};

const SkyBoxModel SkyBoxToLoad = {"SkyBoxCube.obj", {"sky/bkg1_right.png", "sky/bkg1_left.png", "sky/bkg1_top.png", "sky/bkg1_bot.png", "sky/bkg1_front.png", "sky/bkg1_back.png"}};

struct SingleText {
	int usedLines;
	const char *l[4];
	int start;
	int len;
};

std::vector<SingleText> SceneText = {
	{1, {"MT1 - Translate of +2 on the x axis, and -1 on the y axis", "", "", ""}, 0, 0},
	{1, {"MT2 - Rotate of 45 degrees on the x axis", "", "", ""}, 0, 0},
	{1, {"MT3 - Make the object 2 times smaller", "", "", ""}, 0, 0},
	{2, {"MT4 - Make the object 2 times longer on the z axis,", "and half on the other axis", "", ""}, 0, 0},
	{1, {"MT5 - Mirror over the z axis", "", "", ""}, 0, 0},
	{1, {"MT6 - Flatten over the z axis", "", "", ""}, 0, 0},
	{2, {"MT7 - Make a shear along the Y axis,", "with a factor of 1 along the x axis", "", ""}, 0, 0}};

#include "transforms.cpp"
float VisMT[7*16];

namespace std {
	template<> struct hash<std::vector<float>> {
		size_t operator()(std::vector<float> const& vertex) const {
			size_t h = 0;
			for(int i = 0; i < vertex.size(); i++) {
				h ^= static_cast<size_t>(vertex[i]);
			}
			return h;
		}
	};
}


struct VertexDescriptor {
	bool hasPos;
	bool hasNormal;
	bool hasTexCoord;
	bool hasColor;
	bool hasTangent;
	
	int deltaPos;
	int deltaNormal;
	int deltaTexCoord;
	int deltaColor;
	int deltaTangent;
	
	int locPos;
	int locNormal;
	int locTexCoord;
	int locColor;
	int locTangent;
	
	int size;
	int loc;
	
	VertexDescriptor(bool hPos, bool hNormal, bool hTexCoord, bool hColor, bool hTangent) {
		size = 0;
		loc = 0;
		
		hasPos = hPos;
		hasNormal = hNormal;
		hasTexCoord = hTexCoord;
		hasColor = hColor;
		hasTangent = hTangent;
		
		if(hasPos) {deltaPos = size; size += 3; locPos = loc; loc++;} else {deltaPos = -1; locPos = -1;}
		if(hasNormal) {deltaNormal = size; size += 3; locNormal = loc; loc++;} else {deltaNormal = -1; locNormal = -1;}
		if(hasTexCoord) {deltaTexCoord = size; size += 2; locTexCoord = loc; loc++;} else {deltaTexCoord = -1; locTexCoord = -1;}
		if(hasColor) {deltaColor = size; size += 4; locColor = loc; loc++;} else {deltaColor = -1; locColor = -1;}
		if(hasTangent) {deltaTangent = size; size += 4; locTangent = loc; loc++;} else {deltaTangent = -1; locTangent = -1;}
	}

	
	VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = size * sizeof(float);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		
		return bindingDescription;
	}
	
	std::vector<VkVertexInputAttributeDescription>
						getAttributeDescriptions(int binding = 0) {
		std::vector<VkVertexInputAttributeDescription>
						attributeDescriptions{};
		attributeDescriptions.resize(loc);
		if(hasPos) {
			attributeDescriptions[locPos].binding = binding;
			attributeDescriptions[locPos].location = locPos;
			attributeDescriptions[locPos].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[locPos].offset = deltaPos * sizeof(float);
		}
						
		if(hasNormal) {
			attributeDescriptions[locNormal].binding = binding;
			attributeDescriptions[locNormal].location = locNormal;
			attributeDescriptions[locNormal].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[locNormal].offset = deltaNormal * sizeof(float);
		}
		
		if(hasTexCoord) {
			attributeDescriptions[locTexCoord].binding = binding;
			attributeDescriptions[locTexCoord].location = locTexCoord;
			attributeDescriptions[locTexCoord].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[locTexCoord].offset = deltaTexCoord * sizeof(float);
		}
								
		if(hasColor) {
			attributeDescriptions[locColor].binding = binding;
			attributeDescriptions[locColor].location = locColor;
			attributeDescriptions[locColor].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[locColor].offset = deltaColor * sizeof(float);
		}
						
		if(hasTangent) {
			attributeDescriptions[locTangent].binding = binding;
			attributeDescriptions[locTangent].location = locTangent;
			attributeDescriptions[locTangent].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[locTangent].offset = deltaTangent * sizeof(float);
		}
					
		return attributeDescriptions;
	}

	glm::vec3 getPos(float *data, int i) {
		if(hasPos) {
			return glm::vec3(data[i * size + deltaPos], data[i * size + deltaPos + 1], data[i * size + deltaPos + 2]);
		} else {
			return glm::vec3(0.0f);
			std::cerr << "Vertex has no position \n";
		}
	}
	
	void setPos(float *data, int i, glm::vec3 pos) {
		if(hasPos) {
			data[i * size + deltaPos] = pos.x;
			data[i * size + deltaPos + 1] = pos.y;
			data[i * size + deltaPos + 2] = pos.z;
		} else {
			std::cerr << "Vertex has no position \n";
		}
	}

	glm::vec3 getNormal(float *data, int i) {
		if(hasPos) {
			return glm::vec3(data[i * size + deltaNormal], data[i * size + deltaNormal + 1], data[i * size + deltaNormal + 2]);
		} else {
			return glm::vec3(0.0f);
			std::cerr << "Vertex has no normal \n";
		}
	}
	
	void setNormal(float *data, int i, glm::vec3 norm) {
		if(hasNormal) {
			data[i * size + deltaNormal] = norm.x;
			data[i * size + deltaNormal + 1] = norm.y;
			data[i * size + deltaNormal + 2] = norm.z;
		} else {
			std::cerr << "Vertex has no normal \n";
		}
	}

	glm::vec2 getTexCoord(float *data, int i) {
		if(hasPos) {
			return glm::vec2(data[i * size + deltaTexCoord], data[i * size + deltaTexCoord + 1]);
		} else {
			return glm::vec2(0.0f);
			std::cerr << "Vertex has no UV \n";
		}
	}
	
	void setTexCoord(float *data, int i, glm::vec3 uv) {
		if(hasNormal) {
			data[i * size + deltaTexCoord] = uv.x;
			data[i * size + deltaTexCoord + 1] = uv.y;
		} else {
			std::cerr << "Vertex has no UV \n";
		}
	}
};







struct CharData {
	int x;
	int y;
	int width;
	int height;
	int xoffset;
	int yoffset;
	int xadvance;
};

struct FontDef {
	int lineHeight;
	std::vector<CharData> P;
};

std::vector<FontDef> Fonts = {{73,{{0,0,0,0,0,0,21},{116,331,18,61,4,4,21},{379,444,29,30,-1,4,26},{135,331,51,61,-4,4,41},{206,0,46,74,-2,0,41},{320,143,70,62,-2,5,65},{0,143,54,63,-1,4,49},{409,444,18,30,1,4,14},{25,0,27,77,1,3,24},{53,0,28,77,-2,3,24},{345,444,33,30,-1,4,29},{156,444,48,46,-1,19,43},{428,444,18,28,2,48,21},{26,492,29,17,-1,32,24},{56,492,18,17,2,48,21},{478,0,33,61,-4,4,21},{55,143,46,62,-2,5,41},{225,331,29,60,3,5,41},{255,331,46,60,-2,5,41},{102,143,46,62,-2,5,41},{302,331,47,60,-2,5,41},{453,143,46,61,-2,6,41},{149,143,46,62,-2,5,41},{350,331,47,59,-2,6,41},{196,143,46,62,-2,5,41},{243,143,46,62,-2,5,41},{137,444,18,47,4,18,21},{427,331,18,58,4,18,21},{344,393,52,48,-3,18,43},{247,444,48,31,-1,27,43},{397,393,52,48,-3,18,43},{391,143,43,62,1,3,41},
{116,78,68,64,4,3,74},{124,207,57,61,-3,4,49},{182,207,51,61,1,4,49},{360,0,57,64,-1,3,52},{234,207,53,61,2,4,52},{288,207,49,61,2,4,49},{338,207,47,61,2,4,44},{418,0,59,64,-1,3,57},{386,207,53,61,1,4,52},{440,207,18,61,3,4,21},{411,78,41,63,-3,4,36},{0,269,54,61,1,4,49},{55,269,45,61,1,4,41},{101,269,61,61,1,4,60},{163,269,52,61,1,4,52},{0,78,61,64,-1,3,57},{216,269,50,61,2,4,49},{253,0,61,67,-1,3,57},{267,269,54,61,2,4,52},{62,78,53,64,-1,3,49},{459,207,53,61,-3,4,44},{453,78,52,63,2,4,52},{322,269,56,61,-2,4,49},{379,269,77,61,-3,4,68},{0,331,57,61,-3,4,49},{58,331,57,61,-3,4,49},{457,269,52,61,-3,4,44},{154,0,25,76,0,4,21},{187,331,37,61,-7,4,21},{180,0,25,76,-3,4,21},{205,444,41,40,0,4,34},{75,492,51,13,-4,61,41},
{0,492,25,20,-3,3,24},{0,393,46,50,-1,17,41},{185,78,44,63,0,4,41},{184,393,43,49,-2,17,36},{230,78,44,63,-2,4,41},{47,393,46,50,-2,17,41},{290,143,29,62,-3,3,21},{315,0,44,64,-2,17,41},{0,207,42,61,0,4,41},{43,207,18,61,0,4,16},{0,0,24,77,-6,4,16},{62,207,43,61,0,4,36},{106,207,17,61,1,4,16},{446,331,62,48,0,17,60},{228,393,42,48,0,17,41},{94,393,46,50,-2,17,41},{275,78,44,63,0,17,41},{320,78,44,63,-2,17,41},{271,393,29,48,1,17,24},{141,393,42,50,-2,17,36},{398,331,28,59,-3,7,21},{301,393,42,48,0,18,41},{0,444,46,47,-4,18,36},{450,393,62,47,-4,18,52},{47,444,46,47,-4,18,36},{365,78,45,63,-3,18,36},{94,444,42,47,-2,18,36},{82,0,35,77,-6,3,25},{435,143,17,62,1,3,19},{118,0,35,77,-3,3,25},{447,444,53,24,-4,30,43},{296,444,48,31,-1,27,43}}},
{30,{{512,0,0,0,0,0,9},{740,149,11,28,0,0,9},{723,80,16,15,-2,0,11},{600,58,25,28,-3,0,17},{631,103,22,34,-2,-2,17},{512,59,32,28,-2,1,27},{542,147,26,29,-2,0,21},{740,235,11,15,-1,0,6},{723,96,15,34,-1,0,10},{723,131,15,34,-2,0,10},{599,239,17,16,-2,0,12},{600,125,24,22,-2,6,18},{740,35,12,15,-1,18,9},{700,242,16,9,-2,12,10},{740,51,12,10,-1,18,9},{700,213,17,28,-3,0,9},{655,0,22,28,-2,1,17},{655,228,16,27,0,1,17},{631,228,22,27,-2,1,17},{655,29,22,28,-2,1,17},{631,0,23,27,-2,1,17},{655,58,22,28,-2,1,17},{655,87,22,28,-2,1,17},{631,28,23,27,-2,1,17},{655,116,22,28,-2,1,17},{655,145,22,28,-2,1,17},{754,0,11,22,0,6,9},{740,207,11,27,0,6,9},{573,116,26,23,-3,6,18},{600,148,24,15,-2,10,18},{573,140,26,23,-3,6,18},{678,195,21,28,-1,0,17},
{512,29,32,29,0,0,31},{545,59,27,28,-3,0,21},{573,194,25,28,-1,0,21},{545,29,27,29,-2,0,22},{542,177,26,28,-1,0,22},{600,96,24,28,-1,0,21},{599,210,23,28,-1,0,19},{512,224,28,29,-2,0,24},{573,223,25,28,-1,0,22},{740,120,11,28,0,0,9},{678,107,21,29,-3,0,15},{542,206,26,28,-1,0,21},{655,174,22,28,-1,0,17},{512,149,29,28,-1,0,25},{605,0,25,28,-1,0,22},{512,119,29,29,-2,0,24},{600,29,25,28,-1,0,21},{512,88,29,30,-2,0,24},{578,0,26,28,-1,0,22},{542,117,26,29,-2,0,21},{573,29,26,28,-3,0,19},{573,164,25,29,-1,0,22},{573,58,26,28,-2,0,21},{512,0,36,28,-3,0,29},{542,88,27,28,-3,0,21},{549,0,28,28,-3,0,21},{573,87,26,28,-3,0,19},{740,0,13,34,-1,0,9},{700,184,18,28,-4,0,9},{723,202,14,34,-3,0,9},{700,94,20,19,-1,0,15},{600,87,25,8,-3,24,17},
{723,237,14,11,-3,0,10},{631,56,23,23,-2,6,17},{678,47,21,29,-1,0,17},{678,224,21,24,-2,5,15},{631,138,22,29,-2,0,17},{678,0,22,23,-2,6,17},{723,0,16,28,-3,0,9},{631,168,22,29,-2,6,17},{678,137,21,28,-1,0,17},{740,62,11,28,-1,0,7},{723,166,14,35,-4,0,7},{678,166,21,28,-1,0,15},{740,91,11,28,-1,0,7},{512,178,29,22,-1,6,25},{701,24,21,22,-1,6,17},{655,203,22,24,-2,5,17},{678,77,21,29,-1,6,17},{631,198,22,29,-2,6,17},{723,57,16,22,-1,6,10},{701,0,21,23,-2,6,15},{723,29,16,27,-3,2,9},{700,70,20,23,-1,6,17},{631,80,23,22,-3,6,15},{512,201,29,22,-3,6,22},{678,24,22,22,-3,6,15},{599,180,23,29,-3,6,15},{700,47,21,22,-2,6,15},{700,149,18,34,-4,0,11},{740,178,11,28,-1,0,8},{700,114,19,34,-3,0,11},{542,235,26,13,-3,11,18},{599,164,24,15,-2,10,18}}},
{16,{{768,0,0,0,0,0,5},{914,51,8,16,-1,-1,5},{902,17,11,9,-3,-1,6},{825,127,15,16,-3,-1,9},{789,135,14,19,-3,-2,9},{768,17,20,17,-3,-1,15},{768,121,17,17,-3,-1,11},{914,126,8,9,-2,-1,4},{902,27,10,20,-2,-1,6},{902,48,10,20,-3,-1,6},{825,144,12,10,-3,-1,7},{873,87,14,13,-2,2,10},{914,116,8,9,-2,9,5},{842,147,11,7,-3,5,6},{914,136,8,6,-2,9,5},{902,0,11,16,-3,-1,5},{842,21,14,17,-3,-1,9},{858,138,11,16,-2,-1,9},{858,72,14,16,-3,-1,9},{842,39,14,17,-3,-1,9},{825,42,15,16,-3,-1,9},{858,89,14,16,-3,0,9},{842,57,14,17,-3,-1,9},{768,139,15,15,-3,0,9},{842,75,14,17,-3,-1,9},{842,93,14,17,-3,-1,9},{914,102,8,13,-1,2,5},{914,68,8,16,-1,2,5},{807,119,16,14,-3,2,10},{873,101,14,10,-2,4,10},{825,0,16,14,-3,2,10},{888,34,13,16,-2,-1,9},
{768,35,19,17,-1,-1,17},{790,0,17,16,-3,-1,11},{825,59,15,16,-2,-1,11},{768,103,17,17,-3,-1,12},{808,0,16,16,-2,-1,12},{825,76,15,16,-2,-1,11},{825,93,15,16,-2,-1,10},{789,99,16,17,-2,-1,13},{807,17,16,16,-2,-1,12},{914,34,8,16,-2,-1,5},{873,123,13,17,-3,-1,8},{807,34,16,16,-2,-1,11},{858,106,14,16,-2,-1,9},{789,17,17,16,-2,-1,14},{807,51,16,16,-2,-1,12},{768,53,18,17,-3,-1,13},{825,110,15,16,-2,-1,11},{768,71,18,17,-3,-1,13},{807,68,16,16,-2,-1,12},{825,24,15,17,-2,-1,11},{807,85,16,16,-3,-1,10},{789,117,16,17,-2,-1,12},{789,34,17,16,-3,-1,11},{768,0,21,16,-3,-1,16},{789,51,17,16,-3,-1,11},{789,68,17,16,-3,-1,11},{807,102,16,16,-3,-1,10},{902,129,9,20,-2,-1,5},{888,121,12,16,-4,-1,5},{902,69,10,20,-3,-1,5},{888,66,13,12,-2,-1,8},{842,15,15,5,-3,12,9},
{902,121,10,7,-3,-1,6},{842,0,15,14,-3,2,9},{842,111,14,17,-2,-1,9},{858,123,14,14,-3,2,8},{842,129,14,17,-3,-1,9},{873,0,14,14,-3,2,9},{888,138,10,16,-3,-1,5},{858,0,14,17,-3,2,9},{888,0,13,16,-2,-1,9},{914,0,8,16,-2,-1,4},{807,134,10,20,-4,-1,4},{888,17,13,16,-2,-1,8},{914,17,8,16,-2,-1,4},{768,89,18,13,-2,2,14},{873,141,13,13,-2,2,9},{873,15,14,14,-3,2,9},{858,18,14,17,-2,2,9},{858,36,14,17,-3,2,9},{902,107,10,13,-2,2,6},{873,30,14,14,-3,2,8},{902,90,10,16,-3,0,5},{888,51,13,14,-2,2,9},{873,45,14,13,-3,2,8},{789,85,17,13,-3,2,12},{873,59,14,13,-3,2,8},{858,54,14,17,-3,2,8},{873,73,14,13,-3,2,8},{888,79,12,20,-4,-1,6},{914,85,8,16,-2,-1,5},{888,100,12,20,-3,-1,6},{825,15,16,8,-3,5,10},{873,112,14,10,-2,4,10}}}};
const float VisV[] = {1.0,0.0,0.0,2.0,0.0,1.0,0.0,-1.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,0.0,0.707,-0.707,0.0,0.0,0.707,0.707,0.0,0.0,0.0,0.0,1.0,0.5,0.0,0.0,0.0,0.0,0.5,0.0,0.0,0.0,0.0,0.5,0.0,0.0,0.0,0.0,1.0,0.5,0.0,0.0,0.0,0.0,0.5,0.0,0.0,0.0,0.0,2.0,0.0,0.0,0.0,0.0,1.0,-1.0,0.0,0.0,0.0,0.0,-1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,1.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0};



// Questi dovrebbero essere definibili a piacimento e nel modo piu' semplice possibile,
// traferirsi nello shader
struct UniformBufferObject {
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

struct GlobalUniformBufferObject {
	alignas(16) glm::vec3 lightDir;
	alignas(16) glm::vec4 lightColor;
	alignas(16) glm::vec3 eyePos;
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
				vkGetInstanceProcAddr(instance,
					"vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
			VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
				vkGetInstanceProcAddr(instance,
					"vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() &&
			   presentFamily.has_value();
	}
};
    
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


struct errorcode {
	VkResult resultCode;
	std::string meaning;
}
ErrorCodes[ ] = {
	{ VK_NOT_READY, "Not Ready" },
	{ VK_TIMEOUT, "Timeout" },
	{ VK_EVENT_SET, "Event Set" },
	{ VK_EVENT_RESET, "Event Reset" },
	{ VK_INCOMPLETE, "Incomplete" },
	{ VK_ERROR_OUT_OF_HOST_MEMORY, "Out of Host Memory" },
	{ VK_ERROR_OUT_OF_DEVICE_MEMORY, "Out of Device Memory" },
	{ VK_ERROR_INITIALIZATION_FAILED, "Initialization Failed" },
	{ VK_ERROR_DEVICE_LOST, "Device Lost" },
	{ VK_ERROR_MEMORY_MAP_FAILED, "Memory Map Failed" },
	{ VK_ERROR_LAYER_NOT_PRESENT, "Layer Not Present" },
	{ VK_ERROR_EXTENSION_NOT_PRESENT, "Extension Not Present" },
	{ VK_ERROR_FEATURE_NOT_PRESENT, "Feature Not Present" },
	{ VK_ERROR_INCOMPATIBLE_DRIVER, "Incompatible Driver" },
	{ VK_ERROR_TOO_MANY_OBJECTS, "Too Many Objects" },
	{ VK_ERROR_FORMAT_NOT_SUPPORTED, "Format Not Supported" },
	{ VK_ERROR_FRAGMENTED_POOL, "Fragmented Pool" },
	{ VK_ERROR_SURFACE_LOST_KHR, "Surface Lost" },
	{ VK_ERROR_NATIVE_WINDOW_IN_USE_KHR, "Native Window in Use" },
	{ VK_SUBOPTIMAL_KHR, "Suboptimal" },
	{ VK_ERROR_OUT_OF_DATE_KHR, "Error Out of Date" },
	{ VK_ERROR_INCOMPATIBLE_DISPLAY_KHR, "Incompatible Display" },
	{ VK_ERROR_VALIDATION_FAILED_EXT, "Valuidation Failed" },
	{ VK_ERROR_INVALID_SHADER_NV, "Invalid Shader" },
	{ VK_ERROR_OUT_OF_POOL_MEMORY_KHR, "Out of Pool Memory" },
	{ VK_ERROR_INVALID_EXTERNAL_HANDLE, "Invalid External Handle" },

};

void PrintVkError( VkResult result ) {
	const int numErrorCodes = sizeof( ErrorCodes ) / sizeof( struct errorcode );
	std::string meaning = "";
	for( int i = 0; i < numErrorCodes; i++ ) {
		if( result == ErrorCodes[i].resultCode ) {
			meaning = ErrorCodes[i].meaning;
			break;
		}
	}
	std::cout << "Error: " << result << ", " << meaning << "\n";
}

struct ModelData {
	VertexDescriptor *vertDesc;
	std::vector<float> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;	
};

struct TextureData {
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	uint32_t mipLevels;
};	

struct SceneModel {
	// Model data
	ModelData MD;
	
	// Texture data
	TextureData TD;
};


class Assignment02 {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	VkRenderPass renderPass;
	
	VkDescriptorPool descriptorPool;

	VertexDescriptor 	phongAndSkyBoxVertices = VertexDescriptor(true, true, true, false, false);
	VertexDescriptor 	textVertices = VertexDescriptor(true, false, true, false, false);

	// Phong pipeline
 	VkDescriptorSetLayout PhongDescriptorSetLayout;
  	VkPipelineLayout PhongPipelineLayout;
  	VkPipelineLayout PhongWirePipelineLayout;
	VkPipeline PhongPipeline;
	VkPipeline PhongWirePipeline;
	//// For the first uniform (per object)
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	//// For the second uniform (per scene)
	std::vector<VkBuffer> globalUniformBuffers;
	std::vector<VkDeviceMemory> globalUniformBuffersMemory;	
	// to access uniforms in the pipeline
	std::vector<VkDescriptorSet> PhongDescriptorSets;
	// Scene graph using the Phong pipeline
	std::vector<SceneModel> Scene;
	
	//  Skybox pipeline
 	VkDescriptorSetLayout SkyBoxDescriptorSetLayout; // for skybox
  	VkPipelineLayout SkyBoxPipelineLayout;	// for skybox
	VkPipeline SkyBoxPipeline;		// for skybox
	std::vector<VkBuffer> SkyBoxUniformBuffers;
	std::vector<VkDeviceMemory> SkyBoxUniformBuffersMemory;
	// to access uniforms in the pipeline
	std::vector<VkDescriptorSet> SkyBoxDescriptorSets;
	// Skybox definition, using the Skybox pipeline
	SceneModel SkyBox;

	//  Text pipeline
 	VkDescriptorSetLayout TextDescriptorSetLayout; 
  	VkPipelineLayout TextPipelineLayout;
	VkPipeline TextPipeline;	
	std::vector<VkBuffer> TextUniformBuffers;
	std::vector<VkDeviceMemory> TextUniformBuffersMemory;
	// to access uniforms in the pipeline
	std::vector<VkDescriptorSet> TextDescriptorSets;
	SceneModel SText;


	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame = 0;
	bool framebufferResized = false;

	// Questi dovrebbero essere i render target
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;
	
	
	// Camera
	glm::mat3 CamDir = glm::mat3(1.0f);
	glm::vec3 CamPos = glm::vec3(0.0f, 0.5f, 2.5f);
	
	// Other global variables
	int curText = 0;
	
    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<Assignment02*>
						(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}    

    void initVulkan() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createDescriptorSetLayouts();
		createPipelines();
		createCommandPool();
		createColorResources();
		createDepthResources();
		createFramebuffers();
		loadModels();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
		createSyncObjects();
    }

    void createInstance() {
    	VkApplicationInfo appInfo{};
       	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    	appInfo.pApplicationName = "Hello Triangle";
    	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    	appInfo.pEngineName = "No Engine";
    	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;
		
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
				
		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		createInfo.enabledLayerCount = 0;
		
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		 
		std::vector<VkExtensionProperties> extensionsPr(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, 
		 										extensionsPr.data());
		 
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount =
			static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();		
		
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enableValidationLayers) {
			createInfo.enabledLayerCount =
				static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
			
			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)
									&debugCreateInfo;
		} else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}
		
		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
		
		if(result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create instance!");
		}
    }
    
    std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions =
			glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions,
			glfwExtensions + glfwExtensionCount);
		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		
		return extensions;
	}
	
	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount,
					availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;
			
			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
		
			if (!layerFound) {
				return false;
			}
		}
		
		return true;    
	}

    void populateDebugMessengerCreateInfo(
    		VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    	createInfo = {};
		createInfo.sType =
			VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
//			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | // disabled in the tutorial
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;
    }

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;		
		return VK_FALSE;
	}

	void setupDebugMessenger() {
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		populateDebugMessengerCreateInfo(createInfo);
		
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
				&debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

    void createSurface() {
    	if (glfwCreateWindowSurface(instance, window, nullptr, &surface)
    			!= VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
    }

    void pickPhysicalDevice() {
    	uint32_t deviceCount = 0;
    	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    	 
    	if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		
		std::cout << "Physical devices found: " << deviceCount << "\n";
		
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				msaaSamples = getMaxUsableSampleCount();
				std::cout << "\n\nMaximum samples for anti-aliasing: " << msaaSamples << "\n\n\n";
				break;
			}
		}
		
		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
 		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() &&
								!swapChainSupport.presentModes.empty();
		}
		
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
		
		return indices.isComplete() && extensionsSupported && swapChainAdequate &&
						supportedFeatures.samplerAnisotropy;
	}
    
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;
		
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
						nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
								queueFamilies.data());
								
		int i=0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}
				
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
												 &presentSupport);
			if (presentSupport) {
			 	indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}			
			i++;
		}

		return indices;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr,
					&extensionCount, nullptr);
					
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr,
					&extensionCount, availableExtensions.data());
					
		std::set<std::string> requiredExtensions(deviceExtensions.begin(),
					deviceExtensions.end());
					
		for (const auto& extension : availableExtensions){
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;
		
		 vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
		 		&details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
				nullptr);
				
		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
					&formatCount, details.formats.data());
		}
		
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
			&presentModeCount, nullptr);
		
		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
					&presentModeCount, details.presentModes.data());
		}
		 
		return details;
	}

	VkSampleCountFlagBits getMaxUsableSampleCount() {
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		
		VkSampleCountFlags counts =
				physicalDeviceProperties.limits.framebufferColorSampleCounts &
				physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}	
	
	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies =
				{indices.graphicsFamily.value(), indices.presentFamily.value()};
		
		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}
		
		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.sampleRateShading = VK_TRUE;
		deviceFeatures.fillModeNonSolid = VK_TRUE;
		
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = 
			static_cast<uint32_t>(queueCreateInfos.size());
		
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount =
				static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = 
					static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}
		
		VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
		
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create logical device!");
		}
		
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}
	
	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport =
				querySwapChainSupport(physicalDevice);
		VkSurfaceFormatKHR surfaceFormat =
				chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode =
				chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
		
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		
		if (swapChainSupport.capabilities.maxImageCount > 0 &&
				imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}
		
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
										 indices.presentFamily.value()};
		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}
		
		 createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		 createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		 createInfo.presentMode = presentMode;
		 createInfo.clipped = VK_TRUE;
		 createInfo.oldSwapchain = VK_NULL_HANDLE;
		 
		 VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
		 if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create swap chain!");
		}
		
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount,
				swapChainImages.data());
				
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(
				const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		
		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(
			const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}
	
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		} else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			
			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};
			actualExtent.width = std::max(capabilities.minImageExtent.width,
					std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height,
					std::min(capabilities.maxImageExtent.height, actualExtent.height));
			return actualExtent;
		}
	}

	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());
		
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			swapChainImageViews[i] = createImageView(swapChainImages[i],
													 swapChainImageFormat,
													 VK_IMAGE_ASPECT_COLOR_BIT, 1,
													 VK_IMAGE_VIEW_TYPE_2D, 1);
		}
	}
	
	VkImageView createImageView(VkImage image, VkFormat format,
								VkImageAspectFlags aspectFlags,
								uint32_t mipLevels, VkImageViewType type, int layerCount) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = type;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = layerCount;
		VkImageView imageView;

		VkResult result = vkCreateImageView(device, &viewInfo, nullptr,
				&imageView);
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create image view!");
		}
		return imageView;
	}

    void createRenderPass() {
		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = swapChainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout =
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = msaaSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout =
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = 
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    	VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = msaaSamples;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout =
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;
		
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 3> attachments =
								{colorAttachment, depthAttachment,
								 colorAttachmentResolve};

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr,
					&renderPass);
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create render pass!");
		}		
	}

	void createDescriptorSetLayouts() {
		createPhongDescriptorSetLayout();
		createSkyBoxDescriptorSetLayout();
		createTextDescriptorSetLayout();
	}
	
	void createPhongDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorType =
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		
		VkDescriptorSetLayoutBinding globalUboLayoutBinding{};
		globalUboLayoutBinding.binding = 2;
		globalUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		globalUboLayoutBinding.descriptorCount = 1;
		globalUboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		globalUboLayoutBinding.pImmutableSamplers = nullptr;

		std::array<VkDescriptorSetLayoutBinding, 3> bindings =
							{uboLayoutBinding, samplerLayoutBinding, globalUboLayoutBinding};
		
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();
		
		VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo,
									nullptr, &PhongDescriptorSetLayout);
		if (result != VK_SUCCESS) {
			PrintVkError(result);
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void createSkyBoxDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorType =
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		
		std::array<VkDescriptorSetLayoutBinding, 2> bindings =
							{uboLayoutBinding, samplerLayoutBinding};
		
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();
		
		VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo,
									nullptr, &SkyBoxDescriptorSetLayout);
		if (result != VK_SUCCESS) {
			PrintVkError(result);
			throw std::runtime_error("failed to create SkyBox descriptor set layout!");
		}
	}

	void createTextDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorType =
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		
		std::array<VkDescriptorSetLayoutBinding, 2> bindings =
							{uboLayoutBinding, samplerLayoutBinding};
		
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();
		
		VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo,
									nullptr, &TextDescriptorSetLayout);
		if (result != VK_SUCCESS) {
			PrintVkError(result);
			throw std::runtime_error("failed to create Text descriptor set layout!");
		}
	}

 	void createPipelines() {
 		createPhongPipeline();
 		createSkyBoxPipeline();
 		createTextPipeline();
 	}
 	
 	void createPhongPipeline() {
 		createPipeline("PhongVert.spv", "PhongFrag.spv",
 					    PhongPipelineLayout, PhongPipeline,
 					    PhongDescriptorSetLayout, VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL,
 					    VK_CULL_MODE_NONE, false, phongAndSkyBoxVertices);
 		createPipeline("PhongVert.spv", "PhongFrag.spv",
 					    PhongWirePipelineLayout, PhongWirePipeline,
 					    PhongDescriptorSetLayout, VK_COMPARE_OP_LESS, VK_POLYGON_MODE_LINE,
 					    VK_CULL_MODE_BACK_BIT, false, phongAndSkyBoxVertices);				    
 	}

 	void createSkyBoxPipeline() {
 		createPipeline("SkyBoxVert.spv", "SkyBoxFrag.spv",
 					    SkyBoxPipelineLayout, SkyBoxPipeline,
 					    SkyBoxDescriptorSetLayout, VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
 					    VK_CULL_MODE_BACK_BIT, false, phongAndSkyBoxVertices);
 	}

 	void createTextPipeline() {
 		createPipeline("TextVert.spv", "TextFrag.spv",
 					    TextPipelineLayout, TextPipeline,
 					    TextDescriptorSetLayout, VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
 					    VK_CULL_MODE_NONE, true, textVertices);
 	}

 	void createPipeline(const char* VertexShaderName, const char* FragShaderName,
 						VkPipelineLayout& PipelineLayout,
 						VkPipeline& Pipeline,
 						VkDescriptorSetLayout &descriptorSetLayout,
 						VkCompareOp compareOp, VkPolygonMode polyModel,
 						VkCullModeFlagBits CM, bool transp,
 						VertexDescriptor &VD) {
		auto vertShaderCode = readFile((SHADER_PATH + VertexShaderName).c_str());
		auto fragShaderCode = readFile((SHADER_PATH + FragShaderName).c_str());
		
		VkShaderModule vertShaderModule =
				createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule =
				createShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType =
        		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType =
        		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] =
        		{vertShaderStageInfo, fragShaderStageInfo};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType =
				VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		auto bindingDescription = VD.getBindingDescription();
		auto attributeDescriptions = VD.getAttributeDescriptions();
				
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount =
				static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions =
				attributeDescriptions.data();		

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType =
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float) swapChainExtent.width;
		viewport.height = (float) swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		
		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = swapChainExtent;
		
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType =
				VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
		
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType =
				VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = polyModel;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = CM;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
		
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType =
				VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_TRUE;
		multisampling.minSampleShading = .2f;
		multisampling.rasterizationSamples = msaaSamples;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional
		
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask =
				VK_COLOR_COMPONENT_R_BIT |
				VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = transp ? VK_TRUE : VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor =
				transp ? VK_BLEND_FACTOR_SRC_ALPHA : VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor =
				transp ? VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA : VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp =
				VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor =
				VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor =
				VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp =
				VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType =
				VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional
		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
		
		VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
					&PipelineLayout);
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create pipeline layout!");
		}
		
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = 
				VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = compareOp;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType =
				VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional
		pipelineInfo.layout = PipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional
		
		result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
				&pipelineInfo, nullptr, &Pipeline);
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create graphics pipeline!");
		}
		
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	static std::vector<char> readFile(const std::string& filename) {
			std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}
		
		size_t fileSize = (size_t) file.tellg();
		std::vector<char> buffer(fileSize);
		 
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		 
		file.close();
		 
		return buffer;
	}

	VkShaderModule createShaderModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		
		VkShaderModule shaderModule;

		VkResult result = vkCreateShaderModule(device, &createInfo, nullptr,
						&shaderModule);
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create shader module!");
		}
		
		return shaderModule;
	}

    void createFramebuffers() {
		swapChainFramebuffers.resize(swapChainImageViews.size());
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			std::array<VkImageView, 3> attachments = {
				colorImageView,
				depthImageView,
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType =
				VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount =
							static_cast<uint32_t>(attachments.size());;
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapChainExtent.width; 
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;
			
			VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr,
						&swapChainFramebuffers[i]);
			if (result != VK_SUCCESS) {
			 	PrintVkError(result);
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

    void createCommandPool() {
    	QueueFamilyIndices queueFamilyIndices = 
    			findQueueFamilies(physicalDevice);
    			
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = 0; // Optional
		
		VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void createColorResources() {
		VkFormat colorFormat = swapChainImageFormat;
		createImage(swapChainExtent.width, swapChainExtent.height, 1,
					msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					colorImage, colorImageMemory);
		colorImageView = createImageView(colorImage, colorFormat,
									VK_IMAGE_ASPECT_COLOR_BIT, 1,
									VK_IMAGE_VIEW_TYPE_2D, 1);
	}

	void createDepthResources() {
		VkFormat depthFormat = findDepthFormat();
		
		createImage(swapChainExtent.width, swapChainExtent.height, 1, 
					msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					depthImage, depthImageMemory);
		depthImageView = createImageView(depthImage, depthFormat,
										 VK_IMAGE_ASPECT_DEPTH_BIT, 1,
										 VK_IMAGE_VIEW_TYPE_2D, 1);

		transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
							  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, 1);
	}

	VkFormat findDepthFormat() {
		return findSupportedFormat({VK_FORMAT_D32_SFLOAT,
									VK_FORMAT_D32_SFLOAT_S8_UINT,
									VK_FORMAT_D24_UNORM_S8_UINT},
									VK_IMAGE_TILING_OPTIMAL, 
								VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT );
	}
	
	VkFormat findSupportedFormat(const std::vector<VkFormat> candidates,
						VkImageTiling tiling, VkFormatFeatureFlags features) {
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
			if (tiling == VK_IMAGE_TILING_LINEAR &&
						(props.linearTilingFeatures & features) == features) {
				return format;
			} else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
						(props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}
		
		throw std::runtime_error("failed to find supported format!");
	}
	
	bool hasStencilComponent(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
			   format == VK_FORMAT_D24_UNORM_S8_UINT;
	}
		
	void createTextureImage(const char* FName, TextureData& TD) {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load((TEXTURE_PATH + FName).c_str(), &texWidth, &texHeight,
							&texChannels, STBI_rgb_alpha);
		if (!pixels) {
			std::cout << (TEXTURE_PATH + FName).c_str() << "\n";
			throw std::runtime_error("failed to load texture image!");
		}
		std::cout << FName << " -> size: " << texWidth
				  << "x" << texHeight << ", ch: " << texChannels <<"\n";

		VkDeviceSize imageSize = texWidth * texHeight * 4;
		TD.mipLevels = static_cast<uint32_t>(std::floor(
						std::log2(std::max(texWidth, texHeight)))) + 1;
		
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		 
		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		  						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		  						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		  						stagingBuffer, stagingBufferMemory);
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device, stagingBufferMemory);
		
		stbi_image_free(pixels);
		
		createImage(texWidth, texHeight, TD.mipLevels,
					VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB,
					VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
					VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, TD.textureImage,
					TD.textureImageMemory);
					
		transitionImageLayout(TD.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, TD.mipLevels, 1);
		copyBufferToImage(stagingBuffer, TD.textureImage,
				static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);

		generateMipmaps(TD.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
						texWidth, texHeight, TD.mipLevels, 1);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}
	
	
	void createCubicTextureImage(const char *const FName[6], TextureData& TD) {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels[6];

		for(int i = 0; i < 6; i++) {		
			pixels[i] = stbi_load((TEXTURE_PATH + FName[i]).c_str(), &texWidth, &texHeight,
								&texChannels, STBI_rgb_alpha);
			if (!pixels[i]) {
				std::cout << (TEXTURE_PATH + FName[i]).c_str() << "\n";
				throw std::runtime_error("failed to load texture image!");
			}
			std::cout << FName[i] << " -> size: " << texWidth
					  << "x" << texHeight << ", ch: " << texChannels <<"\n";
		}	

		VkDeviceSize imageSize = texWidth * texHeight * 4;
		VkDeviceSize totalImageSize = texWidth * texHeight * 4 * 6;
		TD.mipLevels = static_cast<uint32_t>(std::floor(
						std::log2(std::max(texWidth, texHeight)))) + 1;
		
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		 
		createBuffer(totalImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		  						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		  						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		  						stagingBuffer, stagingBufferMemory);
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, totalImageSize, 0, &data);
		for(int i = 0; i < 6; i++) {
			memcpy(static_cast<char *>(data) + imageSize * i, pixels[i], static_cast<size_t>(imageSize));
		}
		vkUnmapMemory(device, stagingBufferMemory);
			
		for(int i = 0; i < 6; i++) {
			stbi_image_free(pixels[i]);
		}			
		createSkyBoxImage(texWidth, texHeight, TD.mipLevels, TD.textureImage,
					TD.textureImageMemory);
					
		transitionImageLayout(TD.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, TD.mipLevels, 6);
		copyBufferToImage(stagingBuffer, TD.textureImage,
				static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 6);

		generateMipmaps(TD.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
						texWidth, texHeight, TD.mipLevels, 6);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void generateMipmaps(VkImage image, VkFormat imageFormat,
						 int32_t texWidth, int32_t texHeight,
						 uint32_t mipLevels, int layerCount) {
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat,
							&formatProperties);

		if (!(formatProperties.optimalTilingFeatures &
					VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
			throw std::runtime_error("texture image format does not support linear blitting!");
		}

		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layerCount;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++) { 
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			
			vkCmdPipelineBarrier(commandBuffer,
								 VK_PIPELINE_STAGE_TRANSFER_BIT,
								 VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
								 0, nullptr, 0, nullptr,
								 1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = layerCount;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1,
								   mipHeight > 1 ? mipHeight/2:1, 1};
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = layerCount;
			
			vkCmdBlitImage(commandBuffer, image,
						   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						   image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
						   &blit, VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			
			vkCmdPipelineBarrier(commandBuffer,
								 VK_PIPELINE_STAGE_TRANSFER_BIT,
								 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
								 0, nullptr, 0, nullptr,
								 1, &barrier);
			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(commandBuffer,
							 VK_PIPELINE_STAGE_TRANSFER_BIT,
							 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
							 0, nullptr, 0, nullptr,
							 1, &barrier);

		endSingleTimeCommands(commandBuffer);
	}


	void createSkyBoxImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkImage& image,
				 	 VkDeviceMemory& imageMemory) {		
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 6;
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		
		VkResult result = vkCreateImage(device, &imageInfo, nullptr, &image);
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
		 	throw std::runtime_error("failed to create image!");
		}
		
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
											VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) !=
								VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device, image, imageMemory, 0);
	}

							 
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
					 VkSampleCountFlagBits numSamples, VkFormat format, 
					 VkImageTiling tiling, VkImageUsageFlags usage,
				 	 VkMemoryPropertyFlags properties, VkImage& image,
				 	 VkDeviceMemory& imageMemory) {		
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = numSamples;
		imageInfo.flags = 0; // Optional
		
		VkResult result = vkCreateImage(device, &imageInfo, nullptr, &image);
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
		 	throw std::runtime_error("failed to create image!");
		}
		
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
											properties);
		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) !=
								VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device, image, imageMemory, 0);
	}

	void transitionImageLayout(VkImage image, VkFormat format,
					VkImageLayout oldLayout, VkImageLayout newLayout,
					uint32_t mipLevels, int layersCount) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (hasStencilComponent(format)) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}else{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layersCount;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
					newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
				   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
				   newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
									VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		} else {
			throw std::invalid_argument("unsupported layout transition!");
		}
		vkCmdPipelineBarrier(commandBuffer,
								sourceStage, destinationStage, 0,
								0, nullptr, 0, nullptr, 1, &barrier);

		endSingleTimeCommands(commandBuffer);
	}
	
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t
						   width, uint32_t height, int layerCount) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layerCount;
		region.imageOffset = {0, 0, 0};
		region.imageExtent = {width, height, 1};
		
		vkCmdCopyBufferToImage(commandBuffer, buffer, image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		endSingleTimeCommands(commandBuffer);
	}
	
	void createTextureImageView(TextureData& TD) {
		TD.textureImageView = createImageView(TD.textureImage,
										   VK_FORMAT_R8G8B8A8_SRGB,
										   VK_IMAGE_ASPECT_COLOR_BIT,
										   TD.mipLevels,
										   VK_IMAGE_VIEW_TYPE_2D, 1);
	}
	
	void createSkyBoxImageView(TextureData& TD) {
		TD.textureImageView = createImageView(TD.textureImage,
										   VK_FORMAT_R8G8B8A8_SRGB,
										   VK_IMAGE_ASPECT_COLOR_BIT,
										   TD.mipLevels,
										   VK_IMAGE_VIEW_TYPE_CUBE, 6);
	}
	
	void createTextureSampler(TextureData& TD) {
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(TD.mipLevels);
		
		VkResult result = vkCreateSampler(device, &samplerInfo, nullptr,
										  &TD.textureSampler);
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
		 	throw std::runtime_error("failed to create texture sampler!");
		}
	}

	void loadModels() {
		Scene.resize(SceneToLoad.size());
		int i = 0;

		for (const auto& M : SceneToLoad) {
			loadModelWithTexture(M, i);
			i++;
		}
		
		loadSkyBox();
		createTexts();
		for(int i = 0; i < 16; i++) {VisMT[i] = MT1[i];VisMT[i+16] = MT2[i];VisMT[i+32] = MT3[i];VisMT[i+48] = MT4[i];VisMT[i+64] = MT5[i];	VisMT[i+80] = MT6[i];	VisMT[i+96] = MT7[i];}
//throw std::runtime_error("Now We Stop Here!");			
	}
	
	void loadModelWithTexture(const Model& M, int i) {
		loadMesh(M.ObjFile, Scene[i].MD, phongAndSkyBoxVertices);
		createVertexBuffer(Scene[i].MD);
		createIndexBuffer(Scene[i].MD);
		
		createTextureImage(M.TextureFile, Scene[i].TD);
		createTextureImageView(Scene[i].TD);
		createTextureSampler(Scene[i].TD);
	}
	
	void loadSkyBox() {
		loadMesh(SkyBoxToLoad.ObjFile, SkyBox.MD, phongAndSkyBoxVertices);
		createVertexBuffer(SkyBox.MD);
		createIndexBuffer(SkyBox.MD);

		createCubicTextureImage(SkyBoxToLoad.TextureFile, SkyBox.TD);
		createSkyBoxImageView(SkyBox.TD);
		createTextureSampler(SkyBox.TD);
	}
	
	void createTexts() {
		createTextMesh(SText.MD, textVertices);
		createVertexBuffer(SText.MD);
		createIndexBuffer(SText.MD);

		createTextureImage("Fonts.png", SText.TD);
		createTextureImageView(SText.TD);
		createTextureSampler(SText.TD);
	}
	
	void loadMesh(const char* FName, ModelData& MD, VertexDescriptor &VD) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, 
						(MODEL_PATH + FName).c_str())) {
			throw std::runtime_error(warn + err);
		}
		
		MD.vertDesc = &VD;

		std::cout << FName << "\n";

		std::vector<float> vertex{};
		vertex.resize(VD.size);

//		std::unordered_map<std::vector<float>, uint32_t> uniqueVertices{};
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				
				vertex[VD.deltaPos + 0] = attrib.vertices[3 * index.vertex_index + 0];
				vertex[VD.deltaPos + 1] = attrib.vertices[3 * index.vertex_index + 1];
				vertex[VD.deltaPos + 2] = attrib.vertices[3 * index.vertex_index + 2];
				vertex[VD.deltaTexCoord + 0] = attrib.texcoords[2 * index.texcoord_index + 0];
				vertex[VD.deltaTexCoord + 1] = 1 - attrib.texcoords[2 * index.texcoord_index + 1];
				vertex[VD.deltaNormal + 0] = attrib.normals[3 * index.normal_index + 0];
				vertex[VD.deltaNormal + 1] = attrib.normals[3 * index.normal_index + 1];
				vertex[VD.deltaNormal + 2] = attrib.normals[3 * index.normal_index + 2];
				
//				if (uniqueVertices.count(vertex) == 0) {
					int j = MD.vertices.size() / VD.size;
//					uniqueVertices[vertex] =
//							static_cast<uint32_t>(j);
					int s = MD.vertices.size();
					MD.vertices.resize(s + VD.size);
					for(int k = 0; k < VD.size; k++) {
						MD.vertices[s+k] = vertex[k];
					}
/**/				MD.indices.push_back(j);
//				}
				
//				MD.indices.push_back(uniqueVertices[vertex]);
			}
		}
		
		std::cout << FName << " -> V: " << MD.vertices.size()
				  << ", I: " << MD.indices.size() << "\n";
	}
	void createTextMesh(ModelData& MD, VertexDescriptor &VD) {
		MD.vertDesc = &VD;

		int totLen = 0;
		for(auto& Txt : SceneText) {
			for(int i = 0; i < Txt.usedLines; i++) {
				totLen += strlen(Txt.l[i]);
			}
		}
		std::cout << "Total characters: " << totLen << "\n";
		
		MD.vertices.resize(4 * VD.size * totLen);
		MD.indices.resize(6 * totLen);
		
		int FontId = 1;
		
		float PtoTdx = -0.95;
		float PtoTdy = -0.95;
		float PtoTsx = 2.0/800.0;
		float PtoTsy = 2.0/600.0;
		
		int minChar = 32;
		int maxChar = 127;
		int texW = 1024;
		int texH = 512;
		
		int tpx = 0;
		int tpy = 0;
		
		int vb = 0, ib = 0, k = 0;
		for(auto& Txt : SceneText) {
			Txt.start = ib;
			for(int i = 0; i < Txt.usedLines; i++) {
				for(int j = 0; j < strlen(Txt.l[i]); j++) {
					int c = ((int)Txt.l[i][j]) - minChar;
					if((c >= 0) && (c <= maxChar)) {
//std::cout << k << " " << j << " " << i << " " << vb << " " << ib << " " << c << "\n";
						CharData d = Fonts[FontId].P[c];
				
						MD.vertices[vb +  0] = (float)(tpx + d.xoffset) * PtoTsx + PtoTdx;
						MD.vertices[vb +  1] = (float)(tpy + d.yoffset) * PtoTsy + PtoTdy;
						MD.vertices[vb +  2] = 0.0;
						MD.vertices[vb +  3] = (float)d.x / texW;
						MD.vertices[vb +  4] = (float)d.y / texH;
						
						MD.vertices[vb +  5] = (float)(tpx + d.xoffset + d.width) * PtoTsx + PtoTdx;
						MD.vertices[vb +  6] = (float)(tpy + d.yoffset) * PtoTsy + PtoTdy;
						MD.vertices[vb +  7] = 0.0;
						MD.vertices[vb +  8] = (float)(d.x + d.width) / texW ;
						MD.vertices[vb +  9] = (float)d.y / texH;
				
						MD.vertices[vb + 10] = (float)(tpx + d.xoffset) * PtoTsx + PtoTdx;
						MD.vertices[vb + 11] = (float)(tpy + d.yoffset + d.height) * PtoTsy + PtoTdy;
						MD.vertices[vb + 12] =  0.0;
						MD.vertices[vb + 13] = (float)d.x / texW;
						MD.vertices[vb + 14] = (float)(d.y + d.height) / texH;
						
						MD.vertices[vb + 15] = (float)(tpx + d.xoffset + d.width) * PtoTsx + PtoTdx;
						MD.vertices[vb + 16] = (float)(tpy + d.yoffset + d.height) * PtoTsy + PtoTdy;
						MD.vertices[vb + 17] = 0.0;
						MD.vertices[vb + 18] = (float)(d.x + d.width) / texW;
						MD.vertices[vb + 19] = (float)(d.y + d.height) / texH;
						
						MD.indices[ib + 0] = 4 * k + 0;
						MD.indices[ib + 1] = 4 * k + 1;
						MD.indices[ib + 2] = 4 * k + 2;
						MD.indices[ib + 3] = 4 * k + 1;
						MD.indices[ib + 4] = 4 * k + 2;
						MD.indices[ib + 5] = 4 * k + 3;
	
						vb += 4 * VD.size;
						ib += 6;
						tpx += d.xadvance;
						k++;
					}
				}
				tpy += Fonts[FontId].lineHeight;
				tpx = 0;	
			}
			tpx = 0;
			tpy = 0;
			Txt.len = ib - Txt.start;
		}		
		std::cout << "Text: " << MD.vertices.size()
				  << ", I: " << MD.indices.size() << "\n";
	}

	void createVertexBuffer(ModelData& Md) {
		VkDeviceSize bufferSize = sizeof(Md.vertices[0]) * Md.vertices.size();
		
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, Md.vertices.data(), (size_t) bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);	
		
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
								 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
								VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
								Md.vertexBuffer, Md.vertexBufferMemory);
								
		copyBuffer(stagingBuffer, Md.vertexBuffer, bufferSize);
		
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}
	
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
					  VkMemoryPropertyFlags properties,
					  VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		
		VkResult result =
				vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create vertex buffer!");
		}
		
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex =
				findMemoryType(memRequirements.memoryTypeBits, properties);

		result = vkAllocateMemory(device, &allocInfo, nullptr,
				&bufferMemory);
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}
		
		vkBindBufferMemory(device, buffer, bufferMemory, 0);	
	}
	
	uint32_t findMemoryType(uint32_t typeFilter,
							VkMemoryPropertyFlags properties) {
		 VkPhysicalDeviceMemoryProperties memProperties;
		 vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
		 
		 for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		 	if ((typeFilter & (1 << i)) && 
		 		(memProperties.memoryTypes[i].propertyFlags & properties) ==
		 				properties) {
				return i;
			}
		}
		
		throw std::runtime_error("failed to find suitable memory type!");
	}
    
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		
		endSingleTimeCommands(commandBuffer);
	}
	
	VkCommandBuffer beginSingleTimeCommands() { 
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;
		
		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
		
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		
		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		
		return commandBuffer;
	}
	
	void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);
		
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);
		
		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}
	
	void createIndexBuffer(ModelData& Md) {
		VkDeviceSize bufferSize = sizeof(Md.indices[0]) * Md.indices.size();
	
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
								 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
								 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								 stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, Md.indices.data(), (size_t) bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
								 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
								 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
								 Md.indexBuffer, Md.indexBufferMemory);

		copyBuffer(stagingBuffer, Md.indexBuffer, bufferSize);
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(swapChainImages.size() * Scene.size());
		uniformBuffersMemory.resize(swapChainImages.size() * Scene.size());

		for (size_t i = 0; i < swapChainImages.size() * Scene.size(); i++) {
			createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
								 	 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
								 	 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								 	 uniformBuffers[i], uniformBuffersMemory[i]);
		}

		bufferSize = sizeof(GlobalUniformBufferObject);

		globalUniformBuffers.resize(swapChainImages.size());
		globalUniformBuffersMemory.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
								 	 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
								 	 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								 	 globalUniformBuffers[i], globalUniformBuffersMemory[i]);
		}

		bufferSize = sizeof(UniformBufferObject);

		SkyBoxUniformBuffers.resize(swapChainImages.size());
		SkyBoxUniformBuffersMemory.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
								 	 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
								 	 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								 	 SkyBoxUniformBuffers[i], SkyBoxUniformBuffersMemory[i]);
		}
		
		bufferSize = sizeof(UniformBufferObject);

		TextUniformBuffers.resize(swapChainImages.size());
		TextUniformBuffersMemory.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
								 	 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
								 	 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								 	 TextUniformBuffers[i], TextUniformBuffersMemory[i]);
		}
	}

	void createDescriptorPool() {
		std::array<VkDescriptorPoolSize, 9> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size() * Scene.size());
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size() * Scene.size());
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[2].descriptorCount = static_cast<uint32_t>(swapChainImages.size() * Scene.size());
		poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[3].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
		poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[4].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
		poolSizes[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[5].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
		poolSizes[6].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[6].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
		poolSizes[7].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[7].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
		poolSizes[8].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[8].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size() * (Scene.size() + 2));
		
		VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr,
									&descriptorPool);
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}
	
	void createDescriptorSets() {
		createPhongDescriptorSets();
		createSkyBoxDescriptorSets();
		createTextDescriptorSets();
	}
	
	void createPhongDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size() * Scene.size(),
												   PhongDescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size() * Scene.size());
		allocInfo.pSetLayouts = layouts.data();

		PhongDescriptorSets.resize(swapChainImages.size() * Scene.size());
		
		VkResult result = vkAllocateDescriptorSets(device, &allocInfo,
											PhongDescriptorSets.data());
		if (result != VK_SUCCESS) {
			PrintVkError(result);
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		
		for (size_t k = 0; k < swapChainImages.size(); k++) {
			for (size_t j = 0; j < Scene.size(); j++) {
				size_t i = j * swapChainImages.size() + k;
				
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = uniformBuffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(UniformBufferObject);
				
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = Scene[j].TD.textureImageView;
				imageInfo.sampler = Scene[j].TD.textureSampler;
				
				VkDescriptorBufferInfo globalBufferInfo{};
				globalBufferInfo.buffer = globalUniformBuffers[k];
				globalBufferInfo.offset = 0;
				globalBufferInfo.range = sizeof(GlobalUniformBufferObject);
				
				std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = PhongDescriptorSets[i];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount =  1;
				descriptorWrites[0].pBufferInfo = &bufferInfo;
	
				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = PhongDescriptorSets[i];
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType =
											VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].pImageInfo = &imageInfo;
				
				descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[2].dstSet = PhongDescriptorSets[i];
				descriptorWrites[2].dstBinding = 2;
				descriptorWrites[2].dstArrayElement = 0;
				descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[2].descriptorCount =  1;
				descriptorWrites[2].pBufferInfo = &globalBufferInfo;

				vkUpdateDescriptorSets(device,
							static_cast<uint32_t>(descriptorWrites.size()),
							descriptorWrites.data(), 0, nullptr);
			}
		}
	}

	void createSkyBoxDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(),
												   SkyBoxDescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
		allocInfo.pSetLayouts = layouts.data();

		SkyBoxDescriptorSets.resize(swapChainImages.size());
		
		VkResult result = vkAllocateDescriptorSets(device, &allocInfo,
											SkyBoxDescriptorSets.data());
		if (result != VK_SUCCESS) {
			PrintVkError(result);
			throw std::runtime_error("failed to allocate Skybox descriptor sets!");
		}
		
		for (size_t k = 0; k < swapChainImages.size(); k++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = SkyBoxUniformBuffers[k];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);
				
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = SkyBox.TD.textureImageView;
			imageInfo.sampler = SkyBox.TD.textureSampler;

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = SkyBoxDescriptorSets[k];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount =  1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = SkyBoxDescriptorSets[k];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType =
										VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(device,
						static_cast<uint32_t>(descriptorWrites.size()),
						descriptorWrites.data(), 0, nullptr);
		}
	}

	void createTextDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(),
												   TextDescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
		allocInfo.pSetLayouts = layouts.data();

		TextDescriptorSets.resize(swapChainImages.size());
		
		VkResult result = vkAllocateDescriptorSets(device, &allocInfo,
											TextDescriptorSets.data());
		if (result != VK_SUCCESS) {
			PrintVkError(result);
			throw std::runtime_error("failed to allocate Text descriptor sets!");
		}
		
		for (size_t k = 0; k < swapChainImages.size(); k++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = TextUniformBuffers[k];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);
				
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = SText.TD.textureImageView;
			imageInfo.sampler = SText.TD.textureSampler;

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = TextDescriptorSets[k];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount =  1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = TextDescriptorSets[k];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType =
										VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(device,
						static_cast<uint32_t>(descriptorWrites.size()),
						descriptorWrites.data(), 0, nullptr);
		}
	}

    void createCommandBuffers() {
    	commandBuffers.resize(swapChainFramebuffers.size());
    	
    	VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
		
		VkResult result = vkAllocateCommandBuffers(device, &allocInfo,
				commandBuffers.data());
		if (result != VK_SUCCESS) {
		 	PrintVkError(result);
			throw std::runtime_error("failed to allocate command buffers!");
		}
		
		for (size_t i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0; // Optional
			beginInfo.pInheritanceInfo = nullptr; // Optional

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) !=
						VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}
			
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass; 
			renderPassInfo.framebuffer = swapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = swapChainExtent;

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
			clearValues[1].depthStencil = {1.0f, 0};

			renderPassInfo.clearValueCount =
							static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();
			
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
					VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
					PhongPipeline);					
			for(int j = 0; j < Scene.size(); j++) {
				if(SceneToLoad[j].pt == Flat) {
					VkBuffer vertexBuffers[] = {Scene[j].MD.vertexBuffer};
					VkDeviceSize offsets[] = {0};
					vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
					vkCmdBindIndexBuffer(commandBuffers[i], Scene[j].MD.indexBuffer, 0,
											VK_INDEX_TYPE_UINT32);
					vkCmdBindDescriptorSets(commandBuffers[i],
									VK_PIPELINE_BIND_POINT_GRAPHICS,
									PhongPipelineLayout, 0, 1,
									&PhongDescriptorSets[i + j * swapChainImages.size()],
									0, nullptr);
									
					vkCmdDrawIndexed(commandBuffers[i],
								static_cast<uint32_t>(Scene[j].MD.indices.size()), 1, 0, 0, 0);
				}
			}

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
					PhongWirePipeline);					
			for(int j = 0; j < Scene.size(); j++) {
				if(SceneToLoad[j].pt == Wire) {
					VkBuffer vertexBuffers[] = {Scene[j].MD.vertexBuffer};
					VkDeviceSize offsets[] = {0};
					vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
					vkCmdBindIndexBuffer(commandBuffers[i], Scene[j].MD.indexBuffer, 0,
											VK_INDEX_TYPE_UINT32);
					vkCmdBindDescriptorSets(commandBuffers[i],
									VK_PIPELINE_BIND_POINT_GRAPHICS,
									PhongPipelineLayout, 0, 1,
									&PhongDescriptorSets[i + j * swapChainImages.size()],
									0, nullptr);
									
					vkCmdDrawIndexed(commandBuffers[i],
								static_cast<uint32_t>(Scene[j].MD.indices.size()), 1, 0, 0, 0);
				}
			}

			// Draws the Skybox
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
					SkyBoxPipeline);
			VkBuffer vertexBuffers[] = {SkyBox.MD.vertexBuffer};
			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffers[i], SkyBox.MD.indexBuffer, 0,
									VK_INDEX_TYPE_UINT32);
			vkCmdBindDescriptorSets(commandBuffers[i],
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							SkyBoxPipelineLayout, 0, 1,
							&SkyBoxDescriptorSets[i],
							0, nullptr);							
			vkCmdDrawIndexed(commandBuffers[i],
						static_cast<uint32_t>(SkyBox.MD.indices.size()), 1, 0, 0, 0);
			
			
			// Draws the text
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
					TextPipeline);
			VkBuffer TvertexBuffers[] = {SText.MD.vertexBuffer};
			VkDeviceSize Toffsets[] = {0};
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, TvertexBuffers, Toffsets);
			vkCmdBindIndexBuffer(commandBuffers[i], SText.MD.indexBuffer, 0,
									VK_INDEX_TYPE_UINT32);
			vkCmdBindDescriptorSets(commandBuffers[i],
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							TextPipelineLayout, 0, 1,
							&TextDescriptorSets[i],
							0, nullptr);							
			vkCmdDrawIndexed(commandBuffers[i],
						static_cast<uint32_t>(SceneText[curText].len), 1, static_cast<uint32_t>(SceneText[curText].start), 0, 0);
			
			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}
    
    void createSyncObjects() {
    	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);
    	    	
    	VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkResult result1 = vkCreateSemaphore(device, &semaphoreInfo, nullptr,
								&imageAvailableSemaphores[i]);
			VkResult result2 = vkCreateSemaphore(device, &semaphoreInfo, nullptr,
								&renderFinishedSemaphores[i]);
			VkResult result3 = vkCreateFence(device, &fenceInfo, nullptr,
								&inFlightFences[i]);
			if (result1 != VK_SUCCESS ||
				result2 != VK_SUCCESS ||
				result3 != VK_SUCCESS) {
			 	PrintVkError(result1);
			 	PrintVkError(result2);
			 	PrintVkError(result3);
				throw std::runtime_error("failed to create synchronization objects for a frame!!");
			}
		}
	}
    
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
            
            if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            	break;
            }
        }
        
        vkDeviceWaitIdle(device);
    }
    
    void drawFrame() {
		vkWaitForFences(device, 1, &inFlightFences[currentFrame],
						VK_TRUE, UINT64_MAX);
		
		uint32_t imageIndex;
		
		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
				imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(device, 1, &imagesInFlight[imageIndex],
							VK_TRUE, UINT64_MAX);
		}
		imagesInFlight[imageIndex] = inFlightFences[currentFrame];
		
		updateUniformBuffer(imageIndex);
		
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
		VkPipelineStageFlags waitStages[] =
			{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
		VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		
		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo,
				inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}
		
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		
		VkSwapchainKHR swapChains[] = {swapChain};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional
		
		result = vkQueuePresentKHR(presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
			framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
		
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

	void updateUniformBuffer(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();
		static float lastTime = 0.0f;
		
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>
					(currentTime - startTime).count();
		float deltaT = time - lastTime;
		lastTime = time;
					
		const float ROT_SPEED = glm::radians(60.0f);
		const float MOVE_SPEED = 0.75f;

		static float debounce = time;
		
		if(glfwGetKey(window, GLFW_KEY_SPACE)) {
			if(time - debounce > 0.33) {
				curText = (curText + 1) % SceneText.size();
				debounce = time;
				framebufferResized = true;
//std::cout << curText << "\n";
			}
		}			

		if(glfwGetKey(window, GLFW_KEY_LEFT)) {
			CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f),
							   deltaT * ROT_SPEED,
							   glm::vec3(CamDir[1])) * glm::mat4(CamDir));
		}
		if(glfwGetKey(window, GLFW_KEY_RIGHT)) {
			CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f),
							   -deltaT * ROT_SPEED,
							   glm::vec3(CamDir[1])) * glm::mat4(CamDir));
		}
		if(glfwGetKey(window, GLFW_KEY_UP)) {
			CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f),
							   deltaT * ROT_SPEED,
							   glm::vec3(CamDir[0])) * glm::mat4(CamDir));
		}
		if(glfwGetKey(window, GLFW_KEY_DOWN)) {
			CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f),
							   -deltaT * ROT_SPEED,
							   glm::vec3(CamDir[0])) * glm::mat4(CamDir));
		}
		if(glfwGetKey(window, GLFW_KEY_Q)) {
			CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f),
							   deltaT * ROT_SPEED,
							   glm::vec3(CamDir[2])) * glm::mat4(CamDir));
		}
		if(glfwGetKey(window, GLFW_KEY_E)) {
			CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f),
							   -deltaT * ROT_SPEED,
							   glm::vec3(CamDir[2])) * glm::mat4(CamDir));
		}
		

		if(glfwGetKey(window, GLFW_KEY_A)) {
			CamPos -= MOVE_SPEED * glm::vec3(CamDir[0]) * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_D)) {
			CamPos += MOVE_SPEED * glm::vec3(CamDir[0]) * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_S)) {
			CamPos += MOVE_SPEED * glm::vec3(CamDir[2]) * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_W)) {
			CamPos -= MOVE_SPEED * glm::vec3(CamDir[2]) * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_F)) {
			CamPos -= MOVE_SPEED * glm::vec3(CamDir[1]) * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_R)) {
			CamPos += MOVE_SPEED * glm::vec3(CamDir[1]) * deltaT;
		}

// std::cout << "Cam Pos: " << CamPos[0] << " " << CamPos[1] << " " << CamPos[2] << "\n";

		glm::mat4 CamMat = glm::translate(glm::transpose(glm::mat4(CamDir)), -CamPos);
					
		glm::mat4 Prj = glm::perspective(glm::radians(45.0f),
						swapChainExtent.width / (float) swapChainExtent.height,
						0.1f, 50.0f);
		Prj[1][1] *= -1;

		// Updates unifoms for the objects
		for(int j = 0; j < Scene.size(); j++) {
			UniformBufferObject ubo{};
			glm::vec3 delta;
			
			float rotAng = 0.0f;

			if(j == 1) {
				int v = curText*16;
				ubo.mMat = glm::transpose(glm::mat4(VisV[v],VisV[v+1],VisV[v+2], VisV[v+3],VisV[v+4],VisV[v+5],VisV[v+6],
									 VisV[v+7],VisV[v+8],VisV[v+9],VisV[v+10], VisV[v+11],VisV[v+12],VisV[v+13],VisV[v+14],VisV[v+15]));			
			} else {
//				ubo.mMat = glm::mat4(1.0f);
//				ubo.mMat = glm::translate(ubo.mMat, SceneToLoad[j].pos);
//				ubo.mMat = glm::rotate(ubo.mMat, 0.0f,
//										glm::vec3(0.0f, 1.0f, 0.0f));
				int v = curText*16;
				ubo.mMat = glm::transpose(glm::mat4(VisMT[v],VisMT[v+1],VisMT[v+2], VisMT[v+3],VisMT[v+4],VisMT[v+5],VisMT[v+6],
									 VisMT[v+7],VisMT[v+8],VisMT[v+9],VisMT[v+10], VisMT[v+11],VisMT[v+12],VisMT[v+13],VisMT[v+14],VisMT[v+15]));			
			}
			ubo.mMat = glm::scale(ubo.mMat, glm::vec3(SceneToLoad[j].scale));
			ubo.mvpMat = Prj * CamMat * ubo.mMat;
			ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
//			ubo.nMat = glm::inverse(glm::transpose(glm::mat3(ubo.mMat)));
//
//for(int ti=0;ti<3;ti++){for(int tj=0;tj<3;tj++){std::cout << ubo.nMat[ti][tj] << " ";}std::cout<<"\n";}std::cout<<"\n";
//glm::mat4 tM = glm::inverse(glm::transpose(ubo.mMat));
//for(int ti=0;ti<4;ti++){for(int tj=0;tj<4;tj++){std::cout << tM[ti][tj] << " ";}std::cout<<"\n";}std::cout<<"\n";


			int i = j * swapChainImages.size() + currentImage;
			
			void* data;
			vkMapMemory(device, uniformBuffersMemory[i], 0,
								sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(device, uniformBuffersMemory[i]);
		}
		
		// updates global uniforms
		GlobalUniformBufferObject gubo{};
		gubo.lightDir = glm::vec3(cos(glm::radians(135.0f)), sin(glm::radians(135.0f)), 0.0f);
		gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		gubo.eyePos = CamPos;

		void* data;
		vkMapMemory(device, globalUniformBuffersMemory[currentImage], 0,
							sizeof(gubo), 0, &data);
		memcpy(data, &gubo, sizeof(gubo));
		vkUnmapMemory(device, globalUniformBuffersMemory[currentImage]);

		// updates SkyBox uniforms
		UniformBufferObject ubo{};
		ubo.mMat = glm::mat4(1.0f);
		ubo.nMat = glm::mat4(1.0f);
		ubo.mvpMat = Prj * glm::transpose(glm::mat4(CamDir));
		vkMapMemory(device, SkyBoxUniformBuffersMemory[currentImage], 0,
							sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, SkyBoxUniformBuffersMemory[currentImage]);
	}

    void recreateSwapChain() {
    	int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device);
    	
    	cleanupSwapChain();

		createSwapChain();
		createImageViews();
		createRenderPass();
		createPipelines();
		createColorResources();
		createDepthResources();
		createFramebuffers();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
	}

	void cleanupSwapChain() {
    	vkDestroyImageView(device, colorImageView, nullptr);
    	vkDestroyImage(device, colorImage, nullptr);
    	vkFreeMemory(device, colorImageMemory, nullptr);
    	
		vkDestroyImageView(device, depthImageView, nullptr);
		vkDestroyImage(device, depthImage, nullptr);
		vkFreeMemory(device, depthImageMemory, nullptr);

		for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
			vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
		}
		
		vkFreeCommandBuffers(device, commandPool,
				static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		vkDestroyPipeline(device, PhongPipeline, nullptr);
		vkDestroyPipeline(device, PhongWirePipeline, nullptr);
		vkDestroyPipelineLayout(device, PhongPipelineLayout, nullptr);
		vkDestroyPipelineLayout(device, PhongWirePipelineLayout, nullptr);

		vkDestroyPipeline(device, SkyBoxPipeline, nullptr);
		vkDestroyPipelineLayout(device, SkyBoxPipelineLayout, nullptr);

		vkDestroyPipeline(device, TextPipeline, nullptr);
		vkDestroyPipelineLayout(device, TextPipelineLayout, nullptr);

		vkDestroyRenderPass(device, renderPass, nullptr);

		for (size_t i = 0; i < swapChainImageViews.size(); i++){
			vkDestroyImageView(device, swapChainImageViews[i], nullptr);
		}
		
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		
		for (size_t i = 0; i < swapChainImages.size() * Scene.size(); i++) {
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		}
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			vkDestroyBuffer(device, globalUniformBuffers[i], nullptr);
			vkFreeMemory(device, globalUniformBuffersMemory[i], nullptr);
		}		
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			vkDestroyBuffer(device, SkyBoxUniformBuffers[i], nullptr);
			vkFreeMemory(device, SkyBoxUniformBuffersMemory[i], nullptr);
		}
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			vkDestroyBuffer(device, TextUniformBuffers[i], nullptr);
			vkFreeMemory(device, TextUniformBuffersMemory[i], nullptr);
		}
		
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	}


    void cleanup() {
    	cleanupSwapChain();
    	
    	for (size_t i = 0; i < Scene.size(); i++) {
	    	vkDestroySampler(device, Scene[i].TD.textureSampler, nullptr);
	    	vkDestroyImageView(device, Scene[i].TD.textureImageView, nullptr);
			vkDestroyImage(device, Scene[i].TD.textureImage, nullptr);
			vkFreeMemory(device, Scene[i].TD.textureImageMemory, nullptr);
	    	
	    	vkDestroyBuffer(device, Scene[i].MD.indexBuffer, nullptr);
	    	vkFreeMemory(device, Scene[i].MD.indexBufferMemory, nullptr);
	
			vkDestroyBuffer(device, Scene[i].MD.vertexBuffer, nullptr);
	    	vkFreeMemory(device, Scene[i].MD.vertexBufferMemory, nullptr);
    	}
    	vkDestroySampler(device, SkyBox.TD.textureSampler, nullptr);
    	vkDestroyImageView(device, SkyBox.TD.textureImageView, nullptr);
		vkDestroyImage(device, SkyBox.TD.textureImage, nullptr);
		vkFreeMemory(device, SkyBox.TD.textureImageMemory, nullptr);

    	vkDestroyBuffer(device, SkyBox.MD.indexBuffer, nullptr);
    	vkFreeMemory(device, SkyBox.MD.indexBufferMemory, nullptr);

		vkDestroyBuffer(device, SkyBox.MD.vertexBuffer, nullptr);
    	vkFreeMemory(device, SkyBox.MD.vertexBufferMemory, nullptr);
    	    	
    	vkDestroySampler(device, SText.TD.textureSampler, nullptr);
    	vkDestroyImageView(device, SText.TD.textureImageView, nullptr);
		vkDestroyImage(device, SText.TD.textureImage, nullptr);
		vkFreeMemory(device, SText.TD.textureImageMemory, nullptr);

    	vkDestroyBuffer(device, SText.MD.indexBuffer, nullptr);
    	vkFreeMemory(device, SText.MD.indexBufferMemory, nullptr);

		vkDestroyBuffer(device, SText.MD.vertexBuffer, nullptr);
    	vkFreeMemory(device, SText.MD.vertexBufferMemory, nullptr);
    	    	
    	vkDestroyDescriptorSetLayout(device, PhongDescriptorSetLayout, nullptr);
    	vkDestroyDescriptorSetLayout(device, SkyBoxDescriptorSetLayout, nullptr);
    	vkDestroyDescriptorSetLayout(device, TextDescriptorSetLayout, nullptr);
    	
    	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
    	}
    	
    	vkDestroyCommandPool(device, commandPool, nullptr);
    	
 		vkDestroyDevice(device, nullptr);
		
	   	if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		
		vkDestroySurfaceKHR(instance, surface, nullptr);
    	vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }







//// To put in the right place

};

int main() {
    Assignment02 app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}