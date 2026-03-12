#ifndef APPPLATFORM_SWITCH_H__
#define APPPLATFORM_SWITCH_H__

#include <fstream>
#include <string>
#include <png.h>
#include <cstdio>
#include <cstdlib>

#if defined(__SWITCH__)
#include <switch.h>
#endif

#include "AppPlatform.h"
#include "NinecraftApp.h"
#include "platform/log.h"

static const int width = 1280;
static const int height = 720;

static void png_funcReadFile(png_structp pngPtr, png_bytep data, png_size_t length) {
	((std::istream*)png_get_io_ptr(pngPtr))->read((char*)data, length);
}

static std::string switch_keyboard_text = "";

class AppPlatform_NX : public AppPlatform
{
public:
	bool supportsTouchscreen() override { return true; }

	int getScreenWidth() override { return width; }
	int getScreenHeight() override { return height; }

	void buyGame() override {
		LOGI("buyGame() called, stubbed for Switch.\n");
	}

	void showKeyboard() override {
#if defined(__SWITCH__)
		SwkbdConfig swkbd;
		char tmpoutstr[512] = {0};

		Result rc = swkbdCreate(&swkbd, 0);
		if (R_SUCCEEDED(rc)) {
			swkbdConfigMakePresetDefault(&swkbd);
			rc = swkbdShow(&swkbd, tmpoutstr, sizeof(tmpoutstr));
			swkbdClose(&swkbd);

			if (R_SUCCEEDED(rc)) {
				switch_keyboard_text = std::string(tmpoutstr);
			} else {
				switch_keyboard_text = "";
			}
		}
#endif
	}

	void hideKeyboard() override {
	}

	bool isKeyboardVisible() override {
		return false;
	}

	std::string getKeyboardInput() override {
		return switch_keyboard_text;
	}

	bool isPowerVR() override {
		return false;
	}

	std::string defaultUsername() override {
#if defined(__SWITCH__)
		AccountUid uid;
		AccountProfile profile;
		AccountProfileBase base;
		std::string username = "Switch";

		accountInitialize(AccountServiceType_Application);
		if (R_SUCCEEDED(accountGetPreselectedUser(&uid))) {
			if (R_SUCCEEDED(accountGetProfile(&profile, uid))) {
				if (R_SUCCEEDED(accountProfileGet(&profile, NULL, &base))) {
					username = std::string(base.nickname);
				}
				accountProfileClose(&profile);
			}
		}
		accountExit();
		return username;
#else
		return "Player";
#endif
	}

	BinaryBlob readAssetFile(const std::string& filename) override {

		std::string fullAssetPath = "sdmc:/switch/minecraftpe/" + filename;

		FILE* fd = fopen(fullAssetPath.c_str(), "rb");
		if (!fd) {
			fclose(fd);
			fullAssetPath = "romfs:/data/" + filename;
			fd = fopen(fullAssetPath.c_str(), "rb");
			if (!fd) {
				LOGI("failed to open: %s\n", fullAssetPath.c_str());
				return BinaryBlob();
			}
		}

		fseek(fd, 0, SEEK_END);
		size_t size = ftell(fd);
		fseek(fd, 0, SEEK_SET);

		BinaryBlob blob;
		blob.size = size;
		blob.data = new unsigned char[blob.size];

		size_t rd = fread(blob.data, 1, blob.size, fd);
		fclose(fd);

		if (rd != blob.size) {
			LOGI("wrong size read: %s\n", fullAssetPath.c_str());
			return BinaryBlob();
		}

		return blob;
	}

	TextureData loadTexture(const std::string& filename_, bool textureFolder) override {
		TextureData out;
		out.data = nullptr;


		/*std::string fullAssetPath = "sdmc:/switch/minecraft/" + filename;

		FILE* fd = fopen(fullAssetPath.c_str(), "rb");
		if (!fd) {
			fclose(fd);
			fd = fopen("romfs:/data/" + filename, "rb");
			if (!fd) {
				LOGI("failed to open: %s\n", fullAssetPath.c_str());
				return BinaryBlob();
			}
		}*/
		////
		std::string filename = textureFolder ? "romfs:/data/images/" + filename_ : "romfs:/" + filename_;
		FILE* fd = fopen(filename.c_str(), "rb");
		if (!fd) {
			fclose(fd);
			filename = "sdmc:/switch/minecraftpe/images/" + filename_;
			fd = fopen(filename.c_str(), "rb");
			if (!fd) {
				fclose(fd);
				filename = "sdmc:/switch/minecraftpe/" + filename_;
				fd = fopen(filename.c_str(), "rb");
				if (!fd) {
					LOGI("failed to open: %s\n", filename.c_str());
					fclose(fd);
				}
			}
		}
		std::ifstream source(filename.c_str(), std::ios::binary);

		if (source) {
			png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

			if (!pngPtr) return out;

			png_infop infoPtr = png_create_info_struct(pngPtr);

			if (!infoPtr) {
				png_destroy_read_struct(&pngPtr, NULL, NULL);
				return out;
			}

			png_set_read_fn(pngPtr, (void*)&source, png_funcReadFile);
			png_read_info(pngPtr, infoPtr);

			out.w = png_get_image_width(pngPtr, infoPtr);
			out.h = png_get_image_height(pngPtr, infoPtr);

			png_bytep* rowPtrs = new png_bytep[out.h];
			out.data = new unsigned char[4 * out.w * out.h];
			out.memoryHandledExternally = false;

			int rowStrideBytes = 4 * out.w;
			for (int i = 0; i < out.h; i++) {
				rowPtrs[i] = (png_bytep)&out.data[i * rowStrideBytes];
			}

			png_read_image(pngPtr, rowPtrs);

			png_destroy_read_struct(&pngPtr, &infoPtr, (png_infopp)0);
			delete[] (png_bytep)rowPtrs;
			source.close();

			return out;
		} else {
			LOGI("Couldn't find file: %s\n", filename.c_str());
			return out;
		}
	}
};

#endif