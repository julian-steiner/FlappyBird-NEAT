#include<iostream>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#define OLC_PGEX_ANIMSPR
#include "olcPGEX_AnimatedSprite.h"

#include "Pipe.h"
#include "Bird.h"
#include <string>
#include <chrono>
#include <vector>

#include "Population.h"
#include "Logger.h"

class FlappyBird : public olc::PixelGameEngine
{
private:
	int birdPosition = 200;
	double highScore;
	int collisionPipeIndex = 0;
	Pipe* collisionPipe = nullptr;
	Pipe* nearestPipe = nullptr;

	int numBirds = 100;
	std::vector<olc::AnimatedSprite> birdSprites;
	std::unique_ptr<olc::Sprite> pipeDownSpr;
	std::unique_ptr<olc::Sprite> pipeUpSpr;
	std::unique_ptr<olc::Decal> pipeDown;
	std::unique_ptr<olc::Decal> pipeUp;
	std::unique_ptr<olc::Sprite> backGroundSpr;
	std::unique_ptr<olc::Decal> backGround;

	logging::Logger* logger;
	bool logging = false;

private:
	void handlePipesCreation()
	{
		for (int i = 0; i < 5; i++)
		{
			pipeBuffer[i] = Pipe();
		}
		pipeBuffer[0].moving = true;
		collisionPipe = nullptr;
		nearestPipe = &pipeBuffer[0];
	}

	void handlePipesMovement(float fElapsedTime)
	{

		// Loop through all the pipes
		for (int i = 0; i < 5; i++)
		{
			Pipe& currentPipe = pipeBuffer[i];

			// Move the pipe forward
			if (currentPipe.moving)
			{	
				currentPipe.position += 300 * fElapsedTime;

				// Checks if the pipe could collide with the bird
				if (screenSize[0] - currentPipe.position < birdPosition && screenSize[0] - currentPipe.position + currentPipe.size > birdPosition && collisionPipe != &currentPipe)
				{
					collisionPipeIndex = i;
					collisionPipe = &currentPipe;
				}
			}

			// Add a new pipe to the ring buffer if it passes the border
			if ((int)currentPipe.position > 400 && !currentPipe.passed)
			{
				currentPipe.passed = true;
				int pipeNumber = (i + 1);
				if (pipeNumber > 4){
					pipeNumber = 0;
				}
				pipeBuffer[pipeNumber] = Pipe();
				pipeBuffer[pipeNumber].moving = true;
			}
		}

		// Disables collision checking if the pipe is over
		if (collisionPipe == nullptr) return;
		if (screenSize[0] - collisionPipe->position + collisionPipe->size < birdPosition)
		{
		 	collisionPipe = nullptr;

			// Determining the nearest pipe (next from the collision pipe)
			if (collisionPipeIndex < 4)
			{
				nearestPipe = &pipeBuffer[collisionPipeIndex+1];
			}

			else 
			{
				nearestPipe = &pipeBuffer[0];
			}
		}
	}

	void handleBirdMovement(float fTimeElapsed, bool clicked, Bird& bird)
	{
		if (!bird.alive) return;
		fTimeElapsed *= 2;
		// Physics for the movement
		bird.velocity += 800 * fTimeElapsed;
		bird.position += bird.velocity * fTimeElapsed;
		bird.timeSurvived += fTimeElapsed;

		// Boost at a click
		if (clicked && bird.clickTimer == 0)
		{
			bird.velocity = -400;
			bird.clickTimer = bird.clickCooldown;
		}
		if (bird.clickTimer > 0)	bird.clickTimer -= fTimeElapsed;
		else if (bird.clickTimer < 0) bird.clickTimer = 0;
	}

	void drawPipes()
	{
		for (Pipe& currentPipe : pipeBuffer)
		{
			int lowerPartHeight = (int)currentPipe.holeLocation + (int)currentPipe.holeSize;
			int upperPartHeight = (int)currentPipe.holeLocation;
			//FillRect({(int)screenSize[0] - (int)currentPipe.position, lowerPartHeight}, {currentPipe.width, screenSize[1] - lowerPartHeight}, olc::Pixel(0, 255, 0));
			//FillRect({(int)screenSize[0] - (int)currentPipe.position, 0}, {currentPipe.width, upperPartHeight}, olc::Pixel(0, 255, 0));
			DrawDecal({(float)screenSize[0] - (float)currentPipe.position, (float)upperPartHeight-(160 * currentPipe.width / 26.0f)}, pipeDown.get(), {currentPipe.width / (float)26, currentPipe.width / (float)26});
			DrawDecal({(float)screenSize[0] - (float)currentPipe.position, (float)lowerPartHeight}, pipeUp.get(), {currentPipe.width / (float)26, currentPipe.width / (float)26});
		}
	}

	void drawBird(const float& fElapsedTime)
	{
		//for (Bird& bird : birds)
		for (int i = 0; i < numBirds; i++)
		{
			Bird& bird = birds[i];
			if (!bird.alive) continue;

			//FillCircle({200, (int)bird.position}, bird.size, olc::Pixel(255, 120, 0));
			if (bird.velocity >= 0) birdSprites.at(i).SetState("down");
			else birdSprites.at(i).SetState("up");
			birdSprites.at(i).Draw(fElapsedTime, {200-26 * 1.5, (float)bird.position - (26 * 1.5f)});
		}
	}

	void handleCollisions()
	{
		//for (Bird& currentBird : birds)
		for (int i = 0; i < numBirds; i++)
		{
			Bird& currentBird = birds[i];
			if (!currentBird.alive) continue;
			if (currentBird.position > screenSize[1] || currentBird.position < 0)
			{
				currentBird.alive = false;
				return;
			}

			if (collisionPipe == nullptr) continue;
			if (currentBird.position - currentBird.size <= collisionPipe->holeLocation || currentBird.position + currentBird.size >= collisionPipe->holeLocation + collisionPipe->holeSize)
			{
				currentBird.alive = false;
			}
		}
	}

public:
	int generationCounter = 0; 

	void resetGame()
	{
		// Finding the fittest bird
		double highestFitness = 0;
		int highestBird = 0;
		for (int i = 0; i < numBirds; i++)
		{
			if (birds[i].timeSurvived > highestFitness)
			{
				highestFitness = birds[i].timeSurvived;
				highestBird = i;
			}
			birds[i] = Bird();
		}
		nn::NeuralNetwork highest = p->getNetwork(highestBird)->getCopy<nn::NeuralNetwork>();


		handlePipesCreation();

		p->speciate();
		p->crossover();
		p->mutate();

		std::cout << "Number Of Species: " << p->getNumberOfSpecies() << std::endl;
		std::cout << "Generation: " << generation << std::endl;
		if (generationCounter == 10)
		{
			std::cout << "Highest Fitness: " << highestFitness << std::endl;
			std::vector<std::string> structure = highest.getConnectionScheme();
			for (const std::string& str : structure)
			{
				std::cout << str << std::endl;
			}
			generationCounter = 0;
		}
		std::cout << std::endl;
		generation ++;
		generationCounter ++;

		collisionPipe = nullptr;
	}

public:
	//std::vector<Bird> birds = std::vector<Bird>();
	//Bird birds[10];
	std::vector<Bird> birds;
	double saveTimer;

	population::Population* p;
	Pipe pipeBuffer[5];
	int generation = 0;

	FlappyBird()
	{
		sAppName = "FlappyBird";
		p = new population::Population(numBirds, 3, 1);

		p->weightChangingRate = 0.3;
		p->connectionAddingRate = 0.3;
		p->neuronAddingRate = 0.01;
		p->learningRate = 0.2;

		p->targetNumberOfSpecies = 5;

		birds = std::vector<Bird>();
		birds.reserve(numBirds);

	}
	int screenSize[2] = {1280, 720};

	void attachLogger(logging::Logger* logger)
	{
		logging = true;
		this->logger = logger;
	}

public:
	bool OnUserCreate() override
	{
		birdSprites.reserve(numBirds);
		for (int i = 0; i < numBirds; i++)
		{
			birdSprites.push_back(olc::AnimatedSprite());
			olc::AnimatedSprite& currentSprite = birdSprites.at(i);
			// Loading Sprites
			currentSprite.type = olc::AnimatedSprite::SPRITE_TYPE::DECAL;
			currentSprite.mode = olc::AnimatedSprite::SPRITE_MODE::MULTI;
			currentSprite.SetSpriteSize({26, 26});
			currentSprite.SetSpriteScale(3);
			currentSprite.AddState("up", {
				"../src/bird/Bird_up1.png",
				"../src/bird/Bird_up2.png",
				"../src/bird/Bird_up3.png"
			});
			currentSprite.AddState("down", {
				"../src/bird/Bird_down1.png",
				"../src/bird/Bird_down2.png",
				"../src/bird/Bird_down3.png"
			});
		}
		pipeUpSpr = std::make_unique<olc::Sprite>("../src/pipe/Pipe_up.png");
		pipeDownSpr = std::make_unique<olc::Sprite>("../src/pipe/Pipe_down.png");
		pipeUp = std::make_unique<olc::Decal>(pipeUpSpr.get());
		pipeDown = std::make_unique<olc::Decal>(pipeDownSpr.get());
		backGroundSpr = std::make_unique<olc::Sprite>("../src/game/Background.png");
		backGround = std::make_unique<olc::Decal>(backGroundSpr.get());

		// Called once at the start, so create things here
		resetGame();
		handlePipesCreation();
		p->mutate();
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);
		DrawDecal({0, 0}, backGround.get(), {screenSize[1] / 256.0f, screenSize[1] / 256.0f});
		DrawDecal({144 * screenSize[1] / 256.0f, 0}, backGround.get(), {screenSize[1] / 256.0f, screenSize[1] / 256.0f});
		DrawDecal({288 * screenSize[1] / 256.0f, 0}, backGround.get(), {screenSize[1] / 256.0f, screenSize[1] / 256.0f});
		DrawDecal({432 * screenSize[1] / 256.0f, 0}, backGround.get(), {screenSize[1] / 256.0f, screenSize[1] / 256.0f});

		// called once per frame
		handlePipesMovement(fElapsedTime);
		for (int i = 0; i < numBirds; i++)
		{
			Bird& bird = birds[i];
			nn::NeuralNetwork* currentNetwork = p->getNetwork(i);
			//double action = currentNetwork->predict({bird.position, nearestPipe->holeLocation + nearestPipe->holeSize})[0];
			//bool action = currentNetwork->predict({nearestPipe->position - birdPosition, nearestPipe->holeLocation + nearestPipe->holeSize/2 - bird.position, bird.clickTimer})[0] >= 0.5;
			bool action = currentNetwork->predict({nearestPipe->position, nearestPipe->holeLocation + nearestPipe->holeSize/2, bird.position})[0] >= 0.5;
			//double action = GetMouse(0).bPressed;
			handleBirdMovement(fElapsedTime, (bool)action, bird);

			currentNetwork->fitness = bird.timeSurvived;
		}

		// Save the connection scheme of the fittest when the mouse key is pressed
		saveTimer += fElapsedTime;
		if (GetMouse(0).bHeld && saveTimer >= 2) 
		{
			saveTimer = 0;	

			double highestFitness = 0;
			int highestBird = 0;
			for (int i = 0; i < numBirds; i++)
			{
				if (birds[i].timeSurvived > highestFitness)
				{
					highestFitness = birds[i].timeSurvived;
					highestBird = i;
				}
			}

			nn::NeuralNetwork highest = p->getNetwork(highestBird)->getCopy<nn::NeuralNetwork>();
			highest.saveConnectionScheme("Highest.tex");
			std::cout << "Saved connection Scheme" << std::endl;

			for(int i = 0; i < highest.connections->size(); i++)
			{
				connection::Connection* connection = highest.connections->at(i);
				std::cout << "[" << connection->inNeuronLocation.layer << ", " << connection->inNeuronLocation.number << "] -> [" << connection->outNeuronLocation.layer << ", " << connection->outNeuronLocation.number << "] "<<  connection->weight << std::endl;
			}
		}

		double highestFitness = 0;
		int highestBird = 0;
		for (int i = 0; i < numBirds; i++)
		{
			if (birds[i].timeSurvived > highestFitness)
			{
				highestFitness = birds[i].timeSurvived;
				highestBird = i;
			}
		}

		nn::NeuralNetwork highest = p->getNetwork(highestBird)->getCopy<nn::NeuralNetwork>();
		if (highestFitness >= 120 || generation >= 200)
		{
			logger->addNetwork(highest, generation);
			return false;
		}

		handleCollisions();

		// Handle drawing of the pipes
		drawPipes();
		drawBird(fElapsedTime);

		std::stringstream scoreText;
		std::stringstream generationText;
		int birdCounter = 0;

		for (int i = 0; i < numBirds; i++)
		{
			const Bird& bird = birds[i];
			if (bird.alive)
			{
				birdCounter ++;
				highScore = bird.timeSurvived;
			}
		}

		if (birdCounter == 0)
		{ 
			resetGame();
			return true;
		}

		scoreText << "Score: " << highScore;
		generationText << "Generation: " << generation;
		DrawString({100, 100}, scoreText.str(), olc::Pixel(255, 255, 255), 2);
		DrawString({100, 120}, generationText.str(), olc::Pixel(255, 255, 255), 2);

		return true;
	}
};

int main()
{
	logging::Logger logger("Logfiles/GenerationsLog.tex");

	for (int i = 0; i < 30; i++)
	{
		FlappyBird flappyBird;
		flappyBird.attachLogger(&logger);
		if (flappyBird.Construct(flappyBird.screenSize[0], flappyBird.screenSize[1l], 1, 1))
			flappyBird.Start();

		std::cout << i << std::endl;
	}

	return 0;
}