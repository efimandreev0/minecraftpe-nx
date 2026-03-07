#include "OptionsPane.h"
#include "OptionsGroup.h"
#include "OptionsItem.h"
#include "ImageButton.h"
#include "Slider.h"
#include "../../Minecraft.h"
#include "../../../util/Mth.h"
#include "../../renderer/gles.h"
#include "../Gui.h"
#include "../../../platform/input/Mouse.h"

OptionsPane::OptionsPane() {
	scrollOffset = 0;
	targetScrollOffset = 0;
	contentHeight = 0;
	isDragging = false;
    isDraggingThumb = false;
}

void OptionsPane::setupPositions() {
	int currentHeight = 0; // Relative to pane y
	for(std::vector<GuiElement*>::iterator it = children.begin(); it != children.end(); ++it ) {
		(*it)->width = width;
		(*it)->x = x;
		(*it)->y = y + currentHeight - scrollOffset;
		(*it)->setupPositions();
		currentHeight += (*it)->height + 1;
	}
	contentHeight = currentHeight;
}

OptionsGroup& OptionsPane::createOptionsGroup( std::string label ) {
	OptionsGroup* newGroup = new OptionsGroup(label);
	children.push_back(newGroup);
	// create and return a new group index
	return *newGroup;
}

void OptionsPane::render(Minecraft* minecraft, int xm, int ym) {
	if (!visible) return;

	// Smooth scrolling
	if (Mth::abs((float)(targetScrollOffset - scrollOffset)) > 1.0f) {
		scrollOffset += (targetScrollOffset - scrollOffset) * 0.2f;
		setupPositions();
	} else if (scrollOffset != targetScrollOffset) {
		scrollOffset = targetScrollOffset;
		setupPositions();
	}

	// OpenGL clipping (Scissors)
    float s = Gui::GuiScale;
    int winH = minecraft->height;
    
    glEnable(GL_SCISSOR_TEST);
    glScissor((int)(x * s), (int)(winH - (y + height) * s), (int)(width * s), (int)(height * s));

	super::render(minecraft, xm, ym);

    glDisable(GL_SCISSOR_TEST);

    if (contentHeight > height) {
        int trackX = x + width - 4;
        int trackW = 4;
        
        float visibleRatio = (float)height / contentHeight;
        int thumbH = (int)(height * visibleRatio);
        if (thumbH < 10) thumbH = 10;
        
        int maxScroll = contentHeight - height;
        float scrollRatio = (float)scrollOffset / maxScroll;
        int thumbY = y + (int)((height - thumbH) * scrollRatio);
        
        // Draw Track
        fill(trackX, y, trackX + trackW, y + height, 0x80000000);
        // Draw Thumb
        fill(trackX, thumbY, trackX + trackW, thumbY + thumbH, 0xFFC0C0C0);
    }
}

void OptionsPane::mouseClicked(Minecraft* minecraft, int x, int y, int buttonNum) {
	if (x >= this->x && x < this->x + width && y >= this->y && y < this->y + height) {
		if (buttonNum == MouseAction::ACTION_LEFT) {
            
            if (contentHeight > height) {
                int trackX = this->x + width - 4;
                int trackW = 4;
                
                if (x >= trackX && x <= trackX + trackW) {
                    float visibleRatio = (float)height / contentHeight;
                    int thumbH = (int)(height * visibleRatio);
                    if (thumbH < 10) thumbH = 10;
                    
                    int maxScroll = contentHeight - height;
                    float scrollRatio = (float)scrollOffset / maxScroll;
                    int thumbY = this->y + (int)((height - thumbH) * scrollRatio);
                    
                    if (y >= thumbY && y <= thumbY + thumbH) {
                        isDraggingThumb = true;
                    } else {
                        // Clicked track, jump to drag position
                        float yOffset = (y - this->y) - thumbH / 2.0f;
                        float trackHeight = height - thumbH;
                        if (trackHeight <= 0) trackHeight = 1;

                        float scrollFraction = yOffset / trackHeight;
                        if (scrollFraction < 0.0f) scrollFraction = 0.0f;
                        if (scrollFraction > 1.0f) scrollFraction = 1.0f;

                        targetScrollOffset = (int)(scrollFraction * maxScroll);
                        scrollOffset = targetScrollOffset;
                        setupPositions();
                        isDraggingThumb = true;
                    }
                } else {
                    isDragging = true;
                }
            } else {
			    isDragging = true;
            }
			dragY = y;
            lastDragY = y;
		}
		super::mouseClicked(minecraft, x, y, buttonNum);
	}
}

void OptionsPane::mouseReleased(Minecraft* minecraft, int x, int y, int buttonNum) {
	if (buttonNum == MouseAction::ACTION_LEFT) {
		isDragging = false;
        isDraggingThumb = false;
	}
	super::mouseReleased(minecraft, x, y, buttonNum);
}

void OptionsPane::tick(Minecraft* minecraft) {
    if (isDraggingThumb) {
        if (Mouse::isButtonDown(1)) {
            int currentY = Mouse::getY();
            
            float s = Gui::GuiScale;
            if (s <= 0) s = 1.0f;
            int scaledY = (int)(currentY / s);
            
            int dy = scaledY - lastDragY;
            if (dy != 0) {
                float visibleRatio = (float)height / contentHeight;
                int thumbH = (int)(height * visibleRatio);
                if (thumbH < 10) thumbH = 10;
                
                float trackHeight = height - thumbH;
                if (trackHeight > 0) {
                    int maxScroll = contentHeight - height;
                    float scrollRatioPerPixel = maxScroll / trackHeight;
                    targetScrollOffset += (int)(dy * scrollRatioPerPixel);
                }
                
                lastDragY = scaledY;
                setupPositions();
            }
        } else {
            isDraggingThumb = false;
        }
    } else if (isDragging) {
        if (Mouse::isButtonDown(1)) {
            int currentY = Mouse::getY();
            
            // Mouse coords are in window pixels [0, winH].
            // logicalY = currentY * height / winH
            float s = Gui::GuiScale;
            if (s <= 0) s = 1.0f;
            int scaledY = (int)(currentY / s);
            
            int dy = scaledY - lastDragY;
            if (dy != 0) {
                targetScrollOffset -= dy;
                lastDragY = scaledY;
                setupPositions();
            }
        } else {
            isDragging = false;
        }
    }
    
    // Boundary check for scrolling
    if (targetScrollOffset < 0) targetScrollOffset = 0;
    int maxScroll = contentHeight - height;
    if (maxScroll < 0) maxScroll = 0;
    if (targetScrollOffset > maxScroll) targetScrollOffset = maxScroll;

	super::tick(minecraft);
}