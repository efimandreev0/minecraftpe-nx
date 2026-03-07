#include "OptionsGroup.h"
#include "../../Minecraft.h"
#include "ImageButton.h"
#include "OptionsItem.h"
#include "Slider.h"
#include "Button.h"
#include "../../../locale/I18n.h"
OptionsGroup::OptionsGroup( std::string labelID )  {
	label = I18n::get(labelID);
}

void OptionsGroup::setupPositions() {
	// First we write the header and then we add the items
	int curY = y + 10;
	for(std::vector<GuiElement*>::iterator it = children.begin(); it != children.end(); ++it) {
		(*it)->x = x + 15;
		(*it)->y = curY;
		(*it)->width = width - 30;
		(*it)->setupPositions();
		curY += (*it)->height + 3;
	}
	height = curY - y;
}

void OptionsGroup::render( Minecraft* minecraft, int xm, int ym ) {
	minecraft->font->draw(label, (float)x + 2, (float)y, 0xffffffff, false);
	super::render(minecraft, xm, ym);
}

OptionsGroup& OptionsGroup::addOptionItem( const Options::Option* option, Minecraft* minecraft ) {
	if(option->isBoolean())
		createToggle(option, minecraft);
	else if(option->isProgress())
		createProgressSlider(option, minecraft);
	else if(option->isInt())
		createStepSlider(option, minecraft);
	return *this;
}

void OptionsGroup::createToggle( const Options::Option* option, Minecraft* minecraft ) {
	ImageDef def;
	def.setSrc(IntRectangle(160, 206, 38, 19)); 
	def.name = "gui/touchgui.png";
	def.width = 39 * 0.7f;
	def.height = 20 * 0.7f;
	OptionButton* element = new OptionButton(option);
	element->setImageDef(def, true);
	element->updateImage(&minecraft->options);
	std::string itemLabel = I18n::get(option->getCaptionId());
	OptionsItem* item = new OptionsItem(itemLabel, element);
	addChild(item);
	setupPositions();
}

void OptionsGroup::createProgressSlider( const Options::Option* option, Minecraft* minecraft ) {
	Slider* element = new Slider(minecraft,
									option,
									minecraft->options.getProgrssMin(option),
									minecraft->options.getProgrssMax(option));
	element->width = 100;
	element->height = 20;
	std::string itemLabel = I18n::get(option->getCaptionId());
	OptionsItem* item = new OptionsItem(itemLabel, element);
	addChild(item);
	setupPositions();
}

class StepOptionButton : public Button {
public:
	const Options::Option* _option;
	StepOptionButton(const Options::Option* option, Minecraft* mc) : Button(9999999, ""), _option(option) {
		std::string key = mc->options.getMessage(_option);
		msg = I18n::get(key);
	}
	virtual void mouseClicked(Minecraft* minecraft, int x, int y, int buttonNum) {
		if(buttonNum == MouseAction::ACTION_LEFT && clicked(minecraft, x, y)) {
			minecraft->options.toggle(_option, 1);
			std::string key = minecraft->options.getMessage(_option);
			msg = I18n::get(key);
		}
	}
};

class TouchStepOptionButton : public Touch::TButton {
public:
	const Options::Option* _option;
	TouchStepOptionButton(const Options::Option* option, Minecraft* mc) : Touch::TButton(9999999, ""), _option(option) {
		std::string key = mc->options.getMessage(_option);
		msg = I18n::get(key);
	}
	virtual void mouseClicked(Minecraft* minecraft, int x, int y, int buttonNum) {
		if(buttonNum == MouseAction::ACTION_LEFT && clicked(minecraft, x, y)) {
			minecraft->options.toggle(_option, 1);
			std::string key = minecraft->options.getMessage(_option);
			msg = I18n::get(key);
		}
	}
};

void OptionsGroup::createStepSlider( const Options::Option* option, Minecraft* minecraft ) {
	Button* element;
	if (minecraft->useTouchscreen()) {
		element = new TouchStepOptionButton(option, minecraft);
	} else {
		element = new StepOptionButton(option, minecraft);
	}
	element->width = 100;
	element->height = 20;
	std::string itemLabel = I18n::get(option->getCaptionId());
	OptionsItem* item = new OptionsItem(itemLabel, element);
	addChild(item);
	setupPositions();
}

OptionsGroup& OptionsGroup::addOptionTextEntry( std::string text, int id, Minecraft* minecraft, Button** outButton ) {
	Button* element;
	if (minecraft->useTouchscreen()) {
		element = new Touch::TButton(id, text);
	} else {
		element = new Button(id, text);
	}
	element->width = 160;
	element->height = 30;
	OptionsItem* item = new OptionsItem("", element);
	addChild(item);
	setupPositions();
	if (outButton) *outButton = element;
	return *this;
}

OptionsGroup& OptionsGroup::addTextLabel( std::string text ) {
	OptionsItem* item = new OptionsItem(text, NULL);
	addChild(item);
	setupPositions();
	return *this;
}

OptionsGroup& OptionsGroup::addButtonItem(int id, std::string text, Minecraft* minecraft) {
	Button* element;
	if (minecraft->useTouchscreen()) {
		element = new Touch::TButton(id, text);
	} else {
		element = new Button(id, text);
	}
	element->width = 160;
	element->height = 24;
	OptionsItem* item = new OptionsItem("", element);
	addChild(item);
	setupPositions();
	return *this;
}
