#include "container.h"

#include "AppPlatform_Symbian.h"

#include <escapeutils.h>

static inline CMcpeContainer *container() { return CMcpeContainer::instance(); }


int AppPlatform_Symbian::getScreenWidth() { return container()->Size().iWidth; }

int AppPlatform_Symbian::getScreenHeight() { return container()->Size().iHeight; }

void AppPlatform_Symbian::showKeyboard(std::string defaultText, int maxLength) {
	iBuffer = defaultText;
	container()->PromptTextL(iBuffer, maxLength);
}

bool AppPlatform_Symbian::isKeyboardVisible() { return container()->IsImeShown(); }
