#include "plugin.hpp"

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include "json.hpp"
#include "httplib.h"

#include <vector>
#include <fstream>
#include "portaudio.h"


using namespace ILLIXR;
using namespace ILLIXR::data_format;

constexpr double SAMPLE_RATE = 44100.0;
constexpr unsigned long FRAMES_PER_BUFFER = 512;
constexpr int NUM_SECONDS = 5;
constexpr int NUM_CHANNELS = 1;
using SAMPLE = float;
constexpr SAMPLE SAMPLE_SILENCE = 0.0f;

struct RecordData {
    std::vector<SAMPLE> buffer;
    int frameIndex = 0;
    int maxFrameIndex = 0;
};

[[maybe_unused]] sr_plugin::sr_plugin(const std::string& name, phonebook* pb)
    : plugin{name, pb}
    , switchboard_{phonebook_->lookup_impl<switchboard>()} 
    , transcript_publisher_{switchboard_->get_writer<string_data>("transcript_topic")} {}

void sr_plugin::start() {

    const std::string audio_file = "/scratch/bmishra3/dog.wav"; // replace with your audio file path

    httplib::Client cli("http://127.0.0.1:9999");

    httplib::MultipartFormDataItems items = {
    { "file", audio_file },
    { "temperature", "0.0" },
    { "temperature_inc", "0.2"},
    { "response_format", "text" }
    };

    auto res = cli.Post("/inference", items);
    // std::cout << "Response status after POST: " << res->status << std::endl;
    if (res->status != 200) {
        std::cerr << "Error: " << res->status << " - " << res->body << std::endl;
        return;
    }
    std::cout << "Transcript: " << res->body << std::endl;

    string_data data{res->body};
    transcript_publisher_.put(std::make_shared<string_data>(data));
    std::cout << "record:" << record() << std::endl;
}

namespace ILLIXR {
static int recordCallback(const void* inputBuffer, void* /*outputBuffer*/,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* /*timeInfo*/,
                          PaStreamCallbackFlags /*statusFlags*/,
                          void* userData)
{
    auto* data = static_cast<RecordData*>(userData);
    const SAMPLE* rptr = static_cast<const SAMPLE*>(inputBuffer);
    if (!rptr) {
        // input silence
        for (unsigned long i = 0; i < framesPerBuffer; ++i) {
            for (int c = 0; c < NUM_CHANNELS; ++c) {
                data->buffer[data->frameIndex * NUM_CHANNELS + c] = SAMPLE_SILENCE;
            }
            data->frameIndex++;
        }
    } else {
        unsigned long framesToCopy = framesPerBuffer;
        unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;
        if (framesLeft < framesToCopy) framesToCopy = framesLeft;
        for (unsigned long i = 0; i < framesToCopy; ++i) {
            for (int c = 0; c < NUM_CHANNELS; ++c) {
                data->buffer[data->frameIndex * NUM_CHANNELS + c] = *rptr++;
            }
            data->frameIndex++;
        }
    }
    return (data->frameIndex >= data->maxFrameIndex) ? paComplete : paContinue;
}
}

int sr_plugin::record() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::fprintf(stderr, "Pa_Initialize error: %s\n", Pa_GetErrorText(err));
        return 1;
    }

    RecordData data;
    data.maxFrameIndex = static_cast<int>(NUM_SECONDS * SAMPLE_RATE);
    data.buffer.resize(data.maxFrameIndex * NUM_CHANNELS, SAMPLE_SILENCE);

    PaStreamParameters inputParams {};
    inputParams.device = Pa_GetDefaultInputDevice();
    if (inputParams.device == paNoDevice) {
        std::fprintf(stderr, "No default input device.\n");
        Pa_Terminate();
        return 1;
    }
    inputParams.channelCount = NUM_CHANNELS;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    PaStream* stream = nullptr;
    err = Pa_OpenStream(&stream,
                        &inputParams,
                        nullptr,
                        SAMPLE_RATE,
                        FRAMES_PER_BUFFER,
                        paClipOff,
                        recordCallback,
                        &data);
    if (err != paNoError) {
        std::fprintf(stderr, "Pa_OpenStream error: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();
        return 1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::fprintf(stderr, "Pa_StartStream error: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    std::cout << "Recording " << NUM_SECONDS << " seconds..." << std::endl;
    while ((err = Pa_IsStreamActive(stream)) == 1) {
        Pa_Sleep(100);
    }
    if (err < 0) {
        std::fprintf(stderr, "Pa_IsStreamActive error: %s\n", Pa_GetErrorText(err));
    }

    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        std::fprintf(stderr, "Pa_CloseStream error: %s\n", Pa_GetErrorText(err));
    }

    Pa_Terminate();

    // Save raw float32 interleaved data to file
    const char* outFilename = "recorded.raw";
    std::ofstream ofs(outFilename, std::ios::binary);
    if (!ofs) {
        std::cerr << "Failed to open " << outFilename << " for writing\n";
        return 1;
    }
    ofs.write(reinterpret_cast<const char*>(data.buffer.data()),
              data.buffer.size() * sizeof(SAMPLE));
    ofs.close();

    std::cout << "Recorded " << data.frameIndex << " frames into " << outFilename << std::endl;
    std::cout << "To play: ffmpeg -f f32le -ar " << static_cast<int>(SAMPLE_RATE)
              << " -ac " << NUM_CHANNELS << " " << outFilename << std::endl;

    return 0;
}




PLUGIN_MAIN(sr_plugin)
