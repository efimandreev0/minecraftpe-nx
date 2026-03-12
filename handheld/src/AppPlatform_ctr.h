#ifndef APPPLATFORM_3DS_H__
#define APPPLATFORM_3DS_H__

#include <fstream>
#include <string>
#include <png.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(__3DS__)
#include <3ds.h>
#endif

#include "AppPlatform.h"
#include "NinecraftApp.h"
#include "platform/log.h"

// Разрешение верхнего экрана 3DS
static const int width = 400;
static const int height = 240;

static void png_funcReadFile(png_structp pngPtr, png_bytep data, png_size_t length) {
	((std::istream*)png_get_io_ptr(pngPtr))->read((char*)data, length);
}

static std::string _3ds_keyboard_text = "";

class AppPlatform_3ds : public AppPlatform
{
public:
	bool supportsTouchscreen() override { return true; }

	int getScreenWidth() override { return width; }
	int getScreenHeight() override { return height; }

	void buyGame() override {
		LOGI("buyGame() called, stubbed for 3DS.\n");
	}

	void showKeyboard() override {
#if defined(__3DS__)
		SwkbdState swkbd;
		char mybuf[512] = {0};

		// Инициализируем стандартную клавиатуру 3DS с 2 кнопками (ОК/Отмена)
		swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, -1);
		swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
		swkbdSetFeatures(&swkbd, SWKBD_DARKEN_TOP_SCREEN);
		swkbdSetHintText(&swkbd, "Enter text");

		// Показываем клавиатуру и ждем ввода
		SwkbdButton button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));

		if (button == SWKBD_BUTTON_CONFIRM) {
			_3ds_keyboard_text = std::string(mybuf);
		} else {
			_3ds_keyboard_text = "";
		}
#endif
	}

	void hideKeyboard() override {
	}

	bool isKeyboardVisible() override {
		return false;
	}

	std::string getKeyboardInput() override {
		return _3ds_keyboard_text;
	}

	bool isPowerVR() override {
		return false;
	}

	std::string defaultUsername() override {
#if defined(__3DS__)
		u16 username_utf16[16] = {0};
		char username_utf8[64] = {0};

		// Читаем имя профиля из настроек 3DS
		if (R_SUCCEEDED(cfguInit())) {
			// 0x000A0000 - Блок памяти, где хранится никнейм консоли
			if (R_SUCCEEDED(CFGU_GetConfigInfoBlk2(sizeof(username_utf16), 0x000A0000, (u8*)username_utf16))) {
				// Конвертируем UTF-16 в UTF-8
				utf16_to_utf8((uint8_t*)username_utf8, username_utf16, sizeof(username_utf8) - 1);
				cfguExit();
				return std::string(username_utf8);
			}
			cfguExit();
		}
#endif
		return "Player";
	}

	BinaryBlob readAssetFile(const std::string& filename) override {
		std::string fullAssetPath = "sdmc:/3ds/minecraftpe/" + filename;

		FILE* fd = fopen(fullAssetPath.c_str(), "rb");
		if (!fd) {
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
			return BinaryBlob(); // В идеале тут бы delete[] blob.data, но оставим как было
		}

		return blob;
	}

	TextureData loadTexture(const std::string& filename_, bool textureFolder) override {
		TextureData out;
		out.data = nullptr;

		std::string filepath = textureFolder ? "romfs:/data/images/" + filename_ : "romfs:/" + filename_;
		
		FILE* fd = fopen(filepath.c_str(), "rb");
		if (!fd) {
			filepath = "sdmc:/3ds/minecraftpe/images/" + filename_;
			fd = fopen(filepath.c_str(), "rb");
			if (!fd) {
				filepath = "sdmc:/3ds/minecraftpe/" + filename_;
				fd = fopen(filepath.c_str(), "rb");
				if (!fd) {
					LOGI("failed to open: %s\n", filepath.c_str());
				}
			}
		}
		
		// Закрываем файл, потому что дальше его будет открывать std::ifstream
		if (fd) fclose(fd);

		std::ifstream source(filepath.c_str(), std::ios::binary);

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

			// Конвертируем всякие индексированные палитры в обычный RGBA
			if (png_get_color_type(pngPtr, infoPtr) == PNG_COLOR_TYPE_PALETTE)
				png_set_palette_to_rgb(pngPtr);
			if (png_get_color_type(pngPtr, infoPtr) == PNG_COLOR_TYPE_RGB)
				png_set_filler(pngPtr, 0xFF, PNG_FILLER_AFTER);
			if (png_get_color_type(pngPtr, infoPtr) == PNG_COLOR_TYPE_GRAY ||
				png_get_color_type(pngPtr, infoPtr) == PNG_COLOR_TYPE_GRAY_ALPHA)
				png_set_gray_to_rgb(pngPtr);
			
			png_read_update_info(pngPtr, infoPtr);

			png_bytep* rowPtrs = new png_bytep[out.h];
			out.data = new unsigned char[4 * out.w * out.h];
			out.memoryHandledExternally = false;

			int rowStrideBytes = 4 * out.w;
			for (int i = 0; i < out.h; i++) {
				rowPtrs[i] = (png_bytep)&out.data[i * rowStrideBytes];
			}

			png_read_image(pngPtr, rowPtrs);

			png_destroy_read_struct(&pngPtr, &infoPtr, (png_infopp)0);
			delete[] rowPtrs;
			source.close();

			return out;
		} else {
			LOGI("Couldn't open stream for file: %s\n", filepath.c_str());
			return out;
		}
	}
};

#endif