// Snowboy: Node Module (author: Evan Cohen)

SnowboyDetect = require('./build/Release/snowboy.node');

console.log("SnowboyDetect", SnowboyDetect);
console.log("isListening",SnowboyDetect.isListening());
SnowboyDetect.detect("../../resources/snowboy.umdl", "0.5", 
    function(detectedEvent){
        // Handel message codes
        var event = "";
        switch (detectedEvent) {
            case -2:
                event = "Silence";
                break;
            case -1:
                event = "Error";
                break;
            case 0:
                event = "Sound";
                break;
            default:
                event = "Keyword: " + detectedEvent;
                break;
        }
        console.log(event);
    }, function(){
        console.log("Recognition Stopped")
    }
);

setTimeout(function() {
    console.log("Listening: ", SnowboyDetect.isListening()); 
}, 2000);

setTimeout(function() {
    SnowboyDetect.stop()
    console.log("Listening: ", SnowboyDetect.isListening()); 
}, 5000);