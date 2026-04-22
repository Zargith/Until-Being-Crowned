//
// Created by Gegel85 on 28/09/2019.
//

#ifndef UBC_INPUT_HPP
#define UBC_INPUT_HPP

#include <optional>
#include <vector>
#include <string>
#include <SFML/Window/Event.hpp>

namespace UntilBeingCrowned
{
	class Input {
	public:
		enum Action {
			UP,
			DOWN,
			LEFT,
			RIGHT,
			SKIP_DIALOG,
			ADVANCE_DIALOG,
			NB_OF_ACTION
		};

		struct Event {
			enum Type {
				EVENT_TRIGGERED,
				EVENT_STOPPED_TRIGGER,
			};

			Type type;
			Action action;
			Event(Type type, Action action);
		};

		virtual ~Input() = default;
		static std::string actionToString(Action action);

		virtual bool changeKeyFor(Action action) = 0;

		virtual void cancelChangeKey() = 0;

		virtual std::optional<Event> pollEvent() = 0;

		virtual bool actionPressed(Action action) const = 0;

		virtual void handleEvent(sf::Event) = 0;

		virtual std::vector<Action> getActions() = 0;

		virtual double getDirectionAngle() const = 0;

		virtual void unserialize(std::istream &stream) = 0;

		virtual void serialize(std::ostream &stream) const = 0;

		virtual std::string getEnumControlString(Action code) = 0;
	};
}


#endif //UBC_INPUT_HPP
