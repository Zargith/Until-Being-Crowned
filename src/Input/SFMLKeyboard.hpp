//
// Created by Gegel85 on 28/09/2019.
//

#ifndef UBC_SFMLKEYBOARD_HPP
#define UBC_SFMLKEYBOARD_HPP


#include <map>
#include <vector>
#include <optional>
#include <SFML/Window/Keyboard.hpp>
#include "Input.hpp"

namespace UntilBeingCrowned::Inputs
{
	class SFMLKeyboard : public Input {
	private:
		bool _keyChanged = false;

		std::optional<Action> _keyChanging;

		std::vector<sf::Keyboard::Key> _keys;

		sf::RenderWindow *_window;

		std::vector<bool> _state;

		std::vector<Event> _events;

		static const std::map<sf::Keyboard::Key, std::string> _keysToString;

		void _updateState();
	public:
		SFMLKeyboard(sf::RenderWindow *window);
		~SFMLKeyboard() override = default;
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


#endif //UBC_SFMLKEYBOARD_HPP
