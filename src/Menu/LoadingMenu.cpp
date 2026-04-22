//
// Created by timcav on 28/10/2020.
//

#include "LoadingMenu.hpp"


namespace UntilBeingCrowned
{
    LoadingMenu::LoadingMenu(MenuMgr &mgr, tgui::Gui &gui, Resources &res) :
        _gui(gui),
        _mgr(mgr),
        _res(res)
    {
    }

    void LoadingMenu::switched(bool isActive)
    {
        if (!isActive) {
            this->_gui.removeAllWidgets();
            return;
        }

        this->_gui.loadWidgetsFromFile("gui/loadMenu.gui");
//        this->_gui.get("back")->cast<tgui::Button>()->connect(tgui::Signals::Button::Pressed, &LoadingMenu::_backButton, this);
        this->_gui.get("back")->cast<tgui::Button>()->onPress.connect(&LoadingMenu::_backButton, this);

    }

    void LoadingMenu::render()
    {

    }

    void LoadingMenu::handleEvent(const Input::Event &)
    {

    }

    void LoadingMenu::_backButton() {
        this->_res.playSound("click_button");
        this->_mgr.changeMenu("main");
    }
}

