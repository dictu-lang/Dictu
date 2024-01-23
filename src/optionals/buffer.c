#include "buffer.h"
#include <string.h>

typedef struct {
  uint8_t *bytes;
  int size;
} Buffer;

#define AS_BUFFER(v) ((Buffer *)AS_ABSTRACT(v)->data)

ObjAbstract *newBufferObj(DictuVM *vm, double capacity);

void freeBuffer(DictuVM *vm, ObjAbstract *abstract) {
  Buffer *buffer = (Buffer *)abstract->data;
  free(buffer->bytes);
  FREE(vm, Buffer, abstract->data);
}

char *bufferToString(ObjAbstract *abstract) {
UNUSED(abstract);

  char *queueString = malloc(sizeof(char) * 9);
  snprintf(queueString, 9, "<Buffer>");
  return queueString;
}

void grayBuffer(DictuVM *vm, ObjAbstract *abstract) {
  (void)vm;
  Buffer *ffi = (Buffer *)abstract->data;

  if (ffi == NULL)
    return;
}

bool ensureSize(Buffer* buffer, size_t offset, size_t size){
    return buffer->size - offset >= size;
}

static Value bufferResize(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 1) {
    runtimeError(vm, "resize() takes 1 argument (%d given).", argCount);
    return EMPTY_VAL;
  }

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "resize() argument must be a numbers");
    return EMPTY_VAL;
  }

  double capacity = AS_NUMBER(args[1]);
  if (capacity <= 0) {
    return newResultError(vm, "capacity must be greater than 0");
  }
  Buffer *buffer = AS_BUFFER(args[0]);
  buffer->bytes = realloc(buffer->bytes, capacity);
  if (capacity > buffer->size) {
    size_t added = capacity - buffer->size;
    memset(buffer->bytes + buffer->size, 0, added);
  }
  buffer->size = capacity;
  return newResultSuccess(vm, args[0]);
}

static Value bufferLen(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 0) {
    runtimeError(vm, "len() takes no arguments");
    return EMPTY_VAL;
  }
    Buffer *buffer = AS_BUFFER(args[0]);

    return NUMBER_VAL(buffer->size);
}

static Value bufferString(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 0) {
    runtimeError(vm, "string() takes no arguments");
    return EMPTY_VAL;
  }
    Buffer *buffer = AS_BUFFER(args[0]);

   return OBJ_VAL(copyString(vm, (const char*)buffer->bytes, buffer->size));
}

static Value bufferWriteUint16LE(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 2) {
    runtimeError(vm, "writeUint16LE() takes 2 argument");
    return EMPTY_VAL;
  }
    Buffer *buffer = AS_BUFFER(args[0]);

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "writeUint16LE() index argument must be a numbers");
    return EMPTY_VAL;
  }
    if (!IS_NUMBER(args[2])) {
    runtimeError(vm, "writeUint16LE() value argument must be a numbers");
    return EMPTY_VAL;
  }
  double index = AS_NUMBER(args[1]);
  double value = AS_NUMBER(args[2]);

  uint16_t correctVal = (uint16_t)value;

  if(!ensureSize(buffer, index, sizeof(uint16_t))){
     return newResultError(vm, "index must be smaller then buffer size -2");
  }
  uint8_t* ptr = (uint8_t*)&correctVal;
  memcpy(buffer->bytes+(size_t)index, ptr, sizeof(uint16_t));
    return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferWriteUint32LE(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 2) {
    runtimeError(vm, "writeUint32LE() takes 2 argument");
    return EMPTY_VAL;
  }
    Buffer *buffer = AS_BUFFER(args[0]);

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "writeUint32LE() index argument must be a numbers");
    return EMPTY_VAL;
  }
    if (!IS_NUMBER(args[2])) {
    runtimeError(vm, "writeUint32LE() value argument must be a numbers");
    return EMPTY_VAL;
  }
  double index = AS_NUMBER(args[1]);
  double value = AS_NUMBER(args[2]);

  uint32_t correctVal = (uint32_t)value;

  if(!ensureSize(buffer, index, sizeof(uint32_t))){
     return newResultError(vm, "index must be smaller then buffer size - 4");
  }
  uint8_t* ptr = (uint8_t*)&correctVal;
  memcpy(buffer->bytes+(size_t)index, ptr, sizeof(uint32_t));
    return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferWriteUint64LE(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 2) {
    runtimeError(vm, "writeUint64LE() takes 2 argument");
    return EMPTY_VAL;
  }
    Buffer *buffer = AS_BUFFER(args[0]);

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "writeUint64LE() index argument must be a numbers");
    return EMPTY_VAL;
  }
    if (!IS_NUMBER(args[2])) {
    runtimeError(vm, "writeUint64LE() value argument must be a numbers");
    return EMPTY_VAL;
  }
  double index = AS_NUMBER(args[1]);
  double value = AS_NUMBER(args[2]);

  uint64_t correctVal = (uint64_t)value;

  if(!ensureSize(buffer, index, sizeof(uint64_t))){
     return newResultError(vm, "index must be smaller then buffer size - 8");
  }
  uint8_t* ptr = (uint8_t*)&correctVal;
  memcpy(buffer->bytes+(size_t)index, ptr, sizeof(uint64_t));
    return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferWriteint64LE(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 2) {
    runtimeError(vm, "writeint64LE() takes 2 argument");
    return EMPTY_VAL;
  }
    Buffer *buffer = AS_BUFFER(args[0]);

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "writeint64LE() index argument must be a numbers");
    return EMPTY_VAL;
  }
    if (!IS_NUMBER(args[2])) {
    runtimeError(vm, "writeint64LE() value argument must be a numbers");
    return EMPTY_VAL;
  }
  double index = AS_NUMBER(args[1]);
  double value = AS_NUMBER(args[2]);

  int64_t correctVal = (int64_t)value;

  if(!ensureSize(buffer, index, sizeof(int64_t))){
     return newResultError(vm, "index must be smaller then buffer size - 8");
  }
  uint8_t* ptr = (uint8_t*)&correctVal;
  memcpy(buffer->bytes+(size_t)index, ptr, sizeof(int64_t));
    return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferWriteint32LE(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 2) {
    runtimeError(vm, "writeint32LE() takes 2 argument");
    return EMPTY_VAL;
  }
    Buffer *buffer = AS_BUFFER(args[0]);

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "writeint32LE() index argument must be a numbers");
    return EMPTY_VAL;
  }
    if (!IS_NUMBER(args[2])) {
    runtimeError(vm, "writeint32LE() value argument must be a numbers");
    return EMPTY_VAL;
  }
  double index = AS_NUMBER(args[1]);
  double value = AS_NUMBER(args[2]);

  int32_t correctVal = (int32_t)value;

  if(!ensureSize(buffer, index, sizeof(int32_t))){
     return newResultError(vm, "index must be smaller then buffer size - 4");
  }
  uint8_t* ptr = (uint8_t*)&correctVal;
  memcpy(buffer->bytes+(size_t)index, ptr, sizeof(int32_t));
    return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferWriteint16LE(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 2) {
    runtimeError(vm, "writeint16LE() takes 2 argument");
    return EMPTY_VAL;
  }
    Buffer *buffer = AS_BUFFER(args[0]);

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "writeint16LE() index argument must be a numbers");
    return EMPTY_VAL;
  }
    if (!IS_NUMBER(args[2])) {
    runtimeError(vm, "writeint16LE() value argument must be a numbers");
    return EMPTY_VAL;
  }
  double index = AS_NUMBER(args[1]);
  double value = AS_NUMBER(args[2]);

  int16_t correctVal = (int16_t)value;

  if(!ensureSize(buffer, index, sizeof(int16_t))){
     return newResultError(vm, "index must be smaller then buffer size - 2");
  }
  uint8_t* ptr = (uint8_t*)&correctVal;
  memcpy(buffer->bytes+(size_t)index, ptr, sizeof(int16_t));
    return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferWritefloat32LE(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 2) {
    runtimeError(vm, "writeFloatLE() takes 2 argument");
    return EMPTY_VAL;
  }
    Buffer *buffer = AS_BUFFER(args[0]);

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "writeFloatLE() index argument must be a numbers");
    return EMPTY_VAL;
  }
    if (!IS_NUMBER(args[2])) {
    runtimeError(vm, "writeFloatLE() value argument must be a numbers");
    return EMPTY_VAL;
  }
  double index = AS_NUMBER(args[1]);
  double value = AS_NUMBER(args[2]);

  float correctVal = (float)value;

  if(!ensureSize(buffer, index, sizeof(float))){
     return newResultError(vm, "index must be smaller then buffer size - 4");
  }
  uint8_t* ptr = (uint8_t*)&correctVal;
  memcpy(buffer->bytes+(size_t)index, ptr, sizeof(float));
    return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferWritefloat64LE(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 2) {
    runtimeError(vm, "writeDoubleLE() takes 2 argument");
    return EMPTY_VAL;
  }
    Buffer *buffer = AS_BUFFER(args[0]);

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "writeDoubleLE() index argument must be a numbers");
    return EMPTY_VAL;
  }
    if (!IS_NUMBER(args[2])) {
    runtimeError(vm, "writeDoubleLE() value argument must be a numbers");
    return EMPTY_VAL;
  }
  double index = AS_NUMBER(args[1]);
  double value = AS_NUMBER(args[2]);

  double correctVal = value;

  if(!ensureSize(buffer, index, sizeof(double))){
     return newResultError(vm, "index must be smaller then buffer size - 8");
  }
  uint8_t* ptr = (uint8_t*)&correctVal;
  memcpy(buffer->bytes+(size_t)index, ptr, sizeof(double));
    return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferReadfloat64LE(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 1) {
    runtimeError(vm, "readDoubleLE() takes 1 argument");
    return EMPTY_VAL;
  }
  Buffer *buffer = AS_BUFFER(args[0]);

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "readDoubleLE() index argument must be a numbers");
    return EMPTY_VAL;
  }
  double index = AS_NUMBER(args[1]);
  double value;
  if(!ensureSize(buffer, index, sizeof(value))){
     return newResultError(vm, "index must be smaller then buffer size - 8");
  }
  memcpy(&value, buffer->bytes+(size_t)index, sizeof(value));
 return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferReadfloat32LE(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 1) {
    runtimeError(vm, "readFloatLE() takes 1 argument");
    return EMPTY_VAL;
  }
  Buffer *buffer = AS_BUFFER(args[0]);

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "readFloatLE() index argument must be a numbers");
    return EMPTY_VAL;
  }
  double index = AS_NUMBER(args[1]);
  float value;
  if(!ensureSize(buffer, index, sizeof(value))){
     return newResultError(vm, "index must be smaller then buffer size - 4");
  }
  memcpy(&value, buffer->bytes+(size_t)index, sizeof(value));
 return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferReadUint64LE(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 1) {
    runtimeError(vm, "readUint64LE() takes 1 argument");
    return EMPTY_VAL;
  }
  Buffer *buffer = AS_BUFFER(args[0]);

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "readUint64LE() index argument must be a numbers");
    return EMPTY_VAL;
  }
  double index = AS_NUMBER(args[1]);
  uint64_t value;
  if(!ensureSize(buffer, index, sizeof(value))){
     return newResultError(vm, "index must be smaller then buffer size - 8");
  }
  memcpy(&value, buffer->bytes+(size_t)index, sizeof(value));
 return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferReadUint32LE(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 1) {
    runtimeError(vm, "readUint32LE() takes 1 argument");
    return EMPTY_VAL;
  }
  Buffer *buffer = AS_BUFFER(args[0]);

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "readUint32LE() index argument must be a numbers");
    return EMPTY_VAL;
  }
  double index = AS_NUMBER(args[1]);
  uint32_t value;
  if(!ensureSize(buffer, index, sizeof(value))){
     return newResultError(vm, "index must be smaller then buffer size - 4");
  }
  memcpy(&value, buffer->bytes+(size_t)index, sizeof(value));
 return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferReadUint16LE(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 1) {
    runtimeError(vm, "readUint16LE() takes 1 argument");
    return EMPTY_VAL;
  }
  Buffer *buffer = AS_BUFFER(args[0]);

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "readUint16LE() index argument must be a numbers");
    return EMPTY_VAL;
  }
  double index = AS_NUMBER(args[1]);
  uint16_t value;
  if(!ensureSize(buffer, index, sizeof(value))){
     return newResultError(vm, "index must be smaller then buffer size - 4");
  }
  memcpy(&value, buffer->bytes+(size_t)index, sizeof(value));
 return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferReadint64LE(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 1) {
    runtimeError(vm, "readint64LE() takes 1 argument");
    return EMPTY_VAL;
  }
  Buffer *buffer = AS_BUFFER(args[0]);

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "readint64LE() index argument must be a numbers");
    return EMPTY_VAL;
  }
  double index = AS_NUMBER(args[1]);
  int64_t value;
  if(!ensureSize(buffer, index, sizeof(value))){
     return newResultError(vm, "index must be smaller then buffer size - 4");
  }
  memcpy(&value, buffer->bytes+(size_t)index, sizeof(value));
 return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferReadint32LE(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 1) {
    runtimeError(vm, "readint32LE() takes 1 argument");
    return EMPTY_VAL;
  }
  Buffer *buffer = AS_BUFFER(args[0]);

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "readint32LE() index argument must be a numbers");
    return EMPTY_VAL;
  }
  double index = AS_NUMBER(args[1]);
  int32_t value;
  if(!ensureSize(buffer, index, sizeof(value))){
     return newResultError(vm, "index must be smaller then buffer size - 4");
  }
  memcpy(&value, buffer->bytes+(size_t)index, sizeof(value));
 return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferReadint16LE(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 1) {
    runtimeError(vm, "readint32LE() takes 1 argument");
    return EMPTY_VAL;
  }
  Buffer *buffer = AS_BUFFER(args[0]);

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "readint32LE() index argument must be a numbers");
    return EMPTY_VAL;
  }
  double index = AS_NUMBER(args[1]);
  int16_t value;
  if(!ensureSize(buffer, index, sizeof(value))){
     return newResultError(vm, "index must be smaller then buffer size - 4");
  }
  memcpy(&value, buffer->bytes+(size_t)index, sizeof(value));
 return newResultSuccess(vm, NUMBER_VAL(value));
}


static Value bufferGet(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 1) {
    runtimeError(vm, "get() takes 1 argument (%d given).", argCount);
    return EMPTY_VAL;
  }

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "get() argument must be a numbers");
    return EMPTY_VAL;
  }

  double index = AS_NUMBER(args[1]);
  if (index < 0) {
    return newResultError(vm, "index must be greater than -1");
  }
  Buffer *buffer = AS_BUFFER(args[0]);
  if (index >= buffer->size) {
    return newResultError(vm, "index must be smaller then buffer size");
  }

  return newResultSuccess(vm, NUMBER_VAL(buffer->bytes[(size_t)index]));
}

static Value bufferSet(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 2) {
    runtimeError(vm, "set() takes 2 argument (%d given).", argCount);
    return EMPTY_VAL;
  }

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "set() index argument must be a numbers");
    return EMPTY_VAL;
  }

  if (!IS_NUMBER(args[2])) {
    runtimeError(vm, "set() value argument must be a numbers");
    return EMPTY_VAL;
  }

  double index = AS_NUMBER(args[1]);
  double value = AS_NUMBER(args[2]);
  if (index < 0) {
    return newResultError(vm, "index must be greater than -1");
  }
  Buffer *buffer = AS_BUFFER(args[0]);
  if (index >= buffer->size) {
    return newResultError(vm, "index must be smaller then buffer size");
  }

  buffer->bytes[(size_t)index] = (uint8_t)value;

  return newResultSuccess(vm, NUMBER_VAL(buffer->bytes[(size_t)index]));
}

static Value bufferWriteString(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 2) {
    runtimeError(vm, "writeString() takes 2 argument (%d given).", argCount);
    return EMPTY_VAL;
  }

  if (!IS_NUMBER(args[1])) {
    runtimeError(vm, "writeString() index argument must be a numbers");
    return EMPTY_VAL;
  }

  if (!IS_STRING(args[2])) {
    runtimeError(vm, "writeString() value argument must be a string");
    return EMPTY_VAL;
  }

  double index = AS_NUMBER(args[1]);
  ObjString* str = AS_STRING(args[2]);
  if (index < 0) {
    return newResultError(vm, "index must be greater than -1");
  }
  Buffer *buffer = AS_BUFFER(args[0]);
  if (index >= buffer->size) {
    return newResultError(vm, "index must be smaller then buffer size");
  }

  if(buffer->size - index < str->length) {
    return newResultError(vm, "buffer is not large enough to fit the string");
  }
  memcpy(buffer->bytes+(size_t)index, str->chars, str->length);
  return newResultSuccess(vm, NIL_VAL);
}

static Value bufferSubAarray(DictuVM *vm, int argCount, Value *args) {
  Buffer *buffer = AS_BUFFER(args[0]);
  size_t start = 0;
  size_t end = buffer->size;
  int length = buffer->size;
  if (argCount > 0) {
    if (!IS_NUMBER(args[1])) {
      runtimeError(vm, "subarray() start argument must be a number");
      return EMPTY_VAL;
    }
    double startParam = AS_NUMBER(args[1]);
    if (startParam >= buffer->size) {
            return newResultError(vm, "start greater or equals then buffer length");
    } else {
        start = startParam;
        length = end - start;
    }
  }
  if (argCount == 2) {
    if (!IS_NUMBER(args[2])) {
      runtimeError(vm, "subarray() end argument must be a number");
      return EMPTY_VAL;
    }
    double endParam = AS_NUMBER(args[2]);
    if(endParam > buffer->size){
        return newResultError(vm, "end greater then buffer length");
    } else {
        end = endParam;
        length = end - start;
    }
  }
  if(length <= 0){
     return newResultError(vm, "length is 0");
  }
  ObjAbstract* newBuffer = newBufferObj(vm, length);
  Buffer* nb = (Buffer *)newBuffer->data;
  for(int i = 0; i < length; i++){
    nb->bytes[i] = buffer->bytes[start + i];
  }
  return newResultSuccess(vm, OBJ_VAL(newBuffer));
}

ObjAbstract *newBufferObj(DictuVM *vm, double capacity) {
  ObjAbstract *abstract = newAbstract(vm, freeBuffer, bufferToString);
  push(vm, OBJ_VAL(abstract));

  Buffer *buffer = ALLOCATE(vm, Buffer, 1);
  buffer->bytes = calloc(1, capacity);
  buffer->size = capacity;

  /**
   * Setup Buffer object methods
   */
  defineNative(vm, &abstract->values, "resize", bufferResize);
  defineNative(vm, &abstract->values, "set", bufferSet);
  defineNative(vm, &abstract->values, "get", bufferGet);
  defineNative(vm, &abstract->values, "subarray", bufferSubAarray);
  defineNative(vm, &abstract->values, "string", bufferString);
  defineNative(vm, &abstract->values, "len", bufferLen);


  defineNative(vm, &abstract->values, "writeString", bufferWriteString);

  defineNative(vm, &abstract->values, "readUInt64LE", bufferReadUint64LE);
  defineNative(vm, &abstract->values, "readUInt32LE", bufferReadUint32LE);
  defineNative(vm, &abstract->values, "readUInt16LE", bufferReadUint16LE);
  defineNative(vm, &abstract->values, "readInt64LE", bufferReadint64LE);
  defineNative(vm, &abstract->values, "readInt32LE", bufferReadint32LE);
  defineNative(vm, &abstract->values, "readInt16LE", bufferReadint16LE);

  defineNative(vm, &abstract->values, "readFloatLE", bufferReadfloat32LE);
  defineNative(vm, &abstract->values, "readDoubleLE", bufferReadfloat64LE);

  defineNative(vm, &abstract->values, "writeUInt64LE", bufferWriteUint64LE);
  defineNative(vm, &abstract->values, "writeUInt32LE", bufferWriteUint32LE);
  defineNative(vm, &abstract->values, "writeUInt16LE", bufferWriteUint16LE);
  defineNative(vm, &abstract->values, "writeInt64LE", bufferWriteint64LE);
  defineNative(vm, &abstract->values, "writeInt32LE", bufferWriteint32LE);
  defineNative(vm, &abstract->values, "writeInt16LE", bufferWriteint16LE);

  defineNative(vm, &abstract->values, "writeFloatLE", bufferWritefloat32LE);
  defineNative(vm, &abstract->values, "writeDoubleLE", bufferWritefloat64LE);



  abstract->data = buffer;
  abstract->grayFunc = grayBuffer;
  pop(vm);

  return abstract;
}

static Value newBuffer(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 1) {
    runtimeError(vm, "new() takes 1 argument (%d given).", argCount);
    return EMPTY_VAL;
  }

  if (!IS_NUMBER(args[0])) {
    runtimeError(vm, "new() argument must be a numbers");
    return EMPTY_VAL;
  }

  double capacity = AS_NUMBER(args[0]);
  if (capacity <= 0) {
    return newResultError(vm, "capacity must be greater than 0");
  }

  return newResultSuccess(vm, OBJ_VAL(newBufferObj(vm, capacity)));
}

static Value newBufferFromString(DictuVM *vm, int argCount, Value *args) {
  if (argCount != 1) {
    runtimeError(vm, "fromString() takes 1 argument (%d given).", argCount);
    return EMPTY_VAL;
  }

  if (!IS_NUMBER(args[0])) {
    runtimeError(vm, "fromString() argument must be a string");
    return EMPTY_VAL;
  }

  ObjString* str = AS_STRING(args[0]);
  if (str->length <= 0) {
    return newResultError(vm, "capacity must be greater than 0");
  }
  ObjAbstract* b = newBufferObj(vm, str->length);
  Buffer* buffer = (Buffer*) b->data;
  memcpy(buffer->bytes, str->chars, str->length);
  return newResultSuccess(vm, OBJ_VAL(b));
}


Value createBufferModule(DictuVM *vm) {
  ObjString *name = copyString(vm, "Buffer", 6);
  push(vm, OBJ_VAL(name));
  ObjModule *module = newModule(vm, name);
  push(vm, OBJ_VAL(module));

  /**
   * Define Buffer methods
   */
  defineNative(vm, &module->values, "new", newBuffer);
  defineNative(vm, &module->values, "fromString", newBufferFromString);

  pop(vm);
  pop(vm);

  return OBJ_VAL(module);
}