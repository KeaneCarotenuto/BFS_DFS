#include <vector>
#include <string>
#include <iostream>
#include <time.h>
#include <math.h>
#include <typeinfo>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <Windows.h>

#include "CManager.h"
#include "CNode.h"

int FixedUpdate();
void CreateConnection(CNode* from, CNode* to);
void CreateLine(CNode* from, CNode* to);
void CheckButtonsPressed();
void Draw();

void CreateButton(void(*function)(), std::string _string, int _fontSize, sf::Color _tColour, sf::Text::Style _style, float _x, float _y, sf::Color _bgColour, float _padding);

CManager manager;

void Print() {
	std::cout << "Started\n";
	manager.search = true;
}

int main() {
	sf::RenderWindow window(sf::VideoMode(800, 800), "A* Pathfinding - By Keane Carotenuto");
	sf::RenderWindow controlWindow(sf::VideoMode(200, 200), "Controls");
	controlWindow.setPosition(sf::Vector2i(window.getPosition().x + window.getSize().x, window.getPosition().y));

	manager.window = &window;
	manager.controlWindow = &controlWindow;

	if (!manager.font.loadFromFile("Fonts/Roboto.ttf")) std::cout << "Failed to load Roboto\n";

	CreateButton(&Print, "Test", 25, sf::Color::White, sf::Text::Style::Bold, 0, 0, sf::Color::Color(0, 150, 0), 5);

	float stepTime = 0;
	bool drawn = false;


	while (window.isOpen() == true)
	{
		stepTime += manager.clock.getElapsedTime().asSeconds();
		manager.clock.restart();

		while (stepTime >= manager.step)
		{
			//Main Loop of Game
			if (FixedUpdate() == 0) return 0;

			stepTime -= manager.step;
			drawn = false;
		}

		//Draws After Updates
		if (drawn)
		{
			//sf::sleep(sf::seconds(0.01f));
		}
		else
		{
			Draw();

			drawn = true;
		}

		CheckButtonsPressed();

		sf::Event newEvent;

		//Quit
		while (window.pollEvent(newEvent))
		{
			if (newEvent.type == sf::Event::Closed)
			{
				window.close();
			}
		}
	}

	return 0;
}

int FixedUpdate()
{
	manager.currentStep++;
	manager.ToDrawList.clear();

	if (manager.search &&  !manager.found) {
		if (!manager.searchList.empty()) {
			//Get First Item in search
			CNode* currentNode = manager.searchList[0];

			//Push it to done list
			manager.doneList.push_back(currentNode);

			//Remove it from search list
			std::vector<CNode*>::iterator pos = std::find(manager.searchList.begin(), manager.searchList.end(), currentNode);
			if (pos != manager.searchList.end()) {
				manager.searchList.erase(pos);
			}

			//Check if current node is the target
			if (currentNode == manager.target) {
				manager.found = true; 
				goto found;
			}

			//Otherwise loop through connected nodes and add them to search list if valid
			for (CNode* _node : currentNode->goesTo) {
				bool canAdd = true;

				//If already searched or done, dont add
				for (CNode* tempnode : manager.searchList) {
					if (_node == tempnode) canAdd = false;
				}
				for (CNode* tempnode : manager.doneList) {
					if (_node == tempnode) canAdd = false;
				}

				//If valid, add
				if (canAdd) {
					//Add to back if BFS, add to front is DFS
					if (manager.bfs) {
						manager.searchList.push_back(_node);
					}
					else {
						manager.searchList.insert(manager.searchList.begin(), _node);
					}
				}
			}
		}
	}
	else {
		//If Left Click
		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {

			//Prevents multiple clicks while holding
			if (!manager.frozenPlace) {
				bool canPlace = true;

				//Cant Place more than 26 nodes
				if (manager.nodes.size() >= 26) {
					std::cout << "Max Nodes Reached\n";
					canPlace = false;
				}

				//Checks if user has clicked on a node, tells them off 
				for (CNode* _node : manager.nodes) {
					if (_node->sprite->getGlobalBounds().contains((sf::Vector2f)sf::Mouse::getPosition(*manager.window))) {
						std::cout << "Too Close\n";
						canPlace = false;
						break;
					}
				}

				sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(*manager.window);

				//If not too close, place a new node at cursor
				if (canPlace && mousePos.x > 0 && mousePos.x < manager.window->getSize().x && mousePos.y > 0 && mousePos.y < manager.window->getSize().y) {


					std::string s(1, char(65 + (int)manager.nodes.size()));
					manager.nodes.push_back(new CNode(s, (sf::Vector2f)sf::Mouse::getPosition(*manager.window) - sf::Vector2f(32, 32), manager.font));

					if (manager.nodes.size() == 1) {
						manager.searchList.push_back(manager.nodes[0]);
					}

					manager.target = *(manager.nodes.end() - 1);
				}

				manager.frozenPlace = true;
			}
		}
		else {
			manager.frozenPlace = false;
		}

		//If Holding Right Click
		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {

			//If no current node selected, select one, otherwise draw line from said node to cursor
			if (manager.connecting == nullptr) {
				for (CNode* _node : manager.nodes) {
					if (_node->sprite->getGlobalBounds().contains((sf::Vector2f)sf::Mouse::getPosition(*manager.window))) {
						manager.connecting = _node;
						break;
					}
				}
			}
			else {
				manager.templine->operator[](0).position = sf::Vector2f(manager.connecting->sprite->getPosition().x + 32, manager.connecting->sprite->getPosition().y + 32);
				manager.templine->operator[](0).color = sf::Color::Yellow;
				manager.templine->operator[](1).position = (sf::Vector2f)sf::Mouse::getPosition(*manager.window);
				manager.templine->operator[](1).color = sf::Color::Yellow;
			}
		}
		else {

			//If has node selected, and you release right click, otherwise hide line
			if (manager.connecting != nullptr) {

				//Checks if cursor is over node, then tries to create connection
				for (CNode* _node : manager.nodes) {
					if (_node->sprite->getGlobalBounds().contains((sf::Vector2f)sf::Mouse::getPosition(*manager.window))) {
						CreateConnection(manager.connecting, _node);
						break;
					}
				}
			}
			else {
				manager.templine->operator[](0).position = sf::Vector2f(0, 0);
				manager.templine->operator[](1).position = sf::Vector2f(0, 0);
			}

			manager.connecting = nullptr;
		}
	}

found:
	if (manager.found) {
		manager.found = false;
		manager.search = false;

		for (CNode* _node : manager.doneList) {
			std::cout << _node->name << std::endl;
		}

		manager.doneList.clear();
		manager.searchList.clear();
		manager.searchList.push_back(manager.nodes[0]);
	}

	//Draw Nodes first
	for (CNode* _node : manager.nodes) {
		manager.ToDrawList.push_back(_node->sprite);
		_node->text->setFont(manager.font);
		manager.ToDrawList.push_back(_node->text);
	}

	//Draw Node's lines next
	for (CNode* _node : manager.nodes) {
		for (sf::VertexArray* _line : _node->lines) {
			manager.ToDrawList.push_back(_line);
		}
	}

	//Draw cursor line
	manager.ToDrawList.push_back(manager.templine);

	return 1;
}

/// <summary>
/// Creates a connection between two nodes
/// </summary>
void CreateConnection(CNode* from, CNode* to) {

	//If trying to connect self, return
	if (from == to) {
		std::cout << "Can't Connect to Self\n";
		return;
	}

	//If connection already made, return
	for (CNode* _node : from->goesTo) {
		if (_node == to) {
			std::cout << "Already Connected\n";
			return;
		}
	}

	//Otherwise make connection
	std::cout << "Created Connection\n";
	from->goesTo.push_back(to);
	to->goesTo.push_back(from);

	CreateLine(from, to);
}

/// <summary>
/// Creates a line between two nodes
/// </summary>
void CreateLine(CNode* from, CNode* to)
{
	sf::VertexArray* lines = new sf::VertexArray(sf::LineStrip, 2);
	lines->operator[](0).position = sf::Vector2f(from->sprite->getPosition().x + 32, from->sprite->getPosition().y + 32);
	lines->operator[](0).color = sf::Color::Green;
	lines->operator[](1).position = sf::Vector2f(to->sprite->getPosition().x + 32, to->sprite->getPosition().y + 32);
	lines->operator[](1).color = sf::Color::Green;
	from->lines.push_back(lines);
}

/// <summary>
/// Checks if a button has been clicked
/// </summary>
void CheckButtonsPressed()
{
	//Check Mouse lick
	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {

		//Prevents multiple clicks if holding down
		if (!manager.frozenClick) {
			
			//Loops through all buttons
			for (CButton* _button : manager.Buttons)
			{

				//If click, do func
				if (_button->rect->getGlobalBounds().contains((sf::Vector2f)sf::Mouse::getPosition(*manager.controlWindow))) {
					if (_button->function != nullptr) _button->function();
				}
			}
		}
		manager.frozenClick = true;
	}
	else {
		manager.frozenClick = false;
	}
}

/// <summary>
/// Draw Function 
/// </summary>
void Draw()
{
	//Draws Items
	manager.window->clear();

	for (sf::Drawable* item : manager.ToDrawList) {
		manager.window->draw(*item);
	}

	manager.window->display();


	//Draws Buttons
	manager.controlWindow->clear();

	for (CButton* button : manager.Buttons) {
		manager.controlWindow->draw(*button->rect);
		button->text->setFont(manager.font);
		manager.controlWindow->draw(*button->text);
	}

	manager.controlWindow->display();
}


/// <summary>
/// Creates A button with given parameters
/// </summary>
void CreateButton(void(*function)(), std::string _string, int _fontSize, sf::Color _tColour, sf::Text::Style _style, float _x, float _y, sf::Color _bgColour, float _padding)
{
	//Text
	sf::Text* tempText = new sf::Text;
	tempText->setString(_string);
	tempText->setCharacterSize(_fontSize);
	tempText->setFillColor(_tColour);
	tempText->setStyle(_style);
	tempText->setFont(manager.font);

	//Middle of button
	tempText->setPosition(100 - (tempText->getGlobalBounds().width) / 2, _y);

	//Button rect
	sf::RectangleShape* buttonRect = new sf::RectangleShape;
	buttonRect->setPosition(0, tempText->getGlobalBounds().top - _padding);
	buttonRect->setSize(sf::Vector2f(200, 30));
	buttonRect->setFillColor(_bgColour);

	//create
	CButton* button = new CButton(buttonRect, tempText, function);
	manager.Buttons.push_back(button);
}