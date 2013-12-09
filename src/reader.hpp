// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>

// osmium
#include <osmium/io/any_input.hpp> // bring in XML and PBF support
#include <osmium/io/reader.hpp>
#include <osmium/visitor.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/dummy.hpp>
#include <osmium/index/map/stl_map.hpp>
#include <osmium/index/map/sparse_table.hpp>

// c++11
#include <memory>
#include <sstream>

typedef osmium::index::map::Dummy<osmium::unsigned_object_id_type, osmium::Location> index_neg_type;
typedef osmium::index::map::SparseTable<osmium::unsigned_object_id_type, osmium::Location> index_pos_type;
typedef osmium::handler::NodeLocationsForWays<index_pos_type, index_neg_type> location_handler_type;

using namespace v8;

namespace node_osmium {

typedef std::shared_ptr<osmium::io::Reader> reader_ptr;

class Reader: public node::ObjectWrap {
public:
    static Persistent<FunctionTemplate> constructor;
    static void Initialize(Handle<Object> target);
    static Handle<Value> New(Arguments const& args);
    static Handle<Value> header(Arguments const& args);
    static Handle<Value> apply(Arguments const& args);
    static Handle<Value> close(Arguments const& args);
    Reader(osmium::io::File& infile, osmium::osm_entity::flags entities);
    void _ref() { Ref(); }
    void _unref() { Unref(); }
    inline reader_ptr get() { return this_; }
private:
    ~Reader();
    reader_ptr this_;
    osmium::io::Header header_;
};



Persistent<FunctionTemplate> Reader::constructor;

void Reader::Initialize(Handle<Object> target) {
    HandleScope scope;
    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Reader::New));
    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(String::NewSymbol("Reader"));
    NODE_SET_PROTOTYPE_METHOD(constructor, "header", header);
    NODE_SET_PROTOTYPE_METHOD(constructor, "apply", apply);
    NODE_SET_PROTOTYPE_METHOD(constructor, "close", close);
    target->Set(String::NewSymbol("Reader"), constructor->GetFunction());
}

Reader::Reader(osmium::io::File& file, osmium::osm_entity::flags entities)
  : ObjectWrap(),
    this_(std::make_shared<osmium::io::Reader>(file, entities)),
    header_(this_->header()) { }

Reader::~Reader() { }

Handle<Value> Reader::New(Arguments const& args)
{
    HandleScope scope;
    if (!args.IsConstructCall()) {
        return ThrowException(Exception::Error(String::New("Cannot call constructor as function, you need to use 'new' keyword")));
    }
    if (args.Length() < 1 || args.Length() > 2) {
        return ThrowException(Exception::TypeError(String::New("please provide a File object or string for the first argument and optional options Object when creating a Reader")));
    }
    try {
        osmium::osm_entity::flags read_which_entities = osmium::osm_entity::flags::all;
        if (args.Length() == 2) {
            if (!args[1]->IsObject()) {
                return ThrowException(Exception::TypeError(String::New("Second argument to Reader constructor must be object")));
            }
            read_which_entities = osmium::osm_entity::flags::nothing;
            Local<Object> options = args[1]->ToObject();

            Local<Value> want_nodes = options->Get(String::NewSymbol("node"));
            if (want_nodes->IsBoolean() && want_nodes->BooleanValue()) {
                read_which_entities |= osmium::osm_entity::flags::node;
            }

            Local<Value> want_ways = options->Get(String::NewSymbol("way"));
            if (want_ways->IsBoolean() && want_ways->BooleanValue()) {
                read_which_entities |= osmium::osm_entity::flags::way;
            }

            Local<Value> want_relations = options->Get(String::NewSymbol("relation"));
            if (want_relations->IsBoolean() && want_relations->BooleanValue()) {
                read_which_entities |= osmium::osm_entity::flags::relation;
            }

        }
        if (args[0]->IsString()) {
            osmium::io::File file(*String::Utf8Value(args[0]));
            Reader* q = new Reader(file, read_which_entities);
            q->Wrap(args.This());
            return args.This();
        } else if (args[0]->IsObject() && File::constructor->HasInstance(args[0]->ToObject())) {
            Local<Object> file_obj = args[0]->ToObject();
            File* file_wrap = node::ObjectWrap::Unwrap<File>(file_obj);
            Reader* q = new Reader(*(file_wrap->get()), read_which_entities);
            q->Wrap(args.This());
            return args.This();
        } else {
            return ThrowException(Exception::TypeError(String::New("please provide a File object or string for the first argument when creating a Reader")));
        }
    } catch (std::exception const& ex) {
        return ThrowException(Exception::TypeError(String::New(ex.what())));
    }
    return Undefined();
}


Handle<Value> Reader::header(Arguments const& args)
{
    HandleScope scope;
    Local<Object> obj = Object::New();
    Reader* reader = node::ObjectWrap::Unwrap<Reader>(args.This());
    osmium::io::Header const& header = reader->header_;
    obj->Set(String::NewSymbol("generator"), String::New(header.get("generator").c_str()));
    osmium::Box const& bounds = header.box();
    Local<Array> arr = Array::New(4);
    arr->Set(0,Number::New(bounds.bottom_left().lon()));
    arr->Set(1,Number::New(bounds.bottom_left().lat()));
    arr->Set(2,Number::New(bounds.top_right().lon()));
    arr->Set(3,Number::New(bounds.top_right().lat()));
    obj->Set(String::NewSymbol("bounds"),arr);
    return scope.Close(obj);
}

Handle<Value> Reader::apply(Arguments const& args)
{
    HandleScope scope;

    if (args.Length() != 1 && args.Length() != 2) {
        return ThrowException(Exception::TypeError(String::New("please provide a single handler object")));
    }
    if (!args[0]->IsObject()) {
        return ThrowException(Exception::TypeError(String::New("please provide a single handler object")));
    }
    Local<Object> obj = args[0]->ToObject();
    if (obj->IsNull() || obj->IsUndefined() || !JSHandler::constructor->HasInstance(obj)){
        return ThrowException(Exception::TypeError(String::New("please provide a valid handler object")));
    }

    bool with_location_handler = false;

    if (args.Length() == 2) {
        if (!args[1]->IsObject()) {
            return ThrowException(Exception::TypeError(String::New("second argument must be 'option' object")));
        }
        Local<Value> wlh = args[1]->ToObject()->Get(String::NewSymbol("with_location_handler"));
        if (wlh->BooleanValue()) {
            with_location_handler = true;
        }
    }

    JSHandler *handler = node::ObjectWrap::Unwrap<JSHandler>(obj);
    Reader* reader = node::ObjectWrap::Unwrap<Reader>(args.This());
    reader_ptr r_ptr = reader->get();

    if (with_location_handler) {
        index_pos_type index_pos;
        index_neg_type index_neg;
        location_handler_type location_handler(index_pos, index_neg);
        osmium::apply(*r_ptr, location_handler, *handler);
    } else {
        osmium::apply(*r_ptr, *handler);
    }

    return Undefined();
}

Handle<Value> Reader::close(Arguments const& args) {
    HandleScope scope;
    Reader* reader = node::ObjectWrap::Unwrap<Reader>(args.This());
    reader_ptr r_ptr = reader->get();
    r_ptr->close();
    return Undefined();
}

} // namespace node_osmium
