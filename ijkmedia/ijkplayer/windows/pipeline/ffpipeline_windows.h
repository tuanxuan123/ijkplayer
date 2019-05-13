
#ifndef FFPLAY__FF_FFPIPELINE_WINDOWS_H
#define FFPLAY__FF_FFPIPELINE_WINDOWS_H

#include "../../ff_ffpipeline.h"


typedef struct FFPlayer       FFPlayer;
typedef struct IJKFF_Pipeline IJKFF_Pipeline;

IJKFF_Pipeline *ffpipeline_create_from_windows(FFPlayer *ffp);



#endif
