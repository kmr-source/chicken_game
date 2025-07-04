// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <math.h>

#include "physics_system.hpp"

// Game configuration
const size_t MAX_EAGLES = 15;
const size_t MAX_BUG = 5;
const size_t EAGLE_DELAY_MS = 2000 * 3;
const size_t BUG_DELAY_MS = 5000 * 3;

// Create the bug world
WorldSystem::WorldSystem()
	: points(0)
	, next_eagle_spawn(0.f)
	, next_bug_spawn(0.f) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (chicken_dead_sound != nullptr)
		Mix_FreeChunk(chicken_dead_sound);
	if (chicken_eat_sound != nullptr)
		Mix_FreeChunk(chicken_eat_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Chicken Game Assignment", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	chicken_dead_sound = Mix_LoadWAV(audio_path("chicken_dead.wav").c_str());
	chicken_eat_sound = Mix_LoadWAV(audio_path("chicken_eat.wav").c_str());

	if (background_music == nullptr || chicken_dead_sound == nullptr || chicken_eat_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("chicken_dead.wav").c_str(),
			audio_path("chicken_eat.wav").c_str());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// Set all states to default
    restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Updating window title with points
	std::stringstream title_ss;;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motions_registry = registry.motions;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size()-1; i>=0; --i) {
	    Motion& motion = motions_registry.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			if(!registry.players.has(motions_registry.entities[i])) // don't remove the player
				registry.remove_all_components_of(motions_registry.entities[i]);
		}
	}

	// Spawning new eagles
	next_eagle_spawn -= elapsed_ms_since_last_update * current_speed*0.40; // reduce eagle spawn time 
	if (registry.deadlys.components.size() <= MAX_EAGLES && next_eagle_spawn < 0.f) {
		// Reset timer
		next_eagle_spawn = (EAGLE_DELAY_MS / 2) + uniform_dist(rng) * (EAGLE_DELAY_MS / 2);
		// Create eagle with random initial position
        createEagle(renderer, vec2(50.f + uniform_dist(rng) * (window_width_px - 100.f), 100.f));
	}

	// Spawning new bug
	next_bug_spawn -= elapsed_ms_since_last_update * current_speed*0.40;
	float random = float(rand()) / float((RAND_MAX));
	if (registry.eatables.components.size() <= MAX_BUG && next_bug_spawn < 0.f) {
		// !!!  TODO A1: Create new bug with createBug({0,0}), as for the Eagles above
		//createBug({ 0,0 });
		next_bug_spawn = (EAGLE_DELAY_MS/4) + uniform_dist(rng) * (EAGLE_DELAY_MS/4 );
		Entity bugs = createBug(renderer, vec2(50.f+uniform_dist(rng)* (window_width_px - 100.f),50.f));
		if ((registry.eatables.components.size() % 2) == 0) {
			//printf("\n %d start in if statement so even\n", count_bugs);

			registry.motions.get(bugs).velocity.x = 30 * random;
			//printf("\nit is positive so make it negative\n");
			//printf("\nit is positive so make it negative\n");
		}
		else if (registry.eatables.components.size() % 2 != 0) {
			//printf("\nit is negative... DID IT SET THE VELOCITY TO NEGATIVE???\n");
			//printf("%d", count_bugs);

			registry.motions.get(bugs).velocity.x = -30 * random;
		}
		//createBug(renderer, vec2(0.f + uniform_dist(rng) * (window_width_px - 100.f), 0.f));
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A3: HANDLE EGG SPAWN HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 3
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Processing the chicken state
	assert(registry.screenStates.components.size() <= 1);
    ScreenState &screen = registry.screenStates.components[0];

    float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if(counter.counter_ms < min_counter_ms){
		    min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.deathTimers.remove(entity);
			screen.darken_screen_factor = 0;
            restart_game();
			return true;
		}
	}
	// reduce window brightness if any of the present chickens is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;


	float min_counter_msLU = 3000.f;

	// !!! TODO A1: update LightUp timers and remove if time drops below zero, similar to the death counter
	for (Entity entity : registry.lightup.entities) {
		// progress timer
		Lightup& counterLU = registry.lightup.get(entity);
		counterLU.counter_ms -= elapsed_ms_since_last_update;
		if (counterLU.counter_ms < min_counter_msLU) {
			min_counter_msLU = counterLU.counter_ms;
		}

		// restart the game once the death timer expired
		if (counterLU.counter_ms < 0) {
			registry.lightup.remove(entity);
			registry.players.get(entity).has_eaten = false;
		    //screen.darken_screen_factor = 0;
			//restart_game();
			//return true;
		}
	}

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");
	 
	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
	    registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Create a new chicken
	player_chicken = createChicken(renderer, { window_width_px/2, window_height_px - 200 });
	registry.colors.insert(player_chicken, {1, 0.8f, 0.8f});

	// !! TODO A3: Enable static eggs on the ground
	// Create eggs on the floor for reference
	/*
	for (uint i = 0; i < 20; i++) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		float radius = 30 * (uniform_dist(rng) + 0.3f); // range 0.3 .. 1.3
		Entity egg = createEgg({ uniform_dist(rng) * w, h - uniform_dist(rng) * 20 },
			         { radius, radius });
		float brightness = uniform_dist(rng) * 0.5 + 0.5;
		registry.colors.insert(egg, { brightness, brightness, brightness});
	}
	*/
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions; // TODO: @Tim, is the reference here needed?
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;
		//registry.players.get(entity).has_eaten = false;

		// For now, we are only interested in collisions that involve the chicken
		if (registry.players.has(entity)) {
			Player& player = registry.players.get(entity);
			Motion& motion = registry.motions.get(entity);


			// Checking Player - Deadly collisions
			if (registry.deadlys.has(entity_other)) {
				// initiate death unless already dying
				if (!registry.deathTimers.has(entity)) {
					// Scream, reset timer, and make the chicken sink
					registry.deathTimers.emplace(entity);
					Mix_PlayChannel(-1, chicken_dead_sound, 0);
					// !!! TODO A1: change the chicken orientation and color on death
					//Turn 180 to fall downwards using pi, then put x veloivty to 0 and fall down
					motion.angle = M_PI; 
					motion.velocity = { 0,200 };
					// set is_alive to dead
					registry.players.get(entity).is_alive = false;
					motion.in_motion = false; 
					if (!player.is_alive) {
						// not alive chicken is red 
						registry.colors.get(entity) = { 1.0,0,0 };
					}
					
					

				}
			}
			// Checking Player - Eatable collisions
			else if (registry.eatables.has(entity_other)) {
				if (!registry.deathTimers.has(entity)) {
					// chew, count points, and set the LightUp timer
					registry.remove_all_components_of(entity_other);
					Mix_PlayChannel(-1, chicken_eat_sound, 0);
					registry.players.get(entity).has_eaten = true;
					if (!registry.lightup.has(entity)) {
						registry.lightup.emplace(entity);
					}


					++points;

					// after ++ points set back to false
					// add it 
					//float counter = registry.deathTimers.get(entity).counter_ms;
					// set eaten to true 
			
					//registry.lightup.get(entity).counter_ms= 
					
					//registry.lightup.get(entity).counter_ms = 0; 
					//registry.lightup.get(entity).counter_ms=0;
					
					// !!! TODO A1: create a new struct called LightUp in components.hpp and add an instance to the chicken entity by modifying the ECS registry
					//registry.lightup.emplace(entity);
					//registry.lightup.get(entity).counter_ms =0;
				}
				
			}
		}
	}
	

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE CHICKEN MOVEMENT HERE
	// key is of 'type' GLFW_KEY_
	// action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	for (Entity& player : registry.players.entities) {
		//Motion struct

		Motion& motion = registry.motions.get(player_chicken);
		float angle = motion.angle; 
		// pi/2  to get the angles 
		float pieD2 = M_PI / 2;
		float temp = angle - pieD2;
		float temp2 = angle + pieD2;
		bool is_alive = registry.players.get(player_chicken).is_alive;
		//https://piazza.com/class/kwe7kqx0oj2sb?cid=120
		if (action == GLFW_PRESS) {
			if (key == GLFW_KEY_UP && is_alive) {
				//newx = oldx * cosf(angle) - oldy * sinf(angle);
				//newy = oldx * sinf(angle) + oldy * cosf(angle);
				//motion.position = {newx, newy };
				//motion.position = { cosf(motion.angle),sinf(motion.angle) };4
				motion.velocity.x = 100*temp;
				motion.velocity.y = 100*temp;
				motion.in_motion = true;
			}
			else if (key == GLFW_KEY_DOWN && is_alive) {
				//motion.position = { -cosf(motion.angle), -sinf(motion.angle) };
				motion.velocity.x = 100*temp2;
				motion.velocity.y = 100*temp2;
				motion.in_motion = true;
			}
			else if (key == GLFW_KEY_RIGHT && is_alive ) {
				//motion.angle = -0.03145f + motion.angle;
				motion.velocity.x = 100*temp2;
				motion.velocity.y = 100 *temp;
				motion.in_motion = true;
			}
			else if (key == GLFW_KEY_LEFT&& is_alive) {
				//motion.angle = 0.03145f + motion.angle;
				motion.velocity.x = 100*temp;
				motion.velocity.y = 100*temp2;
				motion.in_motion = true;
			}
		}
		else if (action == GLFW_RELEASE) {
			//motion.velocity.x = 0;
			//motion.velocity.y = 0;
			motion.in_motion = false;

		}
		
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

        restart_game();
	}

	// Debugging
	if (key == GLFW_KEY_D) {
		if (action == GLFW_RELEASE) {
			debugging.in_debug_mode = false;
			debugging.in_freeze_mode = false;
		} else {
			debugging.in_debug_mode = true;
			debugging.in_freeze_mode = true;
		}
		

			
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE CHICKEN ROTATION HERE
	// xpos and ypos are relative to the top-left of the window, the chicken's
	// default facing direction is (1, 0)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	double xpos, ypos;
	//glfwGetCursorPos(window, &xpos, &ypos);
	xpos = mouse_position.x;
	ypos = mouse_position.y;
	for (Entity& player : registry.players.entities) {
		Motion& motion = registry.motions.get(player);
		float x = motion.position.x; 
		float y = motion.position.y;
		bool is_alive = registry.players.get(player_chicken).is_alive;
		//y = ypos - y;
		//x = x-xpos;
		motion.in_motion = true;
		float rotate = atan2(ypos-y, xpos-x);
		float speed = 200; 
		//float rotate = x * 0.01745f;
		motion.angle = rotate; 
		//motion.position.x = cos(rotate);
		//motion.position.y = sin(rotate);
	

		// set velocity to the direction of the cursor, at a magnitude of the player
		if (motion.in_motion && is_alive){
			float speed = 200;
			//motion.position = { xpos,y };
			float x_component = cos(rotate)*speed; 
			float y_component = sin(rotate)*speed;
			motion.velocity = { x_component, y_component };
			motion.angle = rotate + (0.5*M_PI);
			motion.destination = { xpos,ypos };
		}
		if (!is_alive) {
			motion.angle = M_PI;
			motion.destination = { 0,0 };
			motion.velocity = { 0,200 };
		}
	
		//motion.in_motion = false;
		//motion.destination = { 0,0 };
		/*if (!motion.in_motion && !registry.players.get(player).is_alive) {
			motion.velocity = { 0,0 };
			motion.position = { 0,0 };
		}*/
		//double xpos2, ypos2;
		//glfwGetCursorPos(window, &xpos2, &ypos2);
		//motion.position.y = ypos;
		//motion.position.x = xpos;
		//motion.position = { x,y};
	}
	//glfSetKey
	//float rotation = 
	//(vec2)mouse_position; // dummy to avoid compiler warning
}
