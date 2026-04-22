//
// Created by andgel on 13/10/2020.
//

#ifndef UBC_RESOURCES_HPP
#define UBC_RESOURCES_HPP


#include <array>
#include <random>
#include <SFML/Audio.hpp>
#include "version.hpp"
#include "Logger.hpp"
#include "../Rendering/Screen.hpp"

#define MAX_PLAYING_SFX 64

namespace UntilBeingCrowned
{
	struct Resources {
	private:
		mutable unsigned _lastPlayedSound = 0;

	public:
		Rendering::Screen screen{
			"Until Being Crowned " UBC_VERSION_STRING, 1360, 768
		};

		sf::Image icon;

		std::mt19937 random;

		std::map<std::string, std::pair<sf::Music, std::string>> musics;

		std::map<std::string, sf::Texture> textures;

		std::map<std::string, sf::SoundBuffer> soundBuffers;

		mutable std::array<sf::Sound, MAX_PLAYING_SFX> sounds;

		sf::Font font;

		unsigned char playSound(const std::string &id);

		void playMusic(const std::string &id);

		void stopMusic();

		void setMusicVolume(float newVolume);

		float getMusicVolume() const;

		void setSoundVolume(float newVolume);

		float getSoundVolume() const;
	};
}


#endif //UBC_RESOURCES_HPP
