#include <iostream>
#include <Windows.h>
#include <vector>
#include <math.h>
#include <chrono>

#define PI 3.14159265359

using namespace std;

struct coords {
	float x, y, z;
};

struct viewAngle {
	float x, y;
};

coords playerLocation(HANDLE pHandle, uintptr_t player) {
	coords Coords;
	uintptr_t playerCoords = player + 0x4;
	ReadProcessMemory(pHandle, (LPVOID)playerCoords, &Coords, sizeof(Coords), NULL);
	return Coords;
}

coords entityLocation(HANDLE pHandle, uintptr_t entityList, int entityOffset) {
	coords Coords;
	entityList += entityOffset;
	uintptr_t entity;
	ReadProcessMemory(pHandle, (LPVOID)entityList, &entity, sizeof(entity), NULL);
	uintptr_t entityCoords = entity + 0x4;
	ReadProcessMemory(pHandle, (LPVOID)entityCoords, &Coords, sizeof(Coords), NULL);
	return Coords;
}

int entityHealth(HANDLE pHandle, uintptr_t entityList, int entityOffset) {
	int healthValue;
	entityList += entityOffset;
	uintptr_t entity;
	ReadProcessMemory(pHandle, (LPVOID)entityList, &entity, sizeof(entity), NULL);
	uintptr_t health = entity + 0xEC;
	ReadProcessMemory(pHandle, (LPVOID)health, &healthValue, sizeof(healthValue), NULL);
	return healthValue;
}

coords relativeCoords(HANDLE pHandle, uintptr_t player, uintptr_t entityList ,int target) {
	coords playerCoords;
	coords entityTestCoords;
	playerCoords = playerLocation(pHandle, player);
	entityTestCoords = entityLocation(pHandle, entityList, target);
	entityTestCoords.x = entityTestCoords.x - playerCoords.x;
	entityTestCoords.y = entityTestCoords.y - playerCoords.y;
	entityTestCoords.z = entityTestCoords.z - playerCoords.z;
	return entityTestCoords;
}

viewAngle getAngle(HANDLE pHandle, uintptr_t player, uintptr_t entityList, int target) {
	viewAngle playerViewAngleValue;
	coords entityTestCoords = relativeCoords(pHandle, player, entityList, target);
	playerViewAngleValue.x = (atan2(entityTestCoords.y, entityTestCoords.x) * 180) / PI;
	playerViewAngleValue.x += 90;
	if (playerViewAngleValue.x <= -1) {
		playerViewAngleValue.x += 360;
	}
	playerViewAngleValue.y = ((acos(entityTestCoords.z / sqrt(pow(entityTestCoords.x, 2) + pow(entityTestCoords.y, 2) + pow(entityTestCoords.z, 2)))) * 180) / PI;
	playerViewAngleValue.y -= 90;
	playerViewAngleValue.y *= -1;
	return playerViewAngleValue;
}

int main()
{
	int entityAmmount;
	std::cout << "including yourself how many entities are there: ";
	std::cin >> entityAmmount;
	entityAmmount *= 4;
	//setting everything up
	HWND hWnd = FindWindowA(NULL, "AssaultCube");
	DWORD pID = 0;
	GetWindowThreadProcessId(hWnd, &pID);
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, false, pID);
	//getting address of player
	uintptr_t playerBase = 0x0058AC00;
	uintptr_t player;
	ReadProcessMemory(pHandle, (LPVOID)playerBase, &player, sizeof(player), NULL);
	//playerViewAngle
	uintptr_t playerViewAngle = player + 0x34;
	viewAngle currentPlayerViewAngleValue;
	//entityList
	uintptr_t entityListBase = 0x0058AC04;
	uintptr_t entityList;
	ReadProcessMemory(pHandle, (LPVOID)entityListBase, &entityList, sizeof(entityList), NULL);
	//more variables
	int maxEntityOffset;
	int healthOfEntity;
	vector<int> offsetsOfAliveEntities;
	float delta = 0;
	float testDelta;
	coords entityTestCoords;
	viewAngle playerViewAngleValue;
	int target;
	int firstRun;
	while (true) {
		maxEntityOffset = 0;
		offsetsOfAliveEntities.clear();
		firstRun = 0;
		ReadProcessMemory(pHandle, (LPVOID)playerViewAngle, &currentPlayerViewAngleValue, sizeof(currentPlayerViewAngleValue), NULL);
		for (int i = 4; i < entityAmmount; i += 4) {
			healthOfEntity = entityHealth(pHandle, entityList, i);
			if (1 <= healthOfEntity && healthOfEntity <= 100)
			{
				std::cout << healthOfEntity << " ";
				offsetsOfAliveEntities.push_back(i);
			}
		}
		std::cout << "\n";
		for (int j : offsetsOfAliveEntities) {
			healthOfEntity = entityHealth(pHandle, entityList, j);
			if (0 < healthOfEntity) {
				playerViewAngleValue = getAngle(pHandle, player, entityList, j);
				playerViewAngleValue.x = playerViewAngleValue.x - currentPlayerViewAngleValue.x;
				playerViewAngleValue.y = playerViewAngleValue.y - currentPlayerViewAngleValue.y;
				//testDelta = sqrt(pow((playerViewAngleValue.x), 2) + pow((playerViewAngleValue.y), 2));
				testDelta = (sqrt(pow((playerViewAngleValue.x), 2) + pow((playerViewAngleValue.y), 2)) );
				if (firstRun == 0) {
					delta = testDelta;
					target = j;
					firstRun = 1;
				}
				if (testDelta < delta) {
					delta = testDelta;
					target = j;
				}
			}
		}
		playerViewAngleValue = getAngle(pHandle, player, entityList, target);
		healthOfEntity = entityHealth(pHandle, entityList, target);
		if (GetKeyState(VK_CONTROL) & 0x8000)
		{
			if (0 < healthOfEntity) {
				WriteProcessMemory(pHandle, (LPVOID)playerViewAngle, &playerViewAngleValue, sizeof(playerViewAngleValue), NULL);
			}
		}
	}
}

//todo (doubt ill get to this because it works and this was mainly just an exercise to learn stuff rather than making a good cheat)
//need to check if someone is behind wall
//move towards rather than snapping to
//factor distance into targeting calculation 
//ask if you are doing free for all or teams to figure out who to aim at
