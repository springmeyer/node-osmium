// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>

// osmium
#include <osmium/io/any_input.hpp>
#include <osmium/io/input.hpp>
#include <osmium/osm/dump.hpp>

// c++11
#include <memory>
#include <sstream>

namespace node_osmium {

using namespace v8;

// interfaces

typedef std::shared_ptr<osmium::io::Reader> reader_ptr;

class Buffer: public node::ObjectWrap {
public:
    static Persistent<FunctionTemplate> constructor;
    static void Initialize(Handle<Object> target);
    static Handle<Value> New(Arguments const& args);
    static Handle<Value> dump(Arguments const& args);
    Buffer(reader_ptr reader);
    void _ref() { Ref(); }
    void _unref() { Unref(); }
    osmium::memory::Buffer buf;
private:
    ~Buffer();
};

Persistent<FunctionTemplate> Buffer::constructor;

void Buffer::Initialize(Handle<Object> target) {
    HandleScope scope;
    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Buffer::New));
    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(String::NewSymbol("Buffer"));
    NODE_SET_PROTOTYPE_METHOD(constructor, "dump", dump);
    target->Set(String::NewSymbol("Buffer"),constructor->GetFunction());
}

Buffer::Buffer(reader_ptr reader)
  : ObjectWrap(),
    buf(reader.get()->read()) {}

Buffer::~Buffer()
{
}

Handle<Value> Buffer::New(Arguments const& args)
{
    HandleScope scope;
    if (args[0]->IsExternal())
    {
        Local<External> ext = Local<External>::Cast(args[0]);
        void* ptr = ext->Value();
        Buffer* b =  static_cast<Buffer*>(ptr);
        b->Wrap(args.This());
        return args.This();
    }
    else
    {
        return ThrowException(Exception::TypeError(String::New("osmium.Buffer cannot be created in Javascript")));
    }
    return Undefined();
}


Handle<Value> Buffer::dump(Arguments const& args)
{
    HandleScope scope;
    Buffer* b = node::ObjectWrap::Unwrap<Buffer>(args.This());
    std::ostringstream ss;
    osmium::osm::Dump dump(ss);
    osmium::osm::apply_visitor(dump, b->buf);
    Local<String> obj = String::New(ss.str().c_str());
    return scope.Close(obj);
}


class Reader: public node::ObjectWrap {
public:
    static Persistent<FunctionTemplate> constructor;
    static void Initialize(Handle<Object> target);
    static Handle<Value> New(Arguments const& args);
    static Handle<Value> header(Arguments const& args);
    static Handle<Value> next(Arguments const& args);
    Reader(std::string const& infile);
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
    NODE_SET_PROTOTYPE_METHOD(constructor, "next", next);
    target->Set(String::NewSymbol("Reader"),constructor->GetFunction());
}

Reader::Reader(std::string const& infile)
  : ObjectWrap(),
    this_(std::make_shared<osmium::io::Reader>(infile)),
    header_(this_->open()) { }

Reader::~Reader() { }

Handle<Value> Reader::New(Arguments const& args)
{
    HandleScope scope;
    if (!args.IsConstructCall()) {
        return ThrowException(Exception::Error(String::New("Cannot call constructor as function, you need to use 'new' keyword")));
    }
    try {
        if (args.Length() == 1) {
            if (!args[0]->IsString()) {
                return ThrowException(Exception::TypeError(String::New("first argument must be a string")));
            }
            Reader* q = new Reader(*String::Utf8Value(args[0]));
            q->Wrap(args.This());
            return args.This();
        } else {
            return ThrowException(Exception::TypeError(String::New("please provide an object of options for the first argument")));
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
    obj->Set(String::NewSymbol("generator"),String::New(header.generator().c_str()));
    osmium::Bounds const& bounds = header.bounds();
    Local<Array> arr = Array::New(4);
    arr->Set(0,Number::New(bounds.bottom_left().lon()));
    arr->Set(1,Number::New(bounds.bottom_left().lat()));
    arr->Set(2,Number::New(bounds.top_right().lon()));
    arr->Set(3,Number::New(bounds.top_right().lat()));
    obj->Set(String::NewSymbol("bounds"),arr);
    return scope.Close(obj);
}

Handle<Value> Reader::next(Arguments const& args)
{
    HandleScope scope;
    Reader* reader = node::ObjectWrap::Unwrap<Reader>(args.This());
    Buffer* b = new Buffer(reader->get());
    Local<Value> ext = External::New(b);
    Local<Object> obj = Buffer::constructor->GetFunction()->NewInstance(1, &ext);
    return scope.Close(obj);
}

extern "C" {
    static void start(Handle<Object> target) {
        HandleScope scope;
        Buffer::Initialize(target);
        Reader::Initialize(target);
    }
}

} // namespace node_osmium

NODE_MODULE(osmium, node_osmium::start)
