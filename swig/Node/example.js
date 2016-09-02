var snowboy = require('./snowboy.js');

snowboy.addModel("../../resources/snowboy.umdl");

snowboy.detect(function(data){
    console.log("Detected: ", data);
});

// After 2 seconds check if snowboy is listening
setTimeout(function() {
    console.log("Listening: ", snowboy.isListening()); 
}, 2000);


// After 5 seconds terminate snowboy
setTimeout(function() {
    snowboy.stop()
    console.log("Listening: ", snowboy.isListening()); 
}, 5000);