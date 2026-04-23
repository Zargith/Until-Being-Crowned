//
// Created by timcav on 27/10/2020.
//

#include "MainMenu.hpp"
#include "../Loader.hpp"
#include "DialogMenu.hpp"

namespace UntilBeingCrowned
{
	MainMenu::MainMenu(MenuMgr &mgr, tgui::Gui &gui, Resources &res, Game &game) :
		_res(res),
		_gui(gui),
		_mgr(mgr),
		_game(game)
	{
	}

	void MainMenu::switched(bool isActive)
	{
		if (!isActive) {
			this->_gui.removeAllWidgets();
			return;
		}

		this->_res.playMusic("main_menu");

		this->_gui.loadWidgetsFromFile("gui/mainMenu.gui");

		this->_gui.get<tgui::Button>("newGame")->onPress.connect(
			[this]{
				this->_res.playSound("click_button");
				this->_mgr.changeMenu("gender");
			}
		);

		this->_gui.get<tgui::Button>("loadGame")->onPress.connect(
			[this]{
				this->_res.playSound("click_button");
				this->_res.playMusic(in_game_musics[this->_res.random() % in_game_musics.size()]);
				this->_loadGame();
			}
		);

		this->_gui.get<tgui::Button>("options")->onPress.connect(
			[this]{
				this->_res.playSound("click_button");
				this->_mgr.changeMenu("option");
			}
		);

		this->_gui.get<tgui::Button>("exit")->onPress.connect(
			&Rendering::Screen::close,
			&this->_res.screen
		);

	}

	void MainMenu::render()
	{

	}

	void MainMenu::handleEvent(const Input::Event &)
	{

	}

	void MainMenu::_loadGame()
	{
		if (Loader::loadProgression(_game, "progression"))
			this->_mgr.changeMenu("in_game");
	}
}
