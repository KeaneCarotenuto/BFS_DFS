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

//Functions
int FixedUpdate();
void BeginConnecting();
void EndConnecting();
void DrawConnectingLine();
void TryPlaceNode();
void BFS_DFS();
void CreateConnection(CNode* from, CNode* to);
void CreateLine(CNode* from, CNode* to, bool isDoneLine = false);
void DisableButtons();
void RestoreButtons();
void CheckButtonsPressed();
void Draw();

void CreateButton(void(*function)(), std::string _string, int _fontSize, sf::Color _tColour, sf::Text::Style _style, float _x, float _y, sf::Color _bgColour, float _padding);

CManager manager;

/// <summary>
/// Start Search Algorithm
/// </summary>
void Search() {
	if (!manager.search) {
		if (!manager.searchList.empty()) {
			manager.search = true;
			DisableButtons();

			for (CNode* _node : manager.nodes) {
				_node->sprite->setColor(sf::Color::White);
			}
			for (sf::VertexArray* _line : manager.doneLines) {

				delete _line;
			}
			manager.doneLines.clear();

			std::system("CLS");

			std::string type = (manager.bfs ? "BFS" : "DFS");
			std::cout << "Started " + type + "\nStart: " + manager.searchList[0]->name + "\nGoal: " + manager.target->name + "\n";
			
		}
		else {
			std::cout << "Not Enough Nodes to Start\n\n";
		}
	}
	else {
		std::cout << "Please Wait\n\n";
	}
	
}

/// <summary>
/// Swaps Between BFS and DFS
/// </summary>
void SwapMethod(){
	if (!manager.search) {
		manager.bfs = !manager.bfs;

		manager.Buttons[1]->text->setString((manager.bfs ? "BFS" : "DFS"));
	}
}

/// <summary>
/// Clears everything on screen
/// </summary>
void ClearNodes() {
	if (!manager.search) {
		for (CNode* _node : manager.nodes) {
			for (sf::VertexArray* _line : _node->lines) {
				delete _line;
			}
			_node->lines.clear();

			delete _node->sprite;
			delete _node->text;

			delete _node;
		}
		manager.nodes.clear();

		for (sf::VertexArray* _line : manager.doneLines) {

			delete _line;
		}
		manager.doneLines.clear();

		manager.searchList.clear();
		manager.target = nullptr;
	}
}

/// <summary>
/// Clears all connections
/// </summary>
void ClearConnections() {
	if (!manager.search) {
		for (CNode* _node : manager.nodes) {
			for (sf::VertexArray* _line : _node->lines) {
				delete _line;
			}
			_node->lines.clear();

			_node->goesTo.clear();

			_node->sprite->setColor(sf::Color::White);
		}

		for (sf::VertexArray* _line : manager.doneLines) {

			delete _line;
		}
		manager.doneLines.clear();
	}
}

//Start
int main() {
	//make windows
	sf::RenderWindow window(sf::VideoMode(800, 800), "Breadth First & Depth First Search - By Keane Carotenuto & Nerys Thamm");
	sf::RenderWindow controlWindow(sf::VideoMode(200, 200), "Controls");
	controlWindow.setPosition(sf::Vector2i(window.getPosition().x + window.getSize().x, window.getPosition().y));

	manager.window = &window;
	manager.controlWindow = &controlWindow;

	if (!manager.font.loadFromFile("Fonts/Roboto.ttf")) std::cout << "Failed to load Roboto\n\n";

	//Create buttons
	CreateButton(&Search, "Search", 25, sf::Color::White, sf::Text::Style::Bold, 0, 0, sf::Color::Color(0, 150, 0), 5);
	CreateButton(&SwapMethod, "BFS", 25, sf::Color::White, sf::Text::Style::Bold, 0, 40, sf::Color::Color(0, 150, 0), 5);
	CreateButton(&ClearNodes, "Clear All", 25, sf::Color::White, sf::Text::Style::Bold, 0, 80, sf::Color::Color(0, 150, 0), 5);
	CreateButton(&ClearConnections, "Clear Lines", 25, sf::Color::White, sf::Text::Style::Bold, 0, 120, sf::Color::Color(0, 150, 0), 5);

	float stepTime = 0;
	bool drawn = false;

	//Fixed update loop
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

/// <summary>
/// Fixed Update happens at fixed rate
/// </summary>
int FixedUpdate()
{
	manager.currentStep++;
	manager.ToDrawList.clear();

	if (manager.search &&  !manager.found && manager.currentStep % 30 == 0 ) {
		BFS_DFS();
	}
	else {
		//If Left Click
		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {

			TryPlaceNode();
		}
		else {
			manager.frozenPlace = false;
		}

		//If Holding Right Click
		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {

			//If no current node selected, select one, otherwise draw line from said node to cursor
			if (manager.connecting == nullptr) {
				BeginConnecting();
			}
			else {
				DrawConnectingLine();
			}
		}
		else {

			//If has node selected, and you release right click, otherwise hide line
			if (manager.connecting != nullptr) {

				//Checks if cursor is over node, then tries to create connection
				EndConnecting();
			}
			else {
				manager.templine->operator[](0).position = sf::Vector2f(0, 0);
				manager.templine->operator[](1).position = sf::Vector2f(0, 0);
			}

			manager.connecting = nullptr;
		}
	}

	//If the end has been found
	if (manager.found) {
		manager.found = false;
		manager.search = false;

		RestoreButtons();

		std::string str = "Search Order: ";

		//Draw path
		CNode* prevNode = nullptr;
		for (CNode* _node : manager.doneList) {
			if (prevNode != nullptr) {
				CreateLine(prevNode, _node, true);
			}
			prevNode = _node;

			str += _node->name;
		}

		std::cout << str + "\n\n";

		manager.doneList.clear();
		manager.searchList.clear();
		manager.searchList.push_back(manager.nodes[0]);
	}

	//Draw Node's lines next
	for (CNode* _node : manager.nodes) {
		for (sf::VertexArray* _line : _node->lines) {
			manager.ToDrawList.push_back(_line);
		}
	}

	//Draw Nodes first
	for (CNode* _node : manager.nodes) {
		manager.ToDrawList.push_back(_node->sprite);
		_node->text->setFont(manager.font);
		manager.ToDrawList.push_back(_node->text);
	}

	

	//Draw cursor line
	manager.ToDrawList.push_back(manager.templine);

	for (sf::VertexArray* _line : manager.doneLines) {
		manager.ToDrawList.push_back(_line);
	}

	return 1;
}

/// <summary>
/// The actual BFS and DFS algorithm
/// </summary>
void BFS_DFS()
{
	if (!manager.searchList.empty()) {
		//Get First Item in search
		CNode* currentNode = manager.searchList[0];

		//Push it to done list
		manager.doneList.push_back(currentNode);
		currentNode->sprite->setColor(sf::Color::Yellow);

		//Remove it from search list
		std::vector<CNode*>::iterator pos = std::find(manager.searchList.begin(), manager.searchList.end(), currentNode);
		if (pos != manager.searchList.end()) {
			manager.searchList.erase(pos);
		}

		//Check if current node is the target
		if (currentNode == manager.target) {
			manager.found = true;
			std::cout << "Completed Search!\n";
			return;
		}

		std::sort(currentNode->goesTo.begin(), currentNode->goesTo.end(), CNode::IsSmaller);
		if (!manager.bfs) {
			std::reverse(currentNode->goesTo.begin(), currentNode->goesTo.end());
		}

		//Otherwise loop through connected nodes and add them to search list if valid
		for (CNode* _node : currentNode->goesTo) {
			bool canAdd = true;

			//If already searched or done, dont add
			for (CNode* tempnode : manager.searchList) {
				if (_node == tempnode) {
					if (!manager.bfs) {
						std::vector<CNode*>::iterator pos = std::find(manager.searchList.begin(), manager.searchList.end(), tempnode);
						if (pos != manager.searchList.end()) {
							manager.searchList.erase(pos);
						}
					}
					else {
						canAdd = false;
					}
					
				}
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

				_node->sprite->setColor(sf::Color::Green);
			}
		}
	}
	else {
		std::cout << "Failed to complete Search\nCheck Nodes and Connections, then try again\n\n";
		manager.found = true;
	}
}

/// <summary>
/// Starts connecting one node to another node
/// </summary>
void BeginConnecting()
{
	for (CNode* _node : manager.nodes) {
		if (_node->sprite->getGlobalBounds().contains((sf::Vector2f)sf::Mouse::getPosition(*manager.window))) {
			manager.connecting = _node;
			break;
		}
	}
}

/// <summary>
/// Attemps to connect first node to another node
/// </summary>
void EndConnecting()
{
	for (CNode* _node : manager.nodes) {
		if (_node->sprite->getGlobalBounds().contains((sf::Vector2f)sf::Mouse::getPosition(*manager.window))) {
			CreateConnection(manager.connecting, _node);
			break;
		}
	}
}

/// <summary>
/// Creates line between selected node, and mouse, to show show what you are about to connect
/// </summary>
void DrawConnectingLine()
{
	manager.templine->operator[](0).position = sf::Vector2f(manager.connecting->sprite->getPosition().x + 32, manager.connecting->sprite->getPosition().y + 32);
	manager.templine->operator[](0).color = sf::Color::Yellow;
	manager.templine->operator[](1).position = (sf::Vector2f)sf::Mouse::getPosition(*manager.window);
	manager.templine->operator[](1).color = sf::Color::Yellow;
}

/// <summary>
/// Tries to place a node at the current cursor location
/// </summary>
void TryPlaceNode()
{
	//Prevents multiple clicks while holding
	if (!manager.frozenPlace) {
		bool canPlace = true;

		//Cant Place more than 26 nodes
		if (manager.nodes.size() >= 26) {
			std::cout << "Max Nodes Reached\n\n";
			canPlace = false;
		}

		//Checks if user has clicked on a node, tells them off 
		for (CNode* _node : manager.nodes) {
			if (_node->sprite->getGlobalBounds().contains((sf::Vector2f)sf::Mouse::getPosition(*manager.window))) {
				canPlace = false;

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
					manager.searchList.clear();
					manager.searchList.push_back(_node);
					std::cout << "Start Node Set: " + std::string(1,_node->name) + "\n\n";
					break;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
					manager.target = _node;
					std::cout << "Goal Node Set: " + std::string(1, _node->name) + "\n\n";
				}
				else {
					std::cout << "Too Close\n\n";
					break;
				}

				
			}
		}

		sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(*manager.window);

		//If not too close, place a new node at cursor
		if (canPlace && mousePos.x > 0 && mousePos.x < manager.window->getSize().x && mousePos.y > 0 && mousePos.y < manager.window->getSize().y) {

			manager.nodes.push_back(new CNode(char(65 + (int)manager.nodes.size()), (sf::Vector2f)sf::Mouse::getPosition(*manager.window) - sf::Vector2f(32, 32), manager.font));

			if (manager.nodes.size() == 1) {
				manager.searchList.push_back(manager.nodes[0]);
			}

			manager.target = *(manager.nodes.end() - 1);

			std::sort(manager.nodes.begin(), manager.nodes.end(), CNode::IsSmaller);
		}

		manager.frozenPlace = true;
	}
}

/// <summary>
/// Creates a connection between two nodes
/// </summary>
void CreateConnection(CNode* from, CNode* to) {

	//If trying to connect self, return
	if (from == to) {
		std::cout << "Can't Connect to Self\n\n";
		return;
	}

	//If connection already made, return
	for (CNode* _node : from->goesTo) {
		if (_node == to) {
			std::cout << "(" + std::string(1, from->name) + "," + std::string(1, to->name) + ") Already Connected\n\n";
			return;
		}
	}

	//Otherwise make connection
	std::cout << "Created Connection (" + std::string(1,from->name) + "," + std::string(1, to->name) +")\n\n";
	from->goesTo.push_back(to);
	to->goesTo.push_back(from);

	std::sort(from->goesTo.begin(), from->goesTo.end(), CNode::IsSmaller);
	std::sort(to->goesTo.begin(), to->goesTo.end(), CNode::IsSmaller);

	CreateLine(from, to);
}

/// <summary>
/// Creates a line between two nodes
/// </summary>
void CreateLine(CNode* from, CNode* to, bool isDoneLine)
{

	for (int i = 0; i <= 1000; i++) {
		sf::VertexArray* lines = new sf::VertexArray(sf::LineStrip, 2);
		int width = (isDoneLine ? 6 : 4);

		lines->operator[](0).position = sf::Vector2f(from->sprite->getPosition().x + 32 + rand() % width * (rand() % 2 == 0 ? -1 : 1), from->sprite->getPosition().y + 32 + rand() % width * (rand() % 2 == 0 ? -1 : 1));
		lines->operator[](0).color = (isDoneLine ? sf::Color::Color(0,255,0,1) : sf::Color::White);
		lines->operator[](1).position = sf::Vector2f(to->sprite->getPosition().x + 32 + rand() % width * (rand() % 2 == 0 ? -1 : 1), to->sprite->getPosition().y + 32 + rand() % width * (rand() % 2 == 0 ? -1 : 1));
		lines->operator[](1).color = (isDoneLine ? sf::Color::Color(255, 0, 0, 1) : sf::Color::White);

		if (isDoneLine) {
			manager.doneLines.push_back(lines);
		}
		else {
			from->lines.push_back(lines);
		}
	}

	
	
	
}

/// <summary>
/// Disables buttons while searching
/// </summary>
void DisableButtons()
{
	manager.Buttons[0]->text->setString("Wait...");
	manager.Buttons[0]->rect->setFillColor(sf::Color::Red);
	manager.Buttons[1]->text->setString("");
	manager.Buttons[1]->rect->setFillColor(sf::Color::Red);
	manager.Buttons[2]->text->setString("");
	manager.Buttons[2]->rect->setFillColor(sf::Color::Red);
	manager.Buttons[3]->text->setString("");
	manager.Buttons[3]->rect->setFillColor(sf::Color::Red);
}

/// <summary>
/// Restore buttons to initial state
/// </summary>
void RestoreButtons()
{
	manager.Buttons[0]->text->setString("Search");
	manager.Buttons[0]->rect->setFillColor(sf::Color::Color(0, 150, 0));
	manager.Buttons[1]->text->setString(manager.bfs ? "BFS" : "DFS");
	manager.Buttons[1]->rect->setFillColor(sf::Color::Color(0, 150, 0));
	manager.Buttons[2]->text->setString("Clear All");
	manager.Buttons[2]->rect->setFillColor(sf::Color::Color(0, 150, 0));
	manager.Buttons[3]->text->setString("Clear Lines");
	manager.Buttons[3]->rect->setFillColor(sf::Color::Color(0, 150, 0));
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