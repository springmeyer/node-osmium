// v8
#include <v8.h>

// node
#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>
#include <node_buffer.h>

// osmium
#include <osmium/handler.hpp>
#include <osmium/geom/wkb.hpp>
#include <osmium/geom/wkt.hpp>

// c++11
#include <memory>
#include <iostream>

namespace node_osmium {

using namespace v8;

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
        HandleScope scope;
        if (/*node.tags().begin() != node.tags().end() &&*/ !node_cb.IsEmpty()) {
            const int argc = 1;
            Local<Object> obj = Object::New();
            obj->Set(String::NewSymbol("id"), Number::New(node.id()));
            obj->Set(String::NewSymbol("version"), Number::New(node.version()));
            obj->Set(String::NewSymbol("changeset"), Number::New(node.changeset()));
            obj->Set(String::NewSymbol("timestamp"), Number::New(node.timestamp()));
            std::string iso { node.timestamp().to_iso() };
            obj->Set(String::NewSymbol("timestamp_iso"), String::New(iso.c_str()));
            obj->Set(String::NewSymbol("uid"), Number::New(node.uid()));
            obj->Set(String::NewSymbol("user"), String::New(node.user()));
            obj->Set(String::NewSymbol("lon"), Number::New(node.lon()));
            obj->Set(String::NewSymbol("lat"), Number::New(node.lat()));

            {
                std::string wkb { wkb_factory.create_point(node) };
                obj->Set(String::NewSymbol("wkb"), node::Buffer::New(wkb.data(), wkb.size())->handle_);
            }

            {
                std::string wkt { wkt_factory.create_point(node) };
                obj->Set(String::NewSymbol("wkt"), String::New(wkt.c_str()));
            }

            Local<Object> tags = Object::New();
            for (auto& tag : node.tags()) {
                tags->Set(String::NewSymbol(tag.key()), String::New(tag.value()));
            }
            obj->Set(String::NewSymbol("tags"), tags);

            Local<Value> argv[argc] = { obj };

            TryCatch trycatch;
            Handle<Value> v = node_cb->Call(Context::GetCurrent()->Global(), argc, argv);
            if (v.IsEmpty()) {
                Handle<Value> exception = trycatch.Exception();
                String::AsciiValue exception_str(exception);
                printf("Exception: %s\n", *exception_str);
                exit(1);
            }
        }
    }

    void way(const osmium::Way& way) {
        HandleScope scope;
        if (!way_cb.IsEmpty()) {
            const int argc = 1;
            Local<Object> obj = Object::New();
            obj->Set(String::NewSymbol("id"), Number::New(way.id()));
            obj->Set(String::NewSymbol("version"), Number::New(way.version()));
            obj->Set(String::NewSymbol("changeset"), Number::New(way.changeset()));
            obj->Set(String::NewSymbol("timestamp"), Number::New(way.timestamp()));
            std::string iso { way.timestamp().to_iso() };
            obj->Set(String::NewSymbol("timestamp_iso"), String::New(iso.c_str()));
            obj->Set(String::NewSymbol("uid"), Number::New(way.uid()));
            obj->Set(String::NewSymbol("user"), String::New(way.user()));

            try {
                std::string wkb { wkb_factory.create_linestring(way) };
                obj->Set(String::NewSymbol("wkb"), node::Buffer::New(wkb.data(), wkb.size())->handle_);
            } catch (osmium::geom::geometry_error&) {
                obj->Set(String::NewSymbol("wkb"), Undefined());
            }

            try {
                std::string wkt { wkt_factory.create_linestring(way) };
                obj->Set(String::NewSymbol("wkt"), String::New(wkt.c_str()));
            } catch (osmium::geom::geometry_error&) {
                obj->Set(String::NewSymbol("wkt"), Undefined());
            }

            Local<Object> tags = Object::New();
            for (auto& tag : way.tags()) {
                tags->Set(String::NewSymbol(tag.key()), String::New(tag.value()));
            }
            obj->Set(String::NewSymbol("tags"), tags);

            Local<Array> nodes = Array::New(way.nodes().size());
            int i = 0;
            for (auto& node : way.nodes()) {
                nodes->Set(i, Number::New(node.ref()));
                ++i;
            }
            obj->Set(String::NewSymbol("nodes"), nodes);

            Local<Value> argv[argc] = { obj };

            TryCatch trycatch;
            Handle<Value> v = way_cb->Call(Context::GetCurrent()->Global(), argc, argv);
            if (v.IsEmpty()) {
                Handle<Value> exception = trycatch.Exception();
                String::AsciiValue exception_str(exception);
                printf("Exception: %s\n", *exception_str);
                exit(1);
            }
        }
    }

    void relation(const osmium::Relation& relation) {
        HandleScope scope;
        if (!relation_cb.IsEmpty()) {
            const int argc = 1;
            Local<Object> obj = Object::New();
            obj->Set(String::NewSymbol("id"), Number::New(relation.id()));
            obj->Set(String::NewSymbol("version"), Number::New(relation.version()));
            obj->Set(String::NewSymbol("changeset"), Number::New(relation.changeset()));
            obj->Set(String::NewSymbol("timestamp"), Number::New(relation.timestamp()));
            std::string iso { relation.timestamp().to_iso() };
            obj->Set(String::NewSymbol("timestamp_iso"), String::New(iso.c_str()));
            obj->Set(String::NewSymbol("uid"), Number::New(relation.uid()));
            obj->Set(String::NewSymbol("user"), String::New(relation.user()));

            Local<Object> tags = Object::New();
            for (auto& tag : relation.tags()) {
                tags->Set(String::NewSymbol(tag.key()), String::New(tag.value()));
            }
            obj->Set(String::NewSymbol("tags"), tags);

            Local<Array> members = Array::New();
            int i = 0;
            char typec[2] = " ";
            for (auto& member : relation.members()) {
                Local<Object> jsmember = Object::New();
                typec[0] = osmium::item_type_to_char(member.type());
                jsmember->Set(String::NewSymbol("type"), String::New(typec));
                jsmember->Set(String::NewSymbol("ref"), Number::New(member.ref()));
                jsmember->Set(String::NewSymbol("role"), String::New(member.role()));
                members->Set(i, jsmember);
                ++i;
            }
            obj->Set(String::NewSymbol("members"), members);

            Local<Value> argv[argc] = { obj };

            TryCatch trycatch;
            Handle<Value> v = relation_cb->Call(Context::GetCurrent()->Global(), argc, argv);
            if (v.IsEmpty()) {
                Handle<Value> exception = trycatch.Exception();
                String::AsciiValue exception_str(exception);
                printf("Exception: %s\n", *exception_str);
                exit(1);
            }
        }
    }

    void done() {
        if (!done_cb.IsEmpty()) {
            Local<Value> argv[0] = { };
            done_cb->Call(Context::GetCurrent()->Global(), 0, argv);
        }
    }
    Persistent<Function> node_cb;
    Persistent<Function> way_cb;
    Persistent<Function> relation_cb;
    Persistent<Function> done_cb;
private:
    ~JSHandler();
    osmium::geom::WKBFactory wkb_factory;
    osmium::geom::WKTFactory wkt_factory;
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
    if (!way_cb.IsEmpty()) {
        way_cb.Dispose();
    }
    if (!relation_cb.IsEmpty()) {
        relation_cb.Dispose();
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
    } else if (callback_name == String::NewSymbol("way")) {
        if (!handler->way_cb.IsEmpty()) {
            handler->way_cb.Dispose();
        }
        handler->way_cb = Persistent<Function>::New(callback);
    } else if (callback_name == String::NewSymbol("relation")) {
        if (!handler->relation_cb.IsEmpty()) {
            handler->relation_cb.Dispose();
        }
        handler->relation_cb = Persistent<Function>::New(callback);
    } else if (callback_name == String::NewSymbol("done")) {
        if (!handler->done_cb.IsEmpty()) {
            handler->done_cb.Dispose();
        }
        handler->done_cb = Persistent<Function>::New(callback);
    }
    return scope.Close(Undefined());
}



} // namespace node_osmium
