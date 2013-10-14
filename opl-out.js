
var osmium = require('./lib/osmium.js');
var bw = require('buffered-writer');

stream = bw.open("out.opl");

var handler = new osmium.Handler();

function obj2str(type, object) {
    return type + object.id + ' v' + object.version + ' dV c' + object.changeset + ' t' + object.timestamp_iso + ' i' + object.uid + ' u' + object.user;
}

function tags2str(tags) {
    var str = '';
    for (key in tags) {
        str += key + '=' + tags[key] + ',';
    }
    return str.substring(0, str.length-1);
}

handler.on('node', function(node) {
    stream.write(obj2str('n', node) + ' x' + node.lon + ' y' + node.lat + ' T' + tags2str(node.tags) + "\n");
});

handler.on('way', function(way) {
    var str = '';
    for (var i=0; i < way.nodes.length; i+=1) {
        str += way.nodes[i] + ',';
    }
    stream.write(obj2str('w', way) + ' N' + str.substring(0, str.length-1) + ' T' + tags2str(way.tags) + "\n");
});

handler.on('relation', function(relation) {
    var str = '';
    for (var i=0; i < relation.members.length; i+=1) {
        str += relation.members[i].type + relation.members[i].ref + '@' + relation.members[i].role + ',';
    }
    stream.write(obj2str('r', relation) + ' M' + str.substring(0, str.length-1) + ' T' + tags2str(relation.tags) + "\n");
});

handler.on('done', function() {
    stream.close();
});

var reader = new osmium.Reader("winthrop.osm");
//var reader = new osmium.Reader("berlin-latest.osm.pbf");
//var reader = new osmium.Reader("../bremen.osm.pbf");

reader.apply(handler);

