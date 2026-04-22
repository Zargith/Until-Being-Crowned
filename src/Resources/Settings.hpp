//
// Created by andgel on 13/10/2020.
//

#ifndef UBC_SETTINGS_HPP
#define UBC_SETTINGS_HPP


#include "../Input/Input.hpp"

namespace UntilBeingCrowned
{
	struct Settings {
		float musicVolume;

		float sfxVolume;

		bool fullscreen;

		std::unique_ptr<Input> input;
	};
}


#endif //UBC_SETTINGS_HPP
