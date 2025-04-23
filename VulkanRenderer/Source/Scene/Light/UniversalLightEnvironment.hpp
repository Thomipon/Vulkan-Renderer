#pragma once
#include "BasicLights.hpp"
#include "CompositeLights.hpp"

using UniversalLightEnvironment = LightPair<LightArray<PointLight, 10>, LightPair<LightArray<DirectionalLight, 3>, AmbientLight>>;
