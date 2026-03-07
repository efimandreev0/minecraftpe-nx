#include "OptionsScreen.h"

#include "StartMenuScreen.h"
#include "DialogDefinitions.h"
#include "../../Minecraft.h"
#include "../../../AppPlatform.h"
#include "../../../platform/input/Mouse.h"

#include "../components/OptionsPane.h"
#include "../components/ImageButton.h"
#include "../components/OptionsGroup.h"
OptionsScreen::OptionsScreen()
: btnClose(NULL),
  bHeader(NULL),
  btnNextPage(NULL),
  btnPrevPage(NULL),
  btnResetFov(NULL),
  optionPane(NULL) {
  	isEditingUsername = false;
  	btnUsername = NULL;
	currentPage = 0;
	maxPages = 3;
}

OptionsScreen::~OptionsScreen() {
	if(btnClose != NULL) {
		delete btnClose;
		btnClose = NULL;
	}
	if(bHeader != NULL) {
		delete bHeader;
		bHeader = NULL;
	}
	if(btnNextPage != NULL) {
		delete btnNextPage;
		btnNextPage = NULL;
	}
	if(btnPrevPage != NULL) {
		delete btnPrevPage;
		btnPrevPage = NULL;
	}
	if(btnResetFov != NULL) {
		delete btnResetFov;
		btnResetFov = NULL;
	}
	if(optionPane != NULL) {
		delete optionPane;
		optionPane = NULL;
	}
}

void OptionsScreen::init() {
	bHeader = new Touch::THeader(0, "Options");
	btnPrevPage = new Button(201, "<");
	btnNextPage = new Button(202, ">");
	btnClose = new ImageButton(1, "");
	ImageDef def;
	def.name = "gui/touchgui.png";
	def.width = 34;
	def.height = 26;

	def.setSrc(IntRectangle(150, 0, (int)def.width, (int)def.height));
	btnClose->setImageDef(def, true);

	buttons.push_back(bHeader);
	buttons.push_back(btnPrevPage);
	buttons.push_back(btnNextPage);
	buttons.push_back(btnClose);

	btnResetFov = new Button(101, "Reset FOV");
	buttons.push_back(btnResetFov);
	
	generateOptionScreens();
}

void OptionsScreen::setupPositions() {
	btnClose->x = width - btnClose->width - 20;
	btnClose->y = 0;

	btnPrevPage->width = 40;
	btnPrevPage->height = btnClose->height;
	btnPrevPage->x = 20;
	btnPrevPage->y = height - btnPrevPage->height - 10;

	btnNextPage->width = 40;
	btnNextPage->height = btnClose->height;
	btnNextPage->x = width - btnNextPage->width - 20;
	btnNextPage->y = height - btnNextPage->height - 10;

	btnResetFov->width = 80;
	btnResetFov->height = btnClose->height;
	btnResetFov->x = (width - btnResetFov->width) / 2;
	btnResetFov->y = height - btnResetFov->height - 10;
	btnResetFov->visible = (currentPage == 0 && minecraft->options.fovSetting != 70.0f);

	bHeader->x = 0;
	bHeader->y = 0;
	bHeader->width = width; 
	bHeader->height = btnClose->height;

	if (optionPane != NULL) {
		int paneWidth = (width > 400) ? 360 : 260; 
		if (paneWidth > width - 40) paneWidth = width - 40;
		optionPane->width = paneWidth;
		optionPane->x = (width - paneWidth) / 2; 
		optionPane->y = bHeader->height;
        optionPane->height = height - bHeader->height - btnPrevPage->height - 15;
		optionPane->setupPositions();
	}
}

void OptionsScreen::render( int xm, int ym, float a ) {
	renderBackground();
	int xmm = xm * width / minecraft->width;
	int ymm = ym * height / minecraft->height - 1;
	if(optionPane != NULL)
		optionPane->render(minecraft, xmm, ymm);
	super::render(xm, ym, a);
}

void OptionsScreen::removed()
{
}

void OptionsScreen::buttonClicked( Button* button ) {
	if(button == btnClose) {
		minecraft->reloadOptions();
		minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
	} else if (button == btnPrevPage) {
		if (currentPage > 0) {
			currentPage--;
		} else {
			currentPage = maxPages - 1;
		}
		btnUsername = NULL;
		generateOptionScreens();
	} else if (button == btnNextPage) {
		if (currentPage < maxPages - 1) {
			currentPage++;
		} else {
			currentPage = 0;
		}
		btnUsername = NULL;
		generateOptionScreens();
	} else if (button->id == 100) {
#ifndef __VITA__
		minecraft->platform()->createUserInput(DialogDefinitions::DIALOG_ENTER_USERNAME);
#endif
		isEditingUsername = true;
	} else if (button->id == 101) {
		minecraft->options.fovSetting = 70.0f;
		minecraft->options.save();
		generateOptionScreens();
	}
}

void OptionsScreen::generateOptionScreens() {
	if (optionPane != NULL) {
		delete optionPane;
	}
	optionPane = new OptionsPane();
	char buf[32];
	sprintf(buf, "Options (%d/%d)", currentPage + 1, maxPages);
	if (bHeader) bHeader->msg = buf;
	
	if (currentPage == 0) {
		optionPane->createOptionsGroup("Video")
			.addOptionItem(&Options::Option::GRAPHICS, minecraft)
			.addOptionItem(&Options::Option::FOV, minecraft)
			.addOptionItem(&Options::Option::RENDER_DISTANCE, minecraft)
			.addOptionItem(&Options::Option::VSYNC, minecraft)
			.addOptionItem(&Options::Option::MSAA_4X, minecraft)
			.addOptionItem(&Options::Option::AMBIENT_OCCLUSION, minecraft)
			.addOptionItem(&Options::Option::VIEW_BOBBING, minecraft)
			.addOptionItem(&Options::Option::ANAGLYPH, minecraft)
			.addOptionItem(&Options::Option::HIGH_PERFORMANCE, minecraft)
            .addOptionItem(&Options::Option::GUI_SCALE, minecraft);
	} else if (currentPage == 1) {
		optionPane->createOptionsGroup("Game")
			.addOptionItem(&Options::Option::DIFFICULTY, minecraft)
			.addOptionItem(&Options::Option::THIRD_PERSON, minecraft)
			.addOptionItem(&Options::Option::DAY_COUNTER, minecraft)
			.addOptionItem(&Options::Option::SHOW_COORDINATES, minecraft)
			.addOptionItem(&Options::Option::HIDE_GUI, minecraft)
			.addOptionItem(&Options::Option::SERVER_VISIBLE, minecraft)
			.addOptionItem(&Options::Option::FPS_METER, minecraft)
			.addOptionItem(&Options::Option::RPI_MENUS, minecraft);
	} else if (currentPage == 2) {
		optionPane->createOptionsGroup("Controls")
			.addOptionItem(&Options::Option::SENSITIVITY, minecraft)
			.addOptionItem(&Options::Option::INVERT_MOUSE, minecraft)
			.addOptionItem(&Options::Option::LEFT_HANDED, minecraft)
			.addOptionItem(&Options::Option::USE_TOUCHSCREEN, minecraft)
			.addOptionItem(&Options::Option::USE_TOUCH_JOYPAD, minecraft)
			.addOptionItem(&Options::Option::AUTO_JUMP, minecraft)
			.addOptionItem(&Options::Option::DESTROY_VIBRATION, minecraft);

		optionPane->createOptionsGroup("Sound & Username")
			.addOptionItem(&Options::Option::SOUND, minecraft)
			.addOptionTextEntry(minecraft->options.username, 100, minecraft, &btnUsername);
	}
	
	if (optionPane != NULL) {
		this->setupPositions();
	}
	if (btnResetFov)
		btnResetFov->visible = (currentPage == 0 && minecraft->options.fovSetting != 70.0f);
}

void OptionsScreen::mouseClicked( int x, int y, int buttonNum ) {
	if(optionPane != NULL)
		optionPane->mouseClicked(minecraft, x, y, buttonNum);
	super::mouseClicked(x, y, buttonNum);
	
	if (buttonNum == MouseAction::ACTION_LEFT && btnUsername && btnUsername->clicked(minecraft, x, y)) {
		buttonClicked(btnUsername);
	}
}

void OptionsScreen::mouseReleased( int x, int y, int buttonNum ) {
	if(optionPane != NULL)
		optionPane->mouseReleased(minecraft, x, y, buttonNum);
	super::mouseReleased(x, y, buttonNum);
}

void OptionsScreen::tick() {
	if (isEditingUsername) {
		int status = minecraft->platform()->getUserInputStatus();
		if (status > -1) {
			if (status == 1) {
				StringVector v = minecraft->platform()->getUserInput();
				if (!v.empty()) {
					if (v[0].length() > 0) {
						minecraft->options.username = v[0];
						if (btnUsername) btnUsername->msg = v[0];
						minecraft->options.save();
					}
				}
			}
			isEditingUsername = false;
		}
	} else {
		if(optionPane != NULL)
			optionPane->tick(minecraft);
		super::tick();
	}
}

void OptionsScreen::keyPressed(int key) {
	super::keyPressed(key);
}

void OptionsScreen::keyboardNewChar(char c) {
	super::keyboardNewChar(c);
}
