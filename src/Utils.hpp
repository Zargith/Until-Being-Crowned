//
// Created by Gegel85 on 19/10/2019.
//

#ifndef THFGAME_UTILS_HPP
#define THFGAME_UTILS_HPP

#include <string>
#include <iomanip>
#include <cmath>
#include "DataType/Vector.hpp"

#ifndef _WIN32
#define MB_ICONERROR 1
#define MB_YESNO 2
#else
#include <windows.h>
#endif

namespace UntilBeingCrowned::Utils
{
	template<typename type>
	std::string toString(const Vector2<type> &vec)
	{
		return "(" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ")";
	}

	template<typename type>
	std::string toHex(type nb)
	{
		unsigned long long n = static_cast<typename std::make_unsigned<type>::type>(nb);
		std::stringstream s;

		s << std::setfill ('0') << std::setw(sizeof(nb) * 2) << std::hex << std::uppercase << n;
		return s.str();
	}

	int	dispMsg(const std::string &title, const std::string &content, int variate);

	std::string floatToString(float nb);

	void rename(const std::string &oldName, const std::string &newName);
}

#endif //THFGAME_UTILS_HPP
