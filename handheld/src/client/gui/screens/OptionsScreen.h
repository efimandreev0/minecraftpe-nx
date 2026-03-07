#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__OptionsScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__OptionsScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include "../../../platform/input/Keyboard.h"

class ImageButton;
class OptionsPane;

class OptionsScreen: public Screen
{
	typedef Screen super;
	void init();

	void generateOptionScreens();

public:
	OptionsScreen();
	~OptionsScreen();
	void setupPositions();
	void buttonClicked( Button* button );
	void render(int xm, int ym, float a);
	void removed();

	virtual void mouseClicked( int x, int y, int buttonNum );
	virtual void mouseReleased( int x, int y, int buttonNum );
	virtual void tick();
	virtual void keyPressed(int key);
	virtual void keyboardNewChar(char c);
private:
	Touch::THeader* bHeader;
	ImageButton* btnClose;
	Button* btnNextPage;
	Button* btnPrevPage;
	Button* btnResetFov;
	OptionsPane* optionPane;
	bool isEditingUsername;
	Button* btnUsername;
	int currentPage;
	int maxPages;
};

#endif 
