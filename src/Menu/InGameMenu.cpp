//
// Created by andgel on 15/10/2020.
//

#include "InGameMenu.hpp"
#include "../Resources/State.hpp"
#include "../Loader.hpp"

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

#define UPDATE_LABEL(var)                                                                                                                     \
	if (                                                                                                                                  \
		std::find(                                                                                                                    \
			this->_state.flags.begin(),                                                                                           \
			this->_state.flags.end(),                                                                                             \
			"no_"#var                                                                                                             \
		) != this->_state.flags.end()                                                                                                 \
	) {                                                                                                                                   \
		this->_##var##PassiveLabel->setText("+/-0");                                                                                  \
		this->_##var##PassiveLabel->getRenderer()->setTextColor("#FF8800");                                                           \
	} else if (this->_state.var##Passive + (this->_state.var##Happiness + 1) / 2 < 0) {                                                   \
		this->_##var##PassiveLabel->setText(std::to_string(this->_state.var##Passive + (this->_state.var##Happiness + 1) / 2));       \
		this->_##var##PassiveLabel->getRenderer()->setTextColor("red");                                                               \
	} else if (this->_state.var##Passive + (this->_state.var##Happiness + 1) / 2 > 0) {                                                   \
		this->_##var##PassiveLabel->getRenderer()->setTextColor("green");                                                             \
		this->_##var##PassiveLabel->setText("+" + std::to_string(this->_state.var##Passive + (this->_state.var##Happiness + 1) / 2)); \
	} else {                                                                                                                              \
		this->_##var##PassiveLabel->getRenderer()->setTextColor("#FF8800");                                                           \
		this->_##var##PassiveLabel->setText("+/-0");                                                                                  \
	}

static const std::array<std::string, 12> _monthsNames{
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"
};

namespace UntilBeingCrowned
{
	InGameMenu::InGameMenu(MenuMgr &mgr, tgui::Gui &gui, Resources &res, QuestMgr &dialogs, GameState &state) :
		_res(res),
		_gui(gui),
		_mgr(mgr),
		_questsMgr(dialogs),
		_state(state)
	{
	}

	InGameMenu::~InGameMenu()
	{
		this->_questsMgr.onClick(nullptr);
	}

	void InGameMenu::switched(bool isActive)
	{
		if (!isActive) {
			this->_gui.removeAllWidgets();
			this->_questsMgr.onClick(nullptr);
			return;
		}
		this->_gui.loadWidgetsFromFile("gui/igmenu.gui");
		this->_goldsLabel = this->_gui.get<tgui::Label>("Gold");
		this->_armyLabel = this->_gui.get<tgui::Label>("Army");
		this->_foodLabel = this->_gui.get<tgui::Label>("Food");
		this->_goldPassiveLabel = this->_gui.get<tgui::Label>("PassiveGold");
		this->_armyPassiveLabel = this->_gui.get<tgui::Label>("PassiveArmy");
		this->_foodPassiveLabel = this->_gui.get<tgui::Label>("PassiveFood");

		auto newQuestsList = this->_gui.get<tgui::Button>("NewQuests");

		this->_gui.get<tgui::Label>("Month")->setText(_monthsNames[this->_state.week % 12]);
		newQuestsList->setVisible(!this->_questsMgr.getNewQuests().empty());
		this->_goldsLabel->setText(std::to_string(this->_state.gold));
		this->_armyLabel->setText(std::to_string(this->_state.army));
		this->_foodLabel->setText(std::to_string(this->_state.food));

		UPDATE_LABEL(gold);
		UPDATE_LABEL(army);
		UPDATE_LABEL(food);
		this->_hookHandlers();
		this->_questsMgr.nextWeek();
		for (auto &quest : this->_questsMgr.getNewQuests())
			if (quest->forceOpen)
				this->_questsMgr.showDialog(quest->getId(), this->_gui);

		this->_gui.get<tgui::Button>("Back")->onPress.connect(&InGameMenu::_saveAndQuit, this);
	}

	void InGameMenu::render()
	{
		auto itv = std::find_if(this->_state.flags.begin(), this->_state.flags.end(), [](const std::string &str){
			return str.substr(0, strlen("victory_")) == "victory_";
		});
		auto itgo = std::find_if(this->_state.flags.begin(), this->_state.flags.end(), [](const std::string &str){
			return str.substr(0, strlen("killed_")) == "killed_";
		});
		auto newQuestsList = this->_gui.get<tgui::Button>("NewQuests");
		auto nextWeek = this->_gui.get<tgui::Button>("Next");

		if (itgo != this->_state.flags.end())
			this->_mgr.changeMenu("game over");
		if (itv != this->_state.flags.end())
			this->_mgr.changeMenu("victory");
		nextWeek->setVisible(this->_questsMgr.getNewQuests().empty());
		newQuestsList->setVisible(!this->_questsMgr.getNewQuests().empty());
		this->_goldsLabel->setText(std::to_string(this->_state.gold));
		this->_armyLabel->setText(std::to_string(this->_state.army));
		this->_foodLabel->setText(std::to_string(this->_state.food));
		this->_state.foodHappiness = std::min(this->_state.foodHappiness, 40);
		this->_state.armyHappiness = std::min(this->_state.armyHappiness, 40);
		this->_state.goldHappiness = std::min(this->_state.goldHappiness, 40);
		UPDATE_LABEL(gold);
		UPDATE_LABEL(army);
		UPDATE_LABEL(food);
	}

	void InGameMenu::handleEvent(const Input::Event &)
	{

	}

	void InGameMenu::_showQuestList(const std::vector<std::shared_ptr<QuestMgr::Quest>> &quests, const std::string &name)
	{
		auto label = tgui::Label::create(name);
		auto picture = tgui::Panel::create({600, 700});
		auto panelBack = tgui::Panel::create({"100%", "100%"});
		std::weak_ptr<tgui::Panel> panelBackPtr = panelBack;
		auto panel = tgui::ScrollablePanel::create({450, 550});
		auto close = [panelBackPtr, label, picture, this]{
			this->_gui.remove(panelBackPtr.lock());
			this->_gui.remove(picture);
			this->_gui.remove(label);
		};
		auto closeButton = tgui::Button::create("OK");

		label->getRenderer()->setFont("assets/kenpixel.ttf");
		label->setTextSize(25);
		panel->getRenderer()->setBackgroundColor({0, 0, 0, 0});
		panelBack->getRenderer()->setBackgroundColor({0, 0, 0, 175});
		picture->getRenderer()->setTextureBackground(this->_res.textures["list_bg"]);
		closeButton->setPosition("&.w - w - 10", "&.h - h - 10");
		picture->setPosition("&.w / 2 - w / 2", "&.h / 2 - h / 2");
		label->setPosition("&.w / 2 - w / 2", 45);
		panel->setPosition("&.w / 2 - w / 2", "&.h / 2 - h / 2");
		picture->add(closeButton);
		closeButton->getRenderer()->setFont("assets/kenpixel.ttf");
		closeButton->onClick.connect(close);
		panelBack->onClick.connect(close);
		picture->add(panel);
		picture->add(label);
		this->_gui.add(panelBack);
		this->_gui.add(picture, "Panel");
		for (size_t i = 0; i < quests.size(); i++) {
			const auto &quest = quests[i];
			auto button = tgui::Button::create(quest->title);

			button->getRenderer()->setFont("assets/kenpixel.ttf");
			button->setPosition(10, i * 30 + 10);
			button->setSize("&.w - 20", 20);
			button->onClick.connect([this, quest, close]{
				close();
				this->_questsMgr.showDialog(quest->getId(), this->_gui);
			});
			panel->add(button);
		}
	}

	void InGameMenu::_hookHandlers()
	{
		auto newQuestsList = this->_gui.get<tgui::Button>("NewQuests");
		auto unlockedQuestsList = this->_gui.get<tgui::Button>("Quests");
		auto nextWeek = this->_gui.get<tgui::Button>("Next");

		nextWeek->onPress.connect(&InGameMenu::_nextWeek, this);
		newQuestsList->onClick.connect([this]{
			this->_res.playSound("paper");
			this->_showQuestList(this->_questsMgr.getNewQuests(), "New quests");
		});
		unlockedQuestsList->onClick.connect([this]{
			this->_res.playSound("tampon");
			this->_showQuestList(this->_questsMgr.getUnlockedQuests(), "Unlocked Quests");
		});
	}

	void InGameMenu::_nextWeek()
	{
		INCREMENT_VAR(gold);
		INCREMENT_VAR(army);
		INCREMENT_VAR(food);
		this->_state.week++;
		this->_res.playSound("inkwell");
		this->_mgr.changeMenu("dialog");
	}

	void InGameMenu::_saveAndQuit() {
		this->_res.playSound("click_button");
		Loader::saveProgression(this->_state, this->_questsMgr, "progression");
		this->_mgr.changeMenu("main");
	}
}
