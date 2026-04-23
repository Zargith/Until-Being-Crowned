//
// Created by Gegel85 on 15/01/2019.
//

#ifndef UBC_SCREEN_HPP
#define UBC_SCREEN_HPP


#include <SFML/Graphics.hpp>
#include "../DataType/Vector.hpp"

namespace UntilBeingCrowned::Rendering
{
	class Screen : public sf::RenderWindow {
	private:
		Vector2f _cameraCenter{0, 0};

		sf::View _view;

		sf::RectangleShape _rect;

		sf::CircleShape _circle;

		sf::Text _text;

		sf::Clock _clock;

		sf::Sprite _sprite;

		std::string _title;

		double _fps;

		bool _isFullscreen;

	public:
		Screen(const std::string &title, unsigned int width = 640, unsigned int height = 480, bool fullscreen=false);

		~Screen() override;

		const std::string	&getTitle() const;

		void	setTitle(const std::string &);

		void    fillColor(const sf::Color &color = sf::Color{0xFF, 0xFF, 0xFF, 0xFF});

		void	setFont(const sf::Font &font);

		void	textSize(const size_t &size);

		void	setCameraCenter(Vector2f);

		void	draw(sf::IntRect rect);

		void	draw(float radius, Vector2f pos);

		void	draw(const std::string &str, Vector2f pos, unsigned style = sf::Text::Regular);

		void	draw(sf::Sprite &sprite, Vector2f pos);

		Vector2f getCameraCenter() const;

		void	draw(sf::Texture &texture, Vector2f pos, Vector2u size = {0, 0}, sf::IntRect rect = {0, 0, 0, 0});

		double	getFPS();

		void	display();

		float getTextWidth(const std::string &text);

		bool isFullscreen() const;

		void reOpen(const std::string &title, unsigned int width = 640, unsigned int height = 480, bool fullscreen=false);
	};
}


#endif //UBC_RESOURCES_HPP
