#include "plugin.hpp"

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include "json.hpp"
#include "httplib.h"
// #include <chrono>
// #include <regex>
// #include <thread>

using namespace ILLIXR;
using namespace ILLIXR::data_format;


[[maybe_unused]] sr_plugin::sr_plugin(const std::string& name, phonebook* pb)
    : plugin{name, pb}
    , switchboard_{phonebook_->lookup_impl<switchboard>()} 
    , transcript_publisher_{switchboard_->get_writer<string_data>("transcript_topic")} {}

void sr_plugin::start() {
    // record_audio_with_ffmpeg(); // we use provided sample here to test
    const std::string audio_file = "samples/jfk.wav"; // replace with your audio file path

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
}
void sr_plugin::record_audio_with_ffmpeg() {
    std::string cmd =
        "ffmpeg -y -f alsa -i default -t 5 -ac 1 -ar 16000 -sample_fmt s16 audio.wav";
    std::cout << "Recording audio...\n";
    int result = std::system(cmd.c_str());
    if (result != 0) {
        std::cerr << "FFmpeg failed.\n";
    }
}



PLUGIN_MAIN(sr_plugin)
