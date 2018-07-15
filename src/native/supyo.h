//
// Created by Fabio Cigliano on 02/07/18.
//

#ifndef CAMERA_WRAPPER_SUPYO_H
#define CAMERA_WRAPPER_SUPYO_H

#include "types.h"
#include "pico/picort.h"

/*
 * object detection parameters
 */

/**
 * how much to rescale the window during the multiscale detection process
 * increasing this value leads to lower number of detections and higher processing
 * speed for example, set to 1.2f if you're using pico on a mobile device
 */
#ifndef SCALEFACTOR
#define SCALEFACTOR 1.1f
#endif

/**
 * how much to move the window between neighboring detections increasing this
 * value leads to lower number of detections and higher processing speed
 * for example, set to 0.05f if you want really high recall
 */
#ifndef STRIDEFACTOR
#define STRIDEFACTOR 0.1f
#endif

/**
 * max number of detected objects
 */
#define MAXNDETECTIONS 2048

bool detect(cv::Mat greyFrame, int32_t minsize, float cutoff);

#endif //CAMERA_WRAPPER_SUPYO_H
