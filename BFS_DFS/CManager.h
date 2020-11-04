//The manager that takes care of vars

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

	std::vector<CNode*> searchList;
	std::vector<CNode*> doneList;

	sf::VertexArray* templine = new sf::VertexArray(sf::LineStrip, 2);

	CNode* connecting = nullptr;

	std::vector<sf::VertexArray*> doneLines;

	sf::Clock clock;
	int slowed = 1;
	float step = (1.0f / 60.0f);
	int currentStep = 0;

	bool search = false;
	bool found = false;
	bool bfs = true;

	CNode* target;

	bool frozenClick = false;
	bool frozenPlace = false;
};

