#pragma once

#include <string>
#include <vector>

struct FMapEntity {
	std::string type;
	float x;
	float y;
};

class FMap {
	public:
		FMap();
		~FMap();
	
		void Load(const std::string fileName);

		int mapWidth;
		int mapHeight;
		unsigned int **mapData;
		std::vector<FMapEntity> entities;
	
	private:
	
		bool ReadHeader(std::ifstream &stream);
		bool ReadLayerData(std::ifstream &stream);
		bool ReadEntityData(std::ifstream &stream);
	
};
