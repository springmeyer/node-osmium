#!/usr/bin/env node
/*

 Reads OSM file and outputs some data for post boxes and highways.

 Call as:

 node csv-out.js OSMFILE OUTFILE

*/

var osmium = require('../lib/osmium.js');
var buffered_writer = require('buffered-writer');

if (process.argv.length != 4) {
    console.log("Usage: " + process.argv[0] + ' ' + process.argv[1] + " OSMFILE OUTFILE");
    process.exit(1);
}

var input_filename = process.argv[2];
var output_filename = process.argv[3];

var stream = buffered_writer.open(output_filename);

// =====================================

var handler = new osmium.Handler();
handler.options({ "tagged_nodes_only": true });

handler.on('node', function(node) {
    if (node.tags.amenity == 'post_box') {
        stream.write('n' + node.id + ' ' + (node.tags.operator || '') + ' ' +  node.wkt + "\n"); 
    }
});

handler.on('way', function(way) {
    if (way.tags.highway) {
        if (way.wkt) {
            stream.write('w' + way.id + ' '  + way.tags.highway + ' ' + way.wkt + "\n"); 
        }
    }
});

handler.on('done', function() {
    stream.close();
});

var reader = new osmium.Reader(input_filename);
reader.apply(handler, { "with_location_handler": true });

