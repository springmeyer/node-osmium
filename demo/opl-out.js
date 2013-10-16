#!/usr/bin/env node
/*

 Reads OSM file and outputs data in a format similar to OPL.

 Call as:

 node opl-out.js OSMFILE OUTFILE

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

function obj2str(type, object) {
    return type + object.id + ' v' + object.version + ' d' + (object.visible ? 'V' : 'D') + ' c' + object.changeset + ' t' + object.timestamp_iso + ' i' + object.uid + ' u' + object.user;
}

function tags2str(tags) {
    var str = '';
    for (key in tags) {
        str += key + '=' + tags[key] + ',';
    }
    return str.substring(0, str.length-1);
}

var handler = new osmium.Handler();

handler.on('node', function(node) {
    stream.write(obj2str('n', node) + ' x' + node.lon + ' y' + node.lat + ' T' + tags2str(node.tags) + "\n");
});

handler.on('way', function(way) {
    stream.write(obj2str('w', way) + ' N' + way.nodes.join(',') + ' T' + tags2str(way.tags) + "\n");
});

handler.on('relation', function(relation) {
    var members = relation.members.map(function(member) {
        return (member.type + member.ref + '@' + member.role);
    }).join(',');
    stream.write(obj2str('r', relation) + ' M' + members + ' T' + tags2str(relation.tags) + "\n");
});

handler.on('done', function() {
    stream.close();
});

var reader = new osmium.Reader(input_filename);
reader.apply(handler);

