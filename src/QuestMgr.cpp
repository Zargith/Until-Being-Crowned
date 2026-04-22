//
// Created by andgel on 20/10/2020.
//

#include <filesystem>
#include <iostream>
// #include <TGUI/Widgets/TextBox.hpp>
#include <TGUI/Widgets/TextArea.hpp>
#include <utility>
#include "QuestMgr.hpp"
#include "Exceptions.hpp"
#include "Resources/Game.hpp"

#define NB_BUTTONS 1

namespace UntilBeingCrowned
{
	std::string jsonToString(const nlohmann::json& val, const std::string& index)
	{
		std::stringstream s;

		if (val.contains(index)) {
			s << val[index].dump();
		} else {
			s << '"' << index << "\" not found !";
		}
		s << " in value " << val.dump();
		return s.str();
	}

	QuestMgr::QuestMgr(GameState &state) :
		_state(state)
	{
		this->_panel = tgui::ScrollablePanel::create({1300, 700});
		this->_panel->getRenderer()->setBackgroundColor("transparent");
		this->_panel->setPosition("&.w / 2 - w / 2", "&.h / 2 - h / 2");
		this->_panel->loadWidgetsFromFile("gui/quest.gui");
	}

	void QuestMgr::loadFile(const std::string &path, Resources &resources)
	{
		std::ifstream stream{path};
		nlohmann::json val;

		if (!stream)
			throw InvalidQuestFileException("Cannot open " + path + ": " + strerror(errno));

		stream >> val;
		this->_checkJsonValidity(val, resources.textures);
		for (const auto &v : val)
			this->_quests.push_back(std::make_shared<Quest>(this->_quests.size(), v, resources.textures));
		stream.close();
		this->_usedQuests.resize(this->_quests.size(), false);
		this->_addNewUnlockedQuests();
	}

	void QuestMgr::showDialog(unsigned int id, tgui::Gui &gui)
	{
		auto panel = tgui::Panel::create({"100%", "100%"});
		unsigned index = 0;
		auto val = this->_quests.at(id);
		int size = val->buttons.size();
		auto title = this->_panel->get<tgui::Label>("Title");
		auto desc = this->_panel->get<tgui::TextArea>("TextBox1");
		auto fct = [this, val, id, &gui, panel](unsigned butId) {
			if (this->_onClickButton)
				this->_onClickButton({*val, butId, id});

			this->_newQuests.erase(
				std::remove(
					this->_newQuests.begin(),
					this->_newQuests.end(),
					val
				),
				this->_newQuests.end()
			);
			if (butId < val->buttons_effects.size()) {
				val->buttons_effects[butId].apply(this->_state);
				this->_unlockedQuests.erase(
					std::remove(
						this->_unlockedQuests.begin(),
						this->_unlockedQuests.end(),
						val
					),
					this->_unlockedQuests.end()
				);
				this->_usedQuests[val->getId()] = true;
			}
			for (auto &but : this->_buttons)
				this->_panel->remove(but);
			gui.remove(this->_panel);
			gui.remove(panel);
		};
		this->_buttons.clear();
		this->_selected = id;
		title->setText(val->title);
		desc->setText(val->description);
		desc->setVerticalScrollbarValue(0);
		this->_panel->get<tgui::Picture>("Picture1")->getRenderer()->setTexture(val->pic);
		this->_panel->get<tgui::Label>("Gold")->setText("Gold: " + std::to_string(this->_state.gold));
		this->_panel->get<tgui::Label>("Army")->setText("Army: " + std::to_string(this->_state.army));
		this->_panel->get<tgui::Label>("Food")->setText("Food: " + std::to_string(this->_state.food));
		this->_panel->get<tgui::Button>("Back")->onPress.connect([&gui, panel, this]{
			gui.remove(this->_panel);
			gui.remove(panel);
		});
		for (int y = 0; y < size; y++) {
			auto but = tgui::Button::create(val->buttons[index]);
			auto *renderer = but->getRenderer();

			if (index < val->buttons_effects.size() && !val->buttons_effects[index].canApply(this->_state))
				but->setEnabled(false);

			renderer->setBorders({0, 0, 0, 0});
			renderer->setTexture("assets/answer_text_box.png");
			renderer->setFont("assets/kenpixel.ttf");
			but->setTextSize(16);
			but->setSize(600, 30);
			but->setPosition(625, 40 * y);
			but->onPress.connect(fct, index);
			index++;
			this->_buttons.push_back(but);
			this->_panel->add(but);
		}
		panel->getRenderer()->setBackgroundColor({0, 0, 0, 175});
		gui.add(panel);
		gui.add(this->_panel);
	}

	void QuestMgr::onClick(const std::function<void(const ClickEvent &event)> &handler)
	{
		this->_onClickButton = handler;
	}

	std::vector<std::shared_ptr<QuestMgr::Quest>> QuestMgr::getUnlockedQuests()
	{
		return this->_unlockedQuests;
	}

	std::vector<std::shared_ptr<QuestMgr::Quest>> QuestMgr::getNewQuests()
	{
		return this->_newQuests;
	}

	void QuestMgr::nextWeek()
	{
		if (!this->_continued) {
			this->_newQuests.clear();
			this->_unlockedQuests.erase(
				std::remove_if(
					this->_unlockedQuests.begin(),
					this->_unlockedQuests.end(),
					[this](const std::shared_ptr<Quest> &quest) {
						return !quest->isUnlocked(this->_state);
					}
				),
				this->_unlockedQuests.end()
			);
			this->_addNewUnlockedQuests();
		} else
			for (auto &quest : this->_quests) {
				if (
					quest->isUnlocked(this->_state) &&
					!this->_usedQuests[quest->getId()] &&
					std::find(this->_unlockedQuests.begin(), this->_unlockedQuests.end(), quest) == this->_unlockedQuests.end()
				)
					this->_unlockedQuests.push_back(quest);
			}
		this->_continued = false;
	}

	void QuestMgr::_checkJsonValidity(const nlohmann::json &val, const std::map<std::string, sf::Texture> &textures)
	{
		if (!val.is_array())
			throw InvalidQuestFileException("File is expected to contain an array of objects (Value " + val.dump() + " is not an array)");
		if (val.empty())
			throw InvalidQuestFileException("Array is empty");
		for (size_t i = 0; i < val.size(); i++) {
			const auto &v = val[i];

			if (!v.is_object())
				throw InvalidQuestFileException(i, "Element in array is not an object (Value " + v.dump() + " is not an array)");
			if (!v.contains("title") || !v["title"].is_string())
				throw InvalidQuestFileException(i, "title field is not a string (Value " + jsonToString(v, "title") + " is not a string)");
			if (!v.contains("description") || !v["description"].is_string())
				throw InvalidQuestFileException(i, "description field is not a string (Value " + jsonToString(v, "description") + " is not a string)");
			if (!v.contains("picture") || !v["picture"].is_string())
				throw InvalidQuestFileException(i, "picture field is not a string (Value " + jsonToString(v, "picture") + " is not a string)");
			if (textures.find(v["picture"]) == textures.end())
				throw InvalidQuestFileException(i, "no texture with id \"" + v["picture"].get<std::string>() + "\" has been loaded");
			if (!v.contains("buttons") || !v["buttons"].is_array())
				throw InvalidQuestFileException(i, "button field is not an array (Value " + jsonToString(v, "buttons") + " is not an array)");
			for (const auto &k : v["buttons"])
				if (!k.is_string())
					throw InvalidQuestFileException(i, "button field contains a non string element (Value " + k.dump() + " is not a string)");

			if (!v.contains("requirements") || !v["requirements"].is_array())
				throw InvalidQuestFileException(i, "requirements field is not an array (Value " + jsonToString(v, "requirement") + " is not an arra)y");
			for (const auto &k : v["requirements"])
				if (!k.is_string())
					throw InvalidQuestFileException(i, "requirements field contains a non string element (Value " + k.dump() + " is not a string)");
			if (!v.contains("week_no") || !v["week_no"].is_number_unsigned())
				throw InvalidQuestFileException(i, "week_no field is not an unsigned integer");
			if (!v.contains("happiness_requirement") || !v["happiness_requirement"].is_object())
				throw InvalidQuestFileException(i, "happiness_requirement field is not an object (Value " + jsonToString(v, "happiness_requirement") + " is not an object)");
			if (!v["happiness_requirement"].contains("traders") || !v["happiness_requirement"]["traders"].is_object())
				throw InvalidQuestFileException(i, "happiness_requirement.traders field is not an object (Value " + jsonToString(v["happiness_requirement"], "traders") + " is not an object)");
			if (!v["happiness_requirement"].contains("nobility") || !v["happiness_requirement"]["nobility"].is_object())
				throw InvalidQuestFileException(i, "happiness_requirement.nobility field is not an object (Value " + jsonToString(v["happiness_requirement"], "nobility") + " is not an object)");
			if (!v["happiness_requirement"].contains("peasants") || !v["happiness_requirement"]["peasants"].is_object())
				throw InvalidQuestFileException(i, "happiness_requirement.peasants field is not an object (Value " + jsonToString(v["happiness_requirement"], "peasants") + " is not an object)");

			if (!v["happiness_requirement"]["traders" ].contains("min") || !v["happiness_requirement"]["traders"]["min"].is_number_integer())
				throw InvalidQuestFileException(i, "happiness_requirement.traders.min field is not an integer (Value " + jsonToString(v["happiness_requirement"]["traders"], "min") + " is not an integer)");
			if (!v["happiness_requirement"]["traders" ].contains("max") || !v["happiness_requirement"]["traders"]["max"].is_number_integer())
				throw InvalidQuestFileException(i, "happiness_requirement.traders.max field is not an integer (Value " + jsonToString(v["happiness_requirement"]["traders"], "max") + " is not an integer)");
			if (!v["happiness_requirement"]["nobility"].contains("min") || !v["happiness_requirement"]["nobility"]["min"].is_number_integer())
				throw InvalidQuestFileException(i, "happiness_requirement.nobility.min field is not an integer (Value " + jsonToString(v["happiness_requirement"]["nobility"], "min") + " is not an integer)");
			if (!v["happiness_requirement"]["nobility"].contains("max") || !v["happiness_requirement"]["nobility"]["max"].is_number_integer())
				throw InvalidQuestFileException(i, "happiness_requirement.nobility.max field is not an integer (Value " + jsonToString(v["happiness_requirement"]["nobility"], "max") + " is not an integer)");
			if (!v["happiness_requirement"]["peasants"].contains("min") || !v["happiness_requirement"]["peasants"]["min"].is_number_integer())
				throw InvalidQuestFileException(i, "happiness_requirement.peasants.min field is not an integer (Value " + jsonToString(v["happiness_requirement"]["peasants"], "min") + " is not an integer)");
			if (!v["happiness_requirement"]["peasants"].contains("max") || !v["happiness_requirement"]["peasants"]["max"].is_number_integer())
				throw InvalidQuestFileException(i, "happiness_requirement.peasants.max field is not an integer (Value " + jsonToString(v["happiness_requirement"]["peasants"], "max") + " is not an integer)");

			if (!v.contains("expire_time") || !v["expire_time"].is_number_integer())
				throw InvalidQuestFileException(i, "expire_time field is not an integer (Value " + jsonToString(v, "expire_time") + " is not an integer)");
			if (!v.contains("buttons_effects") || !v["buttons_effects"].is_array())
				throw InvalidQuestFileException(i, "buttons_effects field is not an array (Value " + jsonToString(v, "buttons_effects") + " is not an array)");
			for (size_t j = 0; j < v["buttons_effects"].size(); j++) {
				const auto &k = v["buttons_effects"][j];

				if (!k.is_object())
					throw InvalidQuestFileException(i, "In effect #"+ std::to_string(j) +": buttons_effects field contains a non object element (Value " + k.dump() + " is not an object)");
				if (!k.contains("gold") || !k["gold"].is_number_integer())
					throw InvalidQuestFileException(i, "In effect #"+ std::to_string(j) +": gold field is not an integer (Value " + jsonToString(k, "gold") + " is not an integer)");
				if (!k.contains("food") || !k["food"].is_number_integer())
					throw InvalidQuestFileException(i, "In effect #"+ std::to_string(j) +": food field is not an integer (Value " + jsonToString(k, "food") + " is not an integer)");
				if (!k.contains("army") || !k["army"].is_number_integer())
					throw InvalidQuestFileException(i, "In effect #"+ std::to_string(j) +": army field is not an integer (Value " + jsonToString(k, "army") + " is not an integer)");
				if (!k.contains("passive_gold") || !k["passive_gold"].is_number_integer())
					throw InvalidQuestFileException(i, "In effect #"+ std::to_string(j) +": passive_gold field is not an integer (Value " + jsonToString(k, "passive_gold") + " is not an integer)");
				if (!k.contains("passive_food") || !k["passive_food"].is_number_integer())
					throw InvalidQuestFileException(i, "In effect #"+ std::to_string(j) +": passive_food field is not an integer (Value " + jsonToString(k, "passive_food") + " is not an integer)");
				if (!k.contains("passive_army") || !k["passive_army"].is_number_integer())
					throw InvalidQuestFileException(i, "In effect #"+ std::to_string(j) +": passive_army field is not an integer (Value " + jsonToString(k, "passive_army") + " is not an integer)");
				if (!k.contains("peasants_happiness") || !k["peasants_happiness"].is_number_integer())
					throw InvalidQuestFileException(i, "In effect #"+ std::to_string(j) +": peasants_happiness field is not an integer (Value " + jsonToString(k, "peasants_happiness") + " is not an integer)");
				if (!k.contains("nobility_happiness") || !k["nobility_happiness"].is_number_integer())
					throw InvalidQuestFileException(i, "In effect #"+ std::to_string(j) +": nobility_happiness field is not an integer (Value " + jsonToString(k, "nobility_happiness") + " is not an integer)");
				if (!k.contains("traders_happiness") || !k["traders_happiness"].is_number_integer())
					throw InvalidQuestFileException(i, "In effect #"+ std::to_string(j) +": traders_happiness field is not an integer (Value " + jsonToString(k, "traders_happiness") + " is not an integer)");
				if (!k.contains("set_flags") || !k["set_flags"].is_array())
					throw InvalidQuestFileException(i, "In effect #"+ std::to_string(j) +": set_flags field is not an array (Value " + jsonToString(k, "set_flags") + " is not an array)");
				for (auto &f : k["set_flags"])
					if (!f.is_string())
						throw InvalidQuestFileException(
							i, "In effect #"+ std::to_string(j) +
							   ": set_flags field contains a non string element (Value " + f.dump() + " is not a string)"
						);
				if (!k.contains("unset_flags") || !k["unset_flags"].is_array())
					throw InvalidQuestFileException(i, "unset_flags field is not an array (Value " + jsonToString(k, "unset_flags") + " is not an array)");
				for (auto &f : k["unset_flags"])
					if (!f.is_string())
						throw InvalidQuestFileException(
							i, "In effect #"+ std::to_string(j) +
							   ": unset_flags field contains a non string element (Value " + f.dump() + " is not a string)"
						);
			}
		}
	}

	void QuestMgr::_addNewUnlockedQuests()
	{
		for (auto &quest : this->_quests) {
			if (
				quest->isUnlocked(this->_state) &&
				!this->_usedQuests[quest->getId()] &&
				std::find(this->_unlockedQuests.begin(), this->_unlockedQuests.end(), quest) == this->_unlockedQuests.end()
			) {
				this->_unlockedQuests.push_back(quest);
				this->_newQuests.push_back(quest);
			}
		}
	}

	void QuestMgr::reset()
	{
		std::fill(this->_usedQuests.begin(), this->_usedQuests.end(), false);
	}

	QuestMgr::Quest::Quest(unsigned id, const nlohmann::json &json, const std::map<std::string, sf::Texture> &textures) :
		_id(id),
		pic(textures.at(json["picture"])),
		title(json["title"]),
		description(json["description"]),
		buttons(json["buttons"].get<std::vector<std::string>>()),
		requirements(json["requirements"].get<std::vector<std::string>>()),
		weekRange(json["week_no"], json["expire_time"]),
		buttons_effects(json["buttons_effects"].begin(), json["buttons_effects"].end()),
		tradersHappinessRequirement(json["happiness_requirement"]["traders"]["min"], json["happiness_requirement"]["traders"]["max"]),
		peasantsHappinessRequirement(json["happiness_requirement"]["peasants"]["min"], json["happiness_requirement"]["peasants"]["max"]),
		nobilityHappinessRequirement(json["happiness_requirement"]["nobility"]["min"], json["happiness_requirement"]["nobility"]["max"]),
		forceOpen(json.contains("force_open") && json["force_open"].get<bool>())
	{
		this->weekRange.second += this->weekRange.first;
	}

	bool QuestMgr::Quest::isUnlocked(const GameState &state) const
	{
		for (const auto &elem : this->requirements)
			if (std::find(state.flags.begin(), state.flags.end(), elem) == state.flags.end())
				return false;

		return this->weekRange.first <= state.week &&
		       state.week < this->weekRange.second &&

		       this->nobilityHappinessRequirement.first <= state.armyHappiness &&
		       state.armyHappiness <= this->nobilityHappinessRequirement.second &&

		       this->peasantsHappinessRequirement.first <= state.foodHappiness &&
		       state.foodHappiness <= this->peasantsHappinessRequirement.second &&

		       this->tradersHappinessRequirement.first <= state.goldHappiness &&
		       state.goldHappiness <= this->tradersHappinessRequirement.second;
	}

	bool QuestMgr::Quest::operator==(const QuestMgr::Quest &other) const
	{
		return this->_id == other._id;
	}

	unsigned int QuestMgr::Quest::getId() const
	{
		return this->_id;
	}

	QuestMgr::Effect::Effect(const nlohmann::json &json) :
		setFlags(json["set_flags"].get<std::vector<std::string>>()),
		unsetFlags(json["unset_flags"].get<std::vector<std::string>>()),
		goldChange(json["gold"]),
		foodChange(json["food"]),
		armyChange(json["army"]),
		passiveGoldChange(json["passive_gold"]),
		passiveFoodChange(json["passive_food"]),
		passiveArmyChange(json["passive_army"]),
		peasantsHappiness(json["peasants_happiness"]),
		tradersHappiness(json["traders_happiness"]),
		nobilityHappiness(json["nobility_happiness"])
	{
	}

	bool QuestMgr::Effect::canApply(const GameState &state) const
	{
		return state.gold + this->goldChange >= 0 &&
		       state.army + this->armyChange >= 0 &&
		       state.food + this->foodChange >= 0 &&
		       state.goldPassive + this->passiveGoldChange >= 0 &&
		       state.armyPassive + this->passiveArmyChange >= 0 &&
		       state.foodPassive + this->passiveFoodChange >= 0;
	}

	void QuestMgr::Effect::apply(GameState &state) const
	{
		state.gold              += this->goldChange;
		state.army              += this->armyChange;
		state.food              += this->foodChange;
		state.goldPassive       += this->passiveGoldChange;
		state.armyPassive       += this->passiveArmyChange;
		state.foodPassive       += this->passiveFoodChange;
		state.foodHappiness += this->peasantsHappiness;
		state.goldHappiness  += this->tradersHappiness;
		state.armyHappiness += this->nobilityHappiness;

		for (auto &flag : this->setFlags)
			state.flags.push_back(flag);
		for (auto &flag : this->unsetFlags)
			state.flags.erase(std::remove(state.flags.begin(), state.flags.end(), flag), state.flags.end());
	}

	std::string QuestMgr::serializedUsedQuests() {
		std::string str;
		for (auto b : this->_usedQuests) {
			str += (b ? "1\n" : "0\n");
		}
		return str;
	}

	std::string QuestMgr::serializedNewQuests() {
		std::string str;
		for (auto const &quest : this->_newQuests) {
			str += std::to_string(quest->getId()) + '\n';
		}
		return str;
	}

	std::vector<std::shared_ptr<QuestMgr::Quest>> const & QuestMgr::getQuests() const {
		return this->_quests;
	}

	void QuestMgr::setUsedQuests(std::vector<bool> q) {
		this->_usedQuests = std::move(q);
	}

	void QuestMgr::setNewQuests(std::vector<std::shared_ptr<QuestMgr::Quest>> q) {
		this->_continued = true;
		this->_newQuests = std::move(q);
	}
}