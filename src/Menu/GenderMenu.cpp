//
// Created by timcav on 10/11/2020.
//

#include "GenderMenu.hpp"
#include "../Exceptions.hpp"


namespace UntilBeingCrowned
{
	GenderMenu::GenderMenu(MenuMgr &mgr, tgui::Gui &gui, Resources &res, GameState &state, QuestMgr &quests) :
		_res(res),
		_gui(gui),
		_mgr(mgr),
		_state(state),
		_quests(quests)
	{
	}

	void GenderMenu::switched(bool isActive) {
		if (!isActive) {
			this->_gui.removeAllWidgets();
			return;
		}

		this->_gui.loadWidgetsFromFile("gui/genderMenu.gui");
//		this->_gui.get<tgui::Button>("femaleButton")->connect(tgui::Signals::Button::Pressed, &GenderMenu::_runGame, this, "player_f");
		this->_gui.get<tgui::Button>("femaleButton")->cast<tgui::Button>()->onPress.connect(&GenderMenu::_runGame, this, "player_f");

//		this->_gui.get<tgui::Button>("maleButton")->connect(tgui::Signals::Button::Pressed, &GenderMenu::_runGame, this, "player_m");
		this->_gui.get<tgui::Button>("maleButton")->cast<tgui::Button>()->onPress.connect(&GenderMenu::_runGame, this, "player_m");

	}

	void GenderMenu::render() {

	}

	void GenderMenu::handleEvent(const Input::Event &) {

	}

	void GenderMenu::_runGame(const std::string &flag)
	{
		std::ifstream stream{"assets/baseValues.txt"};
		size_t nb;
		std::string token;

		this->_res.playSound("click_button");
		logger.debug("Loading state init file.");
		if (stream.fail())
			throw InvalidStateException("Cannot open the state init file: " + std::string(strerror(errno)) + ".");
		this->_state.flags.clear();
		stream >> this->_state.gold;
		stream >> this->_state.army;
		stream >> this->_state.food;
		stream >> this->_state.goldPassive;
		stream >> this->_state.armyPassive;
		stream >> this->_state.foodPassive;
		stream >> this->_state.goldHappiness;
		stream >> this->_state.foodHappiness;
		stream >> this->_state.armyHappiness;
		stream >> nb;
		logger.debug(std::to_string(this->_state.gold) + " gold");
		logger.debug(std::to_string(this->_state.army) + " army");
		logger.debug(std::to_string(this->_state.food) + " food");
		logger.debug(std::to_string(this->_state.goldPassive) + " goldPassive");
		logger.debug(std::to_string(this->_state.armyPassive) + " armyPassive");
		logger.debug(std::to_string(this->_state.foodPassive) + " foodPassive");
		logger.debug(std::to_string(this->_state.goldHappiness) + " goldHappiness");
		logger.debug(std::to_string(this->_state.foodHappiness) + " foodHappiness");
		logger.debug(std::to_string(this->_state.armyHappiness) + " armyHappiness");
		logger.debug(std::to_string(nb) + " base flags");
		this->_state.flags.reserve(nb + 1);
		std::getline(stream, token);
		for (; nb; nb--) {
			std::getline(stream, token);
			logger.debug("Added flag '" + token + "'");
			this->_state.flags.push_back(token);
		}
		if (stream.fail())
			throw InvalidStateException("The state init file is invalid.");
		stream.close();
		this->_state.flags.push_back(flag);
		this->_state.week = 0;
		this->_quests.reset();
		this->_mgr.changeMenu("dialog");
	}
}