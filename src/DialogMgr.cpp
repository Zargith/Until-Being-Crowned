//
// Created by andgel on 14/11/2020.
//

#include <json.hpp>
#include <fstream>
#include <TGUI/TGUI.hpp>
#include "DialogMgr.hpp"
#include "Exceptions.hpp"

#define INCREMENT_VAR(var)                                                 \
        if (                                                               \
        	std::find(                                                 \
        		this->_state.flags.begin(),                        \
        		this->_state.flags.end(),                          \
        		"no_"#var                                          \
		) == this->_state.flags.end()                              \
	) {                                                                \
                this->_state.var += this->_state.var##Passive;             \
                this->_state.var += (this->_state.var##Happiness + 1) / 2; \
        }                                                                  \
	this->_state.flags.erase(std::remove(                              \
		this->_state.flags.begin(),                                \
		this->_state.flags.end(),                                  \
		"no_"#var), this->_state.flags.end()                       \
	)

namespace UntilBeingCrowned
{
	DialogMgr::DialogMgr(tgui::Gui &gui, Resources &resources, GameState &state) :
		_gui(gui),
		_resources(resources),
		_state(state)
	{
	}

	const std::map<std::string, std::string (DialogMgr::*)(const std::vector<std::string> &)> DialogMgr::_commands{
		{"",                   &DialogMgr::_dispPercent},
		{"skip",               &DialogMgr::_skipCmd},
		{"setMusic",           &DialogMgr::_setMusic},
		{"playSfx",            &DialogMgr::_playSfx},
		{"setSprite",          &DialogMgr::_setSpriteCmd},
		{"finish",             &DialogMgr::_finishCmd},
		{"choices",            &DialogMgr::_choicesCmd},
		{"setFlag",            &DialogMgr::_setFlagCmd},
		{"unsetFlag",          &DialogMgr::_unsetFlagCmd},
		{"wait",               &DialogMgr::_wait},
		{"hide",               &DialogMgr::_hideCmd},
		{"unhide",             &DialogMgr::_unhideCmd},
		{"buttons",            &DialogMgr::_buttonsPlaceCmd},
		{"skipWeek",           &DialogMgr::_skipWeekCmd},
		{"changeRelationship", &DialogMgr::_changeHappiness},
		{"moveSprite",         &DialogMgr::_moveSprite},
		{"setBackground",      &DialogMgr::_setBackground},
		{"if",                 &DialogMgr::_if},
		{"moveBackground",     &DialogMgr::_moveBg},
	};

	void DialogMgr::update()
	{
		if (this->isDone() || this->_lineEnded || this->_onHold)
			return;

		if (!this->_newBackground.empty()) {
			auto pic = this->_gui.get<tgui::Picture>("Picture1");

			if (this->_waitingTime > 30)
				pic->getRenderer()->setOpacity((this->_waitingTime - 30) / 30.f);
			else if (this->_waitingTime == 30)
				pic->getRenderer()->setTexture(this->_resources.textures.at(this->_newBackground));
			else if (!this->_waitingTime)
				this->_newBackground.clear();
			else
				pic->getRenderer()->setOpacity((30 - this->_waitingTime) / 30.f);
		}

		if (this->_waitingTime) {
			this->_waitingTime--;
			return;
		}

		this->_processTextCharacter();
	}

	void DialogMgr::startDialog(const std::string &id)
	{
		logger.debug("Starting new dialog " + id);
		this->_done = this->_dialogsString[id].empty();
		if (this->_done)
			return;

		auto leftText = this->_gui.get<tgui::Panel>("otherPanel")->get<tgui::TextArea>("otherTextBox");
		auto rightText = this->_gui.get<tgui::Panel>("myPanel")->get<tgui::TextArea>("myTextBox");
		auto skipButton = this->_gui.get<tgui::Button>("Next");
		auto &dialogMap = std::get<0>(this->_currentDialog);
		auto &dialog = std::get<1>(this->_currentDialog);
		auto &textPos = std::get<2>(this->_currentDialog);

		leftText->setText("");
		rightText->setText("");
		for (int i = 0; i < 5; i++) {
			auto but = this->_gui.get<tgui::Button>("Button" + std::to_string(i));
			but->onPress.disconnectAll();
			but->setVisible(false);
		}
		dialogMap = id;
		dialog = 0;
		textPos = 1;
		this->_text.clear();
		this->_skippedWeek = false;
		this->_left = this->_dialogsString[dialogMap][dialog][0] == 'l';
		this->_onHold = false;
		this->_lineEnded = this->_dialogsString[id][0].empty();
		skipButton->onPress.disconnectAll();
		skipButton->onPress.connect(&DialogMgr::clicked, this);
	}

	bool DialogMgr::isDone() const
	{
		return this->_done;
	}

	void DialogMgr::_nextLine()
	{
		auto &dialogMap = std::get<0>(this->_currentDialog);
		auto &dialog = std::get<1>(this->_currentDialog);
		auto &textPos = std::get<2>(this->_currentDialog);

		logger.debug("Loading next line");
		dialog++;
		this->_text.clear();
		this->_done = dialog >= this->_dialogsString[dialogMap].size();
		if (!this->_done) {
			this->_lineEnded = this->_dialogsString[dialogMap][dialog].size() == 1;
			this->_left = this->_dialogsString[dialogMap][dialog][0] == 'l';
			textPos = 1;
		} else
			logger.debug("Done !");
	}

	void DialogMgr::loadFile(const std::string &path)
	{
		std::ifstream stream{path};
		nlohmann::json json;

		if (stream.fail())
			throw FileNotFoundException(reinterpret_cast<const std::string &&>(path));
		stream >> json;
		stream.close();

		if (!json.is_object())
			throw InvalidDialogFileException("Dialog file doesn't contain an object");

		this->_dialogsString = json.get<std::map<std::string, std::vector<std::string>>>();
		for (const auto &pair : this->_dialogsString) {
			for (const auto &dialog : pair.second) {
				if (dialog.empty())
					throw InvalidDialogStringException("In dialog chunk '" + pair.first + "': The side character is missing (Empty dialog)");
				if (dialog[0] != 'l' && dialog[0] != 'r')
					throw InvalidDialogStringException("In dialog chunk '" + pair.first + "': The side character is missing (" + dialog[0] + " is not a valid side)");
				for (size_t pos = 1; pos < dialog.size(); pos++) {
					if (dialog[pos] == '%') {
						auto parsed  = DialogMgr::_parseCommand(pos, dialog);

						if (DialogMgr::_commands.find(parsed.first) == DialogMgr::_commands.end())
							throw InvalidDialogStringException("In dialog chunk '" + pair.first + "': Command '" + parsed.first + "' doesn't exist.");
					}
				}
			}
		}
	}

	std::pair<std::string, std::vector<std::string>> DialogMgr::_parseCommand(size_t &pos, const std::string &cmdStart)
	{
		std::string command;
		std::string token;
		std::vector<std::string> args;
		size_t strStart = 0;
		size_t start = pos;
		char sep = 0;

		++pos;
		while (pos < cmdStart.size() && (sep || cmdStart[pos] != '%')) {
			if (cmdStart[pos] == '\'' || cmdStart[pos] == '"') {
				if (!sep) {
					sep = cmdStart[pos];;
					strStart = pos;
				} else if (cmdStart[pos] == sep)
					sep = 0;
			} else if (std::isspace(cmdStart[pos]) && !sep) {
				if (!token.empty()) {
					if (command.empty())
						command = token;
					else
						args.push_back(token);
					token.clear();
				}
			} else
				token += cmdStart[pos];
			++pos;
		}

		if (!token.empty()) {
			if (command.empty())
				command = token;
			else
				args.push_back(token);
			token.clear();
		}

		if (sep)
			throw UnfinishedStringLiteralException("Unfinished command in string '" + cmdStart.substr(strStart) + "'");

		if (cmdStart[pos] != '%')
			throw UnfinishedCommandException("Unfinished command near string '" + cmdStart.substr(start) + "'");

		return {
			command,
			args
		};
	}

	std::string DialogMgr::_notImplemented(const std::vector<std::string> &)
	{
		throw NotImplementedException();
	}

	void DialogMgr::clicked()
	{
		logger.debug("Skipping");
		if (this->isDone() || this->_onHold)
			return;
		if (this->_lineEnded)
			this->_nextLine();
		else while (!this->_lineEnded)
			this->_processTextCharacter();
	}

	bool DialogMgr::hasDialog(const std::string &id)
	{
		return this->_dialogsString.find(id) != this->_dialogsString.end();
	}

	std::string DialogMgr::_skipCmd(const std::vector<std::string> &args)
	{
		if (!args.empty())
			throw InvalidArgumentsException("Expected no argument.");
		this->_nextLine();
		std::get<2>(this->_currentDialog) = 0;
		return {};
	}

	std::string DialogMgr::_setSpriteCmd(const std::vector<std::string> &args)
	{
		tgui::Picture::Ptr pic;

		if (args.size() != 1 && args.size() != 5)
			throw InvalidArgumentsException("Expected a single argument or exactly 5 arguments");

		std::string sprite = args[0];
		std::string playerSprite =
			std::find(this->_state.flags.begin(), this->_state.flags.end(), "player_f") != this->_state.flags.end() ?
			"princess" : "prince";

		for (size_t pos = sprite.find("player"); pos != std::string::npos; pos = sprite.find("player"))
			sprite.replace(pos, 6, playerSprite);
		if (this->_left)
			pic = this->_gui.get<tgui::Picture>("Picture3");
		else
			pic = this->_gui.get<tgui::Picture>("Picture2");

		if (args.size() == 1)
			pic->getRenderer()->setTexture(this->_resources.textures.at(sprite));
		else
			pic->getRenderer()->setTexture(tgui::Texture(
				this->_resources.textures.at(sprite),
				{
					static_cast<unsigned int>(std::stoi(args[1])),
					static_cast<unsigned int>(std::stoi(args[2])),
					static_cast<unsigned int>(std::stoi(args[3])),
					static_cast<unsigned int>(std::stoi(args[4])),
				}
			));
		return {};
	}

	void DialogMgr::_processTextCharacter()
	{
		auto &dialogMap = std::get<0>(this->_currentDialog);
		auto &dialog = std::get<1>(this->_currentDialog);
		auto &textPos = std::get<2>(this->_currentDialog);
		tgui::TextArea::Ptr textBox;

		if (this->_left)
			textBox = this->_gui.get<tgui::Panel>("otherPanel")->get<tgui::TextArea>("otherTextBox");
		else
			textBox = this->_gui.get<tgui::Panel>("myPanel")->get<tgui::TextArea>("myTextBox");

		char c = this->_dialogsString[dialogMap][dialog][textPos];

		if (c == '%') {
			auto cmd = DialogMgr::_parseCommand(textPos, this->_dialogsString[dialogMap][dialog]);

			logger.debug("Execute command " + cmd.first);
			try {
				this->_text += (this->*DialogMgr::_commands.at(cmd.first))(cmd.second);
			} catch (const std::exception &e) {
				this->_text += "Exception '" + getLastExceptionName() + "' thrown when trying to run command '";
				this->_text += cmd.first + "' (" + std::to_string(cmd.second.size()) + " argument(s)" + (cmd.second.empty() ? "" : ": ");
				for (size_t i = 0; i < cmd.second.size(); i++) {
					if (i)
						this->_text += ", ";
					this->_text += "'" + cmd.second[i] + "'";
				}
				this->_text += std::string(") : ") + e.what();
				this->_onHold = true;
				this->_gui.get<tgui::Panel>("otherPanel")->setVisible(true);
				this->_gui.get<tgui::Panel>("myPanel")->setVisible(true);
			}
		} else
			this->_text += c;
		textPos++;
		textBox->setText(this->_text);
		this->_lineEnded = textPos >= this->_dialogsString[dialogMap][dialog].size();
	}

	std::string DialogMgr::_dispPercent(const std::vector<std::string> &)
	{
		return "%";
	}

	std::string DialogMgr::_choicesCmd(const std::vector<std::string> &args)
	{
		auto disableButtons = [this]{
			for (int i = 0; i < 5; i++) {
				auto but = this->_gui.get<tgui::Button>("Button" + std::to_string(i));
				but->onPress.disconnectAll();
				but->setVisible(false);
			}
		};

		if (args.size() < 3 || args.size() > 10)
			throw InvalidArgumentsException("Expected between 3 and 10 arguments");
		for (size_t i = 0; i < args.size(); i += 2) {
			auto button = this->_gui.get<tgui::Button>("Button" + std::to_string(i / 2));

			button->setVisible(true);
			button->setText(args[i]);
			if (i + 1 < args.size()) {
				button->onPress.connect([disableButtons, this](const std::string &warp){
					this->_onHold = false;
					disableButtons();
					this->startDialog(warp);
				}, args[i + 1]);
			} else
				button->onPress.connect([disableButtons, this]{
					this->_onHold = false;
					disableButtons();
					this->_nextLine();
				});
		}
		this->_onHold = true;
		return {};
	}

	std::string DialogMgr::_hideCmd(const std::vector<std::string> &args)
	{
		tgui::Panel::Ptr pan;

		if (args.size() != 1)
			throw InvalidArgumentsException("Expected a single argument");
		if (args[0][0] == 'l')
			pan = this->_gui.get<tgui::Panel>("otherPanel");
		else
			pan = this->_gui.get<tgui::Panel>("myPanel");
		pan->setVisible(false);
		return {};
	}

	std::string DialogMgr::_unhideCmd(const std::vector<std::string> &args)
	{
		tgui::Panel::Ptr pan;

		if (args.size() != 1)
			throw InvalidArgumentsException("Expected a single argument");
		if (args[0][0] == 'l')
			pan = this->_gui.get<tgui::Panel>("otherPanel");
		else
			pan = this->_gui.get<tgui::Panel>("myPanel");
		pan->setVisible(true);
		return {};
	}

	std::string DialogMgr::_finishCmd(const std::vector<std::string> &args)
	{
		if (!args.empty())
			throw InvalidArgumentsException("Expected no argument.");
		this->_done = true;
		return {};
	}

	std::string DialogMgr::_setFlagCmd(const std::vector<std::string> &args)
	{
		if (args.size() != 1)
			throw InvalidArgumentsException("Expected a single argument.");
		this->_state.flags.push_back(args[0]);
		return {};
	}

	std::string DialogMgr::_unsetFlagCmd(const std::vector<std::string> &args)
	{
		if (args.size() != 1)
			throw InvalidArgumentsException("Expected a single argument.");
		this->_state.flags.erase(std::remove(this->_state.flags.begin(), this->_state.flags.end(), args[0]), this->_state.flags.end());
		return {};
	}

	std::string DialogMgr::_buttonsPlaceCmd(const std::vector<std::string> &args)
	{
		if (args.size() != 1)
			throw InvalidArgumentsException("Expected a single argument.");
		if (args[0][0] == 'l')
			this->_gui.get<tgui::Button>("Button0")->setPosition(10, 340);
		else
			this->_gui.get<tgui::Button>("Button0")->setPosition(660, 300);
		return {};
	}

	std::string DialogMgr::_skipWeekCmd(const std::vector<std::string> &args)
	{
		if (!args.empty())
			throw InvalidArgumentsException("Expected no argument.");
		INCREMENT_VAR(gold);
		INCREMENT_VAR(army);
		INCREMENT_VAR(food);
		this->_state.week++;
		this->_skippedWeek = true;
		return {};
	}

	bool DialogMgr::hasSkippedWeek() const
	{
		return this->_skippedWeek;
	}

	std::string DialogMgr::_setMusic(const std::vector<std::string> &args)
	{
		if (args.size() != 1)
			throw InvalidArgumentsException("Expected a single argument.");
		this->_resources.playMusic(args[0]);
		return {};
	}

	std::string DialogMgr::_playSfx(const std::vector<std::string> &args)
	{
		if (args.size() != 1)
			throw InvalidArgumentsException("Expected a single argument.");
		this->_resources.playSound(args[0]);
		return {};
	}

	std::string DialogMgr::_wait(const std::vector<std::string> &args)
	{
		if (args.size() != 1)
			throw InvalidArgumentsException("Expected a single argument.");
		this->_waitingTime = std::stoul(args[0]);
		return {};
	}

	std::string DialogMgr::_changeHappiness(const std::vector<std::string> &args)
	{
		if (args.size() != 2)
			throw InvalidArgumentsException("Expected exactly 2 arguments.");
		switch (args[0][0]) {
		case 't':
			this->_state.goldHappiness += std::stoi(args[1]);
			break;
		case 'p':
			this->_state.foodHappiness += std::stoi(args[1]);
			break;
		case 'n':
			this->_state.armyHappiness += std::stoi(args[1]);
			break;
		}
		return {};
	}

	std::string DialogMgr::_moveSprite(const std::vector<std::string> &args)
	{
		if (args.size() != 2)
			throw InvalidArgumentsException("Expected exactly 2 arguments.");

		tgui::Picture::Ptr pic;

		if (this->_left)
			pic = this->_gui.get<tgui::Picture>("Picture3");
		else
			pic = this->_gui.get<tgui::Picture>("Picture2");

		pic->setPosition(tgui::Layout(args[0]), tgui::Layout(args[1]));
		return {};
	}

	std::string DialogMgr::_setBackground(const std::vector<std::string> &args)
	{
		if (args.size() != 1 && args.size() != 2)
			throw InvalidArgumentsException("Expected 1 or 2 arguments.");

		if (args.size() == 1 || args[1] != "false") {
			this->_newBackground = args[0];
			this->_waitingTime = 60;
			(void)this->_resources.textures.at(this->_newBackground);
		} else
			this->_gui.get<tgui::Picture>("Picture1")->getRenderer()->setTexture(this->_resources.textures.at(args[0]));
		return {};
	}

	std::string DialogMgr::_if(const std::vector<std::string> &args)
	{
		if (args.size() != 2)
			throw InvalidArgumentsException("Expected exactly 2 arguments.");
		if (std::find(this->_state.flags.begin(), this->_state.flags.end(), args[0]) != this->_state.flags.end()) {
			this->startDialog(args[1]);
			std::get<2>(this->_currentDialog) = 0;
		}
		return {};
	}

	std::string DialogMgr::_moveBg(const std::vector<std::string> &args)
	{
		if (args.size() != 2)
			throw InvalidArgumentsException("Expected exactly 2 arguments.");

		this->_gui.get<tgui::Picture>("Picture1")->setPosition(tgui::Layout(args[0]), tgui::Layout(args[1]));
		return {};
	}
}