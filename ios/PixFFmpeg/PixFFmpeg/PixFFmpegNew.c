//
//  PixFFmpegNew.c
//  PixFFmpeg
//
//  Created by xanderdeng on 2023/6/6.
//

#include "PixFFmpegNew.h"

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

void PixVideo_Handle_All_Feature()
{
    avcodec_register_all();
    avformat_network_init();
}
