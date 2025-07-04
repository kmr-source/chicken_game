// internal
#include "ai_system.hpp"
#include "world_init.hpp"

vec2 bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}
void AISystem::step(float elapsed_ms)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE BUG AI HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// You will likely want to write new functions and need to create
	// new data structures to implement a more sophisticated Bug AI.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	(void)elapsed_ms;
	
    /*Make the bugs smarter by enabling them to avoid the chicken. The bugs should
	avoid the chicken by staying at least some minimum distance ǫ away from it.After
	an encounter, the bugs should follow the shortest path to the wall opposite its
	starting point.The goal path should be updated every X frames.The frequency
	X with which these paths are recomputed should be user-controllable*/
	// Contact chicken = bug moves horizontal toward the goal path (wall) 
	// hits the wall move in horizontal line towards the other side of the wall 
	//(void)elapsed_ms; // placeholder to silence unused warning until implemented
	float current_speed = 1.f;
	float maxFrame = 2000 * 3;
	float maxNum = 15;
	float goal_path = 0; 
	//float next_frame = 0;
	//next_frame += -elapsed_ms*current_speed  *0.5;
	//printf("%d\n", next_frame);

	//float min_distance = 10.0f; 
	//float goal_path = 0.0; // for now goal path updated every X frames? 
	float frames = 0.5-elapsed_ms*current_speed; 
	for (Entity e : registry.eatables.entities) {
		Motion& m = registry.motions.get(e);
		vec2 position = registry.motions.get(e).position;
		//float posStartX = position.x; 
		vec2 wallDimensionLR = { 30.0f, window_width_px - 30.0f };

		goal_path = getDistancePath(position, wallDimensionLR, goal_path);

		float min_distance = 200.0f; // min distance between bug and CHICKEN0
		
		//  within range of collision with the player so we need to recalculate distance
		if (player_in_range(position, min_distance)) {
			//printf("it is in range\n");
			if (frames < 0.f) {
				//printf("\nframes less than 0");
				//registry.motions.get(e).position.x += 2.0; // make it 2.0 apart 
				//registry.motions.get(e).position.y += -2.0;
				//registry.motions.get(e).velocity.x += -1.0; // keep the zig zag 
				frames = (maxFrame / 2)*(maxFrame / 2);
				goal_path = getDistancePath(position, wallDimensionLR, goal_path); // updates goal path every time we are within range of the player 
				if(registry.motions.get(e).position.x != goal_path) {
					
					registry.motions.get(e).position.x += 2.0; // make it 2.0 apart 
					registry.motions.get(e).position.y += -2.0;
					registry.motions.get(e).velocity.x += -1.0; // keep the zig zag
				}
				
				//printf("\nframes updated");
			}
			
			//printf("\n end of if statement\n");
			//registry.motions.get(e).velocity.x *= -1.0;
			//registry.motions.get(e).position.x += min_distance;
			//registry.motions.get(e).velocity.x += -1.0;

		} else if (!player_in_range(position, min_distance)) { // not in range, hit wall move opposite direction on normal till dissapear
			
			if (m.position.x < 30.0f) {
				//printf("hello"); left
				m.position.x += 30.0f; // only horizontal along the new line ater fliping sign
				m.position.y += 0;
				//motion.velocity.y *= -10.0;
				m.velocity.x = m.velocity.x*-1.0;
				//motion.velocity.y *= -1.0; //hits wall goes down 
			}

			if (m.position.x > window_width_px - 30.0f) {
				//printf("bye"); right
				m.position.x += -30.0f;
				m.position.y += 0;
				m.velocity.x = m.velocity.x*-1.0;
				//motion.velocity.y =10.0; //hits wall goes down 
			}
		}
	}

		
}
	//float random = float(rand()) / float((RAND_MAX));
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: DRAW DEBUG INFO HERE on AI path
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// You will want to use the createLine from world_init.hpp
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// check if player is within range of bug
bool AISystem::player_in_range(vec2 position, float radius) {

	for (Entity e : registry.players.entities) {
		Motion& motion = registry.motions.get(e);
		float playerX = motion.position.x;
		float playerY = motion.position.y;
		float posX = position.x;
		float posY = position.y;

		//printf("%lf posX: ", posX);
		//printf("%lf playerX: ", playerX);
		
		double absX = abs(posX - playerX);
		double absY = abs(posY - playerY);
		double r = (double)radius; 

		//printf("%d absX: ", absX);
		//printf("%d absY: ", absY);

		double squareX = absX * absX;
		double squareY = absY * absY;

		double total = squareX + squareY; 

		if (absY > r || absX > r) {
			return false;
		}
		if (total <= r * r) {
			return true; 
		}
		return false; 
		
	}
	return false;
}

float  AISystem::getDistancePath(vec2 position, vec2 wall_position, float curr_goal_path) {

	float pathRight = abs(position.x - wall_position.x);
	float pathLeft = abs(position.x - wall_position.y);

	// posX greater value than curr goal path , then return currgoal path
	if (pathRight < abs(curr_goal_path) && pathRight< pathLeft) {
		return pathRight;
	}
	else if (pathLeft < abs(curr_goal_path) && pathLeft < pathRight) {
		return pathLeft;
	}
	// or else we return current path as it is the shortest 
	return curr_goal_path; 
}

