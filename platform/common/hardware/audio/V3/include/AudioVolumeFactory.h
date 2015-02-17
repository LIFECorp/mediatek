#ifndef AUDIO_VOLUME_FACTORY_H
#define AUDIO_VOLUME_FACTORY_H

#include "AudioALSAVolumeController.h"
#include "AudioVolumeInterface.h"

class AudioVolumeFactory
{
    public:
        // here to implement create and
        static AudioVolumeInterface *CreateAudioVolumeController();
        static void DestroyAudioVolumeController();
    private:
};

#endif
