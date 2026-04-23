//
// Created by andgel on 14/11/2020.
//

#ifndef UBC_DIALOGMGR_HPP
#define UBC_DIALOGMGR_HPP

#include <vector>
#include <string>
#include <tuple>
#include <map>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backends/SFML.hpp>
#include "Resources/GameState.hpp"
#include "Resources/Resources.hpp"

namespace UntilBeingCrowned
{
	class DialogMgr {
	private:
		tgui::Gui &_gui;
		static const std::map<std::string, std::string (DialogMgr::*)(const std::vector<std::string> &)> _commands;
		std::map<std::string, std::vector<std::string>> _dialogsString;
		std::tuple<std::string, unsigned, size_t> _currentDialog = {"", 0, 0};
		unsigned _waitingTime = 0;
		bool _left = false;
		bool _onHold = false;
		bool _done = true;
		bool _skippedWeek = false;
		bool _lineEnded = false;
		std::string _text;
		std::string _newBackground;
		Resources &_resources;
		GameState &_state;

		void _processTextCharacter();
		std::string _moveBg(const std::vector<std::string> &args);
		std::string _finishCmd(const std::vector<std::string> &args);
		std::string _setFlagCmd(const std::vector<std::string> &args);
		std::string _unsetFlagCmd(const std::vector<std::string> &args);
		std::string _buttonsPlaceCmd(const std::vector<std::string> &args);
		std::string _skipWeekCmd(const std::vector<std::string> &args);
		std::string _hideCmd(const std::vector<std::string> &args);
		std::string _unhideCmd(const std::vector<std::string> &args);
		std::string _dispPercent(const std::vector<std::string> &);
		std::string _if(const std::vector<std::string> &);
		std::string _choicesCmd(const std::vector<std::string> &args);
		std::string _skipCmd(const std::vector<std::string> &args);
		std::string _setSpriteCmd(const std::vector<std::string> &args);
		std::string _setMusic(const std::vector<std::string> &args);
		std::string _playSfx(const std::vector<std::string> &args);
		std::string _wait(const std::vector<std::string> &args);
		std::string _changeHappiness(const std::vector<std::string> &args);
		std::string _moveSprite(const std::vector<std::string> &args);
		std::string _setBackground(const std::vector<std::string> &args);
		std::string _notImplemented(const std::vector<std::string> &args);
		void _nextLine();
		static std::pair<std::string, std::vector<std::string>> _parseCommand(size_t &pos, const std::string &cmdStart);

	public:
		DialogMgr(tgui::Gui &gui, Resources &resources, GameState &state);
		void clicked();
		bool isDone() const;
		bool hasSkippedWeek() const;
		bool hasDialog(const std::string &id);
		void update();
		void startDialog(const std::string &id);
		void loadFile(const std::string &path);
	};
}


#endif //UBC_DIALOGMGR_HPP
