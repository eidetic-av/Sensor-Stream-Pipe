//
// Created by amourao on 26-06-2019.
//

#include <ctime>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <thread>
#include <unistd.h>


extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}

#include <zmq.hpp>

#include "../structs/FrameStruct.hpp"
#include "../encoders/FrameEncoder.h"
#include "../utils/Utils.h"


int main(int argc, char *argv[]) {

    srand(time(NULL) * getpid());
    //srand(getpid());

    try {
        if (argc < 5) {
            std::cerr << "Usage: server <host> <port> <codec parameters> <frame_file>"
                      << std::endl;
            return 1;
        }

        zmq::context_t context(1);
        zmq::socket_t socket(context, ZMQ_PUSH);


        std::string host = std::string(argv[1]);
        uint port = std::stoul(argv[2]);

        std::string frame_file = std::string(argv[3]);
        std::string codec_parameters_file = std::string(argv[4]);

        int stopAfter = INT_MAX;
        if (argc >= 6) {
            stopAfter = std::stoi(argv[5]);
        }

        FrameReader reader(frame_file);
        FrameEncoder frameEncoder(codec_parameters_file, reader.getFps());

        uint fps = reader.getFps();

        uint64_t last_time = currentTimeMs();
        uint64_t start_time = last_time;
        uint64_t start_frame_time = last_time;
        uint64_t sent_frames = 0;
        uint64_t processing_time = 0;

        double sent_mbytes = 0;

        socket.connect("tcp://" + host + ":" + std::string(argv[2]));

        while (stopAfter > 0) {
            // try to maintain constant FPS by ignoring processing time
            uint64_t sleep_time = (1000 / fps) - processing_time;

            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
            start_frame_time = currentTimeMs();

            if (sent_frames == 0) {
                last_time = currentTimeMs();
                start_time = last_time;
            }

            while (!frameEncoder.hasNextPacket()) {
                frameEncoder.addFrameStruct(reader.currentFrame().front());
                if (!reader.hasNextFrame()) {
                    reader.reset();
                    stopAfter--;
                }
                reader.nextFrame();
            }

            FrameStruct f = frameEncoder.currentFrame();
            std::vector<FrameStruct> v;
            v.push_back(f);

            frameEncoder.nextPacket();

            std::string message = cerealStructToString(v);

            zmq::message_t request(message.size());
            memcpy(request.data(), message.c_str(), message.size());

            socket.send(request);
            sent_frames += 1;
            sent_mbytes += message.size() / 1000.0;

            uint64_t diff_time = currentTimeMs() - last_time;

            double diff_start_time = (currentTimeMs() - start_time);
            int64_t avg_fps;
            if (diff_start_time == 0)
                avg_fps = -1;
            else
                avg_fps = 1000 / (diff_start_time / (double) sent_frames);


            last_time = currentTimeMs();
            processing_time = last_time - start_frame_time;

            std::cout << "Took " << diff_time << " ms; size " << message.size()
                      << "; avg " << avg_fps << " fps; " << 8 * (sent_mbytes / diff_start_time) << " Mbps "
                      << 8 * (sent_mbytes * reader.getFps() / (sent_frames * 1000)) << " Mbps expected "
                      << std::endl;
            for (uint i = 0; i < v.size(); i++) {
                FrameStruct f = v.at(i);
                std::cout << "\t" << f.deviceId << ";" << f.sensorId << ";" << f.frameId << " sent" << std::endl;
            }
        }
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
