//
//  Object.cpp
//  LiquidCoreiOS
//
//  Created by Eric Lange on 2/4/18.
//  Copyright © 2018 LiquidPlayer. All rights reserved.
//

#include "V82JSC.h"

using namespace v8;

Maybe<bool> Object::Set(Local<Context> context, Local<Value> key, Local<Value> value)
{
    JSContextRef ctx = V82JSC::ToContextRef(context);
    IsolateImpl* iso = V82JSC::ToContextImpl(context)->m_isolate;

    LocalException exception(iso);
    JSValueRef args[] = {
        V82JSC::ToJSValueRef<Object>(this, context),
        V82JSC::ToJSValueRef(key, context),
        V82JSC::ToJSValueRef(value, context)
    };
    
    JSValueRef ret = V82JSC::exec(ctx, "return _3 == (_1[_2] = _3)", 3, args, &exception);
    
    _maybe<bool> out;
    if (!exception.ShouldThow()) {
        out.value_ = JSValueToBoolean(ctx, ret);
    }
    out.has_value_ = !exception.ShouldThow();
    
    return out.toMaybe();
}

Maybe<bool> Object::Set(Local<Context> context, uint32_t index,
                                      Local<Value> value)
{
    char ndx[50];
    sprintf(ndx, "%d", index);
    JSStringRef index_ = JSStringCreateWithUTF8CString(ndx);

    JSContextRef ctx = V82JSC::ToContextRef(context);
    JSValueRef obj = V82JSC::ToJSValueRef(this, context);
    JSValueRef value_ = V82JSC::ToJSValueRef(value, context);

    JSValueRef exception = nullptr;
    JSObjectSetProperty(ctx, (JSObjectRef)obj, index_, value_, kJSPropertyAttributeNone, &exception);
    JSStringRelease(index_);
    
    _maybe<bool> out;
    out.has_value_ = true;
    out.value_ = exception == nullptr;
    
    return out.toMaybe();
}

// Implements CreateDataProperty (ECMA-262, 7.3.4).
//
// Defines a configurable, writable, enumerable property with the given value
// on the object unless the property already exists and is not configurable
// or the object is not extensible.
//
// Returns true on success.
Maybe<bool> Object::CreateDataProperty(Local<Context> context,
                               Local<Name> key,
                               Local<Value> value)
{
    assert(0);
    return Nothing<bool>();
}

Maybe<bool> Object::CreateDataProperty(Local<Context> context,
                                       uint32_t index,
                                       Local<Value> value)
{
    assert(0);
    return Nothing<bool>();
}

// Implements DefineOwnProperty.
//
// In general, CreateDataProperty will be faster, however, does not allow
// for specifying attributes.
//
// Returns true on success.
Maybe<bool> Object::DefineOwnProperty(
                                      Local<Context> context, Local<Name> key, Local<Value> value,
                                      PropertyAttribute attributes)
{
    assert(0);
    return Nothing<bool>();
}

// Implements Object.DefineProperty(O, P, Attributes), see Ecma-262 19.1.2.4.
//
// The defineProperty function is used to add an own property or
// update the attributes of an existing own property of an object.
//
// Both data and accessor descriptors can be used.
//
// In general, CreateDataProperty is faster, however, does not allow
// for specifying attributes or an accessor descriptor.
//
// The PropertyDescriptor can change when redefining a property.
//
// Returns true on success.
Maybe<bool> Object::DefineProperty(Local<Context> context, Local<Name> key,
                           PropertyDescriptor& descriptor)
{
    assert(0);
    return Nothing<bool>();
}

// Sets an own property on this object bypassing interceptors and
// overriding accessors or read-only properties.
//
// Note that if the object has an interceptor the property will be set
// locally, but since the interceptor takes precedence the local property
// will only be returned if the interceptor doesn't return a value.
//
// Note also that this only works for named properties.
MaybeLocal<Value> Object::Get(Local<Context> context, Local<Value> key)
{
    JSContextRef ctx = V82JSC::ToContextRef(context);
    IsolateImpl* iso = V82JSC::ToContextImpl(context)->m_isolate;

    LocalException exception(iso);
    JSValueRef args[] = {
        V82JSC::ToJSValueRef<Object>(this, context),
        V82JSC::ToJSValueRef(key, context)
    };
    
    JSValueRef ret = V82JSC::exec(ctx, "return _1[_2]", 2, args, &exception);
    
    if (!exception.ShouldThow()) {
        return ValueImpl::New(V82JSC::ToContextImpl(context), ret);
    }
    return Local<Value>();
}

MaybeLocal<Value> Object::Get(Local<Context> context, uint32_t index)
{
    char ndx[50];
    sprintf(ndx, "%d", index);
    JSStringRef index_ = JSStringCreateWithUTF8CString(ndx);

    JSContextRef ctx = V82JSC::ToContextRef(context);
    JSValueRef obj = V82JSC::ToJSValueRef(this, context);
    IsolateImpl* iso = V82JSC::ToContextImpl(context)->m_isolate;

    LocalException exception(iso);
    JSValueRef prop = JSObjectGetProperty(ctx, (JSObjectRef)obj, index_, &exception);
    JSStringRelease(index_);
    if (!exception.ShouldThow()) {
        return MaybeLocal<Value>(ValueImpl::New(V82JSC::ToContextImpl(context), prop));
    }
    
    return MaybeLocal<Value>();
}

/**
 * Gets the property attributes of a property which can be None or
 * any combination of ReadOnly, DontEnum and DontDelete. Returns
 * None when the property doesn't exist.
 */
Maybe<PropertyAttribute> Object::GetPropertyAttributes(Local<Context> context, Local<Value> key)
{
    JSContextRef ctx = V82JSC::ToContextRef(context);
    IsolateImpl* iso = V82JSC::ToContextImpl(context)->m_isolate;

    LocalException exception(iso);
    JSValueRef args[] = {
        V82JSC::ToJSValueRef<Object>(this, context),
        V82JSC::ToJSValueRef(key, context),
    };
    
    JSValueRef ret = V82JSC::exec(ctx,
                                  "const None = 0, ReadOnly = 1 << 0, DontEnum = 1 << 1, DontDelete = 1 << 2; "
                                  "var d = Object.getOwnPropertyDescriptor(_1, _2); "
                                  "var attr = None; if (!d) return attr; "
                                  "attr |= (!d.writable) ? ReadOnly : 0; "
                                  "attr |= (!d.enumerable) ? DontEnum : 0; "
                                  "attr |= (!d.configurable) ? DontDelete : 0; "
                                  "return attr"
                                  , 2, args, &exception);

    _maybe<PropertyAttribute> out;
    if (!exception.ShouldThow()) {
        JSValueRef excp = 0;
        out.value_ = (PropertyAttribute) JSValueToNumber(ctx, ret, &excp);
        assert(excp==0);
    }
    out.has_value_ = !exception.ShouldThow();
    
    return out.toMaybe();
}

/**
 * Returns Object.getOwnPropertyDescriptor as per ES2016 section 19.1.2.6.
 */
MaybeLocal<Value> Object::GetOwnPropertyDescriptor(Local<Context> context, Local<Name> key)
{
    assert(0);
    return MaybeLocal<Value>();
}

/**
 * Object::Has() calls the abstract operation HasProperty(O, P) described
 * in ECMA-262, 7.3.10. Has() returns
 * true, if the object has the property, either own or on the prototype chain.
 * Interceptors, i.e., PropertyQueryCallbacks, are called if present.
 *
 * Has() has the same side effects as JavaScript's `variable in object`.
 * For example, calling Has() on a revoked proxy will throw an exception.
 *
 * \note Has() converts the key to a name, which possibly calls back into
 * JavaScript.
 *
 * See also v8::Object::HasOwnProperty() and
 * v8::Object::HasRealNamedProperty().
 */
Maybe<bool> Object::Has(Local<Context> context, Local<Value> key)
{
    JSContextRef ctx = V82JSC::ToContextRef(context);
    IsolateImpl* iso = V82JSC::ToContextImpl(context)->m_isolate;

    LocalException exception(iso);
    JSValueRef args[] = {
        V82JSC::ToJSValueRef<Object>(this, context),
        V82JSC::ToJSValueRef(key, context)
    };
    
    JSValueRef ret = V82JSC::exec(ctx, "return (_2 in _1)", 2, args, &exception);
    
    _maybe<bool> out;
    if (!exception.ShouldThow()) {
        out.value_ = JSValueToBoolean(ctx, ret);
    }
    out.has_value_ = !exception.ShouldThow();
    
    return out.toMaybe();
}

Maybe<bool> Object::Delete(Local<Context> context, Local<Value> key)
{
    JSContextRef ctx = V82JSC::ToContextRef(context);
    IsolateImpl* iso = V82JSC::ToContextImpl(context)->m_isolate;

    LocalException exception(iso);
    JSValueRef args[] = {
        V82JSC::ToJSValueRef<Object>(this, context),
        V82JSC::ToJSValueRef(key, context)
    };

    JSValueRef ret = V82JSC::exec(ctx, "return delete _1[_2]", 2, args, &exception);

    _maybe<bool> out;
    if (!exception.ShouldThow()) {
        out.value_ = JSValueToBoolean(ctx, ret);
    }
    out.has_value_ = !exception.ShouldThow();
    
    return out.toMaybe();
}

Maybe<bool> Object::Has(Local<Context> context, uint32_t index)
{
    char ndx[50];
    sprintf(ndx, "%d", index);
    JSStringRef index_ = JSStringCreateWithUTF8CString(ndx);

    JSContextRef ctx = V82JSC::ToContextRef(context);
    JSObjectRef obj = (JSObjectRef) V82JSC::ToJSValueRef<Object>(this, context);

    _maybe<bool> out;
    out.has_value_ = true;
    out.value_ = JSObjectHasProperty(ctx, obj, index_);
    JSStringRelease(index_);
    return out.toMaybe();
}

Maybe<bool> Object::Delete(Local<Context> context, uint32_t index)
{
    char ndx[50];
    sprintf(ndx, "%d", index);
    JSStringRef index_ = JSStringCreateWithUTF8CString(ndx);
    
    JSContextRef ctx = V82JSC::ToContextRef(context);
    JSObjectRef obj = (JSObjectRef) V82JSC::ToJSValueRef<Object>(this, context);

    _maybe<bool> out;
    JSValueRef exception = nullptr;
    out.value_ = JSObjectDeleteProperty(ctx, obj, index_, &exception);
    JSStringRelease(index_);
    out.has_value_ = exception == nullptr;
    return out.toMaybe();
}

#undef O
#define O(v) reinterpret_cast<v8::internal::Object*>(v)

Maybe<bool> Object::SetAccessor(Local<Context> context,
                                Local<Name> name,
                                AccessorNameGetterCallback getter,
                                AccessorNameSetterCallback setter,
                                MaybeLocal<Value> data,
                                AccessControl settings,
                                PropertyAttribute attribute)
{
    ContextImpl *ctximpl = V82JSC::ToContextImpl(context);
    struct AccessorInfo {
        AccessorNameGetterCallback getter;
        AccessorNameSetterCallback setter;
        ContextImpl *m_context;
        JSValueRef m_property;
        JSValueRef m_data;
    };
    
    const auto callback = [](JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
                             size_t argumentCount, const JSValueRef *arguments, JSValueRef *exception)
    {
        AccessorInfo* wrap = reinterpret_cast<AccessorInfo*>(JSObjectGetPrivate(function));
        IsolateImpl* isolateimpl = wrap->m_context->m_isolate;
        Local<Context> context = ContextImpl::New(V82JSC::ToIsolate(isolateimpl), ctx);
        ContextImpl *ctximpl = V82JSC::ToContextImpl(context);
        
        Local<Value> thiz = ValueImpl::New(ctximpl, thisObject);
        Local<Value> data = ValueImpl::New(ctximpl, wrap->m_data);
        
        v8::internal::Object * implicit[] = {
            0 /*FIXME*/,                                         // kShouldThrowOnErrorIndex = 0;
            * reinterpret_cast<v8::internal::Object**>(*thiz),   // kHolderIndex = 1;
            O(isolateimpl),                                      // kIsolateIndex = 2;
            O(isolateimpl->i.roots.the_hole_value),              // kReturnValueDefaultValueIndex = 3;
            O(isolateimpl->i.roots.the_hole_value),              // kReturnValueIndex = 4;
            * reinterpret_cast<v8::internal::Object**>(*data),   // kDataIndex = 5;
            * reinterpret_cast<v8::internal::Object**>(*thiz),   // kThisIndex = 6;
        };
        
        //internal::Object* held_exception = isolateimpl->i.ii.thread_local_top()->scheduled_exception_;
        isolateimpl->i.ii.thread_local_top()->scheduled_exception_ = *isolateimpl->i.roots.the_hole_value;
        TryCatch try_catch(V82JSC::ToIsolate(isolateimpl));

        Local<Value> ret = Undefined(V82JSC::ToIsolate(isolateimpl));
        if (argumentCount == 0) {
            PropertyCallbackImpl<Value> info(implicit);
            wrap->getter(ValueImpl::New(ctximpl, wrap->m_property).As<Name>(), info);
            ret = info.GetReturnValue().Get();
        } else {
            PropertyCallbackImpl<void> info(implicit);
            wrap->setter(ValueImpl::New(ctximpl, wrap->m_property).As<Name>(),
                         ValueImpl::New(ctximpl, arguments[0]),
                         info);
        }
        
        if (try_catch.HasCaught()) {
            *exception = V82JSC::ToJSValueRef(try_catch.Exception(), context);
        } else if (isolateimpl->i.ii.thread_local_top()->scheduled_exception_ != *isolateimpl->i.roots.the_hole_value) {
            Local<Value> excp = _local<Value>(&isolateimpl->i.ii.thread_local_top()->scheduled_exception_).toLocal();
            *exception = V82JSC::ToJSValueRef(excp, context);
            isolateimpl->i.ii.thread_local_top()->scheduled_exception_ = reinterpret_cast<v8::internal::Object*>(isolateimpl->i.roots.the_hole_value);
        }
        //isolateimpl->i.ii.thread_local_top()->scheduled_exception_ = held_exception;

        return V82JSC::ToJSValueRef<Value>(ret, context);
    };
    
    AccessorInfo *wrap = new AccessorInfo();
    wrap->m_context = ctximpl;
    wrap->m_property = V82JSC::ToJSValueRef(name, context);
    JSValueProtect(ctximpl->m_ctxRef, wrap->m_property);
    wrap->getter = getter;
    wrap->setter = setter;
    if (data.IsEmpty()) data = Undefined(V82JSC::ToIsolate(ctximpl->m_isolate));
    wrap->m_data = V82JSC::ToJSValueRef(data.ToLocalChecked(), context);
    JSValueProtect(ctximpl->m_ctxRef, wrap->m_data);

    JSClassDefinition def = kJSClassDefinitionEmpty;
    def.attributes = kJSClassAttributeNoAutomaticPrototype;
    def.callAsFunction = callback;
    JSClassRef claz = JSClassCreate(&def);
    JSObjectRef accessor_function = JSObjectMake(ctximpl->m_ctxRef, claz, wrap);
    JSClassRelease(claz);
    Local<Function> accessor = ValueImpl::New(ctximpl, accessor_function).As<Function>();
    
    TryCatch try_catch(V82JSC::ToIsolate(ctximpl->m_isolate));
    
    SetAccessorProperty(name,
                        (getter) ? accessor : Local<Function>(),
                        (setter) ? accessor : Local<Function>(),
                        attribute,
                        settings);

    if (try_catch.HasCaught()) {
        return Nothing<bool>();
    } else {
        return _maybe<bool>(true).toMaybe();
    }
}

void Object::SetAccessorProperty(Local<Name> name, Local<Function> getter,
                                 Local<Function> setter,
                                 PropertyAttribute attribute,
                                 AccessControl settings)
{
    Local<Context> context = V82JSC::ToCurrentContext(this);
    JSContextRef ctx = V82JSC::ToContextRef(context);
    IsolateImpl* iso = V82JSC::ToContextImpl(context)->m_isolate;
    
    LocalException exception(iso);
    
    // FIXME: Deal with attributes / access control
    
    JSValueRef args[] = {
        V82JSC::ToJSValueRef<Object>(this, context),
        V82JSC::ToJSValueRef(name, context),
        *getter ? V82JSC::ToJSValueRef(getter, context) : 0,
        *setter ? V82JSC::ToJSValueRef(setter, context) : 0
    };
    
    V82JSC::exec(ctx,
                 "delete _1[_2]; "
                 "if (!_4) Object.defineProperty(_1, _2, { get: _3, set: function(v) { delete this[_2]; this[_2] = v; }, configurable: true }); "
                 "else if (!_3) Object.defineProperty(_1, _2, { set: _4, configurable: true }); "
                 "else Object.defineProperty(_1, _2, { get: _3, set: _4, configurable: true });",
                 4, args, &exception);
}

/**
 * Sets a native data property like Template::SetNativeDataProperty, but
 * this method sets on this object directly.
 */
Maybe<bool> Object::SetNativeDataProperty(Local<Context> context, Local<Name> name,
                                          AccessorNameGetterCallback getter,
                                          AccessorNameSetterCallback setter,
                                          Local<Value> data, PropertyAttribute attributes)
{
    return SetAccessor(context, name, getter, setter, data, AccessControl::DEFAULT, attributes);
}

/**
 * Functionality for private properties.
 * This is an experimental feature, use at your own risk.
 * Note: Private properties are not inherited. Do not rely on this, since it
 * may change.
 */
Maybe<bool> Object::HasPrivate(Local<Context> context, Local<Private> key)
{
    JSContextRef ctx = V82JSC::ToContextRef(context);
    JSValueRef obj = V82JSC::ToJSValueRef(this, context);
    IsolateImpl* iso = V82JSC::ToContextImpl(context)->m_isolate;

    InstanceWrap *wrap = V82JSC::getPrivateInstance(ctx, (JSObjectRef)obj);
    if (wrap && wrap->m_private_properties) {
        JSValueRef args[] = {
            obj,
            V82JSC::ToJSValueRef(key, context)
        };
        LocalException exception(iso);
        JSValueRef ret = V82JSC::exec(ctx, "return _1.hasOwnProperty(_2)", 2, args);
        if (exception.ShouldThow()) return Nothing<bool>();
        return _maybe<bool>(JSValueToBoolean(ctx, ret)).toMaybe();
    }
    return _maybe<bool>(false).toMaybe();
}
Maybe<bool> Object::SetPrivate(Local<Context> context, Local<Private> key,
                               Local<Value> value)
{
    JSContextRef ctx = V82JSC::ToContextRef(context);
    JSValueRef obj = V82JSC::ToJSValueRef(this, context);
    IsolateImpl* iso = V82JSC::ToContextImpl(context)->m_isolate;

    InstanceWrap *wrap = V82JSC::getPrivateInstance(ctx, (JSObjectRef)obj);
    if (!wrap) wrap = V82JSC::makePrivateInstance(ctx, (JSObjectRef)obj);
    if (!wrap->m_private_properties) {
        wrap->m_private_properties = JSObjectMake(ctx, 0, 0);
    }
    JSValueRef args[] = {
        obj,
        V82JSC::ToJSValueRef(key, context),
        V82JSC::ToJSValueRef(value, context)
    };
    LocalException exception(iso);
    V82JSC::exec(ctx, "_1[_2] = _3", 3, args, &exception);
    if (exception.ShouldThow()) return Nothing<bool>();
    return _maybe<bool>(true).toMaybe();
}
Maybe<bool> Object::DeletePrivate(Local<Context> context, Local<Private> key)
{
    JSContextRef ctx = V82JSC::ToContextRef(context);
    JSValueRef obj = V82JSC::ToJSValueRef(this, context);
    IsolateImpl* iso = V82JSC::ToContextImpl(context)->m_isolate;

    InstanceWrap *wrap = V82JSC::getPrivateInstance(ctx, (JSObjectRef)obj);
    if (wrap && wrap->m_private_properties) {
        JSValueRef args[] = {
            obj,
            V82JSC::ToJSValueRef(key, context)
        };
        LocalException exception(iso);
        V82JSC::exec(ctx, "return delete _1[_2]", 2, args, &exception);
        if (exception.ShouldThow()) return Nothing<bool>();
        return _maybe<bool>(true).toMaybe();
    }
    return _maybe<bool>(false).toMaybe();
}
MaybeLocal<Value> Object::GetPrivate(Local<Context> context, Local<Private> key)
{
    JSContextRef ctx = V82JSC::ToContextRef(context);
    JSValueRef obj = V82JSC::ToJSValueRef(this, context);
    IsolateImpl* iso = V82JSC::ToContextImpl(context)->m_isolate;

    InstanceWrap *wrap = V82JSC::getPrivateInstance(ctx, (JSObjectRef)obj);
    if (wrap && wrap->m_private_properties) {
        JSValueRef args[] = {
            obj,
            V82JSC::ToJSValueRef(key, context)
        };
        LocalException exception(iso);
        JSValueRef ret = V82JSC::exec(ctx, "return _1[_2]", 2, args);
        if (exception.ShouldThow()) return MaybeLocal<Value>();
        return ValueImpl::New(V82JSC::ToContextImpl(context), ret);
    }
    return Undefined(context->GetIsolate());
}

/**
 * Returns an array containing the names of the enumerable properties
 * of this object, including properties from prototype objects.  The
 * array returned by this method contains the same values as would
 * be enumerated by a for-in statement over this object.
 */
MaybeLocal<Array> Object::GetPropertyNames(Local<Context> context)
{
    ContextImpl *ctx = V82JSC::ToContextImpl(context);
    
    LocalException exception(ctx->m_isolate);
    JSValueRef args[] = {
        V82JSC::ToJSValueRef<Object>(this, context),
    };
    
    JSValueRef ret = V82JSC::exec(ctx->m_ctxRef, "var keys = []; for (var k in _1) keys.push(k); return keys", 1, args, &exception);
    
    if (!exception.ShouldThow()) {
        return ValueImpl::New(ctx, ret).As<Array>();
    }
    return MaybeLocal<Array>();
}
MaybeLocal<Array> Object::GetPropertyNames(Local<Context> context, KeyCollectionMode mode,
                                           PropertyFilter property_filter, IndexFilter index_filter)
{
    assert(0);
    return MaybeLocal<Array>();
}

/**
 * This function has the same functionality as GetPropertyNames but
 * the returned array doesn't contain the names of properties from
 * prototype objects.
 */
MaybeLocal<Array> Object::GetOwnPropertyNames(Local<Context> context)
{
    ContextImpl *ctx = V82JSC::ToContextImpl(context);
    
    LocalException exception(ctx->m_isolate);
    JSValueRef args[] = {
        V82JSC::ToJSValueRef<Object>(this, context),
    };
    
    JSValueRef ret = V82JSC::exec(ctx->m_ctxRef, "return Object.getOwnPropertyNames(_1)", 1, args, &exception);
    
    if (!exception.ShouldThow()) {
        return ValueImpl::New(ctx, ret).As<Array>();
    }
    return MaybeLocal<Array>();
}

/**
 * Returns an array containing the names of the filtered properties
 * of this object, including properties from prototype objects.  The
 * array returned by this method contains the same values as would
 * be enumerated by a for-in statement over this object.
 */
MaybeLocal<Array> Object::GetOwnPropertyNames(Local<Context> context, PropertyFilter filter)
{
    assert(0);
    return MaybeLocal<Array>();
}

/**
 * Get the prototype object.  This does not skip objects marked to
 * be skipped by __proto__ and it does not consult the security
 * handler.
 */
Local<Value> Object::GetPrototype()
{
    Local<Context> context = Isolate::GetCurrent()->GetCurrentContext();
    ContextImpl* ctximpl = V82JSC::ToContextImpl(context);
    JSContextRef ctx = ctximpl->m_ctxRef;
    JSValueRef obj = V82JSC::ToJSValueRef<Value>(this, _local<Context>(ctximpl).toLocal());
    JSValueRef proto = V82JSC::exec(ctx, "return Object.getPrototypeOf(_1)", 1, &obj);
    return ValueImpl::New(ctximpl, proto);
}

/**
 * Set the prototype object.  This does not skip objects marked to
 * be skipped by __proto__ and it does not consult the security
 * handler.
 */
Maybe<bool> Object::SetPrototype(Local<Context> context,
                                 Local<Value> prototype)
{
    JSContextRef ctx = V82JSC::ToContextRef(context);
    JSValueRef obj = V82JSC::ToJSValueRef(this, context);
    JSValueRef proto = V82JSC::ToJSValueRef<Value>(prototype, context);
    
    JSObjectSetPrototype(ctx, (JSObjectRef)obj, proto);
    return _maybe<bool>(true).toMaybe();
}

/**
 * Finds an instance of the given function template in the prototype
 * chain.
 */
Local<Object> Object::FindInstanceInPrototypeChain(Local<FunctionTemplate> tmpl)
{
    Local<Context> context = V82JSC::ToCurrentContext(this);
    JSContextRef ctx = V82JSC::ToContextRef(context);

    FunctionTemplateImpl* tmplimpl = V82JSC::ToImpl<FunctionTemplateImpl>(tmpl);
    
    Local<Value> proto = _local<Value>(this).toLocal();
    while (proto->IsObject()) {
        JSObjectRef obj = (JSObjectRef) V82JSC::ToJSValueRef(proto, context);
        InstanceWrap *instance_wrap = V82JSC::getPrivateInstance(ctx, obj);
        if (instance_wrap && instance_wrap->m_object_template) {
            for (const TemplateImpl *t = instance_wrap->m_object_template->m_constructor_template; t; t = t->m_parent) {
                if (t == tmplimpl) {
                    return proto.As<Object>();
                }
            }
        }
        proto = proto.As<Object>()->GetPrototype();
    }

    return Local<Object>();
}

/**
 * Call builtin Object.prototype.toString on this object.
 * This is different from Value::ToString() that may call
 * user-defined toString function. This one does not.
 */
MaybeLocal<String> Object::ObjectProtoToString(Local<Context> context)
{
    assert(0);
    return MaybeLocal<String>();
}

/**
 * Returns the name of the function invoked as a constructor for this object.
 */
Local<String> Object::GetConstructorName()
{
    Local<Context> context = V82JSC::ToCurrentContext(this);
    JSContextRef ctx = V82JSC::ToContextRef(context);
    JSValueRef obj = V82JSC::ToJSValueRef(this, context);
    JSStringRef ctor = JSStringCreateWithUTF8CString("constructor");
    JSValueRef excp = nullptr;
    JSValueRef vctor = JSObjectGetProperty(ctx, (JSObjectRef) obj, ctor, &excp);
    JSStringRelease(ctor);
    assert(excp==nullptr);
    if (JSValueIsObject(ctx, vctor)) {
        JSStringRef name = JSStringCreateWithUTF8CString("name");
        JSValueRef vname = JSObjectGetProperty(ctx, (JSObjectRef) vctor, name, &excp);
        JSStringRelease(name);
        assert(excp==nullptr);
        return ValueImpl::New(V82JSC::ToContextImpl(context), vname)->ToString(context).ToLocalChecked();
    }

    return Local<String>(nullptr);
}

/**
 * Sets the integrity level of the object.
 */
Maybe<bool> Object::SetIntegrityLevel(Local<Context> context, IntegrityLevel level)
{
    assert(0);
    return Nothing<bool>();
}

/** Gets the number of internal fields for this Object. */
int Object::InternalFieldCount()
{
    Local<Context> context = V82JSC::ToCurrentContext(this);
    JSContextRef ctx = V82JSC::ToContextRef(context);
    JSObjectRef obj = (JSObjectRef) V82JSC::ToJSValueRef(this, context);
    
    InstanceWrap *wrap = V82JSC::getPrivateInstance(ctx, obj);
    if (wrap) return wrap->m_num_internal_fields;
    else if (IsArrayBufferView()) {
        // ArrayBufferViews have internal fields by default.  This was created in JS.
        GetArrayBufferViewInfo(reinterpret_cast<const ArrayBufferView*>(this));
        return ArrayBufferView::kInternalFieldCount;
    }
    return 0;
}

/** Sets the value in an internal field. */
void Object::SetInternalField(int index, Local<Value> value)
{
    Local<Context> context = V82JSC::ToCurrentContext(this);
    JSContextRef ctx = V82JSC::ToContextRef(context);
    JSObjectRef obj = (JSObjectRef) V82JSC::ToJSValueRef(this, context);

    InstanceWrap *wrap = V82JSC::getPrivateInstance(ctx, obj);
    if (!wrap && IsArrayBuffer()) {
        // ArrayBuffers have internal fields by default.  This was created in JS.
        GetArrayBufferInfo(reinterpret_cast<const ArrayBuffer*>(this));
        wrap = V82JSC::getPrivateInstance(ctx, obj);
    }
    if (wrap && index < wrap->m_num_internal_fields) {
        if (wrap->m_internal_fields[index]) {
            JSValueUnprotect(ctx, wrap->m_internal_fields[index]);
        }
        wrap->m_internal_fields[index] = V82JSC::ToJSValueRef(value, context);
        JSValueProtect(ctx, wrap->m_internal_fields[index]);
    }
}

/**
 * Sets a 2-byte-aligned native pointer in an internal field. To retrieve such
 * a field, GetAlignedPointerFromInternalField must be used, everything else
 * leads to undefined behavior.
 */
void Object::SetAlignedPointerInInternalField(int index, void* value)
{
    Local<Context> context = V82JSC::ToCurrentContext(this);
    SetInternalField(index, External::New(context->GetIsolate(), value));
}
void Object::SetAlignedPointerInInternalFields(int argc, int indices[],
                                       void* values[])
{
    for (int i=0; i<argc; i++) {
        SetAlignedPointerInInternalField(indices[i], values[i]);
    }
}

// Testers for local properties.

/**
 * HasOwnProperty() is like JavaScript's Object.prototype.hasOwnProperty().
 *
 * See also v8::Object::Has() and v8::Object::HasRealNamedProperty().
 */
Maybe<bool> Object::HasOwnProperty(Local<Context> context, Local<Name> key)
{
    assert(0);
    return Nothing<bool>();
}
Maybe<bool> Object::HasOwnProperty(Local<Context> context, uint32_t index)
{
    assert(0);
    return Nothing<bool>();
}

/**
 * Use HasRealNamedProperty() if you want to check if an object has an own
 * property without causing side effects, i.e., without calling interceptors.
 *
 * This function is similar to v8::Object::HasOwnProperty(), but it does not
 * call interceptors.
 *
 * \note Consider using non-masking interceptors, i.e., the interceptors are
 * not called if the receiver has the real named property. See
 * `v8::PropertyHandlerFlags::kNonMasking`.
 *
 * See also v8::Object::Has().
 */
Maybe<bool> Object::HasRealNamedProperty(Local<Context> context, Local<Name> key)
{
    JSContextRef ctx = V82JSC::ToContextRef(context);
    IsolateImpl* iso = V82JSC::ToContextImpl(context)->m_isolate;
    JSValueRef args[] = {
        V82JSC::ToJSValueRef(this, context),
        V82JSC::ToJSValueRef(key, context)
    };
    LocalException exception(iso);
    JSValueRef has = V82JSC::exec(ctx,
                                  "return Object.getOwnPropertyDescriptor(_1, _2) !== undefined",
                                  2, args, &exception);
    if (exception.ShouldThow()) {
        return Nothing<bool>();
    }
    
    return _maybe<bool>(JSValueToBoolean(ctx, has)).toMaybe();
}
Maybe<bool> Object::HasRealIndexedProperty(Local<Context> context, uint32_t index)
{
    return HasRealNamedProperty(context, Uint32::New(context->GetIsolate(), index).As<Name>());
}
Maybe<bool> Object::HasRealNamedCallbackProperty(Local<Context> context, Local<Name> key)
{
    assert(0);
    return Nothing<bool>();
}

/**
 * If result.IsEmpty() no real property was located in the prototype chain.
 * This means interceptors in the prototype chain are not called.
 */
MaybeLocal<Value> Object::GetRealNamedPropertyInPrototypeChain(Local<Context> context,
                                                               Local<Name> key)
{
    assert(0);
    return MaybeLocal<Value>();
}

/**
 * Gets the property attributes of a real property in the prototype chain,
 * which can be None or any combination of ReadOnly, DontEnum and DontDelete.
 * Interceptors in the prototype chain are not called.
 */
Maybe<PropertyAttribute>
Object::GetRealNamedPropertyAttributesInPrototypeChain(Local<Context> context,
                                                       Local<Name> key)
{
    assert(0);
    return Nothing<PropertyAttribute>();
}

/**
 * If result.IsEmpty() no real property was located on the object or
 * in the prototype chain.
 * This means interceptors in the prototype chain are not called.
 */
MaybeLocal<Value> Object::GetRealNamedProperty(Local<Context> context, Local<Name> key)
{
    assert(0);
    return MaybeLocal<Value>();
}

/**
 * Gets the property attributes of a real property which can be
 * None or any combination of ReadOnly, DontEnum and DontDelete.
 * Interceptors in the prototype chain are not called.
 */
Maybe<PropertyAttribute> Object::GetRealNamedPropertyAttributes(Local<Context> context, Local<Name> key)
{
    assert(0);
    return Nothing<PropertyAttribute>();
}

/** Tests for a named lookup interceptor.*/
bool Object::HasNamedLookupInterceptor()
{
    assert(0);
    return false;
}

/** Tests for an index lookup interceptor.*/
bool Object::HasIndexedLookupInterceptor()
{
    assert(0);
    return false;
}

/**
 * Returns the identity hash for this object. The current implementation
 * uses a hidden property on the object to store the identity hash.
 *
 * The return value will never be 0. Also, it is not guaranteed to be
 * unique.
 */
int Object::GetIdentityHash()
{
    Local<Context> context = V82JSC::ToCurrentContext(this);
    JSContextRef ctx = V82JSC::ToContextRef(context);
    JSValueRef obj = V82JSC::ToJSValueRef(this, context);
    InstanceWrap *wrap = V82JSC::getPrivateInstance(ctx, (JSObjectRef)obj);
    if (!wrap) {
        wrap = V82JSC::makePrivateInstance(ctx, (JSObjectRef)obj);
    }
    return wrap->m_hash;
}

/**
 * Clone this object with a fast but shallow copy.  Values will point
 * to the same values as the original object.
 */
// TODO(dcarney): take an isolate and optionally bail out?
Local<Object> Object::Clone()
{
    assert(0);
    return Local<Object>();
}

/**
 * Returns the context in which the object was created.
 */
Local<Context> Object::CreationContext()
{
    assert(0);
    return Local<Context>();
}

/**
 * Checks whether a callback is set by the
 * ObjectTemplate::SetCallAsFunctionHandler method.
 * When an Object is callable this method returns true.
 */
bool Object::IsCallable()
{
    assert(0);
    return false;
}

/**
 * True if this object is a constructor.
 */
bool Object::IsConstructor()
{
    assert(0);
    return false;
}

/**
 * Call an Object as a function if a callback is set by the
 * ObjectTemplate::SetCallAsFunctionHandler method.
 */
MaybeLocal<Value> Object::CallAsFunction(Local<Context> context,
                                         Local<Value> recv,
                                         int argc,
                                         Local<Value> argv[])
{
    assert(0);
    return MaybeLocal<Value>();
}

/**
 * Call an Object as a constructor if a callback is set by the
 * ObjectTemplate::SetCallAsFunctionHandler method.
 * Note: This method behaves like the Function::NewInstance method.
 */
MaybeLocal<Value> Object::CallAsConstructor(Local<Context> context,
                                            int argc, Local<Value> argv[])
{
    assert(0);
    return MaybeLocal<Value>();
}

Local<Object> Object::New(Isolate* isolate)
{
    Local<Context> context = V82JSC::OperatingContext(isolate);
    JSContextRef ctx = V82JSC::ToContextRef(context);

    Local<Value> o = ValueImpl::New(V82JSC::ToContextImpl(context), JSObjectMake(ctx, 0, 0));
    return o.As<Object>();
}

void* Object::SlowGetAlignedPointerFromInternalField(int index)
{
    Local<Context> context = V82JSC::ToCurrentContext(this);

    Local<External> external = SlowGetInternalField(index).As<External>();
    JSObjectRef o = (JSObjectRef) V82JSC::ToJSValueRef(external, context);
    return JSObjectGetPrivate(o);
}

Local<Value> Object::SlowGetInternalField(int index)
{
    Local<Context> context = V82JSC::ToCurrentContext(this);
    JSContextRef ctx = V82JSC::ToContextRef(context);

    JSObjectRef obj = (JSObjectRef) V82JSC::ToJSValueRef(this, context);
    InstanceWrap *wrap = V82JSC::getPrivateInstance(ctx, obj);
    if (wrap && index < wrap->m_num_internal_fields) {
        if (wrap->m_internal_fields[index]) {
            return ValueImpl::New(V82JSC::ToContextImpl(context), wrap->m_internal_fields[index]);
        } else {
            return Undefined(context->GetIsolate());
        }
    }
    return Local<Value>();
}

