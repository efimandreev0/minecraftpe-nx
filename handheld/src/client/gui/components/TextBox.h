#ifndef NET_MINECRAFT_CLIENT_GUI_COMPONENTS__TextBox_H__
#define NET_MINECRAFT_CLIENT_GUI_COMPONENTS__TextBox_H__

//package net.minecraft.client.gui;

#include <string>
#include "Button.h"
#include "../../Options.h"

class Font;
class Minecraft;

class TextBox: public Button
{
public:
	TextBox(int id, const std::string& msg);
    TextBox(int id, int x, int y, const std::string& msg);
    TextBox(int id, int x, int y, int w, int h, const std::string& msg);
	virtual ~TextBox();

	virtual void setFocus(Minecraft* minecraft);
	virtual bool loseFocus(Minecraft* minecraft);

    void render(Minecraft* minecraft, int xm, int ym) override;
	void setPressed(Minecraft* minecraft) override;
	void renderBg(Minecraft* minecraft, int xm, int ym ) override;

private:
	std::string defaultText;
	Minecraft* minecraftRef = nullptr;
public:
	std::string text;
	bool focused;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_COMPONENTS__TextBox_H__*/
