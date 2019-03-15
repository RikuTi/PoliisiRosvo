
#include <Windows.h>
#include <iostream>
#include <vector>
//sfml librarys window and rendering utilities
#include <SFML\Window.hpp>
#include <SFML\Graphics.hpp>

void SetPositionRotation(sf::Sprite & object, const sf::Vector2f& new_origin, float rotation)
{
	object.move(new_origin);
	object.setRotation(rotation);
}


void MoveComputercar(sf::Window& window, sf::Sprite& object, float &current_angle, float speed, sf::Sprite&missile, bool &missile_alive, float &missile_direction)
{
	static int direction = 0;

	//randomize direction from time to time
	bool change_dir = (rand() % 10000 + 1) < 5;

	if (change_dir)
	{
		direction = rand() % 3;
	}

	bool fire_rocket = (rand() % 10000 + 1) < 5;


	if (fire_rocket && !missile_alive)
	{
		missile_alive = true;
		missile_direction = current_angle;
		missile.setPosition(object.getPosition());
	}

	//move it based on direction
	if (direction == 0)
	{
		sf::Vector2f new_pos = sf::Vector2f(0, -speed);
		current_angle = -90.0f;
		SetPositionRotation(object, new_pos, current_angle);
	}
	else if (direction == 1)
	{
		sf::Vector2f new_pos = sf::Vector2f(0, speed);
		current_angle = 90.0f;
		SetPositionRotation(object, new_pos, current_angle);
	}
	else if (direction == 2)
	{
		sf::Vector2f new_pos = sf::Vector2f(-speed, 0);
		current_angle = 180.0f;
		SetPositionRotation(object, new_pos, current_angle);
	}
	else if (direction == 3)
	{
		sf::Vector2f new_pos = sf::Vector2f(speed, 0);
		current_angle = 0.0f;
		SetPositionRotation(object, new_pos, current_angle);
	}


	//inverse direction
	sf::Vector2f pos = object.getPosition();
	sf::FloatRect bounds = object.getGlobalBounds();

	if ((pos.x + bounds.width) >= window.getSize().x)
	{
		direction = 2;
	}
	else if ((pos.x) <= 0)
	{
		direction = 3;
	}
	else if ((pos.y + bounds.height) >= window.getSize().y)
	{
		direction = 0;
	}
	else if ((pos.y) <= 0)
	{
		direction = 1;
	}
}


bool CheckCollision(sf::Sprite& car1, sf::Sprite& car2)
{
	if (car1.getGlobalBounds().left < car2.getGlobalBounds().left + car2.getGlobalBounds().width
		&& car1.getGlobalBounds().left + car1.getGlobalBounds().width > car2.getGlobalBounds().left
		&& car1.getGlobalBounds().top < car2.getGlobalBounds().top + car2.getGlobalBounds().height
		&& car1.getGlobalBounds().top + car1.getGlobalBounds().height > car2.getGlobalBounds().top)
	{
		return true;
	}
	return false;
}

void HandleMissileLogic(sf::Window & window, sf::Sprite&missile, sf::Sprite&enemy, float direction, float speed, bool &missile_alive, int& num_hits)
{
	if (direction == -90.0f)
	{
		sf::Vector2f new_pos = sf::Vector2f(0, -speed);
		SetPositionRotation(missile, new_pos, direction);
	}
	else if (direction == 180.0f)
	{
		sf::Vector2f new_pos = sf::Vector2f(-speed, 0);
		SetPositionRotation(missile, new_pos, direction);
	}
	else if (direction == 90.0f)
	{
		sf::Vector2f new_pos = sf::Vector2f(0, speed);
		SetPositionRotation(missile, new_pos, direction);
	}
	else if (direction == 0.0f)
	{
		sf::Vector2f new_pos = sf::Vector2f(speed, 0);
		SetPositionRotation(missile, new_pos, direction);
	}

	sf::Vector2f pos = missile.getPosition();
	sf::FloatRect bounds = missile.getGlobalBounds();

	if ((pos.x + bounds.width) >= window.getSize().x)
	{
		missile_alive = false;
	}
	else if ((pos.x) <= 0)
	{
		missile_alive = false;
	}
	else if ((pos.y + bounds.height) >= window.getSize().y)
	{
		missile_alive = false;
	}
	else if ((pos.y) <= 0)
	{
		missile_alive = false;
	}

	if (CheckCollision(missile, enemy))
	{
		num_hits++;
		missile_alive = false;
	}

}

int main()
{
	//size of window
	unsigned int window_height = 800;
	unsigned int window_width = 1200;
	//custom window settings
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	//load Arial font from Windows system
	sf::Font font;
	font.loadFromFile("C:/Windows/Fonts/Arial.ttf");

	//create our rendering window
	sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Poliisi & Rosvo", sf::Style::Default, settings);


	//game end text at center
	sf::Text text;
	text.setString("Game Ended");
	text.setFillColor(sf::Color::Cyan);
	text.setFont(font);
	text.setCharacterSize(30);
	int center_x = window.getSize().x / 2;
	text.setOrigin(sf::Vector2f((text.getGlobalBounds().width / 2), text.getGlobalBounds().height / 2));
	text.setPosition(sf::Vector2f(center_x, 10));


	//my score
	sf::Text hit_text_computer;
	hit_text_computer.setString("Score me: 0");
	hit_text_computer.setFillColor(sf::Color::Red);
	hit_text_computer.setPosition(sf::Vector2f(window.getSize().x - 350, 10));
	hit_text_computer.setFont(font);
	hit_text_computer.setCharacterSize(30);

	//enemy score
	sf::Text hit_text;
	hit_text.setString("Score computer: 0");
	hit_text.setFillColor(sf::Color::Red);
	//somewhat centered text but sfml dosent offer text measuring funcs
	hit_text.setPosition(sf::Vector2f(window.getSize().x - 350, 50));
	hit_text.setFont(font);
	hit_text.setCharacterSize(30);

	//load images as textures from file
	sf::Texture red_car_texture, blue_car_texture, missile_texture;

	red_car_texture.loadFromFile("red_car.png");
	blue_car_texture.loadFromFile("blue_car.png");
	missile_texture.loadFromFile("missile.png");

	sf::Sprite red_car, blue_car, missile, enemy_missile;


	std::vector<sf::Texture> explosion_textures;
	std::vector<sf::Sprite> explosion;

	//load 4x4 textures from file and push them into vector
	//horizontal row
	for (size_t i = 0; i < 4; i++)
	{
		//vertical row
		for (size_t j = 0; j < 4; j++)
		{
			sf::Texture temp;

			temp.loadFromFile("exp2.png", sf::IntRect(j * 62, i * 62, 62, 62));

			explosion_textures.push_back(temp);
		}
	}

	//set our sprites from texture vector
	for (size_t i = 0; i < explosion_textures.size(); i++)
	{
		sf::Sprite new_sprite;
		new_sprite.setTexture(explosion_textures.at(i));

		//scale to be larger even thought it will decrease quality
		new_sprite.setScale(sf::Vector2f(3.5f, 3.5f));
		explosion.push_back(new_sprite);
	}


	//set our sprite texture from previous loaded files
	red_car.setTexture(red_car_texture);
	blue_car.setTexture(blue_car_texture);
	missile.setTexture(missile_texture);
	//set color of enemy missile so we know which is which
	enemy_missile.setTexture(missile_texture);
	enemy_missile.setColor(sf::Color::Green);

	//make everything get rotated based on the center of the object
	//sfml translates rotation based on origin instead of position
	sf::Vector2f center = sf::Vector2f(red_car.getGlobalBounds().width / 2, red_car.getGlobalBounds().height / 2);
	red_car.setOrigin(center);
	red_car.setPosition(sf::Vector2f(200, 300));

	sf::Vector2f blue_center = sf::Vector2f(blue_car.getGlobalBounds().width / 2, blue_car.getGlobalBounds().height / 2);
	blue_car.setOrigin(blue_center);
	blue_car.setPosition(sf::Vector2f(500, 500));

	sf::Vector2f missile_center = sf::Vector2f(missile.getGlobalBounds().width / 2, missile.getGlobalBounds().height / 2);
	missile.setOrigin(missile_center);
	missile.setPosition(sf::Vector2f(500, 500));

	sf::Vector2f enemy_missile_center = sf::Vector2f(enemy_missile.getGlobalBounds().width / 2, enemy_missile.getGlobalBounds().height / 2);
	enemy_missile.setOrigin(enemy_missile_center);
	enemy_missile.setPosition(sf::Vector2f(500, 500));


	//scale cars down
	red_car.setScale(sf::Vector2f(0.2f, 0.2f));
	blue_car.setScale(sf::Vector2f(0.2f, 0.2f));
	//scale missiles down
	missile.setScale(sf::Vector2f(0.1f, 0.1f));
	enemy_missile.setScale(sf::Vector2f(0.1f, 0.1f));

	sf::Clock clock;

	//how fast do we move
	float speed_modifier = 400.0f;
	//is game running
	bool game_running = true;


	//computer variables
	bool missile_alive = false;
	float missile_direction = 0.0f;
	bool enemy_missile_alive = false;
	float enemy_missile_direction = 0.0f;
	int num_hits = 0;
	float computer_angle = 0.0f;
	int num_computer_hits = 0;

	//my variables
	float current_angle = 0.0f;
	bool can_i_move = true;
	bool computer_can_move = true;
	bool move_localplayer_inverse = false;

	//explosion animation steps
	int sprite_iter = 0;
	int explosion_step = 0;


	//loop as long the window is open
	while (window.isOpen())
	{

		//clear and colour the window
		window.clear(sf::Color(43, 43, 43, 255));


		float frametime = clock.restart().asSeconds();
		float computer_speed = speed_modifier;
		float my_speed = speed_modifier;

		//scale movement speed based on elapsed frametime to get smooth movement
		computer_speed *= frametime;
		my_speed *= frametime;

		float missile_speed = my_speed * 3;

		//poll all window events
		sf::Event event;
		while (window.pollEvent(event))
		{
			// "close requested" event: we close the window
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
		}


		//if game still running
		if (game_running)
		{
			//move our own controlled player based on W,A,S,D
			if (can_i_move)
			{
				//do movement based on pressed keys and inversed if needed
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
				{
					if (move_localplayer_inverse)
					{
						sf::Vector2f new_pos = sf::Vector2f(0, my_speed);
						current_angle = 90.0f;
						SetPositionRotation(blue_car, new_pos, current_angle);
					}
					else
					{
						sf::Vector2f new_pos = sf::Vector2f(0, -my_speed);
						current_angle = -90.0f;
						SetPositionRotation(blue_car, new_pos, current_angle);
					}
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
				{
					if (move_localplayer_inverse)
					{
						sf::Vector2f new_pos = sf::Vector2f(my_speed, 0);
						current_angle = 0.0f;
						SetPositionRotation(blue_car, new_pos, current_angle);
					}
					else
					{
						sf::Vector2f new_pos = sf::Vector2f(-my_speed, 0);
						current_angle = 180.0f;
						SetPositionRotation(blue_car, new_pos, current_angle);
					}
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
				{

					if (move_localplayer_inverse)
					{
						sf::Vector2f new_pos = sf::Vector2f(0, -my_speed);
						current_angle = -90.0f;
						SetPositionRotation(blue_car, new_pos, current_angle);
					}
					else
					{
						sf::Vector2f new_pos = sf::Vector2f(0, my_speed);
						current_angle = 90.0f;
						SetPositionRotation(blue_car, new_pos, current_angle);
					}
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
				{
					if (move_localplayer_inverse)
					{
						sf::Vector2f new_pos = sf::Vector2f(-my_speed, 0);
						current_angle = 180.0f;
						SetPositionRotation(blue_car, new_pos, current_angle);
					}
					else
					{
						sf::Vector2f new_pos = sf::Vector2f(my_speed, 0);
						current_angle = 0.0f;
						SetPositionRotation(blue_car, new_pos, current_angle);
					}
				}
				else
				{
					//move him the other direction until all keys released
					move_localplayer_inverse = false;
				}

				//dont allow multiple missiles fired
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && !missile_alive)
				{
					missile_alive = true;
					missile_direction = current_angle;
					missile.setPosition(blue_car.getPosition());
				}


				//if we go out of window make us go the other direction until w,a,s,d all are released
				sf::Vector2f pos = blue_car.getPosition();
				sf::FloatRect bounds = blue_car.getGlobalBounds();

				if (((pos.x + bounds.width) >= window.getSize().x)
					|| ((pos.x) <= 0)
					|| ((pos.y + bounds.height) >= window.getSize().y)
					|| ((pos.y) <= 0))
				{
					move_localplayer_inverse = true;
				}

			}


			//handle computer car movement
			if (computer_can_move)
				MoveComputercar(window, red_car, computer_angle, computer_speed, enemy_missile, enemy_missile_alive, enemy_missile_direction);


			//if collision end the game
			game_running = !CheckCollision(red_car, blue_car);


			//handle both missiles if they are active
			if (missile_alive)
			{
				HandleMissileLogic(window, missile, red_car, missile_direction, missile_speed, missile_alive, num_hits);
			}

			if (enemy_missile_alive)
			{
				HandleMissileLogic(window, enemy_missile, blue_car, enemy_missile_direction, missile_speed, enemy_missile_alive, num_computer_hits);
			}


			//draw scores
			std::string new_text = "Score me: " + std::to_string(num_hits);
			hit_text.setString(new_text);

			window.draw(hit_text);

			std::string new_text_computer = "Score computer: " + std::to_string(num_computer_hits);
			hit_text_computer.setString(new_text_computer);

			window.draw(hit_text_computer);


			//draw the cars
			window.draw(red_car);
			window.draw(blue_car);

			//draw the missiles
			if (missile_alive)
				window.draw(missile);

			if (enemy_missile_alive)
				window.draw(enemy_missile);

			//check if either one wins if so stop his movement and play explosion animation
			if (num_hits > 3 || num_computer_hits > 3)
			{

				if (num_hits > 3)
					computer_can_move = false;
				else
					can_i_move = false;

				sprite_iter++;

				//slowly move from sprite to sprite
				if (sprite_iter > 800)
				{
					explosion_step++;

					if (explosion_step >= 15)
					{
						game_running = false;
						explosion_step = 0;
					}
					sprite_iter = 0;
				}

				sf::Sprite current_explosion = explosion.at(explosion_step);

				sf::Vector2f position = red_car.getPosition();

				if (num_computer_hits > 3)
					position = blue_car.getPosition();

				current_explosion.setPosition(position - sf::Vector2f(current_explosion.getGlobalBounds().width / 2, current_explosion.getGlobalBounds().height / 2));

				window.draw(current_explosion);
			}

		}
		else//game has ended
		{
			window.clear(sf::Color(103, 43, 103, 255));
			window.draw(text);
		}

		//display everything we have drawn
		window.display();
	}

	return 0;

}
