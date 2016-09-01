// Snowboy: Node Module (author: Evan Cohen)

snowboy = require('./build/Release/snowboy.node');

console.log(snowboy.detect("../../resources/snowboy.umdl", "0.5", 
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
                event = "Keyword: " + detectedEvent
                break;
        }
        console.log(event);
    }));
