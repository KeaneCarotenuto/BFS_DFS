#pragma once
#include <SFML/Graphics.hpp>
#include "CButton.h"
#include "CNode.h"

class CManager
{
public:
	sf::RenderWindow* window = nullptr;
	sf::RenderWindow* controlWindow = nullptr;

	sf::Font font;

	std::vector<sf::Drawable*> ToDrawList;
	std::vector<CButton*> Buttons;

	std::vector<CNode*> nodes;

	sf::VertexArray* templine = new sf::VertexArray(sf::LineStrip, 2);

	CNode* connecting = nullptr;

	sf::Clock clock;
	int slowed = 1;
	float step = (1.0f / 60.0f);
	int currentStep = 0;

	bool frozenClick = false;
	bool frozenPlace = false;
};

