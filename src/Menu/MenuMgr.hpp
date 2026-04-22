//
// Created by Gegel85 on 25/09/2019.
//

#ifndef UBC_MENUMGR_HPP
#define UBC_MENUMGR_HPP


#include <map>
#include "../Input/Input.hpp"
#include "Menu.hpp"

namespace UntilBeingCrowned
{
	struct Game;

	class MenuMgr {
	private:
		std::string _currentMenu;
		std::map<std::string, std::unique_ptr<Menu>> _menus;

	public:
		MenuMgr() = default;

		template<typename result, typename ...Args>
		result &addMenu(const std::string &id, Args &...args) {
			return reinterpret_cast<result &>(*this->_menus.emplace(id, new result(*this, args...)).first->second);
		};

		void renderMenu();

		void handleEvent(const Input::Event &event);

		void changeMenu(const std::string &newMenu);
	};
}


#endif //UBC_MENUMGR_HPP
