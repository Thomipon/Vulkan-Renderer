#pragma once

#include "VulkanBackend.hpp"

class Material {

	// TODO: Separate Material and shader (and probably more)
public:
	vk::raii::PipelineLayout pipelineLayout;
	vk::raii::Pipeline graphicsPipeline;

private:

};
