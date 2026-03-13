#include "OptionsScreen.h"

#include "StartMenuScreen.h"
#include "DialogDefinitions.h"
#include "../../Minecraft.h"
#include "../../../AppPlatform.h"
#include "../../../platform/input/Mouse.h"

#include "../components/OptionsPane.h"
#include "../components/ImageButton.h"
#include "../components/OptionsGroup.h"
#include "../components/TextBox.h"


OptionsScreen::OptionsScreen()
: btnClose(NULL),
  bHeader(NULL),
  btnNextPage(NULL),
  btnPrevPage(NULL),
  optionPane(NULL)
#ifdef EDIT_USERNAME
  editUsername(NULL)
#endif
{
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
	if(optionPane != NULL) {
		delete optionPane;
		optionPane = NULL;
	}
#ifdef EDIT_USERNAME
	if(editUsername != NULL) {
		delete editUsername;
		editUsername = NULL;
	}
#endif

}

void OptionsScreen::init() {
	bHeader = new Touch::THeader(0, "Options");
	if(minecraft->useTouchscreen()) {
		btnPrevPage = new Touch::TButton(201, "<");
		btnNextPage = new Touch::TButton(202, ">");
	}
	else {
		btnPrevPage = new Button(201, "<");
		btnNextPage = new Button(202, ">");
	}
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
	
	generateOptionScreens();
}

void OptionsScreen::setupPositions() {
	btnClose->x = width - btnClose->width;
	btnClose->y = 0;

	btnPrevPage->width = 40;
	btnPrevPage->height = btnClose->height;
	btnPrevPage->x = 20;
	btnPrevPage->y = height - btnPrevPage->height - 10;

	btnNextPage->width = 40;
	btnNextPage->height = btnClose->height;
	btnNextPage->x = width - btnNextPage->width - 20;
	btnNextPage->y = height - btnNextPage->height - 10;

	bHeader->x = 0;
	bHeader->y = 0;
	bHeader->width = width - btnClose->width;
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
		generateOptionScreens();
	} else if (button == btnNextPage) {
		if (currentPage < maxPages - 1) {
			currentPage++;
		} else {
			currentPage = 0;
		}
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
		optionPane->createOptionsGroup("options.group.video")
			.addOptionItem(&Options::Option::GRAPHICS, minecraft)
			.addOptionItem(&Options::Option::RENDER_DISTANCE, minecraft)
			.addOptionItem(&Options::Option::AMBIENT_OCCLUSION, minecraft)
			.addOptionItem(&Options::Option::VIEW_BOBBING, minecraft)
			.addOptionItem(&Options::Option::ANAGLYPH, minecraft)
			.addOptionItem(&Options::Option::RENDER_DEBUG, minecraft)
			.addOptionItem(&Options::Option::LIMIT_FRAMERATE, minecraft)
			.addOptionItem(&Options::Option::GUI_SCALE, minecraft);
	} else if (currentPage == 1) {
		optionPane->createOptionsGroup("options.group.game")
			.addOptionItem(&Options::Option::DIFFICULTY, minecraft)
			.addOptionItem(&Options::Option::THIRD_PERSON, minecraft)
			.addOptionItem(&Options::Option::HIDE_GUI, minecraft)
			.addOptionItem(&Options::Option::SERVER_VISIBLE, minecraft);
	} else if (currentPage == 2) {
		optionPane->createOptionsGroup("options.group.control")
			.addOptionItem(&Options::Option::SENSITIVITY, minecraft)
			.addOptionItem(&Options::Option::INVERT_MOUSE, minecraft)
			.addOptionItem(&Options::Option::LEFT_HANDED, minecraft)
			.addOptionItem(&Options::Option::USE_TOUCHSCREEN, minecraft)
			.addOptionItem(&Options::Option::USE_TOUCH_JOYPAD, minecraft)
			.addOptionItem(&Options::Option::DESTROY_VIBRATION, minecraft);

		optionPane->createOptionsGroup("options.group.audio")
			.addOptionItem(&Options::Option::SOUND, minecraft);
#ifdef EDIT_USERNAME
			.addOptionTextEntry(minecraft->options.username, 100, minecraft, &editUsername);
#endif
	}
	
	if (optionPane != NULL) {
		this->setupPositions();
	}
}

void OptionsScreen::mouseClicked( int x, int y, int buttonNum ) {
	if(optionPane != NULL)
		optionPane->mouseClicked(minecraft, x, y, buttonNum);
	super::mouseClicked(x, y, buttonNum);
	}

void OptionsScreen::mouseReleased( int x, int y, int buttonNum ) {
	if(optionPane != NULL)
		optionPane->mouseReleased(minecraft, x, y, buttonNum);
	super::mouseReleased(x, y, buttonNum);
}

void OptionsScreen::tick() {
#ifdef EDIT_USERNAME
	if(editUsername != nullptr && editUsername->focused) {
		minecraft->options.username = editUsername->text;
	}
#endif
	{
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
