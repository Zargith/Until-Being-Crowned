//
// Created by timcav on 28/10/2020.
//

#include "OptionMenu.hpp"
#include "../DataType/Vector.hpp"
#include "../Rendering//Screen.hpp"
#include "../Loader.hpp"
#include <iostream>

namespace UntilBeingCrowned
{
	OptionMenu::OptionMenu(MenuMgr &mgr, tgui::Gui &gui, Resources &res, Game &game) :
		_res(res),
		_gui(gui),
		_mgr(mgr),
		_game(game),
		_initMusicVolume(100),
		_newMusicVolume(100),
		_initSoundVolume(100),
		_newSoundVolume(100),
		_initFullscreen(false),
		_newFullScreen(false)
	{
	}

	void OptionMenu::switched(bool isActive)
	{
		if (!isActive) {
			this->_gui.removeAllWidgets();
			return;
		}
		this->_gui.loadWidgetsFromFile("gui/optionMenu.gui");

		_initFullscreen = this->_res.screen.isFullscreen();
		_newFullScreen = _initFullscreen;

		auto checkBox = this->_gui.get<tgui::CheckBox>("fullscreenCheckBox");

		this->_gui.get<tgui::Button>("back")->onPress.connect(&OptionMenu::back, this);
		this->_gui.get<tgui::Button>("save")->onPress.connect(&OptionMenu::save, this);
		this->_sliderMusic = this->_gui.get<tgui::Slider>("volumeMusicSlider");
		this->_sliderMusic->onValueChange.connect(&OptionMenu::setMusicVolume, this);
		this->_sliderSfx = this->_gui.get<tgui::Slider>("volumeSoundSlider");
		this->_sliderSfx->onValueChange.connect(&OptionMenu::setSoundVolume, this);
		this->_initMusicVolume = this->_res.getMusicVolume();
		this->_newMusicVolume = this->_initMusicVolume;
		this->_sliderMusic->setValue(this->_initMusicVolume);
		this->_initSoundVolume = this->_res.getSoundVolume();
		this->_newSoundVolume = this->_initSoundVolume;
		this->_sliderSfx->setValue(this->_initSoundVolume);

		checkBox->setChecked(this->_initFullscreen);
		checkBox->onCheck.connect(&OptionMenu::setFullscreen, this, true);
		checkBox->onUncheck.connect(&OptionMenu::setFullscreen, this, false);
		this->_initFullscreen = this->_res.screen.isFullscreen();
		this->_newFullScreen = this->_initFullscreen;
	}

	void OptionMenu::render()
	{

	}

	void OptionMenu::handleEvent(const Input::Event &)
	{

	}

	void OptionMenu::back()
	{
		this->_res.setMusicVolume(this->_initMusicVolume);
		this->_res.setSoundVolume(this->_initSoundVolume);
		if (this->_initFullscreen != this->_newFullScreen)
			this->setFullscreen(this->_initFullscreen);
		this->_res.playSound("click_button");
		this->_mgr.changeMenu("main");
	}

	void OptionMenu::setMusicVolume()
	{
		this->_newMusicVolume = this->_sliderMusic->getValue();
		this->_res.setMusicVolume(_newMusicVolume);
	}

	void OptionMenu::setSoundVolume()
	{
		this->_newSoundVolume = this->_sliderSfx->getValue();
		this->_res.setSoundVolume(_newSoundVolume);
		this->_res.playSound("click_button");
	}

	void OptionMenu::setFullscreen(bool b)
	{
		this->_res.playSound("click_button");
		this->_newFullScreen = b;
		if (this->_res.screen.isFullscreen() != b)
			this->_res.screen.reOpen(
				this->_res.screen.getTitle(),
				1360,
				768,
				b
			);
	}

	void OptionMenu::save()
	{
		this->_res.playSound("click_button");
		this->_game.state.settings.musicVolume = this->_initMusicVolume = this->_newMusicVolume;
		this->_game.state.settings.sfxVolume   = this->_initSoundVolume = this->_newSoundVolume;
		this->_game.state.settings.fullscreen  = this->_initFullscreen  = this->_newFullScreen;
		Loader::saveSettings(this->_game.state.settings);
	}
}
