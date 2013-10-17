// requires:
// npm install geojson-stream
var osmium = require('../'),
    fs = require('fs'),
    geojsonStream = require('geojson-stream');
 
var fileOut = fs.createWriteStream('nodes.geojson');
 
var geojsonOut = geojsonStream.stringify();
 
geojsonOut.pipe(fileOut);
// uncomment this line, and install geojsonio:
//
//     npm install -g geojsonio-cli
//
// and run
//
//     node test_stream.js | geojsonio
//
geojsonOut.pipe(process.stdout);
var handler = new osmium.Handler();
var nodes = 0;
 
handler.on('node',function(node) {
    geojsonOut.write({
        type: 'Feature',
        geometry: {
            type: 'Point',
            coordinates: [node.lon, node.lat]
        },
        properties: {}
    });
});
 
handler.on('done',function() {
    geojsonOut.end();
});
 
var reader = new osmium.Reader("winthrop.osm");
reader.apply(handler);