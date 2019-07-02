//
// Created by amourao on 26-06-2019.
//

#include <ctime>
#include <iostream>
#include <string>
#include <stdlib.h>

#include <asio.hpp>

#include "FrameStruct.hpp"
#include "FrameReader.h"
#include "Utils.hpp"

using asio::ip::tcp;

int main(int argc, char *argv[]) {

    try {
        if (argc < 3) {
            std::cerr << "Usage: server <port> <frame_list> (<stop after>)" << std::endl;
            return 1;
        }

        asio::io_context io_context;
        uint port = std::stoul(argv[1]);

        std::string name = std::string(argv[2]);

        int stopAfter = INT_MAX;
        if (argc >= 4) {
            stopAfter = std::stoi(argv[3]);
        }

        FrameReader reader(name);

        uint fps = reader.getFps();


        uint64_t last_time = current_time_ms();
        uint64_t start_time = last_time;
        uint64_t start_frame_time = last_time;
        uint64_t sent_frames = 0;
        uint64_t processing_time = 0;
        double sent_mbytes = 0;

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

        while (stopAfter > 0) {
            // try to maintain constant FPS by ignoring processing time
            uint64_t sleep_time = (1000 / fps) - processing_time;

            if (sleep_time >= 1)
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
            start_frame_time = current_time_ms();
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            if (sent_frames == 0) {
                last_time = current_time_ms();
                start_time = last_time;
            }

            std::string message = reader.currentFrameBytes();
            uint frameId = reader.currentFrameId();
            if (reader.hasNextFrame())
                reader.nextFrame();
            else {
                reader.reset();
                stopAfter--;
            }



            asio::error_code ignored_error;
            asio::write(socket, asio::buffer(message), ignored_error);

            socket.close();
            sent_frames += 1;
            sent_mbytes += message.size()/1000.0;

            uint64_t diff_time = current_time_ms() - last_time;

            double diff_start_time = (current_time_ms() - start_time);
            int64_t avg_fps;
            if (diff_start_time == 0)
                avg_fps = -1;
            else
                avg_fps = 1000 / (diff_start_time / (double) sent_frames);

            last_time = current_time_ms();
            processing_time = last_time - start_frame_time;

            std::cout << "Frame \t" << frameId << " sent, took " << diff_time << " ms; size " << message.size()
                      << "; avg " << avg_fps << " fps; " << 8*(sent_mbytes/diff_start_time) << " Mbps" << std::endl;



        }
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
