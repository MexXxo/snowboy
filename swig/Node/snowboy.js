// Snowboy: Node Module (author: Evan Cohen)

SnowboyDetect = require('./build/Release/snowboy.node');

console.log("SnowboyDetect", SnowboyDetect);
console.log("isListening",SnowboyDetect.isListening());
SnowboyDetect.doProgress("../../resources/snowboy.umdl", "0.5", 
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
                event = "Sound" + " - isListening: " + SnowboyDetect.isListening();
                break;
            default:
                event = "Keyword: " + detectedEvent;
                break;
        }
        console.log(event);
    }, function(){
        console.log("Done!")
    }
);

console.log("Async logging")

setTimeout(function() {
    SnowboyDetect.stop()
}, 5000);

/*
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
                event = "Sound" + " - isListening: " + SnowboyDetect.isListening();
                break;
            default:
                event = "Keyword: " + detectedEvent;
                SnowboyDetect.stop()
                break;
    }
    console.log(event);
});
*/