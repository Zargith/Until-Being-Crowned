//
// Created by andgel on 18/11/2020.
//

#include "GameoverMenu.hpp"

namespace UntilBeingCrowned
{
	GameoverMenu::GameoverMenu(MenuMgr &mgr, Resources &res, tgui::Gui &gui, GameState &state) :
		_mgr(mgr),
		_gui(gui),
		_state(state),
		_res(res)
	{
	}

	void GameoverMenu::switched(bool isActive)
	{
		if (!isActive)
			return;

		this->_res.playMusic("game_over");
		auto it = std::find_if(this->_state.flags.begin(), this->_state.flags.end(), [](const std::string &str){
			return str.substr(0, strlen("killed_")) == "killed_";
		});

		this->_gui.loadWidgetsFromFile("gui/gameover_menu.gui");
		this->_gui.get<tgui::Label>("EndId")->setText(*it);
		this->_gui.get<tgui::Button>("menu")->onPress.connect(
			[this]{
				this->_res.playSound("click_button");
				this->_mgr.changeMenu("main");
			}
		);
	}

	void GameoverMenu::render()
	{
	}

	void GameoverMenu::handleEvent(const Input::Event &)
	{

	}
}
