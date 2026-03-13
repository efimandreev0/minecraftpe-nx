#include "SoundSystemVita.h"
#include "../../util/Mth.h"
#include "../log.h"

#include <psp2/audioout.h>
#include <psp2common/defs.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/sysmodule.h>
#include <cstring>
#include <cmath>
#include "ngs/ngs.h"


static void checkNgs(SceInt32 rc, const char* tag) {
    if (rc != SCE_OK)
        LOGI("### NGS error [%s]: 0x%08X\n", tag, rc);
}

static void calculateVolume(Vec3 listener_pos, float listener_rot, Vec3 sound_pos, float volume, float* vol_l, float* vol_r) {
    Vec3 dist_v = sound_pos - listener_pos;
    float dist = std::sqrt(dist_v.x*dist_v.x + dist_v.y*dist_v.y + dist_v.z*dist_v.z);

    // get listener facing vector rotated by 90 degrees
    float rad = (listener_rot + 90.0f) * Mth::DEGRAD;
    float lx = -Mth::sin(rad);
    float lz = Mth::cos(rad);

    // normalized vector pointing to sound
    float nx = (dist > 0.001f) ? dist_v.x / dist : 0.0f;
    float nz = (dist > 0.001f) ? dist_v.z / dist : 0.0f;
    
    // pan = dot(r, n) -1 to 1
    float pan = nx * lx + nz * lz;
    // pan = how much to the right the sound is
    // 1.0 = all the way right, -1.0 = all the way left
    float normalizedPan = (pan + 1.0f) * 0.5f; // 0 to 1

    *vol_l = volume * std::sqrt(1.0f - normalizedPan);
    *vol_r = volume * std::sqrt(normalizedPan);
}

int SoundSystemVita::createRack(const SceNgsRackDescription* rackDesc, SceNgsHRack *pRackHandle) {
    SceNgsBufferInfo bufferInfo;
    int ret = sceNgsRackGetRequiredMemorySize(_ngsSys, rackDesc, &bufferInfo.size);
    checkNgs(ret, "GetRackMemSize");
    if(ret < 0) {
        return ret;
    }

    bufferInfo.data = aligned_alloc(16, bufferInfo.size);
    if (!bufferInfo.data) {
        LOGE("NGS rack malloc failed\n");
        return false;
    }

    ret = sceNgsRackInit(_ngsSys, &bufferInfo, rackDesc, pRackHandle);
    checkNgs(ret, "RackInit");
    return ret;
}

int SoundSystemVita::connectRacks(SceNgsHVoice voiceSource, SceNgsHVoice voiceDest, SceNgsHPatch* pPatch) {
    int ret;
    SceNgsHPatch patch;
    SceNgsPatchSetupInfo patchInfo = {
        .hVoiceSource = voiceSource,
        .nSourceOutputIndex = 0,
        .nSourceOutputSubIndex = SCE_NGS_VOICE_PATCH_AUTO_SUBINDEX,
        .hVoiceDestination = voiceDest,
        .nTargetInputIndex = 0
    };
    ret = sceNgsPatchCreateRouting(&patchInfo, &patch);
    checkNgs(ret, "sceNgsPatchCreateRouting");

    SceNgsVolumeMatrix vols;
    vols.m[0][0] = 1.0f; // left to left
    vols.m[0][1] = 0.0f; // left to right
    vols.m[1][0] = 0.0f; // right to left
    vols.m[1][1] = 1.0f; // right to right
    ret = sceNgsVoicePatchSetVolumesMatrix(patch, &vols);
    checkNgs(ret, "sceNgsVoicePatchSetVolumesMatrix");
    *pPatch = patch;
    return 0;
}

SoundSystemVita::SoundSystemVita()
:   available(true),
    _rotation(-9999.9f),
    _ngsSys(SCE_NGS_INVALID_HANDLE),
    _rackPlayer(SCE_NGS_INVALID_HANDLE),
    _rackEq(SCE_NGS_INVALID_HANDLE) {
    init();
}

SoundSystemVita::~SoundSystemVita() {
    for (int i = 0; i < NUM_VOICES; ++i) {
        if (_voices[i] != SCE_NGS_INVALID_HANDLE) {
            sceNgsVoiceKill(_voices[i]);
        }
    }

    if (_rackPlayer != SCE_NGS_INVALID_HANDLE)
        sceNgsRackRelease(_rackPlayer, nullptr);

    if (_ngsSys != SCE_NGS_INVALID_HANDLE)
        sceNgsSystemRelease(_ngsSys);
}

void SoundSystemVita::init() {
    sceSysmoduleLoadModule(SCE_SYSMODULE_NGS);

    SceNgsSystemInitParams sysParams = {
        .nMaxRacks      = 2,
        .nMaxVoices     = NUM_VOICES + 1,
        .nGranularity   = SYS_GRANULARITY,
        .nSampleRate    = 48000,
        .nMaxModules    = NUM_MODULES,
    };

    // system init
    SceNgsBufferInfo sysBuffer;
    int ret = sceNgsSystemGetRequiredMemorySize(&sysParams, &sysBuffer.size);
    checkNgs(ret, "GetSysMemSize");
    if (ret != SCE_OK) { available = false; return; }
    LOGI("sysBuffer.size=%d\n", sysBuffer.size);

    sysBuffer.data = aligned_alloc(16, sysBuffer.size);
    if (!sysBuffer.data) { LOGE("NGS sys malloc failed\n"); available = false; return; }

    ret = sceNgsSystemInit(sysBuffer.data, sysBuffer.size, &sysParams, &_ngsSys);
    checkNgs(ret, "SystemInit");
    if (ret != SCE_OK) { available = false; return; }


    // create master rack
    SceNgsRackDescription rackDesc;
    rackDesc = {
        .pVoiceDefn             = sceNgsVoiceDefGetMasterBuss(),
        .nVoices                = 1,
        .nChannelsPerVoice      = 2,
        .nMaxPatchesPerInput    = NUM_VOICES,
        .nPatchesPerOutput      = 0,
    };
    ret = createRack(&rackDesc, &_rackMaster);
    checkNgs(ret, "createRack(master)");
    if(ret < 0) { available = false; return; }

    ret = sceNgsRackGetVoiceHandle(_rackMaster, 0, &_voiceMaster);
    checkNgs(ret, "sceNgsRackGetVoiceHandle(_rackMaster)");


    // create player rack
    rackDesc = {
        .pVoiceDefn             = sceNgsVoiceDefGetSimpleVoice(),
        .nVoices                = NUM_VOICES,
        .nChannelsPerVoice      = 2,
        .nMaxPatchesPerInput    = 0,
        .nPatchesPerOutput      = 1,
    };
    ret = createRack(&rackDesc, &_rackPlayer);
    checkNgs(ret, "createRack(player)");
    if(ret < 0) { available = false; return; }

    for (int i = 0; i < NUM_VOICES; ++i) {
        ret = sceNgsRackGetVoiceHandle(_rackPlayer, i, &_voices[i]);
        checkNgs(ret, "sceNgsRackGetVoiceHandle(_rackPlayer i)");
        connectRacks(_voices[i], _voiceMaster, &_patches[i]);
    }

    sceNgsVoicePlay(_voiceMaster);

    for ( int i = 0; i < 2; ++i ) {
		this->_outputBuffers[i] = (short *)malloc(OUTPUT_BUF_SIZE);
		if ( this->_outputBuffers[i] == NULL ) {
            LOGI("failed to alloc output buffer\n");
			return;
		}
	}

    ret = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, SYS_GRANULARITY, 48000, SCE_AUDIO_OUT_MODE_STEREO);
    if(ret < 0) {
        LOGI("sceAudioOutOpenPort: %08x\n", ret);
        return;
    }
    this->_audioPort = ret;

    SoundSystemVita* sys = this;
    int thid = sceKernelCreateThread("audio", SoundSystemVita::audioThread, 0x40, 0x4000, 0, SCE_KERNEL_CPU_MASK_USER_2, NULL);
    ret = sceKernelStartThread(thid, 4, &sys);
    checkNgs(ret, "sceKernelStartThread");
}

void SoundSystemVita::enable(bool status) {
    LOGI("SoundSystemVita::enable(%d)\n", status);
    if (!available) return;

    if (!status) {
        for (int i = 0; i < NUM_VOICES; ++i) {
            sceNgsVoiceKill(_voices[i]);
        }
    }
}

void SoundSystemVita::setListenerPos(float x, float y, float z) {
    _position = Vec3(x, y, z);
}

void SoundSystemVita::setListenerAngle(float deg) {
    _rotation = deg;
}

static void initPlayer(SceNgsHVoice voice, const SoundDesc& sound, float pitch) {
    int ret;
    SceNgsBufferInfo bufferInfo;
    SceNgsPlayerParams *pPcmParams;
    
    sceNgsVoiceLockParams(voice, 0, SCE_NGS_PLAYER_PARAMS_STRUCT_ID, &bufferInfo);
    memset(bufferInfo.data, 0, bufferInfo.size);
    pPcmParams = (SceNgsPlayerParams *)bufferInfo.data;
    pPcmParams->desc.id     = SCE_NGS_PLAYER_PARAMS_STRUCT_ID;
	pPcmParams->desc.size   = sizeof(SceNgsPlayerParams);

    pPcmParams->fPlaybackFrequency          = sound.frameRate;
	pPcmParams->fPlaybackScalar             = pitch;
	pPcmParams->nLeadInSamples              = 0;
	pPcmParams->nLimitNumberOfSamplesPlayed = 0;
	pPcmParams->nChannels                   = sound.channels;
	if (sound.channels == 1) {
		pPcmParams->nChannelMap[0] = SCE_NGS_PLAYER_LEFT_CHANNEL;
		pPcmParams->nChannelMap[1] = SCE_NGS_PLAYER_LEFT_CHANNEL;
	} else {
		pPcmParams->nChannelMap[0] = SCE_NGS_PLAYER_LEFT_CHANNEL;
		pPcmParams->nChannelMap[1] = SCE_NGS_PLAYER_RIGHT_CHANNEL;
	}
	pPcmParams->nType            = SCE_NGS_PLAYER_TYPE_PCM;
	pPcmParams->nStartBuffer     = 0;
	pPcmParams->nStartByte       = 0;

	pPcmParams->buffs[0].pBuffer    = sound.frames;
	pPcmParams->buffs[0].nNumBytes  = sound.size;
	pPcmParams->buffs[0].nLoopCount = 0;
	pPcmParams->buffs[0].nNextBuff  = SCE_NGS_PLAYER_NO_NEXT_BUFFER;

    sceNgsVoiceUnlockParams(voice, 0);
}

void SoundSystemVita::playAt(const SoundDesc& sound,
                             float x, float y, float z,
                             float volume, float pitch){
    if (!available) return;
    if (pitch < 0.01f) pitch = 1.0f;

    SceNgsHVoice voice = _voices[_voiceIndex];
    SceNgsHPatch patch = _patches[_voiceIndex];
    _voiceIndex++;
    if(_voiceIndex >= NUM_VOICES) {
        _voiceIndex = 0;
    }
    sceNgsVoiceKill(voice);

    float volume_l = volume;
    float volume_r = volume;
    if(x != 0 || y != 0 || z != 0) { // dont make menu sounds directional
        Vec3 sound_pos = Vec3(x, y, z);
        calculateVolume(_position, _rotation, sound_pos, volume, &volume_l, &volume_r);
    }

    LOGI("playAt x=%.2f y=%.2f z=%.2f volume=%.2f\n", x, y, z, volume);
    SceNgsVolumeMatrix vols;
    vols.m[0][0] = volume_l; // left to left
    vols.m[0][1] = 0.0f;   // left to right
    vols.m[1][0] = 0.0f;   // right to left
    vols.m[1][1] = volume_r; // right to right

    initPlayer(voice, sound, pitch);    
    sceNgsVoicePatchSetVolumesMatrix(patch, &vols);
    sceNgsVoicePlay(voice);
}

int SoundSystemVita::audioThread(size_t argc, void* argv) {
    SoundSystemVita* sys = *(SoundSystemVita**)argv;
    int bufIndex = 0;
    while(1) {
        sceNgsSystemUpdate(sys->_ngsSys);
        sceNgsVoiceGetStateData(sys->_voiceMaster, SCE_NGS_MASTER_BUSS_OUTPUT_MODULE, sys->_outputBuffers[bufIndex], OUTPUT_BUF_SIZE);
        sceAudioOutOutput(sys->_audioPort, sys->_outputBuffers[bufIndex]);

        bufIndex++;
        if(bufIndex >= 2) {
            bufIndex = 0;
        }
    }
}
