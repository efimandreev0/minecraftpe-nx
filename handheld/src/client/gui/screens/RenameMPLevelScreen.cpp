#include "RenameMPLevelScreen.h"
#include "StartMenuScreen.h"
#include "DialogDefinitions.h"
#include "../Gui.h"
#include "../../Minecraft.h"
#include "../../../AppPlatform.h"
#include "../../../platform/log.h"
#include "../../../world/level/storage/LevelStorageSource.h"
#include "../components/ImageButton.h"
#include "../components/Button.h"
#include "../components/TextBox.h"

static char ILLEGAL_FILE_CHARACTERS[] = {
	'/', '\n', '\r', '\t', '\0', '\f', '`', '?', '*', '\\', '<', '>', '|', '\"', ':'
};

RenameMPLevelScreen::RenameMPLevelScreen( const std::string& levelId )
:	_levelId(levelId),
	bHeader(0, "Save world as"),
	bCancel(1, "Cancel"),
	bSave(2, "Save"),
	bLevelName(2, "Saved World")
{
}


RenameMPLevelScreen::~RenameMPLevelScreen()
{
}

void RenameMPLevelScreen::init() {
#if defined(__APPLE__) || defined(ANDROID)
	minecraft->platform()->createUserInput(DialogDefinitions::DIALOG_RENAME_MP_WORLD);
#else
	buttons.push_back(&bHeader);
	buttons.push_back(&bSave);
	buttons.push_back(&bCancel);
	buttons.push_back(&bLevelName);
	bLevelName.setFocus(minecraft);
#endif
}

void RenameMPLevelScreen::tick(){
	if(!bLevelName.focused) {
		buttonClicked(&bSave);
	}
}

void RenameMPLevelScreen::buttonClicked(Button* button) {
	Screen::buttonClicked(button);

	if(button == &bCancel) {
		minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
	}
	if(button == &bSave) {
		std::string levelId = bLevelName.text;

		if (!levelId.empty()) {
			// Read the level name.
			// 1) Trim name 2) Remove all bad chars -) We don't have to getUniqueLevelName, since renameLevel will do that
			for (int i = 0; i < sizeof(ILLEGAL_FILE_CHARACTERS) / sizeof(char); ++i)
				levelId = Util::stringReplace(levelId, std::string(1, ILLEGAL_FILE_CHARACTERS[i]), "");
			if ((int)levelId.length() == 0) {
				levelId = "saved_world";
			}

			minecraft->getLevelSource()->renameLevel(_levelId, levelId);
		}

		minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
	}
}

void RenameMPLevelScreen::setupPositions() {
	int padding = 10;

	bSave.y = 0;
	bSave.x = width - bSave.width;

	bCancel.x = 0;
	bCancel.y = 0;

	bHeader.x = bCancel.width;
	bHeader.width = width - (bCancel.width + bSave.width);
	bHeader.height = bSave.height;

	bLevelName.x = padding;
	bLevelName.y = bHeader.height + (padding*2);
	bLevelName.width = width - (padding*2);
	bLevelName.height = bHeader.height;
}

void RenameMPLevelScreen::render(int xm, int ym, float a)
{
	renderBackground();
	#ifdef WIN32
		minecraft->getLevelSource()->renameLevel(_levelId, "Save?Level");
		minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
	#elif !defined(__APPLE__) && !defined(ANDROID)
		//renderDirtBackground(0);
		glEnable2(GL_BLEND);

		drawCenteredString(minecraft->font, "Enter a name to save this world as:", width/2, bLevelName.y - 10, 0xffcccccc);

		Screen::render(xm, ym, a);
		glDisable2(GL_BLEND);
	#else
		int status = minecraft->platform()->getUserInputStatus();
		if (status > -1) {
			if (status == 1) {
				std::vector<std::string> v = minecraft->platform()->getUserInput();

				if (!v.empty()) {
                    // Read the level name.
					// 1) Trim name 2) Remove all bad chars -) We don't have to getUniqueLevelName, since renameLevel will do that
					std::string levelId = v[0];

					for (int i = 0; i < sizeof(ILLEGAL_FILE_CHARACTERS) / sizeof(char); ++i)
						levelId = Util::stringReplace(levelId, std::string(1, ILLEGAL_FILE_CHARACTERS[i]), "");
                    if ((int)levelId.length() == 0) {
                        levelId = "saved_world";
                    }

					minecraft->getLevelSource()->renameLevel(_levelId, levelId);
                }
			}

			minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
		}
	#endif
}
