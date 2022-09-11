#pragma once
#include "Objects.h"
#include "Framework.h"
#include "PolyMath.h"
#include <string>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std;

#pragma region Assets
enum AssetType {
	AssetGameObject,
	AssetBlock,
	AssetMap
};

class Asset {
public:
	fs::path path;
};
class GameObjectAsset : public Asset {
public:
	fs::path idleSprite;
};
class BlockAsset : public GameObjectAsset {
public:
	fs::path brokenSprite;
	int id;
	int health;
	BlockAsset();
};
#pragma endregion
#pragma region Objects
class GameObject {
private:
	void UpdateBoundingBox();
public:
	Sprite* currentSprite;
	Sprite* idleSprite;
	Vector2 scale;
	Vector2 position;
	BoundingBox boundingBox;
	GameObject() {

	}
	GameObject(Vector2 Position, Vector2 Scale, Sprite* IdleSprite) {
		idleSprite = IdleSprite;
		currentSprite = idleSprite;
		position = Position;
		scale = Scale;
		UpdateBoundingBox();

	}
	void Scale(int x, int y);
	void SetScale(int x, int y);
	void Move(int x, int y);
	void SetPosition(int x, int y);
	bool IsColliding(GameObject otherObject);
};
class Block : public GameObject {
private:
	int id;
	int health;
	Sprite* brokenSprite;
public:
	bool Hurt();
	Block() {

	}
	Block(Vector2 Position, Vector2 Scale, Sprite* IdleSprite, Sprite* BrokenSprite, int Id, int Health) : GameObject(Position, Scale, IdleSprite) {
		brokenSprite = BrokenSprite;
		id = Id;
		health = Health;
	}
};
class PhysicsObject : public GameObject {
protected:
	BoundingBox canvas;
public:
	Vector2 currentVelocity;
	bool CollidesBorder(BoundingBox borders) {
		bool result = false;
		if (boundingBox.a.x <= borders.a.x || boundingBox.b.x >= borders.b.x) {
			InvertVelocity(true, false);
			result = true;
		}
		if (boundingBox.a.y <= borders.b.y || boundingBox.c.y >= borders.d.y) {
			InvertVelocity(false, true);
			result = true;
		}
		return result;
	}
	bool CollidesWith(GameObject otherObject) {
		BoundingBox a = boundingBox;
		BoundingBox b = otherObject.boundingBox;

		return a.Intersects(b);
	}
	void InvertVelocity(bool X, bool Y) {
		currentVelocity.x *= (X == true ? -1 : 1);
		currentVelocity.y *= (Y == true ? -1 : 1);
	}
	void Tick();
};
#pragma endregion

class Bullet : public PhysicsObject {
public:
	Bullet() {

	}
	Bullet(Vector2 Position, Vector2 Scale, Vector2 Velocity, Sprite* Sprite) {
		position = Position;
		scale = Scale;
		currentVelocity = Velocity;
		idleSprite = Sprite;
		currentSprite = idleSprite;
	}
	void Tick();
};