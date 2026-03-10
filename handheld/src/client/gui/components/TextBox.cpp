#include "TextBox.h"
#include "../../Minecraft.h"
#include "../../../AppPlatform.h"
#include "../../renderer/Textures.h"
#include "../screen.h"

#include "Button.h"

TextBox::TextBox( int id, const std::string& msg )
 : Button(id, 0, 0, 0, 0, msg),
 focused(false),
 defaultText(msg),
 text(msg){

}

TextBox::TextBox( int id, int x, int y, const std::string& msg ) 
 : Button(id, x, y, 0, 0, msg),
	focused(false),
	defaultText(msg),
	text(msg){
}

TextBox::TextBox( int id, int x, int y, int w, int h, const std::string& msg )
 : Button(id, x, y, w, h, msg),
   focused(false),
   defaultText(msg),
   text(msg) {
}

TextBox::~TextBox() {
	if(focused) {
		if(minecraftRef != nullptr) {
			this->minecraftRef->platform()->hideKeyboard();
		}
	}

}

void TextBox::setFocus(Minecraft* minecraft) {
	if(!focused) {
		minecraft->platform()->showKeyboard();
		focused = true;

		this->minecraftRef = minecraft;
	}
}

bool TextBox::loseFocus(Minecraft* minecraft) {
	if(focused) {
		minecraft->platform()->showKeyboard();
		focused = false;
		return true;

		this->minecraftRef = minecraft;
	}
	return false;
}

void TextBox::setPressed(Minecraft* minecraft) {
	this->setFocus(minecraft);
}

void TextBox::render( Minecraft* minecraft, int xm, int ym ) {
	if(focused) {
		std::string input = minecraft->platform()->getKeyboardInput();
		if(!minecraft->platform()->isKeyboardVisible()) {
			focused = false;

			if(input.empty()) {
				// set default if it was left empty.
				input = defaultText;
			}
		}

		int padding = 3;

		// find portion that can fit inside the textbox
		int offset = 0;
		int len = input.length();

		while(minecraft->font->width(input.substr(offset, offset-len)) > this->width - padding)
			offset++;

		this->msg = input.substr(offset, offset-len);

		// set text to current input
		this->text = input;


		#if defined(__VITA__) || 1

		int prevY = this->y;

		int keyboardX = 0;
		int keyboardY = 297;

		minecraft->screen->toGUICoordinate(keyboardX, keyboardY);

		if((this->y + this->height) >= keyboardY) {
			this->y = (keyboardY - this->height) - 2;
		}

		Button::render(minecraft, xm, ym);

		this->y = prevY;

		return;
		#endif
	}

	Button::render(minecraft, xm, ym);
}

// use THeader sprite ..
void TextBox::renderBg( Minecraft* minecraft, int xm, int ym ) {
	minecraft->textures->loadAndBindTexture("gui/touchgui.png");

	//printf("ButtonId: %d - Hovered? %d (cause: %d, %d, %d, %d, <> %d, %d)\n", id, hovered, x, y, x+w, y+h, xm, ym);
	glColor4f2(1, 1, 1, 1);

	// Left cap
	blit(x, y, 150, 26, 2, height-1, 2, 25);
	// Middle
	blit(x+2, y, 153, 26, width-3, height-1, 8, 25);
	// Right cap
	blit(x+width-2, y, 162, 26, 2, height-1, 2, 25);
	// Shadow
	glEnable2(GL_BLEND);
	glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	blit(x, y+height-1, 153, 52, width, 3, 8, 3);
}
