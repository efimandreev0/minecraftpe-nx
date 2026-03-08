#include "Options.h"
#include "OptionStrings.h"
#include "Minecraft.h"
#include "player/LocalPlayer.h"
#include "../platform/log.h"
#include "../world/Difficulty.h"
#include <cmath>
#include <sstream>

/*static*/
bool Options::debugGl = false;

void Options::initDefaultValues() {
	difficulty = Difficulty::NORMAL;
	hideGui = false;
	thirdPersonView = false;
	renderDebug = false;
	isFlying = false;
	smoothCamera = true;
	fixedCamera = false;
	flySpeed = 1;
	cameraSpeed = 1;
	guiScale = 0;

	useMouseForDigging = false;
	destroyVibration = true;
	isLeftHanded = false;

	isJoyTouchArea = false;

	music = 1;
	sound = 1;
	sensitivity = 0.5f;
	invertYMouse = false;
	viewDistance = 2;
	bobView = true;
	anaglyph3d = false;
	limitFramerate = false;
	fancyGraphics = true;//false;
	ambientOcclusion = false;
	if(minecraft->supportNonTouchScreen())
		useTouchScreen = false;
	else
		useTouchScreen = true;
	pixelsPerMillimeter = minecraft->platform()->getPixelsPerMillimeter();
	//useMouseForDigging = true;

	//skin     = "Default";
	username = minecraft->platform()->defaultUsername();
	serverVisible = true;

	keyUp	 = KeyMapping("key.forward", Keyboard::KEY_W);
	keyLeft  = KeyMapping("key.left", Keyboard::KEY_A);
	keyDown  = KeyMapping("key.back", Keyboard::KEY_S);
	keyRight = KeyMapping("key.right", Keyboard::KEY_D);
	keyJump  = KeyMapping("key.jump", Keyboard::KEY_SPACE);
	keyBuild = KeyMapping("key.inventory", Keyboard::KEY_E);
	keySneak = KeyMapping("key.sneak", Keyboard::KEY_LSHIFT);
	#ifndef RPI
	keyCraft = KeyMapping("key.crafting", Keyboard::KEY_Q);
	keyDrop  = KeyMapping("key.drop", Keyboard::KEY_Q);
	keyChat  = KeyMapping("key.chat", Keyboard::KEY_T);
	keyFog   = KeyMapping("key.fog", Keyboard::KEY_F);
	keyDestroy=KeyMapping("key.destroy", 88);
	keyUse   = KeyMapping("key.use", Keyboard::KEY_U);
	#endif
	#ifdef __VITA__
	keyCraft   = KeyMapping("key.crafting", Keyboard::KEY_C);
	keyUse     = KeyMapping("key.use", Keyboard::KEY_X);
	keyDestroy = KeyMapping("key.destroy", Keyboard::KEY_Z);
	#endif
	keyMenuNext     = KeyMapping("key.menu.next",     40);
	keyMenuPrevious = KeyMapping("key.menu.previous", 38);
	keyMenuOk       = KeyMapping("key.menu.ok",       13);
	keyMenuCancel   = KeyMapping("key.menu.cancel",   8);

	int k = 0;
	keyMappings[k++] = &keyUp;
	keyMappings[k++] = &keyLeft;
	keyMappings[k++] = &keyDown;
	keyMappings[k++] = &keyRight;
	keyMappings[k++] = &keyJump;
	keyMappings[k++] = &keySneak;
	keyMappings[k++] = &keyDrop;
	keyMappings[k++] = &keyBuild;
	keyMappings[k++] = &keyChat;
	keyMappings[k++] = &keyFog;
	keyMappings[k++] = &keyDestroy;
	keyMappings[k++] = &keyUse;

	keyMappings[k++] = &keyMenuNext;
	keyMappings[k++] = &keyMenuPrevious;
	keyMappings[k++] = &keyMenuOk;
	keyMappings[k++] = &keyMenuCancel;

	#if defined(ANDROID) || defined(__APPLE__) || defined(RPI)
	viewDistance = 2;
	thirdPersonView = 0;
	useMouseForDigging = false;
	fancyGraphics = true;
	ambientOcclusion = true;

	#if !defined(RPI)
	keyUp.key		= 19;
	keyDown.key		= 20;
	keyLeft.key		= 21;
	keyRight.key	= 22;
	keyJump.key		= 23;
	keyUse.key		= 103;
	keyDestroy.key	= 102;
	keyCraft.key    = 109;

	keyMenuNext.key     = 20;
	keyMenuPrevious.key = 19;
	keyMenuOk.key       = 23;
	keyMenuCancel.key   = 4;
	#endif
	#endif
}

void Options::resetDebugOptions() {
	hideGui = false;
	renderDebug = false;
	smoothCamera = false;
	fixedCamera = false;
	flySpeed = 1.0f;
	anaglyph3d = false;

	if (minecraft->player) {
		minecraft->player->noPhysics = false;
		minecraft->player->abilities.flying = false;
	}

	save();
}

const Options::Option
Options::Option::MUSIC				 (0, "options.music",		true, false),
Options::Option::SOUND				 (1, "options.sound",		true, false),
Options::Option::INVERT_MOUSE		 (2, "options.invertMouse",	false, true),
Options::Option::SENSITIVITY		 (3, "options.sensitivity",	true, false),
Options::Option::RENDER_DISTANCE	 (4, "options.renderDistance",false, false),
Options::Option::VIEW_BOBBING		 (5, "options.viewBobbing",	false, true),
Options::Option::ANAGLYPH			 (6, "options.anaglyph",		false, true),
Options::Option::LIMIT_FRAMERATE	 (7, "options.limitFramerate",false, true),
Options::Option::DIFFICULTY			 (8, "options.difficulty",	false, false),
Options::Option::GRAPHICS			 (9, "options.graphics",		false, false),
Options::Option::AMBIENT_OCCLUSION	 (10, "options.ao",		false, true),
Options::Option::GUI_SCALE			 (11, "options.guiScale",	false, false),
Options::Option::THIRD_PERSON		 (12, "options.thirdperson",	false, true),
Options::Option::HIDE_GUI			 (13, "options.hidegui",     false, true),
Options::Option::SERVER_VISIBLE		 (14, "options.servervisible", false, true),
Options::Option::LEFT_HANDED		 (15, "options.lefthanded", false, true),
Options::Option::USE_TOUCHSCREEN	 (16, "options.usetouchscreen", false, true),
Options::Option::USE_TOUCH_JOYPAD	 (17, "options.usetouchpad", false, true),
Options::Option::DESTROY_VIBRATION   (18, "options.destroyvibration", false, true),
Options::Option::PIXELS_PER_MILLIMETER(19, "options.pixelspermilimeter", true, false),
Options::Option::RENDER_DEBUG		  (20, "options.renderDebug", false, true);

const float Options::SOUND_MIN_VALUE = 0.0f;
const float Options::SOUND_MAX_VALUE = 1.0f;
const float Options::MUSIC_MIN_VALUE = 0.0f;
const float Options::MUSIC_MAX_VALUE = 1.0f;
const float Options::SENSITIVITY_MIN_VALUE = 0.0f;
const float Options::SENSITIVITY_MAX_VALUE = 1.0f;
const float Options::PIXELS_PER_MILLIMETER_MIN_VALUE = 3.0f;
const float Options::PIXELS_PER_MILLIMETER_MAX_VALUE = 4.0f;

const char* Options::RENDER_DISTANCE_NAMES[] = {
	"options.renderDistance.far",
	"options.renderDistance.normal",
	"options.renderDistance.short",
	"options.renderDistance.tiny"
};

const char* Options::DIFFICULTY_NAMES[] = {
	"options.difficulty.peaceful",
	"options.difficulty.easy",
	"options.difficulty.normal",
	"options.difficulty.hard"
};

const char* Options::GUI_SCALE[] = {
	"options.guiScale.auto",
	"options.guiScale.small",
	"options.guiScale.normal",
	"options.guiScale.large"
};

void Options::set(const Options::Option* item, float value) {
	if (item == &Options::Option::MUSIC) {
		music = value;
	} else if (item == &Options::Option::SOUND) {
		sound = value;
	} else if (item == &Options::Option::SENSITIVITY) {
		sensitivity = value;
	} else if (item == &Options::Option::RENDER_DISTANCE) {
		viewDistance = value;
	} else if (item == &Options::Option::PIXELS_PER_MILLIMETER) {
		pixelsPerMillimeter = value;
	}
	notifyOptionUpdate(item, value);
	save();
}

void Options::set(const Options::Option* item, int value) {
	if (item == &Options::Option::DIFFICULTY) {
		difficulty = value;
	} else if (item == &Options::Option::GUI_SCALE) {
		guiScale = value;
	}
	notifyOptionUpdate(item, value);
	save();
}


void Options::update() {
	viewDistance = 2;
	sensitivity = 0.5f;
	StringVector optionStrings = optionsFile.getOptionStrings();
	for (unsigned int i = 0; i < optionStrings.size(); i += 2) {
		const std::string& key = optionStrings[i];
		const std::string& value = optionStrings[i+1];

		//LOGI("reading key: %s (%s)\n", key.c_str(), value.c_str());

		// Multiplayer
		if (key == OptionStrings::Multiplayer_Username) username = value;
		if (key == OptionStrings::Multiplayer_ServerVisible) readBool(value, serverVisible);

		// Controls
		if (key == OptionStrings::Controls_Sensitivity) {
			readFloat(value, sensitivity);
		}
		if (key == OptionStrings::Controls_InvertMouse) {
			readBool(value, invertYMouse);
		}
		if (key == OptionStrings::Controls_IsLefthanded) {
			readBool(value, isLeftHanded);
		}
		if (key == OptionStrings::Controls_UseTouchJoypad) {
			readBool(value, isJoyTouchArea);
			if (!minecraft->useTouchscreen())
				isJoyTouchArea = false;
		}

		// Feedback
		if (key == OptionStrings::Controls_FeedbackVibration)
			readBool(value, destroyVibration);

		// Graphics
		if (key == OptionStrings::Graphics_Fancy) {
			readBool(value, fancyGraphics);
		}
		if (key == OptionStrings::Graphics_LowQuality) {
			bool isLow;
			readBool(value, isLow);
			if (isLow) {
				viewDistance = 3;
				fancyGraphics = false;
			}
		}
		// Game
		if (key == OptionStrings::Game_DifficultyLevel) {
			readInt(value, difficulty);
			// Only support peaceful and normal right now
			if (difficulty != Difficulty::PEACEFUL && difficulty != Difficulty::NORMAL)
				difficulty = Difficulty::NORMAL;
		}

		// new settings :

		if (key == OptionStrings::Graphics_Debug) readBool(value, renderDebug);
		if (key == OptionStrings::Graphics_RenderDistance) readInt(value, viewDistance);
		if (key == OptionStrings::Audio_Music) readFloat(value, music);
		if (key == OptionStrings::Audio_Sound) readFloat(value, sound);
		if (key == OptionStrings::Game_HideGui) readBool(value, hideGui);
		if (key == OptionStrings::Game_ThirdPersonView) readBool(value, thirdPersonView);
		if (key == OptionStrings::Graphics_AmbientOcclusion) readBool(value, ambientOcclusion);
		if (key == OptionStrings::Game_ViewBobbing) readBool(value, bobView);
		if (key == OptionStrings::Graphics_Anaglyph3d) readBool(value, anaglyph3d);
		if (key == OptionStrings::Graphics_LimitFramerate) readBool(value, limitFramerate);
	}
}

void Options::load() {
}

void Options::save() {
	StringVector stringVec;
	addOptionToSaveOutput(stringVec, OptionStrings::Multiplayer_Username, username);
	addOptionToSaveOutput(stringVec, OptionStrings::Multiplayer_ServerVisible, serverVisible);

	addOptionToSaveOutput(stringVec, OptionStrings::Controls_InvertMouse, invertYMouse);
	addOptionToSaveOutput(stringVec, OptionStrings::Controls_Sensitivity, sensitivity);
	addOptionToSaveOutput(stringVec, OptionStrings::Controls_IsLefthanded, isLeftHanded);
	addOptionToSaveOutput(stringVec, OptionStrings::Controls_UseTouchScreen, useTouchScreen);
	addOptionToSaveOutput(stringVec, OptionStrings::Controls_UseTouchJoypad, isJoyTouchArea);
	addOptionToSaveOutput(stringVec, OptionStrings::Controls_FeedbackVibration, destroyVibration);

	addOptionToSaveOutput(stringVec, OptionStrings::Graphics_Debug, renderDebug);
	addOptionToSaveOutput(stringVec, OptionStrings::Graphics_RenderDistance, viewDistance);
	addOptionToSaveOutput(stringVec, OptionStrings::Graphics_Fancy, fancyGraphics);
	addOptionToSaveOutput(stringVec, OptionStrings::Graphics_AmbientOcclusion, ambientOcclusion);
	addOptionToSaveOutput(stringVec, OptionStrings::Graphics_Anaglyph3d, anaglyph3d);
	addOptionToSaveOutput(stringVec, OptionStrings::Graphics_LimitFramerate, limitFramerate);

	addOptionToSaveOutput(stringVec, OptionStrings::Audio_Music, music);
	addOptionToSaveOutput(stringVec, OptionStrings::Audio_Sound, sound);

	addOptionToSaveOutput(stringVec, OptionStrings::Game_DifficultyLevel, difficulty);
	addOptionToSaveOutput(stringVec, OptionStrings::Game_HideGui, hideGui);
	addOptionToSaveOutput(stringVec, OptionStrings::Game_ThirdPersonView, thirdPersonView);
	addOptionToSaveOutput(stringVec, OptionStrings::Game_ViewBobbing, bobView);

	optionsFile.save(stringVec);
}

void Options::addOptionToSaveOutput(StringVector& stringVector, std::string name, bool boolValue) {
	std::stringstream ss;
	ss << name << ":" << boolValue;
	stringVector.push_back(ss.str());
}
void Options::addOptionToSaveOutput(StringVector& stringVector, std::string name, float floatValue) {
	std::stringstream ss;
	ss << name << ":" << floatValue;
	stringVector.push_back(ss.str());
}
void Options::addOptionToSaveOutput(StringVector& stringVector, std::string name, int intValue) {
	std::stringstream ss;
	ss << name << ":" << intValue;
	stringVector.push_back(ss.str());
}
void Options::addOptionToSaveOutput(StringVector& stringVector, std::string name, const std::string& strValue) {
	std::stringstream ss;
	ss << name << ":" << strValue;
	stringVector.push_back(ss.str());
}

std::string Options::getMessage(const Option* item) {
	if (item == &Options::Option::RENDER_DISTANCE) {
		return RENDER_DISTANCE_NAMES[viewDistance];
	}
	if (item == &Options::Option::DIFFICULTY) {
		int diff = difficulty;
		if (diff != Difficulty::PEACEFUL && diff != Difficulty::NORMAL) diff = Difficulty::NORMAL; // Basic validation
		return DIFFICULTY_NAMES[diff];
	}
	if (item == &Options::Option::GRAPHICS) {
		return fancyGraphics ? "options.graphics.fancy" : "options.graphics.fast";
	}
	if (item == &Options::Option::GUI_SCALE) {
		return GUI_SCALE[guiScale];
	}

	if (item->isBoolean()) {
		return getBooleanValue(item) ? "options.on" : "options.off";
	}

	return "Options::getMessage - Not implemented";
}

bool Options::readFloat(const std::string& string, float& value) {
	if (string == "true" || string == "1" || string == "YES")  { value = 1.0f; return true; }
	if (string == "false" || string == "0" || string == "NO") { value = 0.0f; return true; }
	return sscanf(string.c_str(), "%f", &value) == 1;
}

bool Options::readInt(const std::string& string, int& value) {
	if (string == "true" || string == "1" || string == "YES")  { value = 1; return true; }
	if (string == "false" || string == "0" || string == "NO") { value = 0; return true; }
	return sscanf(string.c_str(), "%d", &value) == 1;
}

bool Options::readBool(const std::string& string, bool& value) {
	std::string s = Util::stringTrim(string);
	if (s == "true" || s == "1" || s == "YES")  { value = true;  return true; }
	if (s == "false" || s == "0" || s == "NO") { value = false; return true; }
	return false;
}

void Options::notifyOptionUpdate( const Option* option, bool value ) {
	minecraft->optionUpdated(option, value);
}
void Options::notifyOptionUpdate( const Option* option, float value ) {
	minecraft->optionUpdated(option, value);
}
void Options::notifyOptionUpdate( const Option* option, int value ) {
	minecraft->optionUpdated(option, value);
}
