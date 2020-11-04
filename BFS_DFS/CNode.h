//A node class

#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class CNode
{
public:
	CNode(char _name, sf::Vector2f _pos, sf::Font _font);

	sf::Sprite* sprite = new sf::Sprite;
	sf::Texture texture;
	sf::Text* text = new sf::Text;

	std::vector <sf::VertexArray*> lines;

	char name;

	std::vector<CNode*> goesTo;

	static bool IsSmaller(const CNode* _Node, const CNode* _Node2)
	{
		return (_Node->name < _Node2->name);
	}
};