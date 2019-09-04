//
// Created by amourao on 27-06-2019.
//

#pragma once

#include <fstream>
#include <iostream>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}

#include <cereal/archives/binary.hpp>

#include "../structs/FrameStruct.hpp"
#include "../utils/Utils.h"

// TODO: add support to kinect video?
class VideoFileReader {
private:
  unsigned int fps;
  std::string sceneDesc;
  std::string type;
  unsigned int sensorId;
  unsigned int deviceId;
  std::string filename;

  int video_stream_index;

  AVFormatContext *pFormatContext;
  AVCodecParameters *pCodecParameters;
  AVCodecContext *pCodecContext;
  AVCodec *pCodec;

  AVFrame *pFrame;
  AVPacket *pPacket;

  bool libAVReady;

  bool eofReached;
  unsigned int currentFrameCounter;

  std::string streamId;

  void init(std::string &filename);

public:
  VideoFileReader(std::string &filename);

  ~VideoFileReader();

  void reset();

  void goToFrame(unsigned int frameId);

  bool hasNextFrame();

  void nextFrame();

  std::vector<unsigned char> currentFrameBytes();

  unsigned int currentFrameId();

  std::string getSceneDesc();

  unsigned int getFps();

  unsigned int getSensorId();

  unsigned int getDeviceId();

  CodecParamsStruct getCodecParamsStruct();

  std::string getStreamID();
};
