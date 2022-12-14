

#include "Framework.h"
#include "PolyMath.h"
#include <iostream>
#include "Objects.h"
#include "Assets.h"

using namespace std;

class Game : public Framework {
private:
	AssetManager assetManager;

	Player player;
	list<Bullet> bullets;
	list<Block> blocks;
	list<Ability> abilities;

	BoundingBox borderArea;
	BoundingBox mapArea;


	VisualObject bulletsUI[3];
	VisualObject border;
	VisualObject farlands;
	VisualObject logo;
	VisualObject background;

	Vector2 blockSize;
	Vector2 abilitySize;
	Vector2 bulletSize;
	Vector2 playerSize;

	float abilitySpawnDelay;
	float abilitySpawnCooldown;

	unsigned int score;
	int mapIndex;
	int blocksToDestroy;

	int moveInput;
	Vector2 mousePos;

	Timer abilityTimer;
	bool canAbilitySpawn;

	Timer shootCooldown;
	bool canShoot;
	int bulletsAmount;
	int bulletsOnMap;

	int deltaTime;
	clock_t previousTickTime;
	clock_t currentTickTime;

#pragma region Important
	void GameTick() {
		abilityTimer.Tick(deltaTime);
		shootCooldown.Tick(deltaTime);

		if (canAbilitySpawn && bulletsAmount > 0) {
			InstantiateAbility();
		}

		PhysTick();
		DestroyDeadmen();

		if (blocksToDestroy <= 0 && player.catchedBullet) {
			NextMap();
		}
	}
	void PhysTick() {
		player.UpdateInput(moveInput);

		player.Tick(deltaTime);

		for (auto& a : abilities) a.Tick(player);
		for (auto& b : bullets) b.Tick(blocks, player);
	}
	void Shoot() {
		if (bulletsAmount <= 0) {
			Restart();
		}
		if (canShoot && bulletsOnMap < bulletsAmount) {
			shootCooldown.Restart();
			bulletsOnMap++;

			Vector2 velocity = Vector2(2, -2);

			int mX = mousePos.x;
			int pX = player.position.x + (player.scale.x / 2);

			bool isCenter = std::abs(pX - mX) < (int)(player.scale.x * 0.66);

			switch (isCenter)
			{
			case true:
				velocity.x = 0;
				velocity.y = -3;
				break;
			case false:
				velocity.x = mX < pX ? -velocity.x : velocity.x;
				break;
			}

			Vector2 pos = Vector2(player.position.x + (playerSize.x / 2) - (bulletSize.x / 2), player.position.y - (bulletSize.y));
			InstantiateBullet(pos, velocity);
		}
	}
	void DestroyDeadmen() {

		//HERE COMES THE HARDCODE BABY

		std::list<Block>::iterator bi = blocks.begin();

		list<Block>::const_iterator bitr = blocks.cbegin();
		while (bitr != blocks.cend())
		{
			list<Block>::const_iterator curr = bitr++;
			if (curr->IsAlive == false) {
				destroySprite(curr->idleSprite);
				score += 150 * (mapArea.MaxY() / curr->position.y);
				blocksToDestroy--;

				blocks.erase(curr);
			}
		}

		std::list<Bullet>::iterator i = bullets.begin();

		list<Bullet>::const_iterator itr = bullets.cbegin();
		while (itr != bullets.cend())
		{
			list<Bullet>::const_iterator curr = itr++;
			if (curr->IsAlive == false) {
				destroySprite(curr->idleSprite);
				bulletsAmount--;
				bulletsOnMap--;
				if (bulletsAmount <= 0 && mapIndex <= 4) {
					cout << "\n\nYOU LOST\nPRESS LBM TO RESTART THE GAME\n\n";
				}
				bullets.erase(curr);
			}
		}

		list<Ability>::const_iterator aitr = abilities.cbegin();
		while (aitr != abilities.cend())
		{
			list<Ability>::const_iterator curr = aitr++;
			if (curr->IsAlive == false) {
				destroySprite(curr->idleSprite);
				abilities.erase(curr);
			}
		}
	}
#pragma endregion
#pragma region Drawing
	Sprite* GetSprite(fs::path Path) {
		return createSprite(Path.string().c_str());
	}
	void DrawVisualObject(VisualObject& object) {
		if (object.sprite == nullptr) {
			return;
		}
		setSpriteSize(object.sprite, object.scale.x, object.scale.y);
		drawSprite(object.sprite, object.position.x, object.position.y);
	}
	void DrawGameObject(GameObject& object) {
		if (object.currentSprite == nullptr) {
			return;
		}
		setSpriteSize(object.currentSprite, object.scale.x, object.scale.y);
		drawSprite(object.currentSprite, object.position.x, object.position.y);
	}
	void Draw() {
		drawTestBackground(); // DONT TOUCH IT! I INSIST!
		DrawVisualObject(background);

		for each (Block block in blocks) DrawGameObject(block);
		for each (Ability ability in abilities) DrawGameObject(ability);
		for each (Bullet bullet in bullets) DrawGameObject(bullet);

		DrawGameObject(player);
		DrawOverlay();
	}
	void DrawOverlay() {
		if (farlands.sprite != nullptr) {
			DrawVisualObject(farlands);
			DrawVisualObject(logo);
		}
		DrawVisualObject(border);
	}
#pragma endregion
#pragma region Setup
	void SetupOverlay() {
		border = VisualObject();
		border.sprite = GetSprite(assetManager.border.sprite);

		if (cmdScreen.x != cmdScreen.y) {
			logo = VisualObject();
			farlands = VisualObject();

			logo.sprite = GetSprite(assetManager.logo.sprite);
			farlands.sprite = GetSprite(assetManager.farlands.sprite);

			int farlandsScale = 1;
			int logoScale = 10;

			switch (cmdScreen.x > cmdScreen.y) {
			case true:
				farlandsScale = std::min(cmdScreen.x, cmdScreen.y);
				logoScale = cmdScreen.x - borderArea.MaxX();
				farlands.SetPosition(borderArea.b.x, borderArea.b.y);
				logo.SetPosition(borderArea.MaxX(), borderArea.MaxY() * 0.5 - (logoScale * 0.5));
				break;
			case false:
				farlandsScale = std::max(cmdScreen.x, cmdScreen.y);
				logoScale = cmdScreen.y - borderArea.MaxY();
				farlands.SetPosition(borderArea.c.x, borderArea.c.y);
				logo.SetPosition(borderArea.MaxX() * 0.5 - (logoScale * 0.5), borderArea.MaxY());
				break;
			}
			logo.SetScale(logoScale, logoScale);
			farlands.SetScale(farlandsScale, farlandsScale);
		}

		border.SetScale(borderArea.MaxX(), borderArea.MaxY());
	}
	void SetupVariables() {
		score = 0;
		blocksToDestroy = 0;
		mapIndex = -1;
		bulletsAmount = 3;
		bulletsOnMap = 0;

		moveInput = 0;

		canAbilitySpawn = false;
		canShoot = false;

		abilityTimer.Setup(&canAbilitySpawn, 20000, true);
		shootCooldown.Setup(&canShoot, 1000, false);

		int squareWidth = std::min(cmdScreen.x, cmdScreen.y);
		borderArea = BoundingBox(Vector2(0, 0), Vector2(squareWidth, 0), Vector2(0, squareWidth), Vector2(squareWidth, squareWidth));
		mapArea = borderArea;

		mapArea.b.x *= 0.955;
		mapArea.d.x *= 0.955;
		mapArea.a.x = borderArea.b.x - mapArea.b.x;
		mapArea.c.x = mapArea.a.x;

		mapArea.a.y = mapArea.a.x;
		mapArea.b.y = mapArea.a.y;

		background = VisualObject();
		background.SetPosition(mapArea.MinX(), mapArea.MinY());
		background.SetScale(mapArea.MaxX(), mapArea.MaxY());

		blockSize = Vector2((mapArea.MaxX() - mapArea.MinX()) / 8, (mapArea.MaxY() - mapArea.MinX()) / 16);
		abilitySize = Vector2(blockSize.x, blockSize.x);
		bulletSize = Vector2(blockSize.y, blockSize.y);
		playerSize = Vector2(blockSize.x * 2, blockSize.y);
	}
#pragma endregion
#pragma region Map
	void InstantiateMap(int& index) {
		MapAsset& map = assetManager.maps[index];

		background.sprite = GetSprite(map.background);

		blocksToDestroy = 0;
		bulletsOnMap = 0;
		
		cout << "BULLETS IN MAGAZINE: " << bulletsAmount << '\n';
		int value = 0;
		for (int x = 0; x < 16; x++) {
			for (int y = 0; y < 8; y++) {
				value = map.terrain[x][y];
				if (value != 0) {
					BlockAsset& asset = assetManager.blocks[value - 1];
					if (asset.health < 100) {
						blocksToDestroy++;
					}
					InstantiateBlock(asset, y, x);
				}
			}
		}

		abilityTimer.Restart();
		shootCooldown.Restart();
		InstantiatePlayer();

	}
	void DestroyMap() {
		bullets.clear();
		blocks.clear();
		abilities.clear();
	}
	void NextMap() {
		mapIndex++;

		if (mapIndex > 4) {
			background.sprite = GetSprite(assetManager.ending.sprite);
			blocks.clear();
			return;
		}
		switch (mapIndex)
		{
		case 0:
			cout << "\n\n" << "WELCOME TO THE POLYBROID" << "\n\n" << "LEFT ARROW & RIGHT ARROW - MOVE" << "\n" << "MOUSE POINTER - AIM" << "\n" << "LMB - SHOOT" << "\n\n";
		default:
			cout << "\n" << "YOUR CURRENT SCORE IS " << score << " POINTS" << '\n';
			DestroyMap();
			InstantiateMap(mapIndex);
			break;
		}
	}
	void Restart() {
		bulletsAmount = 3;
		bulletsOnMap = 0;
		mapIndex = -1;
		score = 0;
		NextMap();
	}
#pragma endregion
#pragma region Spawning
	void InstantiatePlayer() {
		GameObjectAsset& asset = assetManager.player;
		Vector2 position = Vector2(mapArea.MaxX() / 2 - (playerSize.x/2), mapArea.MaxY() * 0.95 - (playerSize.y));
		Sprite* sprite = GetSprite(asset.idleSprite);
		Player pl = Player(mapArea, position, playerSize, Vector2(4, 0), sprite);
		player = pl;
	}
	void InstantiateBlock(BlockAsset& asset, int x, int y) {
		Vector2 position = Vector2(mapArea.MinX() + x * blockSize.x, mapArea.MinY() + y * blockSize.y);
		Sprite* idleSprite = GetSprite(asset.idleSprite);
		Sprite* brokenSprite = GetSprite(asset.brokenSprite);
		Block block = Block(position, blockSize, idleSprite, brokenSprite, asset.id, asset.health);
		blocks.push_front(block);
	}
	void InstantiateBullet(Vector2 Position, Vector2 Velocity) {
		GameObjectAsset& asset = assetManager.bullet;
		Sprite* idleSprite = GetSprite(asset.idleSprite);
		Bullet bullet = Bullet(mapArea, Position, bulletSize, Velocity, Vector2(2, -2), idleSprite);
		bullets.push_front(bullet);
	}
	void InstantiateAbility() {
		GameObjectAsset& asset = assetManager.blocks[0];
		int pseudoRand = rand() % 3 - 3;
		bool IsPositive = (rand() % 10) >= 6 ? true : false;
		Vector2 position = Vector2(mapArea.MaxX()/2 - abilitySize.x/2, mapArea.MinY()-abilitySize.y);

		Sprite* Sprite;
		switch (IsPositive)
		{
		case true:
			Sprite = GetSprite(assetManager.positiveAbility.idleSprite);
			break;
		case false:
			Sprite = GetSprite(assetManager.negativeAbility.idleSprite);
			break;
		}
		Ability ability = Ability(mapArea, position, abilitySize, Vector2(2/pseudoRand, 2), IsPositive, Sprite);
		
		abilities.push_front(ability);
		//currentAbility = ability;
	}
#pragma endregion

public:
	Vector2 cmdScreen;
#pragma region Important
	virtual void PreInit(int& width, int& height, bool& fullscreen)
	{
		width = cmdScreen.x;
		height = cmdScreen.y;
		fullscreen = false;
	}
	virtual bool Init() {
		assetManager.Setup();

		SetupVariables();
		SetupOverlay();
		NextMap();

		previousTickTime = clock();
		return true;
	}
	virtual void Close() {

	}
	virtual bool Tick() {
		currentTickTime = clock();
		deltaTime = (int)(currentTickTime - previousTickTime);

		GameTick();
		Draw();
		
		previousTickTime = currentTickTime;
		return false;
	}
#pragma endregion
#pragma region Input
	virtual void onMouseMove(int x, int y, int xrelative, int yrelative) {
		mousePos = Vector2(x, y);
	}
	virtual void onMouseButtonClick(FRMouseButton button, bool isReleased) {
		switch (button)
		{
		case FRMouseButton::LEFT:
			Shoot();
			break;
		default:
			break;
		}
	}
	virtual void onKeyPressed(FRKey k) {
		switch (k)
		{
		case FRKey::RIGHT:
			moveInput = 1;
			break;
		case FRKey::LEFT:
			moveInput = -1;
			break;
		default:
			break;
		}
	}
	virtual void onKeyReleased(FRKey k) {
		switch (k)
		{
		case FRKey::RIGHT:
			moveInput = moveInput > 0 ? 0 : moveInput;
			return;
		case FRKey::LEFT:
			moveInput = moveInput < 0 ? 0 : moveInput;
			return;
		default:
			return;
		}
	}
#pragma endregion
	
	virtual const char* GetTitle() override
	{
		return "Polybroid";
	}
	Game() {
		assetManager = AssetManager();

		bullets = list<Bullet>();
		blocks = list<Block>();

		abilityTimer = Timer();
		shootCooldown = Timer();
	}
};
int main(int argc, char* argv[])
{
	Game* framework = new Game();

	Vector2 defaultScreenSize = Vector2(600, 600);
	Vector2 screenSize = defaultScreenSize;
	string command;

	switch (argc)
	{
	case 1:
		break;
	case 4:
		command = argv[1];
		if (command == "-window") {
			try {
				screenSize = Vector2(stoi(argv[2]), stoi(argv[3]));
				break;
			}
			catch (exception e) { cout << e.what(); }
		}
	default:
		cout << "Wrong command line syntax. Usage: \"polybroid.exe -window <width> <height>\"" << endl;
		break;

	}
	if (screenSize.x < 400) screenSize.x = 400;
	if (screenSize.y < 400) screenSize.y = 400;
	framework->cmdScreen = screenSize;

	return run(framework);
}