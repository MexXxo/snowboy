// Snowboy: Node Module (author: Evan Cohen)

SnowboyDetect = require('./build/Release/snowboy.node');

var Snowboy = function(){
    var _self = this;
    var tmodel = []
    var tsensitivity = [];

    // Add a model with the given sensitivity
    this.addModel = function(model, sensitivity){
        sensitivity = sensitivity || "0.5"
        tmodel.push(model);
        tsensitivity.push(sensitivity);
    }

    // Remove a given model by name
    this.removeModel = function(model){
        var i = tmodel.indexOf(model);
        if(i >= 0){
            tmodel.splice(i, 1);
            tsensitivity.splice(i, 1);
            return true;
        }
        return false;
    }

    // Begins detection
    this.detect = function(callback, error){
        console.log("Model: ", tmodel.join());
        console.log("sensitivity: ", tsensitivity.join());
        SnowboyDetect.detect(tmodel.join(), tsensitivity.join(), function(detectedEvent){
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
                    callback(tmodel[detectedEvent - 1]);
                    break;
            }
            console.log(event);
        }, function(){
            // Detection has stopped
        });
    }

    this.stop = function(){
        SnowboyDetect.stop();
    }

    this.isListening = function(){
        return SnowboyDetect.isListening();
    }

}

module.exports = new Snowboy();