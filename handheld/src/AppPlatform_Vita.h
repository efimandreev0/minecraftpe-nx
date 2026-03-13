#ifndef APPPLATFORM_VITA_H__
#define APPPLATFORM_VITA_H__

#include <fstream>
#include <png.h>

#include "AppPlatform.h"
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/appmgr.h>
#include <psp2/libime.h>
#include <psp2/registrymgr.h>

#include <cstdlib>

#include "NinecraftApp.h"

#include "np_mgr.h"

static const int width = 960;
static const int height = 544;

static void png_funcReadFile(png_structp pngPtr, png_bytep data, png_size_t length) {
	((std::istream*)png_get_io_ptr(pngPtr))->read((char*)data, length);
}

static SceWChar16 ime_out[SCE_IME_MAX_PREEDIT_LENGTH + SCE_IME_MAX_TEXT_LENGTH + 1] = {0};
static SceUInt8 ime_out_utf8[sizeof(ime_out)] = {0};
static char ime_inital[8] = { 0 };
static bool ime_is_open = false;

static void Utf16ToUtf8(const uint16_t *src, uint8_t *dst)
{
	int i;
	for (i = 0; src[i]; i++) {
		if (!(src[i] & 0xFF80)) {
			*(dst++) = src[i] & 0xFF;
		} else if (!(src[i] & 0xF800)) {
			*(dst++) = ((src[i] >> 6) & 0xFF) | 0xC0;
			*(dst++) = (src[i] & 0x3F) | 0x80;
		} else if ((src[i] & 0xFC00) == 0xD800 && (src[i + 1] & 0xFC00) == 0xDC00) {
			*(dst++) = (((src[i] + 64) >> 8) & 0x3) | 0xF0;
			*(dst++) = (((src[i] >> 2) + 16) & 0x3F) | 0x80;
			*(dst++) = ((src[i] >> 4) & 0x30) | 0x80 | ((src[i + 1] << 2) & 0xF);
			*(dst++) = (src[i + 1] & 0x3F) | 0x80;
			i += 1;
		} else {
			*(dst++) = ((src[i] >> 12) & 0xF) | 0xE0;
			*(dst++) = ((src[i] >> 6) & 0x3F) | 0x80;
			*(dst++) = (src[i] & 0x3F) | 0x80;
		}
	}

	*dst = '\0';
}

static void ImeEventHandler(void *arg, const SceImeEventData *e)
{


	switch (e->id) {
		case SCE_IME_EVENT_UPDATE_TEXT:
			Utf16ToUtf8((SceWChar16 *)ime_out, (uint8_t*)ime_out_utf8);
			LOGI("text_so_far: %s\n", ime_out_utf8);
			break;
		case SCE_IME_EVENT_PRESS_ENTER:
			sceImeClose();
			ime_is_open = false;
			break;
		case SCE_IME_EVENT_PRESS_CLOSE:
			sceImeClose();
			ime_is_open = false;
			break;
	}
}

class AppPlatform_Vita : public AppPlatform
{
public:
	bool supportsTouchscreen() override { return true; }

	int getScreenWidth() override { return width; }
	int getScreenHeight() override { return height; }

	void buyGame() override {

		int lang = 0;
		SceNpCountryCode code = { 0 };
		sceNpManagerGetAccountRegion(&code, &lang);

		std::string region = std::string(code.data, sizeof(code.data));

		if (region == "jp") {
			sceAppMgrLaunchAppByUri(0x60000, "psts:browse?product=JP0127-PCSG00302_00-MINECRAFTVIT0000");
		} else if (region == "us" || region == "ca") {
			sceAppMgrLaunchAppByUri(0x60000, "psts:browse?product=UP4433-PCSE00491_00-MINECRAFTVIT0000");
		} else {
			sceAppMgrLaunchAppByUri(0x60000, "psts:browse?product=EP4433-PCSB00560_00-MINECRAFTVIT0000");
		}
	}

	void showKeyboard() override {
		int ret;
		static SceUInt32 ime_workram[SCE_IME_WORK_BUFFER_SIZE / sizeof(SceInt32)] = {0};
	
		SceImeParam param;
		sceImeParamInit(&param);

		param.supportedLanguages = SCE_IME_LANGUAGE_ENGLISH_GB;
		param.languagesForced = SCE_FALSE;
		param.type = SCE_IME_TYPE_BASIC_LATIN;
		param.option = 0;

		param.inputTextBuffer = ime_out;
		param.maxTextLength = SCE_IME_MAX_TEXT_LENGTH;
		param.enterLabel = SCE_IME_ENTER_LABEL_DEFAULT;
		param.handler = ImeEventHandler;
		param.filter = NULL;
		param.initialText = (SceWChar16 *)ime_inital;
		param.arg = NULL;
		param.work = ime_workram;

		sceImeOpen(&param);
		ime_is_open = true;
	}

	void hideKeyboard() override {
		sceImeClose();
		ime_is_open = false;
	}

	bool isKeyboardVisible() override {
		return ime_is_open;
	}

	std::string getKeyboardInput() override {
		Utf16ToUtf8(ime_out, ime_out_utf8);
		std::string imetxt = std::string((char*)ime_out_utf8);
		return imetxt;
	}


	bool isPowerVR() override { return true; }

	std::string defaultUsername() override {
		SceNpId npid;
		int ret = sceNpManagerGetNpId(&npid);

		if(ret >= 0) {
			return std::string(npid.handle.data);
		}
		else {
			LOGI("Failed to read npid: %x", ret);

			// read from registry as a fallback
			char rd_username[0x100] = {0};
			ret = sceRegMgrGetKeyStr("/CONFIG/SYSTEM", "username", rd_username, sizeof(rd_username));
			if(ret >= 0) {
				return std::string(rd_username);
			}

		}

		return "Vita";
	}

	BinaryBlob readAssetFile(const std::string& filename) override {
		std::string fullAssetPath = ("app0:data/" + filename);

		LOGI("fullAssetPath: %s\n", fullAssetPath.c_str());
		SceIoStat stat;
		int ret = sceIoGetstat(fullAssetPath.c_str(), &stat);
		if(ret < 0) {
			LOGI("failed to stat: %x %s\n", ret,fullAssetPath.c_str());
			return BinaryBlob();
		}

		SceUID fd = sceIoOpen(fullAssetPath.c_str(), SCE_O_RDONLY, 0777);

		if(fd < 0) {
			LOGI("failed to open: %x %s\n", fd, fullAssetPath.c_str());
			return BinaryBlob();
		}


		BinaryBlob blob;
		blob.size = stat.st_size;
		blob.data = new unsigned char[blob.size];

		int rd = sceIoRead(fd, blob.data, blob.size);
		sceIoClose(fd);

		if(rd != blob.size) {
			LOGI("wrong size: %x %s\n", rd, fullAssetPath.c_str());

			return BinaryBlob();
		}

		LOGI("read %x bytes from %s\n", rd, fullAssetPath.c_str());

		return blob;
	}

	TextureData loadTexture(const std::string& filename_, bool textureFolder) override {
		TextureData out;

		std::string filename = textureFolder ? "data/images/" + filename_
		: filename_;
		std::ifstream source(filename.c_str(), std::ios::binary);

		if (source) {
			png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

			if (!pngPtr)
				return out;

			png_infop infoPtr = png_create_info_struct(pngPtr);

			if (!infoPtr) {
				png_destroy_read_struct(&pngPtr, NULL, NULL);
				return out;
			}

			// Hack to get around the broken libpng for windows
			png_set_read_fn(pngPtr,(void*)&source, png_funcReadFile);

			png_read_info(pngPtr, infoPtr);

			// Set up the texdata properties
			out.w = png_get_image_width(pngPtr, infoPtr);
			out.h = png_get_image_height(pngPtr, infoPtr);

			png_bytep* rowPtrs = new png_bytep[out.h];
			out.data = new unsigned char[4 * out.w * out.h];
			out.memoryHandledExternally = false;

			int rowStrideBytes = 4 * out.w;
			for (int i = 0; i < out.h; i++) {
				rowPtrs[i] = (png_bytep)&out.data[i*rowStrideBytes];
			}
			png_read_image(pngPtr, rowPtrs);

			// Teardown and return
			png_destroy_read_struct(&pngPtr, &infoPtr,(png_infopp)0);
			delete[] (png_bytep)rowPtrs;
			source.close();

			return out;
		}
		else
		{
			LOGI("Couldn't find file: %s\n", filename.c_str());
			return out;
		}
	}
};

#endif
