


#define ArrayLength(Array) (sizeof(Array) / sizeof((Array)[0]))


#define COLOR uint32_t



struct rect
{
	float x;
	float y;
	float w;
	float h;
};


// struct vec2f
// {
// 	float x;
// 	float y;
// };


struct game_input 
{
	bool w;
	bool a;
	bool s;
	bool d;
	bool up;
	bool down;
	bool left;
	bool right;
};


struct game_buffer
{
	void *Memory;
	int Width;
	int Height;
};


struct snow_piece
{
	rect pos;
	int damage;
	bool dead;
};

struct snow_person
{
	rect moveTarget; // what are we moving towards
	bool allDead; // inanimate
	float deadFor; // amount of time dead

	union
	{
		snow_piece pieces[4];
		struct
		{
			snow_piece base;
			snow_piece mid;
			snow_piece head;
			snow_piece hat;
		};
	};

	rect shadowPos;
	float height;


	float delays[4];
	float tx[4];
	float ty[4];
	// float moveTowardsParentDelay; // when hits 0, start moving towards
	// // 100 recent moves
	// rect queuedMove[100];

	float targetOffsetX;
	float targetOffsetY;

	float randomSpeedFactor;

	float jumpUpLeft;        // time left on the up swing
	float timeTillNextJump;  // time till we jump again
	bool landed;
};


struct snow_ball
{
	rect shadowPos;
	rect colPos;
	int dir;  //  1-4 = direction
	float height;  // dies at 0
	int life;    // -1 is dead, 0+ is time alive
	float xSpeed;  // extra speed, in addition to snowBallSpeed
	float ySpeed;  // (from player moving at launch)
};


struct dead_piece
{
	rect pos;
	float height;
	bool isHat;
};


internal COLOR MakeColor(int r, int g, int b);

internal void MainGameLoop(game_buffer *BackBuffer, game_input Input, float dt);


internal int RandomIntLessThan(int max)
{
	return rand() % max;
}


global_variable COLOR White = MakeColor(220, 220, 220);
global_variable COLOR Black = MakeColor(220, 50, 50);

global_variable COLOR DeadBlue = MakeColor(89+20, 66+20, 200+20);
global_variable COLOR ReallyBlue = MakeColor(66, 200, 89); //(70, 200, 80);

global_variable COLOR ReadRed = MakeColor(90+20, 180+20, 180+20);
global_variable COLOR ReallyRed = MakeColor(180, 90, 90);




global_variable COLOR Ground    = MakeColor(221, 233, 219); // lighten a bit
global_variable COLOR SnowMound = MakeColor(210, 228, 208); // less contrast

global_variable COLOR TopHat = MakeColor(45, 45, 45);




global_variable COLOR PlayerBody = ReallyBlue;
global_variable COLOR PlayerMitts = ReallyRed;

// global_variable COLOR PlayerBodyDead = DeadBlue;
// global_variable COLOR PlayerMittsDead = ReadRed;
global_variable COLOR PlayerBodyDead = ~ReallyBlue;
global_variable COLOR PlayerMittsDead = ~ReallyRed;



global_variable COLOR DeadPieceColor = MakeColor(240-8, 250-7, 240-8);
global_variable COLOR DeadHatColor = MakeColor(45+20, 45+20, 45+20);


global_variable COLOR SnowmanBody = MakeColor(240, 250, 240);
global_variable COLOR SnowmanMid  = MakeColor(240, 250, 240);
global_variable COLOR SnowmanHead = MakeColor(240, 250, 240);
global_variable COLOR SnowmanShadow = MakeColor(205, 205, 180);

global_variable COLOR SnowballColor  = MakeColor(250, 255, 249);
global_variable COLOR SnowballShadow = MakeColor(201, 210, 186);  // going more blue



global_variable COLOR TextColor = MakeColor(164, 166, 142);  // still darker
										//   b    r    g  // these are all hilariously backwards

// todo shift H function


//
//
// GAME SETTINGS
global_variable float PlayerSpeed = 0.3f;
global_variable float launchDelay = 510;
global_variable float maxDistFactor = 550.0f;
global_variable float ballSpeed = 0.4f;
global_variable float snowballAffectedByPlayerSpeedFactor = 2.0f;
global_variable float nudgeAmount = 1.0f; //0.6f;
global_variable float PlayerTouchRadius = 18;


// SNOWMEN SETTINGS
const global_variable int MaxSnowmen = 16;
global_variable float SnowmanSpeed = 0.04f;
global_variable float SnowmanSpeedDefault = 0.04f;
global_variable float SnowmanSpeedPercentRange = 40; // 2 sided, ±half this
global_variable float SnowmanStrikeSpeed = SnowmanSpeed*1.2f;
// global_variable float SnowmanSpawnTimer = 5000.0f; //ms?
// global_variable float SnowmanSpawnTimerDefault = 5000.0f; //ms?
global_variable float SnowmanSpawnIn = 0;
global_variable float SnowmanStrikeRadius = 128.0f;

global_variable float SnowmanSpawnTimerBase = 2000.0f; //ms?
global_variable float SnowmanSpawnTimerExtra = 2000.0f; //ms?
global_variable float SnowmanSpawnTimerExtraDefault = 2000.0f; //ms?

// global_variable float SnowmanJumpUpSpeed = 5.3f; //gotta overcome gravitry
// these are in ms i think, each frame is about 8 ms
global_variable float SnowmanJumpLength = 200.0f;  // time spent moving up
global_variable float SnowmanJumpDelay = 10.0f;  //needs to account for time to land


// SNOWBALL PARTICLE SETTINGS
const global_variable int maxSnowBalls = 100;
global_variable snow_ball snowballs[maxSnowBalls];
global_variable int snowBallCounter = 0;
global_variable int snowBallStartingHeight = 22;


// meta game state
global_variable bool GameInitialized = false;
global_variable bool GameOver = false;

global_variable int SessionHighScore = 0;


