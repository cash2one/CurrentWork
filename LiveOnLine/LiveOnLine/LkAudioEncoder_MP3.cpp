
#include "LkAudioEncoder_MP3.h"
#include "LkLog.h"
#include "LkUtil.h"

LkAudioEncoder_MP3::LkAudioEncoder_MP3()
    : m_lame_global_flags(NULL)
{
    m_encoderType = MP3;
}

bool LkAudioEncoder_MP3::init(int samplerate, int channel, int bitrate)
{
    if (channel < 1 || channel > 6)
        return false;

    log_trace("mp3 sample_rate=%d, channel=%d, bitrate=%d", samplerate, channel, bitrate);
    m_samplerate = samplerate;
    m_channels = channel;
    m_bitrate = bitrate;

    m_lame_global_flags = lame_init();
    lame_set_in_samplerate(m_lame_global_flags, samplerate);
    lame_set_out_samplerate(m_lame_global_flags, samplerate);
    lame_set_num_channels(m_lame_global_flags, channel);

    lame_set_mode(m_lame_global_flags, STEREO);

    // @see ${ffmpeg}/libavcodec/libmp3lame.c
    // do not get a Xing VBR header frame from LAME
    lame_set_bWriteVbrTag(m_lame_global_flags,0);

    // bit reservoir usage
    lame_set_disable_reservoir(m_lame_global_flags, 1);

    lame_set_VBR(m_lame_global_flags, vbr_off);

    // limp3lame use kbps
    lame_set_brate(m_lame_global_flags, bitrate / 1000);

    // init params
    lame_init_params(m_lame_global_flags);

    return true;
}

void LkAudioEncoder_MP3::fini()
{

}

bool LkAudioEncoder_MP3::encode(const QByteArray &data, QByteArray &output)
{
    if (data.size() != getFrameSize()) {
        log_error("mp3 data is too not equa to %d", getFrameSize());
        return false;
    }

    unsigned char MP3OutputBuffer[1152*2];

    int encode_bytes = -1;
    if (m_channels == 2) {
        // @note
        // number of samples per channel,
        // _not_ number of samples in pcm[]
        encode_bytes = lame_encode_buffer_interleaved(m_lame_global_flags, (short int*)data.data(), lame_get_framesize(m_lame_global_flags), MP3OutputBuffer, 1152*2);
    } else if (m_channels == 1) {
        const short int *_data = (const short int*)data.data();
        encode_bytes = lame_encode_buffer(m_lame_global_flags, _data, NULL, lame_get_framesize(m_lame_global_flags), MP3OutputBuffer, 1152*2);
    }

    if (encode_bytes < 0) {
        log_error("lame_encode_buffer_interleaved_ieee_float failed.");
        return false;
    } else if (encode_bytes == 0) {
        log_trace("frame delayed in libmp3lame.");
        return true;
    }

    if (m_samplerate == 44100) {
        if (m_channels == 2) {
            output.append(0x2f);
        } else if (m_channels == 1) {
            output.append(0x2e);
        }
    } else if (m_samplerate == 22050) {
        if (m_channels == 2) {
            output.append(0x2b);
        } else if (m_channels == 1) {
            output.append(0x2a);
        }
    } else if (m_samplerate == 11025) {
        if (m_channels == 2) {
            output.append(0x27);
        } else if (m_channels == 1) {
            output.append(0x26);
        }
    }

    output.append((char*)MP3OutputBuffer, encode_bytes);

    return true;
}

int LkAudioEncoder_MP3::getFrameSize()
{
    LkAssert(m_lame_global_flags);
    // 2 byte per sample
    int frameSize = lame_get_framesize(m_lame_global_flags) * (16 / 8) * m_channels;

    return frameSize;
}

float LkAudioEncoder_MP3::getFrameDuration()
{
    LkAssert(m_lame_global_flags);
    return (float)lame_get_framesize(m_lame_global_flags) * 1000.00 / (float)m_samplerate;
}
