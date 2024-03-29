#define OLC_PGE_APPLICATION
#include "../olcPixelGameEngine.h"

#define OLC_PGEX_RAYCASTWORLD
#include "../olcPGEX_RayCastWorld.h"

// ADAPTOR CLASS - Override the RayCastWorld Engine and fill in the blanks!
class ExampleGame : public olc::rcw::Engine
{
public:
	ExampleGame(const int screen_w, const int screen_h, const float fov)
		: olc::rcw::Engine(screen_w, screen_h, fov)
	{ 
		sMap =
			"################################################################"
			"#.........#....................................................#"
			"#.........#....................................................#"
			"#.........#....................................................#"
			"#.........#....................................................#"
			"#.........#############........................................#"
			"#...............#..............................................#"
			"#...............#..............................................#"
			"#...............#..............................................#"
			"#...............#..............................................#"
			"#...............#..............................................#"
			"#...............#..............................................#"
			"#..............................................................#"
			"#..............................................................#"
			"#..............................................................#"
			"#..............................................................#"
			"#..............................................................#"
			"#..............................................................#"
			"#..............................................................#"
			"#..............................................................#"
			"#...........................#..................................#"
			"#.......................#####..................................#"
			"#..............................................................#"
			"#..............................................................#"
			"#..............................................................#"
			"#..............................................................#"
			"#..............................................................#"
			"#######........................................................#"
			"#.....#........................................................#"
			"#..............................................................#"
			"#.....#........................................................#"
			"################################################################";

		vWorldSize = { 64, 32 };	
	}
	
protected:
	// User implementation to retrieve appropriate graphics for scenery
	olc::Pixel SelectSceneryPixel(const int tile_x, const int tile_y, const olc::rcw::Engine::CellSide side, const float sample_x, const float sample_y, const float distance) override
	{
		olc::Pixel p;

		// Choose appropriate colour
		switch (side)
		{
			case olc::rcw::Engine::CellSide::Top: // Location is "Sky"
				p = olc::CYAN; 
				break;

			case olc::rcw::Engine::CellSide::Bottom: // Location is "Ground"
				p =  olc::DARK_GREEN;
				break;

			default: // Location is "Wall"
				p = olc::WHITE; 
				if (sample_x < 0.05f || sample_x > 0.95f || sample_y < 0.05f || sample_y > 0.95f)
					p = olc::BLACK;
				break;			
		}
	
		// Apply directional lighting, by first creating a shadow scalar...
		float fShadow = 1.0f;
		switch (side)
		{
			case olc::rcw::Engine::CellSide::South: fShadow = 0.3f; break;
			case olc::rcw::Engine::CellSide::East:  fShadow = 0.3f; break;
		}

		// ...also shade by distance...
		float fDistance = 1.0f - std::min(distance / 32.0f, 1.0f);

		// ...and applying it to sampled pixel
		p.r = uint8_t(float(p.r) * fDistance);
		p.g = uint8_t(float(p.g) * fDistance);
		p.b = uint8_t(float(p.b) * fDistance);

		return p;
	}

	// User implementation to retrieve if a particular tile is solid
	bool IsLocationSolid(const float tile_x, const float tile_y) override
	{
		if (int(tile_x) >= 0 && int(tile_x) < vWorldSize.x && int(tile_y) >= 0 && int(tile_y) < vWorldSize.y)
			return sMap[int(tile_y) * vWorldSize.x + int(tile_x)] == '#';
		else
			return true;
	}


	// NOTE! Objects are not used in this demonstration ===================

	// User implementation to retrieve dimensions of an in game object
	float GetObjectWidth(const uint32_t id) override
	{ return 1; }

	float GetObjectHeight(const uint32_t id) override
	{ return 1; }

	// User implementation to retrieve appropriate graphics for objects
	olc::Pixel SelectObjectPixel(const uint32_t id, const float sample_x, const float sample_y, const float distance, const float angle) override
	{ return olc::BLACK; }

private:
	std::string sMap;
	olc::vi2d vWorldSize;
};



class RayCastWorldDemo_SIMPLE : public olc::PixelGameEngine
{
public:
	RayCastWorldDemo_SIMPLE()
	{
		sAppName = "RayCastWorld - SIMPLE";
	}

public:
	bool OnUserCreate() override
	{
		// Create game object
		pGame.reset(new ExampleGame(ScreenWidth(), ScreenHeight(), 3.14159f / 3.333f));

		// Add an object "player"
		std::shared_ptr<olc::rcw::Object> player = std::make_shared<olc::rcw::Object>();
		player->pos = { 2.1f, 2.1f };
		player->bVisible = false;

		// Insert into game world
		pGame->mapObjects.insert_or_assign(0, player);		
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Handle User Input =================================================
		auto& player = pGame->mapObjects[0];

		if (GetKey(olc::Key::A).bHeld) // Turn Left
			player->Turn(-fPlayerMoveSpeed * 0.1f * fElapsedTime);
		
		if (GetKey(olc::Key::D).bHeld) // Turn Right
			player->Turn(+fPlayerMoveSpeed * 0.1f * fElapsedTime);

		// Reset speed and velocity so player doesnt move if keys are not pressed
		player->Stop();

		// Walk Forward
		if (GetKey(olc::Key::W).bHeld) player->Walk(+fPlayerMoveSpeed);
		// Walk Backwards
		if (GetKey(olc::Key::S).bHeld) player->Walk(-fPlayerMoveSpeed);
		// Strafe Right
		if (GetKey(olc::Key::E).bHeld) player->Strafe(+fPlayerMoveSpeed);
		// Strafe Left
		if (GetKey(olc::Key::Q).bHeld) player->Strafe(-fPlayerMoveSpeed);

		// Update & Render World ==================================================

		// Update ray cast world engine - this will move the player object
		pGame->Update(fElapsedTime);

		// Link the camera to the play position
		pGame->SetCamera(player->pos, player->fHeading);

		// Render the scene!
		pGame->Render();

		// Draw the players position, cos why not?
		DrawString({ 10,10 }, player->pos.str(), olc::BLACK);
		return true;
	}

private:
	float fPlayerMoveSpeed = 16.0f;
	std::unique_ptr<ExampleGame> pGame = nullptr;
};

int main()
{
	RayCastWorldDemo_SIMPLE demo;
	if (demo.Construct(160, 120, 4, 4))
		demo.Start();
	return 0;
}