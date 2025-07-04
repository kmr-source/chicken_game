// internal
#include "physics_system.hpp"
#include "world_init.hpp"

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Motion& motion1, const Motion& motion2)
{
	vec2 dp = motion1.position - motion2.position;
	float dist_squared = dot(dp,dp);
	const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
}

// distance between 2 positions 
float dist_to(const vec2 position1, const vec2 position2) {
	return sqrt(pow(position2.x - position1.x, 2) + pow(position2.y - position1.y, 2));
}

void PhysicsSystem::step(float elapsed_ms)
{
	// Move bug based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_registry = registry.motions;
	for (uint i = 0; i < motion_registry.size(); i++)
	{
		// !!! TODO A1: update motion.position based on step_seconds and motion.velocity
		//Motion& motion = motion_registry.components[i];
		//Entity entity = motion_registry.entities[i];
		//float step_seconds = elapsed_ms / 1000.f;
		//(void)elapsed_ms; // placeholder to silence unused warning until implemented
		// Eagles should move to the bottom of the screen while the chicken stays stationairy with velocity 
		// {0,0}
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];
		float step_seconds = elapsed_ms / 1000.f;
		vec2 position = motion.position;
		vec2 velocity = motion.velocity;
		vec2 dest = motion.destination;
		float velocity_magnitude = sqrt(pow(velocity.x * step_seconds, 2) + pow(velocity.y * step_seconds, 2));
		vec2 pos_final = { position.x + (velocity.x * step_seconds), position.y + (velocity.y * step_seconds) };
		// behaviour if currently moving
		if (velocity.x * step_seconds != 0 || velocity.y * step_seconds != 0) {

			if (dist_to(pos_final, dest) <= velocity_magnitude) {
				motion.velocity = { 0, 0 };
				motion.destination = motion.position;
				motion.in_motion = false;
			}
		}
		motion.position = pos_final;

		// BUG BOUNCE OFF THE WALL A2 Part 2 implmented here for chicken + bug put int AI 
		/*if (registry.eatables.has(entity)) {
			// left wall
			if (motion.position.x < 30.0f) {
				//printf("hello");
				motion.position.x += 30.0f;
				motion.position.y += 0;
				//motion.velocity.y *= -10.0;
				motion.velocity.x = motion.velocity.x*-1.0;
				//motion.velocity.y *= -1.0; //hits wall goes down 
			}

			if (motion.position.x > window_width_px - 30.0f) {
				//printf("bye");
				motion.position.x += -30.0f;
				motion.position.y += 0;
				motion.velocity.x = motion.velocity.x*-1.0;
				//motion.velocity.y =10.0; //hits wall goes down 
			}
		}*/

		// bounce chicken off the wall 
		if (registry.meshPtrs.has(entity)) {
			float xbox = get_bounding_box(registry.motions.get(entity)).x;
			float ybox = get_bounding_box(registry.motions.get(entity)).y;
			//printf("%d position x of entity chicken is : \n", motion.position.x);
			//printf("%d xbox of entity chicken is : \n", ybox);
			if (motion.position.x < 30.0f || xbox < 30.0f) {
				//printf("hello"); left
				motion.position.x += 30.0f;
				motion.velocity.x = 0;
				
			}

			if (motion.position.x > window_width_px - 30.0f || xbox > window_width_px - 30.0f) {
				//printf("bye"); right
				motion.position.x += -30.0f;
				motion.velocity.x = 0;
				//motion.velocity.y = motion.velocity.y*
			}
		}


	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A3: HANDLE EGG UPDATES HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 3
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Check for collisions between all moving entities
	ComponentContainer<Motion> &motion_container = registry.motions;
	for (uint i = 0; i < motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];

		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for (uint j = i + 1; j < motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			if (collides(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE CHICKEN - WALL collisions HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// you may need the following quantities to compute wall positions
	(float)window_width_px; (float)window_height_px;

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: DRAW DEBUG INFO HERE on Chicken mesh collision
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// You will want to use the createLine from world_init.hpp
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// debugging of bounding boxes
	if (debugging.in_debug_mode && debugging.in_freeze_mode)
	{
		uint size_before_adding_new = (uint)motion_container.components.size();
		for (uint i = 0; i < size_before_adding_new; i++)
		{
			Motion& motion_i = motion_container.components[i];
			Entity entity_i = motion_container.entities[i];

			// don't draw debugging visuals around debug lines
			if (registry.debugComponents.has(entity_i))
				continue;

			// visualize the radius with two axis-aligned lines
			const vec2 bonding_box = get_bounding_box(motion_i);
			float radius = sqrt(dot(bonding_box / 2.f, bonding_box / 2.f));
			vec2 line_scale1 = { motion_i.scale.x / 10, 2 * radius };
			vec2 line_scale2 = { 2 * radius, motion_i.scale.x / 10 };
			vec2 line_scaleE = { 2 * radius, motion_i.scale.x / 15 };
			vec2 line_scaleELR = { motion_i.scale.x / 15, 2 * radius };
			vec2 position = motion_i.position;
			vec2 velocity = motion_i.velocity;
			float newtime = elapsed_ms +100.0;

			float testdot = 3.14*(pow(radius, 2));
			//vec2 dot_line2 = { testdot,testdot };
			vec2 dot_line = { bonding_box.x / 20, bonding_box.y / 20 };
			//Entity line1 = createLine(motion_i.position, line_scale1);
			//Entity line2 = createLine(motion_i.position, line_scale2);
			if (registry.deadlys.has(entity_i)) {
				Entity hTop = createLine({ motion_i.position.x, motion_i.position.y - 105 }, line_scaleE);
				Entity hBottom = createLine({ motion_i.position.x, motion_i.position.y + 105 }, line_scaleE);

				Entity vLeft = createLine({ motion_i.position.x - 105,motion_i.position.y }, line_scaleELR);
				Entity vRight = createLine({ motion_i.position.x + 105,motion_i.position.y }, line_scaleELR);
				if(newtime>elapsed_ms) motion_i.velocity = { 0,0 };
				motion_i.velocity = velocity;
			}
			if (registry.eatables.has(entity_i)) {
				Entity hTop = createLine({ motion_i.position.x, motion_i.position.y - 65 }, line_scale2);
				Entity hBottom = createLine({ motion_i.position.x, motion_i.position.y + 65 }, line_scale2);
				Entity vLeft = createLine({ motion_i.position.x - 65,motion_i.position.y }, line_scale1);
				Entity vRight = createLine({ motion_i.position.x + 65,motion_i.position.y }, line_scale1);
				if (newtime > elapsed_ms) motion_i.velocity = { 0,0 };
				motion_i.velocity = velocity;
			}
			if (registry.players.has(entity_i)) {
				//Entity line1 = createLine(motion_i.position, line_scale1);
				//Entity line2 = createLine(motion_i.position, line_scale2);
				//Entity dot = createLine({ motion_i.position.x - 30,motion_i.position.y }, dot_line);

				//registry.motions.get(dot).position.x = registry.motions.get(entity_i).position.x;
				//Entity dot2 = createLine({motion_i.position.x,motion_i.position.y }, dot_line2);
				//Entity dot = createLine(dot_m, doted );
				Entity hTop = createLine({ motion_i.position.x, motion_i.position.y - 80 }, line_scale2);
				Entity hBottom = createLine({ motion_i.position.x, motion_i.position.y + 80 }, line_scale2);
				Entity vLeft = createLine({ motion_i.position.x - 80,motion_i.position.y }, line_scale1);
				Entity vRight = createLine({ motion_i.position.x + 80,motion_i.position.y }, line_scale1);
				if (newtime > elapsed_ms) motion_i.velocity = { 0,0 };
				motion_i.velocity = velocity;
				//registry.motions.get(vRight).in_motion = false; 
				//registry.motions.get(vLeft).in_motion = false;
				//registry.motions.get(hTop).in_motion = false;
				//registry.motions.get(hTop).in_motion = false;
			}
			// freeze screen for one second

		}
	}


	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A3: HANDLE EGG collisions HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 3
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}