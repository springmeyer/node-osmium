// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>

//#include "reader.hpp"

// osmium
#include <osmium/handler/node_locations_for_ways.hpp> // Handler
#include <osmium/index/map/dummy.hpp>
#include <osmium/index/map/stl_map.hpp>

// c++11
#include <memory>
#include <iostream>

namespace node_osmium {

using namespace v8;

// interfaces

typedef osmium::index::map::Dummy<osmium::unsigned_object_id_type, osmium::Location> index_neg_type;
typedef osmium::index::map::StlMap<osmium::unsigned_object_id_type, osmium::Location> index_pos_type;
typedef osmium::handler::NodeLocationsForWays<index_pos_type, index_neg_type> location_handler_type;

class JSHandler: public node::ObjectWrap , public osmium::handler::Handler<JSHandler> {
public:
    static Persistent<FunctionTemplate> constructor;
    static void Initialize(Handle<Object> target);
    static Handle<Value> New(Arguments const& args);
    static Handle<Value> on(Arguments const& args);
    JSHandler();
    void _ref() { Ref(); }
    void _unref() { Unref(); }
    void node(const osmium::Node& node) {
        if (!node_cb.IsEmpty()) {
            Local<Value> argv[0] = { };
            node_cb->Call(Context::GetCurrent()->Global(), 0, argv);
        }
    }
    void done() {
        if (!done_cb.IsEmpty()) {
            Local<Value> argv[0] = { };
            done_cb->Call(Context::GetCurrent()->Global(), 0, argv);
        }
    }
    Persistent<Function> node_cb;
    Persistent<Function> done_cb;
private:
    ~JSHandler();
};

Persistent<FunctionTemplate> JSHandler::constructor;

void JSHandler::Initialize(Handle<Object> target) {
    HandleScope scope;
    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(JSHandler::New));
    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(String::NewSymbol("Handler"));
    NODE_SET_PROTOTYPE_METHOD(constructor, "on", on);
    target->Set(String::NewSymbol("Handler"),constructor->GetFunction());
}

JSHandler::JSHandler()
  : ObjectWrap(),
    done_cb() {
      done_cb.Clear();
  }

JSHandler::~JSHandler()
{
    if (!done_cb.IsEmpty()) {
        done_cb.Dispose();
    }
    if (!node_cb.IsEmpty()) {
        node_cb.Dispose();
    }
}

Handle<Value> JSHandler::New(Arguments const& args)
{
    HandleScope scope;
    if (args[0]->IsExternal())
    {
        Local<External> ext = Local<External>::Cast(args[0]);
        void* ptr = ext->Value();
        JSHandler* b =  static_cast<JSHandler*>(ptr);
        b->Wrap(args.This());
        return args.This();
    }
    else
    {
        JSHandler* h = new JSHandler();
        h->Wrap(args.This());
        return args.This();
    }
    return Undefined();
}

Handle<Value> JSHandler::on(Arguments const& args)
{
    HandleScope scope;
    if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
        return ThrowException(Exception::TypeError(String::New("please provide an event name and callback function")));
    }
    Local<String> callback_name = args[0]->ToString();
    Local<Function> callback = Local<Function>::Cast(args[1]);
    if (callback->IsNull() || callback->IsUndefined()){
        return ThrowException(Exception::TypeError(String::New("please provide a valid callback function for second arg")));
    }
    JSHandler * handler = node::ObjectWrap::Unwrap<JSHandler>(args.This());
    if (callback_name == String::NewSymbol("node")) {
        if (!handler->node_cb.IsEmpty()) {
            handler->node_cb.Dispose();
        }
        handler->node_cb = Persistent<Function>::New(callback);
    } else if (callback_name == String::NewSymbol("done")) {
        if (!handler->done_cb.IsEmpty()) {
            handler->done_cb.Dispose();
        }
        handler->done_cb = Persistent<Function>::New(callback);
    }
    return scope.Close(Undefined());
}



} // namespace node_osmium
