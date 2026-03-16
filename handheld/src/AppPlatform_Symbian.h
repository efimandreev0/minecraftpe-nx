#pragma once

#ifndef _SRC_APPPLATFORM_SYMBIAN_H_A2B650F7_2078_5E8F_B2DE_0544F8A73562
#define _SRC_APPPLATFORM_SYMBIAN_H_A2B650F7_2078_5E8F_B2DE_0544F8A73562

#include <e32cmn.h>
#include <hwrmvibra.h>

#include <lodepng.h>

#include "AppPlatform.h"

#include "NinecraftApp.h"

#include <algorithm>
#include <vector>
#include <cstdint>

struct AppPlatform_Symbian : AppPlatform {
	AppPlatform_Symbian() {
		TRAPD(err, (iVibra = CHWRMVibra::NewL()));
		if (err != KErrNone) {
			fprintf(stderr, "Vibra open failed: %d\n", err);
			iVibra = NULL;
		}
	}

	bool supportsTouchscreen() override { return true; }

	int getScreenWidth() override;
	int getScreenHeight() override;

	void showKeyboard(std::string defaultText, int maxLength) override;
	void hideKeyboard() override {}

	bool isKeyboardVisible() override;

	std::string getKeyboardInput() override { return iBuffer; }

	bool isPowerVR() override { return false; }

	std::string defaultUsername() override { return "Carla"; /* Symbian^4 would've been Carla */ }

	BinaryBlob readAssetFile(const std::string &filename) override {
		BinaryBlob blob;

		const auto path = "/private/e000c418/data/" + filename;

		auto fh = fopen(path.c_str(), "r");

		if (!fh) {
			return blob;
		}
		fseek(fh, 0, SEEK_END);
		const auto sz = ftell(fh);
		fseek(fh, 0, SEEK_SET);

		auto buf = new uint8_t[sz];
		if (1 != fread(buf, sz, 1, fh)) {
			fclose(fh);
			delete[] buf;
			return blob;
		}

		fclose(fh);

		blob.size = sz;
		blob.data = buf;
		return blob;
	}

	TextureData loadTexture(const std::string &filename_, bool textureFolder) override {
		const std::string filename = textureFolder ? "/private/e000c418/data/images/" + filename_ : filename_;

		TextureData out;
		unsigned err;
		std::vector<uint8_t> pmap;
		if ((err = lodepng::decode(pmap, reinterpret_cast<unsigned int &>(out.w), reinterpret_cast<unsigned int &>(out.h), filename))) {
			return out;
		}

		out.memoryHandledExternally = false;

		out.data = new uint8_t[pmap.size()];
		std::copy(pmap.begin(), pmap.end(), out.data);

		return out;
	}

	void vibrate(int millis) override {
		if (iVibra) {
			TRAPD(err, iVibra->StartVibraL(millis));
			(void)err;
		}
	}

private:
	CHWRMVibra *iVibra;
	std::string iBuffer;
};

#endif

