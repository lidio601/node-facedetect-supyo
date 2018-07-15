//
// Created by Fabio Cigliano on 02/07/18.
//

#include "thread.h"

// Core
#include <iostream>
#include <fstream>
#include <stdio.h>

// Node.js deps
#include <node.h>
#include <v8.h>

// OpenCV deps
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>

// OpenCV face deps
#include <opencv2/features2d.hpp>
#include <opencv2/objdetect.hpp>

#include <vector>

// Pico library wrapper
#include "supyo.h"

/*
 * - send an snapshot
 */
void updateAsync(uv_async_t* req, int status)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    AsyncMessage* asyncMessage = (AsyncMessage*) req->data;

#ifdef DEBUG_WINDOW
    if (asyncMessage->frame.size().height > 0 && asyncMessage->frame.size().width > 0) {
        cv::imshow("Preview", asyncMessage->frame);
        cv::waitKey(20);
    }
#endif

    Local<Array> arr = Array::New(isolate, asyncMessage->image.size());
    int pos = 0;
    for(unsigned char c : asyncMessage->image) {
        arr->Set(pos++, Integer::New(isolate, c));
    }

    // https://github.com/bellbind/node-v4l2camera/blob/master/v4l2camera.cc#L328
    Local<Value> argv[] = {
        arr,
        Boolean::New(isolate, asyncMessage->faceDetected)
    };

    Local<Function> callBack = Local<Function>::New(isolate, bag->callback);
    callBack->Call(isolate->GetCurrentContext()->Global(), 2, argv);

    delete asyncMessage;
}

/*
 * - start camera capture
 * - retrieve image size
 * - send size message
 */
void cameraLoop(uv_work_t* req) {
    TMessage* message = (TMessage*) req->data;
    cv::Mat tmp, grayFrame, rsz;
    std::vector<cv::DetectionBasedTracker::ExtObject> faces;

#ifdef DEBUG_MESSAGE
    float t = getticks();
    printf("thread :: cameraLoop start\n");
#endif

    while(m_brk > 0 && message->capture->isOpened()) {

        AsyncMessage *msg = new AsyncMessage();

        // Assign defaults
        msg->image = std::vector<unsigned char>();
        msg->faceDetected = false;

        // Capture Frame From WebCam
        message->capture->read(tmp);

        if (message->resize) {
            cv::Size size = cv::Size(message->width, message->height);
            cv::resize(tmp, rsz, size);
            msg->frame = rsz;

        } else {
            msg->frame = tmp;

            // Update Size
            message->width = tmp.size().width;
            message->height = tmp.size().height;
        }

        // Do face detect if needed
        if (message->faceDetect) {
            cvtColor(msg->frame, grayFrame, cv::COLOR_BGR2GRAY);
            msg->faceDetected = detect(grayFrame, 100, 5.0);
            grayFrame.release();
        }

        // TODO: Add image parameters here

        // Encode to jpg
        std::vector<int> compression_parameters = std::vector<int>(2);
        compression_parameters[0] = CV_IMWRITE_JPEG_QUALITY;
        compression_parameters[1] = 85;
        if (message->resize) {
            cv::imencode(message->codec, rsz, msg->image, compression_parameters);
        } else {
            cv::imencode(message->codec, tmp, msg->image, compression_parameters);
        }
        compression_parameters.clear();

#ifdef DEBUG_WINDOW
        if (tmp.size().height > 0 && tmp.size().width > 0) {
            cv::imshow("Preview", msg->frame);
            cv::waitKey(20);
        }
#endif

        async.data = msg;

        if (async.type != UV_UNKNOWN_HANDLE) {
            uv_async_send(&async);
        } else {
            delete msg;
        }
    }
#ifdef DEBUG_MESSAGE
    t = getticks() - t;
    printf("thread :: cameraLoop end after %f\n", 1000.0f * t);
#endif

    rsz.release();
    tmp.release();
}

/*
 * - stop camera capture
 */
void cameraClose(uv_work_t* req, int status) {
#ifdef DEBUG_MESSAGE
    printf("thread :: cameraClose\n");
#endif

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    TMessage* message = (TMessage*) req->data;
    message->capture->release();
    delete message->capture;
    delete req;

#ifdef DEBUG_MESSAGE
    printf("thread :: cameraClose end\n");
#endif
}
