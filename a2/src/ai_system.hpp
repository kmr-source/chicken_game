#pragma once

#include <vector>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// DON'T WORRY ABOUT THIS CLASS UNTIL ASSIGNMENT 3
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class AISystem
{
public:
	void step(float elapsed_ms);
	bool player_in_range(vec2 x, float y); // checks if player is in range
	float getDistancePath(vec2 position, vec2 wall_position, float curr_goal_path); // get Distance for shortest path
};