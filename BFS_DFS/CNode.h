#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class CNode
{
public:
	CNode(std::string _name, sf::Vector2f _pos, sf::Font _font);

	sf::Sprite* sprite = new sf::Sprite;
	sf::Texture texture;
	sf::Text* text = new sf::Text;

	std::vector <sf::VertexArray*> lines;

	std::string name;

	std::vector<CNode*> goesTo;
};