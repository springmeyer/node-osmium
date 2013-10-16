var osmium = require('../');
var assert = require('assert');

describe('osmium', function() {

    it('should be able to create an osmium.Reader', function(done) {
        var reader = new osmium.Reader("berlin-latest.osm.pbf");
        var header = reader.header();
        assert.equal(header.generator, 'Osmium (http://wiki.openstreetmap.org/wiki/Osmium)');
        var bounds = header.bounds;
        var expected = [ 13.08283, 52.33446, 13.76136, 52.6783 ];
        assert.ok(Math.abs(bounds[0] - expected[0]) < .000000001);
        assert.ok(Math.abs(bounds[1] - expected[1]) < .000000001);
        assert.ok(Math.abs(bounds[2] - expected[2]) < .000000001);
        assert.ok(Math.abs(bounds[3] - expected[3]) < .000000001);
        var buffer;
        while (buffer = reader.next()) {
            var dump_string = buffer.dump();

            // workaround to break off loop
            if (dump_string == "") {
                break;
            }
        }

        done();
    });

});
