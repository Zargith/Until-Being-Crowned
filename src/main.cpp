//
// Created by andgel on 06/10/2020.
//

#include "Resources/Game.hpp"
#include "Rendering/Screen.hpp"
#include "Resources/Logger.hpp"
#include "Exceptions.hpp"
#include "Utils.hpp"
#include "Loader.hpp"
#include "Menu/InGameMenu.hpp"
#include "Menu/MainMenu.hpp"
#include "Menu/OptionMenu.hpp"
#include "Menu/LoadingMenu.hpp"
#include "Menu/GenderMenu.hpp"
#include "Menu/DialogMenu.hpp"
#include "Menu/VictoryMenu.hpp"
#include "Menu/GameoverMenu.hpp"

static void applyGameView(tgui::GuiSFML &gui, unsigned winW, unsigned winH)
{
	if (winW == 0 || winH == 0)
		return;
	constexpr float GAME_W = 1360.f, GAME_H = 768.f;
	float winAspect  = static_cast<float>(winW) / static_cast<float>(winH);
	float gameAspect = GAME_W / GAME_H;
	float vpX, vpY, vpW, vpH;
	if (winAspect > gameAspect) {
		vpH = static_cast<float>(winH);
		vpW = vpH * gameAspect;
		vpX = (static_cast<float>(winW) - vpW) / 2.f;
		vpY = 0.f;
	} else {
		vpW = static_cast<float>(winW);
		vpH = vpW / gameAspect;
		vpX = 0.f;
		vpY = (static_cast<float>(winH) - vpH) / 2.f;
	}
	gui.setAbsoluteViewport({vpX, vpY, vpW, vpH});
	gui.setAbsoluteView({0.f, 0.f, GAME_W, GAME_H});
}

namespace UntilBeingCrowned
{
	void init(Game &game)
	{
		UntilBeingCrowned::Loader::loadAssets(game);

		game.state.gui.setTarget(game.resources.screen);
		game.state.menuMgr.addMenu<VictoryMenu>("victory", game.resources, game.state.gui, game.state.game);
		game.state.menuMgr.addMenu<GameoverMenu>("game over", game.resources, game.state.gui, game.state.game);
		game.state.menuMgr.addMenu<InGameMenu>("in_game", game.state.gui, game.resources, game.state.questMgr, game.state.game);
		game.state.menuMgr.addMenu<DialogMenu>("dialog", game.resources, game.state.gui, game.state.dialogMgr, game.state.game);
		game.state.menuMgr.addMenu<MainMenu>("main", game.state.gui, game.resources, game);
		game.state.menuMgr.addMenu<OptionMenu>("option", game.state.gui, game.resources, game);
		game.state.menuMgr.addMenu<LoadingMenu>("load", game.state.gui, game.resources);
		game.state.menuMgr.addMenu<GenderMenu>("gender", game.state.gui, game.resources, game.state.game, game.state.questMgr);
		game.state.menuMgr.changeMenu("main");

		// Apply initial letterbox scaling
		auto sz = game.resources.screen.getSize();
		applyGameView(game.state.gui, sz.x, sz.y);
	}
}

int main()
{
	try {
	sf::Event event;
	UntilBeingCrowned::Game game;

	UntilBeingCrowned::init(game);
	sf::Vector2u lastSize{0, 0};
	while (game.resources.screen.isOpen()) {
		// Reapply letterbox whenever the window size changes (resize, fullscreen toggle, etc.)
		auto sz = game.resources.screen.getSize();
		if (sz != lastSize) {
			applyGameView(game.state.gui, sz.x, sz.y);
			lastSize = sz;
		}
		while (game.resources.screen.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				game.resources.screen.close();
			game.state.gui.handleEvent(event);
		}
		game.resources.screen.clear();
		game.state.menuMgr.renderMenu();
		game.state.gui.draw();
		game.resources.screen.display();
	}
	} catch (std::exception &e) {
		UntilBeingCrowned::logger.fatal(getLastExceptionName() + ": " + e.what());
		UntilBeingCrowned::Utils::dispMsg(
			"Fatal Error",
			"An unrecoverable error occurred\n\n" +
			getLastExceptionName() + ":\n" + e.what() + "\n\n"
			"Click OK to close the application",
			MB_ICONERROR
		);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}