#include <iostream>
#include <random>
#include <vector>

#define OLC_PGE_APPLICATION
#include "../olcPixelGameEngine.h"

class CollisionGame : public olc::PixelGameEngine
{
private:
	std::default_random_engine randEngine;

	struct Vec2
	{
		float x, y;
	};

	struct Polygon
	{
		std::vector<Vec2> points; // Transformed points (using model as basis and moving to position, rotating by angle
		Vec2 position = {0.0f, 0.0f};
		float angle = 0.0f;
		std::vector<Vec2> model;
		bool overlap = false; // Is overlapping another shape and needs to be resolved
	};

	std::vector<Polygon> shapes;

	int nMode = 0;

	olc::Pixel getRandomPixel(void)
	{
		std::uniform_int_distribution<> dist(0, 255);
		olc::Pixel p(dist(randEngine), dist(randEngine), dist(randEngine));
		return p;
	}

	bool shapeOverlapSAT(Polygon &s1, Polygon &s2)
	{
		Polygon *poly1 = &s1;
		Polygon *poly2 = &s2;

		for (int shape = 0; shape < 2; ++shape)
		{
			if (shape == 1)
			{
				poly1 = &s2;
				poly2 = &s1;
			}

			for (int a = 0; a < poly1->points.size(); ++a)
			{
				int b = (a + 1) % poly1->points.size();
				Vec2 axisProj = {-(poly1->points[b].y - poly1->points[a].y), (poly1->points[b].x - poly1->points[a].x)};
				float min_r1 = INFINITY, max_r1 = -INFINITY;

				for (int p = 0; p < poly1->points.size(); ++p)
				{
					float q = (poly1->points[p].x * axisProj.x + poly1->points[p].y * axisProj.y);
					min_r1 = std::min(min_r1, q);
					max_r1 = std::max(max_r1, q);
				}

				float min_r2 = INFINITY, max_r2 = -INFINITY;

				for (int p = 0; p < poly2->points.size(); ++p)
				{
					float q = (poly2->points[p].x * axisProj.x + poly2->points[p].y * axisProj.y);
					min_r2 = std::min(min_r2, q);
					max_r2 = std::max(max_r2, q);
				}

				if (!(max_r2 >= min_r1 && max_r1 >= min_r2))
					return false;
			}
		}

		return true;
	}

	bool shapeOverlapDIAG(Polygon &s1, Polygon &s2)
	{
		Polygon *poly1 = &s1;
		Polygon *poly2 = &s2;

		for (int shape = 0; shape < 2; ++shape)
		{
			if (shape == 1)
			{
				poly1 = &s2;
				poly2 = &s1;
			}

			for (int p = 0; p < poly1->points.size(); ++p)
			{
				Vec2 lineR1S = poly1->position;
				Vec2 lineR1E = poly1->points[p];
				for (int q = 0; q < poly2->points.size(); ++q)
				{
					Vec2 lineR2S = poly2->points[q];
					Vec2 lineR2E = poly2->points[(q + 1) % poly2->points.size()];

					float h = (lineR2E.x - lineR2S.x) * (lineR1S.y - lineR1E.y) - (lineR1S.x - lineR1E.x) * (lineR2E.y - lineR2S.y);
					float t1 = ((lineR2S.y - lineR2E.y) * (lineR1S.x - lineR2S.x) + (lineR2E.x - lineR2S.x) * (lineR1S.y - lineR2S.y)) / h;
					float t2 = ((lineR1S.y - lineR1E.y) * (lineR1S.x - lineR2S.x) + (lineR1E.x - lineR1S.x) * (lineR1S.y - lineR2S.y)) / h;

					if (t1 >= 0.0f && t1 < 1.0f && t2 >= 0.0f && t2 < 1.0f)
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	bool shapeOverlapDIAG_STATICRES(Polygon &s1, Polygon &s2)
	{
		Polygon *poly1 = &s1;
		Polygon *poly2 = &s2;

		for (int shape = 0; shape < 2; ++shape)
		{
			if (shape == 1)
			{
				poly1 = &s2;
				poly2 = &s1;
			}

			for (int p = 0; p < poly1->points.size(); ++p)
			{
				Vec2 lineR1S = poly1->position;
				Vec2 lineR1E = poly1->points[p];
				Vec2 displacement = {0.0f, 0.0f};
				for (int q = 0; q < poly2->points.size(); ++q)
				{
					Vec2 lineR2S = poly2->points[q];
					Vec2 lineR2E = poly2->points[(q + 1) % poly2->points.size()];

					float h = (lineR2E.x - lineR2S.x) * (lineR1S.y - lineR1E.y) - (lineR1S.x - lineR1E.x) * (lineR2E.y - lineR2S.y);
					float t1 = ((lineR2S.y - lineR2E.y) * (lineR1S.x - lineR2S.x) + (lineR2E.x - lineR2S.x) * (lineR1S.y - lineR2S.y)) / h;
					float t2 = ((lineR1S.y - lineR1E.y) * (lineR1S.x - lineR2S.x) + (lineR1E.x - lineR1S.x) * (lineR1S.y - lineR2S.y)) / h;

					if (t1 >= 0.0f && t1 < 1.0f && t2 >= 0.0f && t2 < 1.0f)
					{
						displacement.x += (1.0f - t1) * (lineR1E.x - lineR1S.x);
						displacement.y += (1.0f - t1) * (lineR1E.y - lineR1S.y);
					}
				}

				s1.position.x += displacement.x * (shape == 0 ? -1 : 1);
				s1.position.y += displacement.y * (shape == 0 ? -1 : 1);
			}
		}

		return false;
	}

public:
	CollisionGame(void)
	{
		sAppName = "Test...";
	}

	bool OnUserCreate(void) override
	{
		Polygon pentagon;
		float theta = 3.14159265f * 2.0f / 5.0f;
		pentagon.position = {100.0f, 100.0f};
		pentagon.angle = 0.0f;
		for (int i = 0; i < 5; ++i)
		{
			Vec2 p = {30.0f * cosf(theta * i), 30.0f * sinf(theta * i)};
			pentagon.model.push_back(p);
			pentagon.points.push_back(p);
		}

		Polygon triangle;
		theta = 3.14159265f * 2.0f / 3.0f;
		triangle.position = {200.0f, 150.0f};
		triangle.angle = 0.0f;
		for (int i = 0; i < 3; ++i)
		{
			Vec2 p = {20.0f * cosf(theta * i), 20.0f * sinf(theta * i)};
			triangle.model.push_back(p);
			triangle.points.push_back(p);
		}

		Polygon quad;
		quad.position = {120.0f, 50.0f};
		quad.angle = 0.0f;
		quad.model.push_back({-30.0f, -30.0f});
		quad.model.push_back({-30.0f, +10.0f});
		quad.model.push_back({+30.0f, +30.0f});
		quad.model.push_back({+30.0f, -30.0f});
		quad.points.resize(4);

		shapes.push_back(triangle);
		shapes.push_back(pentagon);
		shapes.push_back(quad);

		return true;
	}

	bool OnUserUpdate(float elapsedTime) override
	{
		if (GetKey(olc::Key::LEFT).bHeld)
			shapes[0].angle -= 2.0f * elapsedTime;
		if (GetKey(olc::Key::RIGHT).bHeld)
			shapes[0].angle += 2.0f * elapsedTime;

		if (GetKey(olc::Key::UP).bHeld)
		{
			shapes[0].position.x += cosf(shapes[0].angle) * 60.0f * elapsedTime;
			shapes[0].position.y += sinf(shapes[0].angle) * 60.0f * elapsedTime;
		}

		if (GetKey(olc::Key::DOWN).bHeld)
		{
			shapes[0].position.x -= cosf(shapes[0].angle) * 60.0f * elapsedTime;
			shapes[0].position.y -= sinf(shapes[0].angle) * 60.0f * elapsedTime;
		}

		if (GetKey(olc::Key::A).bHeld)
			shapes[1].angle -= 2.0f * elapsedTime;
		if (GetKey(olc::Key::D).bHeld)
			shapes[1].angle += 2.0f * elapsedTime;

		if (GetKey(olc::Key::W).bHeld)
		{
			shapes[1].position.x += cosf(shapes[1].angle) * 60.0f * elapsedTime;
			shapes[1].position.y += sinf(shapes[1].angle) * 60.0f * elapsedTime;
		}

		if (GetKey(olc::Key::S).bHeld)
		{
			shapes[1].position.x -= cosf(shapes[1].angle) * 60.0f * elapsedTime;
			shapes[1].position.y -= sinf(shapes[1].angle) * 60.0f * elapsedTime;
		}

		for (auto &s : shapes)
		{
			for (int i = 0; i < s.model.size(); ++i)
			{
				s.points[i] = {
					s.model[i].x * cosf(s.angle) - (s.model[i].y * sinf(s.angle)) + s.position.x,
					s.model[i].x * sinf(s.angle) + (s.model[i].y * cosf(s.angle)) + s.position.y};
			}

			s.overlap = false;
		}

		for (int m = 0; m < shapes.size(); ++m)
		{
			for (int n = m + 1; n < shapes.size(); ++n)
			{
				//				shapes[m].overlap |= shapeOverlapSAT(shapes[m], shapes[n]);
				//				shapes[m].overlap |= shapeOverlapDIAG(shapes[m], shapes[n]);
				shapes[m].overlap |= shapeOverlapDIAG_STATICRES(shapes[m], shapes[n]);
			}
		}

		Clear(olc::BLACK);

		for (auto &s : shapes)
		{
			for (int i = 0; i < s.points.size(); ++i)
			{
				DrawLine(s.points[i].x, s.points[i].y, s.points[(i + 1) % s.points.size()].x, s.points[(i + 1) % s.points.size()].y, (s.overlap ? olc::RED : olc::WHITE));
			}

			DrawLine(s.points[0].x, s.points[0].y, s.position.x, s.position.y, s.overlap ? olc::RED : olc::WHITE);
		}

		return true;
	}
};

int main(int argc, char *argv[])
{
	CollisionGame game;

	if (game.Construct(256, 240, 2, 2))
		game.Start();

	return 0;
}
