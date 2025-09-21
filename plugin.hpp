#pragma once
#include "illixr/data_format/string_data.hpp" 
#include "illixr/phonebook.hpp"
#include "illixr/plugin.hpp"
#include "illixr/switchboard.hpp"
#include "portaudio.h"

namespace ILLIXR {

class sr_plugin : public plugin {
public:
    [[maybe_unused]] sr_plugin(const std::string& name, phonebook* pb);
    void start() override;
    ~sr_plugin() override = default;

private:
    const std::shared_ptr<switchboard>                   switchboard_;
    switchboard::writer<data_format::string_data> transcript_publisher_;
    int record();
};

    static int recordCallback(const void* inputBuffer, void* /*outputBuffer*/,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* /*timeInfo*/,
                          PaStreamCallbackFlags /*statusFlags*/,
                          void* userData);
} // namespace ILLIXR