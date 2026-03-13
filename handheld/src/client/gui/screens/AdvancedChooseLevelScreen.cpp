#include "ProgressScreen.h"
#include "../../Minecraft.h"
#include "AdvancedChooseLevelScreen.h"

static char ILLEGAL_FILE_CHARACTERS[] = {
    '/', '\n', '\r', '\t', '\0', '\f', '`', '?', '*', '\\', '<', '>', '|', '\"', ':'
};

AdvancedChooseLevelScreen::AdvancedChooseLevelScreen()
:   bHeader(0, "World name"),
    bBack(1, "Back"),
    bStart(2, "Start!"),
    bLevelName(3, "Unnamed world"),
    bGameMode(4, "Gamemode"),
    bSeed(5, "")
{
}

AdvancedChooseLevelScreen::~AdvancedChooseLevelScreen()
{
}

void AdvancedChooseLevelScreen::init() {
    ChooseLevelScreen::init();

    buttons.push_back(&bHeader);
    buttons.push_back(&bBack);
    buttons.push_back(&bStart);
    buttons.push_back(&bLevelName);
    buttons.push_back(&bGameMode);
    buttons.push_back(&bSeed);
}

void AdvancedChooseLevelScreen::setupPositions() {

    int padding = 30;

    bBack.x = 0;
    bBack.y = 0;

    bStart.y = 0;
    bStart.x = width - bStart.width;

    bHeader.x = bBack.width;
    bHeader.width = width - (bBack.width + bStart.width);
    bHeader.height = bStart.height;

    bLevelName.height = bHeader.height;
    bLevelName.width = width - (padding*2);
    bLevelName.x = padding;
    bLevelName.y = (bHeader.height + (height/3 - 50));

    bGameMode.x = (width - bGameMode.width) / 2;
    bGameMode.y = (2*height/3 - 50);

    bSeed.width = width - (padding*2);
    bSeed.height = bHeader.height;
    bSeed.x = padding;
    bSeed.y = (height - 50);
}

void AdvancedChooseLevelScreen::tick() {
    bGameMode.msg = (this->gameType == GameType::Creative) ? "Creative" : "Survival";
}

void AdvancedChooseLevelScreen::render( int xm, int ym, float a )
{
    renderDirtBackground(0);
    glEnable2(GL_BLEND);

    drawCenteredString(minecraft->font, (this->gameType == GameType::Creative) ? "Unlimited resources and flying" : "Mobs, health and gather resources", width/2, bGameMode.y + bGameMode.height + 3, 0xffcccccc);
    drawCenteredString(minecraft->font, "World Generator seed, Leave blank for random.", width/2, bSeed.y - 10, 0xffcccccc);
    drawCenteredString(minecraft->font, "Leave blank for random seed", width/2, bSeed.y + bSeed.height + 3, 0xffcccccc);

    Screen::render(xm, ym, a);
    glDisable2(GL_BLEND);
}

void AdvancedChooseLevelScreen::buttonClicked(Button* button) {
    if(button == &bBack) {
        minecraft->screenChooser.setScreen(SCREEN_SELECTWORLD);
        return;
    }

    if(button == &bGameMode) {
        // swap game types
        if(this->gameType == GameType::Creative) {
            this->gameType = GameType::Survival;
        } else {
            this->gameType = GameType::Creative;
        }
        return;
    }

    if(button == &bStart) {

        // Read the level name.
        // 1) Trim name 2) Remove all bad chars 3) Append '-' chars 'til the name is unique
        std::string levelName = bLevelName.text;
        std::string levelId = levelName;

        for (int i = 0; i < sizeof(ILLEGAL_FILE_CHARACTERS) / sizeof(char); ++i)
            levelId = Util::stringReplace(levelId, std::string(1, ILLEGAL_FILE_CHARACTERS[i]), "");
                if ((int)levelId.length() == 0) {
                    levelId = "no_name";
            }
            levelId = getUniqueLevelName(levelId);

            // Read the seed
            int seed = getEpochTimeS();
            if (!bSeed.text.empty()) {
                std::string seedString = Util::stringTrim(bSeed.text);
                if (seedString.length() > 0) {
                    int tmpSeed;
                    // Try to read it as an integer
                    if (sscanf(seedString.c_str(), "%d", &tmpSeed) > 0) {
                        seed = tmpSeed;
                    } // Hash the "seed"
                    else {
                        seed = Util::hashCode(seedString);
                    }
                }
            }
            // Read the game mode
            bool isCreative = this->gameType == GameType::Creative;

            // Start a new level with the given name and seed
            LOGI("Creating a level with id '%s', name '%s' and seed '%d'\n", levelId.c_str(), levelName.c_str(), seed);
            LevelSettings settings(seed, isCreative? GameType::Creative : GameType::Survival);
            minecraft->selectLevel(levelId, levelName, settings);
            minecraft->hostMultiplayer();
            minecraft->setScreen(new ProgressScreen());

        return;
    }
}

bool AdvancedChooseLevelScreen::handleBackEvent(bool isDown)
{
    minecraft->screenChooser.setScreen(SCREEN_SELECTWORLD);
    return true;
}

