#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleSceneIntro.h"
#include "ModuleInput.h"
#include "ModuleTextures.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"

ModuleSceneIntro::ModuleSceneIntro(Application* app, bool start_enabled) : Module(app, start_enabled)
{

	// Initialise all the internal class variables, at least to NULL pointer
	//circle = box = rick = NULL;
	ray_on = false;
	sensed = false;

	//Init Animations


}

ModuleSceneIntro::~ModuleSceneIntro()
{
	// You should do some memory cleaning here, if required
}

bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	// Set camera position
	App->renderer->camera.x = App->renderer->camera.y = 0;

	// Load textures
	leftFlipperTex = App->textures->Load("Assets/Textures/leftFlipper.png");
	rightFlipperTex = App->textures->Load("Assets/Textures/rightFlipper.png");
	scene = App->textures->Load("Assets/Textures/Pinball_Scene.png");

	//Load SFX
	kicker_sfx = App->audio->LoadFx("Assets/Audio/FX/kicker.wav");

	//Load Music
	//App->audio->PlayMusic("Assets/Audio/Music/song.ogg", 1.0f);

	//Flippers creation
	b2RevoluteJointDef flipperJointDef_left;
	b2RevoluteJointDef flipperJointDef_right;

	b2RevoluteJoint* flipperJoint_left;
	b2RevoluteJoint* flipperJoint_right;

	//Flippers' colliders
	leftFlipper = App->physics->CreateRectangle(266+35, 672+10, 75, 20, b2BodyType::b2_dynamicBody, ColliderType::FLIPPER);
	rightFlipper = App->physics->CreateRectangle(338+65, 672+10, 75, 20, b2BodyType::b2_dynamicBody, ColliderType::FLIPPER);

	//Flipper's joints colliders
	leftFlipperJoint = App->physics->CreateCircle(248, 679, 3, b2BodyType::b2_staticBody, ColliderType::UNKNOWN);
	rightFlipperJoint = App->physics->CreateCircle(456, 679, 3, b2BodyType::b2_staticBody, ColliderType::UNKNOWN);
	
	flipperJointDef_left.Initialize(leftFlipper->body, leftFlipperJoint->body, leftFlipperJoint->body->GetWorldCenter());
	flipperJointDef_right.Initialize(rightFlipper->body, rightFlipperJoint->body, rightFlipperJoint->body->GetWorldCenter());

	flipperJointDef_left.lowerAngle = -0.2f * b2_pi;
	flipperJointDef_left.upperAngle = 0.2f * b2_pi;
	flipperJointDef_right.lowerAngle = -0.2f * b2_pi;
	flipperJointDef_right.upperAngle = 0.2f * b2_pi;
	
	flipperJointDef_left.enableLimit = true;
	flipperJointDef_right.enableLimit = true;

	flipperJoint_left = (b2RevoluteJoint*)App->physics->world->CreateJoint(&flipperJointDef_left);
	flipperJoint_right = (b2RevoluteJoint*)App->physics->world->CreateJoint(&flipperJointDef_right);


	shooter = App->physics->CreateRectangle(SCREEN_WIDTH - 80, SCREEN_HEIGHT - 50, 28, 16, b2BodyType::b2_kinematicBody, ColliderType::WALL);
	shooter->listener = this;

	/* COLLIDERS */
	//Upper Wall Collider
	//wall = App->physics->CreateRectangle(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH - 2, -64, b2BodyType::b2_staticBody, ColliderType::WALL);
	int points2[59] = { 64, 512,
					  144, 512,
					  144, 602,
					  256, 640,
					  256, 767,
						0, 767,
						0,	 0,
					  767,	 0,
					  767, 767,
					  448, 767,
					  448, 640,
					  560, 602,
					  560, 512,
					  671, 512,
					  671, 736,
					  704, 736,
					  704, 132,
					  701, 123, //First curve, first point
					  697, 112,
					   681, 93, //Half first curve
					   667, 85,
					   650, 80,	//First curve, last point
					   192, 80,	//Second curve, First point
					   153, 86,	
					   125, 99,	
					  105, 113,	//Half second curve
					   82, 142,
					   73, 162,
					   64, 196 };	//Second curve, last point
	wall = App->physics->CreateChain(0, 0, points2, 59, b2BodyType::b2_staticBody, ColliderType::WALL);

	//Death Collider
	through = App->physics->CreateRectangleSensor(SCREEN_WIDTH/2 - 32, (SCREEN_HEIGHT-32) + 16, 64*3, 32, b2BodyType::b2_staticBody, ColliderType::THROUGH);

	//100 points Collider
	App->physics->CreateCircle(298, 244, 14, b2BodyType::b2_staticBody, ColliderType::_100PTS);
	App->physics->CreateCircle(405, 244, 14, b2BodyType::b2_staticBody, ColliderType::_100PTS);

	//200 points Collider
	App->physics->CreateCircle(352, 217, 14, b2BodyType::b2_staticBody, ColliderType::_200PTS);

	//Bumpers Colliders
	App->physics->CreateCircle(197, 332, 14, b2BodyType::b2_staticBody, ColliderType::BUMPER);
	App->physics->CreateCircle(507, 332, 14, b2BodyType::b2_staticBody, ColliderType::BUMPER);
	App->physics->CreateCircle(241, 471, 14, b2BodyType::b2_staticBody, ColliderType::BUMPER);
	App->physics->CreateCircle(464, 471, 14, b2BodyType::b2_staticBody, ColliderType::BUMPER);
	App->physics->CreateCircle(352, 527, 14, b2BodyType::b2_staticBody, ColliderType::BUMPER);

	//Center map wall
	int points3[12] = { 351, 264,
					  288, 327,
					  288, 366,
					  351, 429,
					  414, 366,
					  414, 327 };
	wall = App->physics->CreateChain(0, 0, points3, 12, b2BodyType::b2_staticBody, ColliderType::WALL);
	
	//Down-Left curved wall
	int points4[46] = { 112, 296,
					 112, 345,
					 114, 353,
					 117, 363,
					 120, 372,
					 124, 378,
					 130, 384,
					 138, 391,
					 144, 397,
					 155, 402,
					 166, 405,
					 173, 407,
					 190, 408,
					 190, 393,
					 174, 393,
					 165, 390,
					 157, 386,
					 148, 380,
					 140, 372,
					 132, 361,
					 128, 351,
					 126, 342,
					 126, 296 };
	wall = App->physics->CreateChain(0, 0, points4, 46, b2BodyType::b2_staticBody, ColliderType::WALL);

	//Down-Right curved wall
	int points5[44] = { 577, 297,
					  577, 346,
					  574, 353,
					  571, 361,
					  564, 371,
					  559, 377,
					  553, 382,
					  544, 388,
					  535, 392,
					  526, 395,
					  514, 395,
					  514, 408,
					  530, 408,
					  541, 405,
					  554, 400,
					  565, 393,
					  574, 385,
					  581, 376,
					  586, 366,
					  589, 356,
					  592, 346,
					  592, 297 };
	wall = App->physics->CreateChain(0, 0, points5, 44, b2BodyType::b2_staticBody, ColliderType::WALL);

	//Up-Right curved wall
	int points6[46] = { 311, 177,
					  295, 177,
					  285, 179,
					  274, 183,
					  265, 187,
					  260, 192,
					  253, 198,
					  247, 205,
					  241, 216,
					  238, 225,
					  234, 238,
					  234, 287,
					  248, 287,
					  248, 240,
					  250, 232,
					  254, 223,
					  260, 215,
					  265, 209,
					  271, 203,
					  282, 197,
					  290, 193,
					  298, 190,
					  311, 190 };
	wall = App->physics->CreateChain(0, 0, points6, 46, b2BodyType::b2_staticBody, ColliderType::WALL);

	//Up-Right curved wall
	int points7[44] = { 393, 177,
					  393, 190,
					  407, 191,
					  415, 193,
					  422, 196,
					  430, 201,
					  436, 207,
					  444, 214,
					  450, 224,
					  454, 232,
					  456, 243,
					  456, 286,
					  470, 286,
					  470, 238,
					  467, 225,
					  463, 215,
					  458, 206,
					  451, 198,
					  442, 189,
					  429, 183,
					  420, 180,
					  409, 177 };
	wall = App->physics->CreateChain(0, 0, points7, 44, b2BodyType::b2_staticBody, ColliderType::WALL);

	//Upper left Bouncer
	int points8[8] = { 63, 474,
						77, 473,
						142, 496,
						142, 512 };
	wall = App->physics->CreateChain(0, 0, points8, 8, b2BodyType::b2_staticBody, ColliderType::BUMPER);
	//Lower left bouncer
	int points9[8] = { 146, 512,
						155, 513,
						203, 604,
						202, 624 };
	wall = App->physics->CreateChain(0, 0, points9, 8, b2BodyType::b2_staticBody, ColliderType::BUMPER);
	//Upper right bouncer
	int points10[10] = { 562, 511,
						562, 497,
						628, 473,
						641, 473,
						641, 512};
	wall = App->physics->CreateChain(0, 0, points10, 10, b2BodyType::b2_staticBody, ColliderType::BUMPER);
	//Lower right bouncer
	int points11[8] = { 501, 623,
						501, 604,
						547, 512,
						560, 512};
	wall = App->physics->CreateChain(0, 0, points11, 8, b2BodyType::b2_staticBody, ColliderType::BUMPER);


	// Add this module (ModuleSceneIntro) as a listener for collisions with the sensor.
	// In ModulePhysics::PreUpdate(), we iterate over all sensors and (if colliding) we call the function ModuleSceneIntro::OnCollision()
	
	return ret;
}

bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	App->textures->Unload(scene);
	App->textures->Unload(leftFlipperTex);
	App->textures->Unload(rightFlipperTex);

	return true;
}

update_status ModuleSceneIntro::Update()
{
	App->renderer->Blit(scene, 0, 0, NULL);

	shooter->body->SetAngularVelocity(5);

	// If user presses SPACE, enable RayCast
	if(App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
	{
		// Enable raycast mode
		ray_on = !ray_on;

		// Origin point of the raycast is be the mouse current position now (will not change)
		ray.x = App->input->GetMouseX();
		ray.y = App->input->GetMouseY();
	}

	
	// Prepare for raycast ------------------------------------------------------
	// The target point of the raycast is the mouse current position (will change over game time)
	iPoint mouse;
	mouse.x = App->input->GetMouseX();
	mouse.y = App->input->GetMouseY();

	// Total distance of the raycast reference segment
	int ray_hit = ray.DistanceTo(mouse);

	// Declare a vector. We will draw the normal to the hit surface (if we hit something)
	fVector normal(0.0f, 0.0f);



	//Blit flippers
	if (leftFlipper != NULL)
	{
		int x, y;
		leftFlipper->GetPosition(x, y);
		App->renderer->Blit(leftFlipperTex, x, y, NULL, 0.2f, leftFlipper->GetRotation());
	}
	if (rightFlipper != NULL)
	{
		int x, y;
		rightFlipper->GetPosition(x, y);
		App->renderer->Blit(rightFlipperTex, x, y, NULL, 0.2f, rightFlipper->GetRotation());
	}

	
	return UPDATE_CONTINUE;
}

void ModuleSceneIntro::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	

	// Do something else. You can also check which bodies are colliding (sensor? ball? player?)
}
