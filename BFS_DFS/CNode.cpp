#include "CNode.h"
#include <iostream>

CNode::CNode(char _name, sf::Vector2f _pos, sf::Font _font)
{
	name = _name;

	if (!texture.loadFromFile("Sprites/Node.png")) std::cout << "Failed to load Node Sprite\n";
	sprite->setPosition(_pos);
	sprite->setTexture(texture);

	text->setString(name);
	text->setFillColor(sf::Color::White);
	text->setCharacterSize(50);
	text->setFont(_font);

	text->setPosition(sprite->getPosition().x + text->getGlobalBounds().width / 2, sprite->getPosition().y);
}
