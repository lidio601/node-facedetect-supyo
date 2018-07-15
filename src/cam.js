var cam = require("../build/Release/camera.node");

if (!cam.isStarted()) {
    console.log("Opening Camera", cam.isStarted())
    cam.start({
        //width: 640,
        //height: 480,
        codec: ".jpg",
        //input: "0",
        frameCallback: function (frameRaw, faceDetected) {
            console.log("calling getFrameSize", cam.getFrameSize())

            const frame = Buffer.from(frameRaw)

            require('fs').createWriteStream('result.jpg').end(Buffer(frame))

            cam.stop()
            console.log("calling isStarted", cam.isStarted())
        },
        //faceDetect: true,
        //singleShot: true
    })
}
