//
// Created by Gegel85 on 19/01/2020.
//

#ifndef UBC_SFMLJOYPAD_HPP
#define UBC_SFMLJOYPAD_HPP

#define ACTIVATION_THRESHOLD 20

#include <SFML/Graphics/RenderWindow.hpp>
#include <map>
#include "Input.hpp"

namespace UntilBeingCrowned::Inputs
{
	class SFMLJoypad : public Input {
	private:
		bool _keyChanged = false;

		std::optional<Action> _keyChanging;

		sf::Vector2<sf::Joystick::Axis> _axis;

		std::vector<unsigned> _buttons;

		sf::RenderWindow *_window;

		std::vector<bool> _state;

		std::vector<Event> _events;

		static const std::map<sf::Joystick::Axis, std::string> _axisToString;

		void _updateState();
	public:
		SFMLJoypad(sf::RenderWindow *window);
		~SFMLJoypad() override = default;
		bool changeKeyFor(Action) override;
		void cancelChangeKey() override;
		bool actionPressed(Action) const override;
		void handleEvent(sf::Event) override;
		std::vector<Action> getActions() override;
		std::optional<Event> pollEvent() override;
		void unserialize(std::istream &) override;
		double getDirectionAngle() const override;
		void serialize(std::ostream &) const override;
		std::string getEnumControlString(Action code) override;
	};
}


#endif //UBC_SFMLJOYPAD_HPP
