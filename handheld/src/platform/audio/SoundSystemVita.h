#ifndef SoundSystemVita_H__
#define SoundSystemVita_H__
#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>
#include <vector>
#include <string>
#include "../../client/sound/Sound.h"
#include "SoundSystem.h"
#include "ngs/ngs.h"

class SoundSystemVita : public SoundSystem {
public:
    SoundSystemVita();
    ~SoundSystemVita();

    void enable(bool status) override;

    void setListenerPos(float x, float y, float z) override;
    void setListenerAngle(float deg) override;
    void playAt(const SoundDesc& sound, float x, float y, float z, float volume, float pitch) override;

private:
    static const int NUM_VOICES = 16;
    static const int SYS_GRANULARITY = 512;
    static const int NUM_MODULES = 14;
    static const int OUTPUT_BUF_SIZE = SYS_GRANULARITY * 2 * sizeof(short);

    bool available;

    Vec3 _position;
    float _rotation;

    SceNgsHSynSystem _ngsSys;
    SceNgsHRack   _rackMaster;
    SceNgsHRack   _rackPlayer;
    SceNgsHRack   _rackEq;

    SceNgsHVoice _voiceMaster;
    SceNgsHVoice _voices[NUM_VOICES];
    SceNgsHPatch _patches[NUM_VOICES];
    int          _voiceIndex;

    short* _outputBuffers[2];
    int _audioPort;

    void init();
    int createRack(const SceNgsRackDescription* desc, SceNgsHRack *pRackHandle);
    int connectRacks(SceNgsHVoice voiceSource, SceNgsHVoice voiceDest, SceNgsHPatch* pPatch);
    static int audioThread(size_t argc, void* argv);
};
#endif