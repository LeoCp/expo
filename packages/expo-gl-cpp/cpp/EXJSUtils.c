#include "EXJSUtils.h"

#include <stdlib.h>

#include <JavaScriptCore/JSValueRef.h>
#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include <JavaScriptCore/JSContextRef.h>

#ifdef __APPLE__
bool iosVersionChecked = false;
int32_t TagTypeNumber = 0;
int64_t DoubleEncodeOffset = 0;
#else
#define TagTypeNumber 0xffff0000
#define DoubleEncodeOffset (1ll << 48)
#endif
#define ValueTrue 0x7

double EXJSValueToNumberFast(JSContextRef ctx, JSValueRef v)
{
#if __LP64__ // arm64 version
  union {
    int64_t asInt64;
    double asDouble;
    struct { int32_t asInt; int32_t tag; } asBits;
  } taggedValue = { .asInt64 = (int64_t)v };

#ifdef __APPLE__
  if (!iosVersionChecked) {
    bool newJsc = EXiOSGetOperatingSystemVersion().majorVersion >= 14
    TagTypeNumber = newJsc ? 0xfffe0000 : 0xffff0000;
    DoubleEncodeOffset = newJsc ? 1ll << 49 : 1ll << 48;
    iosVersionChecked = true
  }
#endif

  if( (taggedValue.asBits.tag & TagTypeNumber) == TagTypeNumber ) {
    return taggedValue.asBits.asInt;
  }
  else if( taggedValue.asBits.tag & TagTypeNumber ) {
    taggedValue.asInt64 -= DoubleEncodeOffset;
    return taggedValue.asDouble;
  }
  else if( taggedValue.asBits.asInt == ValueTrue ) {
    return 1.0;
  }
  else {
    return 0; // false, undefined, null, object
  }
#else // armv7 version
  return JSValueToNumber(ctx, v, NULL);
#endif
}

void EXJSConsoleLog(JSContextRef ctx, const char *msg) {
  JSObjectRef global = JSContextGetGlobalObject(ctx);
  JSObjectRef console = (JSObjectRef) EXJSObjectGetPropertyNamed(ctx, global, "console");
  JSObjectRef log = (JSObjectRef) EXJSObjectGetPropertyNamed(ctx, console, "log");
  JSValueRef msgStr = EXJSValueMakeStringFromUTF8CString(ctx, msg);
  JSObjectCallAsFunction(ctx, log, console, 1, &msgStr, NULL);
}

char *EXJSValueToUTF8CStringMalloc(JSContextRef ctx, JSValueRef v, JSValueRef *exception) {
  JSStringRef jsStr = JSValueToStringCopy(ctx, v, exception);
  size_t nBytes = JSStringGetMaximumUTF8CStringSize(jsStr);
  char *cStr = (char *) malloc(nBytes * sizeof(char));
  JSStringGetUTF8CString(jsStr, cStr, nBytes);
  JSStringRelease(jsStr);
  return cStr;
}


void EXJSObjectSetValueWithUTF8CStringName(JSContextRef ctx,
                                           JSObjectRef obj,
                                           const char *name,
                                           JSValueRef val) {
  JSStringRef jsName = JSStringCreateWithUTF8CString(name);
  JSObjectSetProperty(ctx, obj, jsName, val, 0, NULL);
  JSStringRelease(jsName);
}

void EXJSObjectSetFunctionWithUTF8CStringName(JSContextRef ctx,
                                              JSObjectRef obj,
                                              const char *name,
                                              JSObjectCallAsFunctionCallback func) {
  JSStringRef jsName = JSStringCreateWithUTF8CString(name);
  JSObjectRef jsFunction = JSObjectMakeFunctionWithCallback(ctx, jsName, func);
  JSObjectSetProperty(ctx, obj, jsName, jsFunction, 0, NULL);
  JSStringRelease(jsName);
}


#ifndef EXJS_USE_JSC_TYPEDARRAY_HEADER

JS_EXPORT JSTypedArrayType JSValueGetTypedArrayType(JSContextRef ctx, JSValueRef value, JSValueRef* exception) {
  return kJSTypedArrayTypeNone;
}

JS_EXPORT size_t JSObjectGetArrayBufferByteLength(JSContextRef ctx, JSObjectRef object, JSValueRef* exception) {
  EXJSConsoleLog(ctx, "EXJS: Tried to use non-existent TypedArray API");
  return 0;
}

JS_EXPORT void* JSObjectGetArrayBufferBytesPtr(JSContextRef ctx, JSObjectRef object, JSValueRef* exception) {
  EXJSConsoleLog(ctx, "EXJS: Tried to use non-existent TypedArray API");
  return NULL;
}

JS_EXPORT size_t JSObjectGetTypedArrayByteLength(JSContextRef ctx, JSObjectRef object,
                                                 JSValueRef* exception) {
  EXJSConsoleLog(ctx, "EXJS: Tried to use non-existent TypedArray API");
  return 0;
}

JS_EXPORT void* JSObjectGetTypedArrayBytesPtr(JSContextRef ctx, JSObjectRef object,
                                              JSValueRef* exception) {
  EXJSConsoleLog(ctx, "EXJS: Tried to use non-existent TypedArray API");
  return NULL;
}

JS_EXPORT JSObjectRef JSObjectMakeTypedArrayWithBytesNoCopy(JSContextRef ctx, JSTypedArrayType arrayType,
                                                            void* bytes, size_t byteLength,
                                                            JSTypedArrayBytesDeallocator bytesDeallocator,
                                                            void* deallocatorContext,
                                                            JSValueRef* exception) {
  EXJSConsoleLog(ctx, "EXJS: Tried to use non-existent TypedArray API");
  return (JSObjectRef) JSValueMakeNull(ctx);
}

JS_EXPORT JSObjectRef JSObjectMakeTypedArray(JSContextRef ctx, JSTypedArrayType arrayType,
                                             size_t length, JSValueRef* exception) {
  EXJSConsoleLog(ctx, "EXJS: Tried to use non-existent TypedArray API");
  return (JSObjectRef) JSValueMakeNull(ctx);
}

#endif
