#ifndef NET_MINECRAFT_CLIENT_PLAYER__VitaInput_H__
#define NET_MINECRAFT_CLIENT_PLAYER__VitaInput_H__

//package net.minecraft.client.player;

#include "KeyboardInput.h"
#include "ITurnInput.h"
#include "touchscreen/TouchInputHolder.h"
#include "MouseBuildInput.h"
#include "../../../platform/input/Controller.h"

static const int moveStick = 1;
static const int lookStick = 2;

class VitaTurnBuild : public UnifiedTurnBuild {
public:
	VitaTurnBuild(int turnMode, int width, int height, float maxMovementDelta, float sensitivity, IInputHolder* holder, Minecraft* minecraft) :
		UnifiedTurnBuild(turnMode, width, height, maxMovementDelta, sensitivity, holder, minecraft) {}

	TurnDelta getTurnDelta() override {
		TurnDelta td = UnifiedTurnBuild::getTurnDelta();

		float stickX = Controller::getTransformedX(lookStick, 0.1f, 1.25f, true);
		float stickY = Controller::getTransformedY(lookStick, 0.1f, 1.25f, true);

		float dx = 0, dy = 0;
		float dt = getDeltaTime();
		const float MaxTurnX = 250.0f;
		const float MaxTurnY = 200.0f;
		dx = linearTransform( stickX, 0.1f, MaxTurnX ) * dt;
		dy = linearTransform( stickY, 0.1f, MaxTurnY ) * dt;
		td.x += dx;
		td.y += dy;
		return td;
	}

	bool tickBuild(Player* p, BuildActionIntention* bai) override {
		if (Mouse::getButtonState(MouseAction::ACTION_LEFT) != 0) {
			if (mineHoldTicks >= mineDelayTicks) mineHoldTicks = 0;
				if (++mineHoldTicks == 1) {
					*bai = BuildActionIntention(BuildActionIntention::BAI_REMOVE | BuildActionIntention::BAI_ATTACK);
					return true;
				}
		}
		else {
			mineHoldTicks = 0;
		}

		if (Mouse::getButtonState(MouseAction::ACTION_RIGHT) != 0) {
			if (buildHoldTicks >= buildDelayTicks) buildHoldTicks = 0;
				if (++buildHoldTicks == 1) {
					*bai = BuildActionIntention(BuildActionIntention::BAI_BUILD | BuildActionIntention::BAI_INTERACT);
					return true;
				}
		} else {
			buildHoldTicks = 0;
		}

		return false;
	}

	void onConfigChanged(const Config& c) override {
		UnifiedTurnBuild::onConfigChanged(c);
	}
private:
	int mineHoldTicks = 0;
	int mineDelayTicks = 5;
	int buildHoldTicks = 0;
	int buildDelayTicks = 5;
};

class VitaMoveInput : public KeyboardInput {
	typedef KeyboardInput super;
public:
	VitaMoveInput(Options* options)
	:	super(options)
	{}

	void tick(Player* player) override {
		super::tick(player);
		xa += -Controller::getTransformedX(moveStick, 0.1f, 1.25f, true);
		ya += -Controller::getTransformedY(moveStick, 0.1f, 1.25f, true);
	}
};

/*
move = dpad + l stick
turn = r stick
build = touch
*/

class VitaInputHolder : public IInputHolder {
	static const int MovementLimit = 200; // per update

public:
	VitaInputHolder(Minecraft* mc, Options* options) :
		_mc(mc),
		_move(options),
		_turnBuild(UnifiedTurnBuild::MODE_DELTA, mc->width, mc->height, (float)MovementLimit, 1, this, mc)
	{
		onConfigChanged(createConfig(mc));
	}
	~VitaInputHolder() = default;

	void onConfigChanged(const Config& c) override {
		_move.onConfigChanged(c);
		_turnBuild.moveArea = RectangleArea(0,0,0,0);
		_turnBuild.inventoryArea = _mc->gui.getRectangleArea( _mc->options.isLeftHanded? 1 : -1 );
		_turnBuild.setSensitivity(c.options->isJoyTouchArea? 1.8f : 1.0f);
		((ITurnInput*)&_turnBuild)->onConfigChanged(c);
	}

	bool allowPicking() override {
		mousex = 960/2; // Mouse::getX();
		mousey = 544/2; // Mouse::getY();
		return true; // Mouse::getButtonState(MouseAction::ACTION_LEFT) == MouseAction::DATA_DOWN || Mouse::getButtonState(MouseAction::ACTION_RIGHT) == MouseAction::DATA_DOWN;
	}

	void render(float alpha) override {
		_turnBuild.render(alpha);
	}

	IMoveInput*		getMoveInput()  override { return &_move; }
	ITurnInput*		getTurnInput()  override { return &_turnBuild; }
	IBuildInput*	getBuildInput() override { return &_turnBuild; }

private:
	VitaMoveInput _move;
	VitaTurnBuild _turnBuild;
	Minecraft* _mc;
};

#endif /*NET_MINECRAFT_CLIENT_PLAYER__VitaInput_H__*/
