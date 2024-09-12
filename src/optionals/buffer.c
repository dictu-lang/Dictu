#include "buffer.h"

typedef struct {
    uint8_t *bytes;
    int size;
    bool bigEndian;
} Buffer;

#define AS_BUFFER(v) ((Buffer *)AS_ABSTRACT(v)->data)

ObjAbstract *newBufferObj(DictuVM *vm, double capacity);

void freeBuffer(DictuVM *vm, ObjAbstract *abstract) {
    Buffer *buffer = (Buffer *)abstract->data;
    FREE_ARRAY(vm, uint8_t, buffer->bytes, buffer->size);
    FREE(vm, Buffer, abstract->data);
}

char *bufferToString(ObjAbstract *abstract) {
    UNUSED(abstract);

    char *bufferString = malloc(sizeof(char) * 9);
    snprintf(bufferString, 9, "<Buffer>");
    return bufferString;
}

uint8_t *swap(uint8_t *ptr, size_t len, bool bigEndian) {
    if (len < 2)
        return ptr;
    if (!bigEndian && !IS_BIG_ENDIAN) {
        return ptr;
    } else if (IS_BIG_ENDIAN && bigEndian) {
        return ptr;
    }
    int start = 0;
    int end = (len)-1;
    uint8_t temp;
    while (start < end) {
        temp = ptr[start];
        ptr[start] = ptr[end];
        ptr[end] = temp;
        start++;
        end--;
    }
    return ptr;
}

bool ensureSize(Buffer *buffer, size_t offset, size_t size) {
    return buffer->size - offset >= size;
}

bool writeInternal(Buffer *buffer, size_t offset, uint8_t *data, size_t len) {
    if (!ensureSize(buffer, offset, len))
        return false;
    memcpy(buffer->bytes + offset, data, len);
    return true;
}

uint8_t *getReadPtr(Buffer *buffer, size_t offset, size_t len) {
    if (!ensureSize(buffer, offset, len))
        return NULL;
    return buffer->bytes + offset;
}

static Value bufferResize(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "resize() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm, "resize() size argument must be a number");
        return EMPTY_VAL;
    }

    double capacity = AS_NUMBER(args[1]);
    if (capacity <= 0 || capacity >= BUFFER_SIZE_MAX) {
        return newResultError(
            vm, "size must be greater than 0 and smaller than 2147483647");
    }
    Buffer *buffer = AS_BUFFER(args[0]);
    buffer->bytes = reallocate(vm, buffer->bytes, buffer->size, capacity);
    if (capacity > buffer->size) {
        // 0 init everything if we grew the buffer
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

static Value bufferValues(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "values() takes no arguments");
        return EMPTY_VAL;
    }
    Buffer *buffer = AS_BUFFER(args[0]);
    ObjList *list = newList(vm);
    push(vm, OBJ_VAL(list));

    for (int i = 0; i < buffer->size; ++i) {
        writeValueArray(vm, &list->values, NUMBER_VAL(buffer->bytes[i]));
    }
    pop(vm);
    return OBJ_VAL(list);
}

static Value bufferString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError(vm, "string() takes no arguments");
        return EMPTY_VAL;
    }
    Buffer *buffer = AS_BUFFER(args[0]);

    return OBJ_VAL(copyString(vm, (const char *)buffer->bytes, buffer->size));
}
static Value bufferWriteint8(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 2) {
        runtimeError(vm, "writeInt8() takes 2 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm,
                      "writeInt8() index argument must be a number");
        return EMPTY_VAL;
    }
    if (!IS_NUMBER(args[2])) {
        runtimeError(vm,
                    "writeInt8() value argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);
    double value = AS_NUMBER(args[2]);

    int8_t correctVal = (int8_t)value;

    if (!writeInternal(
            buffer, index,
            swap((uint8_t *)&correctVal, sizeof(correctVal), buffer->bigEndian),
            sizeof(correctVal)))
        return newResultError(vm, "index must be smaller than buffer size - 1");
    return newResultSuccess(vm, NUMBER_VAL(correctVal));
}
static Value bufferWriteUint16LE(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 2) {
        runtimeError(vm, buffer->bigEndian
                             ? "writeUInt16BE() takes 2 argument"
                             : "writeUInt16LE() takes 2 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "writeUInt16BE() index argument must be a number"
                         : "writeUInt16LE() index argument must be a number");
        return EMPTY_VAL;
    }
    if (!IS_NUMBER(args[2])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "writeUInt16BE() value argument must be a number"
                         : "writeUInt16LE() value argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);
    double value = AS_NUMBER(args[2]);

    uint16_t correctVal = (uint16_t)value;

    if (!writeInternal(
            buffer, index,
            swap((uint8_t *)&correctVal, sizeof(correctVal), buffer->bigEndian),
            sizeof(correctVal)))
        return newResultError(vm, "index must be smaller than buffer size - 2");
    return newResultSuccess(vm, NUMBER_VAL(correctVal));
}

static Value bufferWriteUint32LE(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 2) {
        runtimeError(vm, buffer->bigEndian
                             ? "writeUInt32BE() takes 2 argument"
                             : "writeUInt32LE() takes 2 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "writeUInt32BE() index argument must be a number"
                         : "writeUInt32LE() index argument must be a number");
        return EMPTY_VAL;
    }
    if (!IS_NUMBER(args[2])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "writeUInt32BE() value argument must be a number"
                         : "writeUInt32LE() value argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);
    double value = AS_NUMBER(args[2]);

    uint32_t correctVal = (uint32_t)value;
    if (!writeInternal(
            buffer, index,
            swap((uint8_t *)&correctVal, sizeof(correctVal), buffer->bigEndian),
            sizeof(correctVal)))
        return newResultError(vm, "index must be smaller than buffer size - 4");
    return newResultSuccess(vm, NUMBER_VAL(correctVal));
}

static Value bufferWriteUint64LE(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 2) {
        runtimeError(vm, buffer->bigEndian
                             ? "writeUInt64BE() takes 2 argument"
                             : "writeUInt64LE() takes 2 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "writeUInt64BE() index argument must be a number"
                         : "writeUInt64LE() index argument must be a number");
        return EMPTY_VAL;
    }
    if (!IS_NUMBER(args[2])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "writeUInt64BE() value argument must be a number"
                         : "writeUInt64LE() value argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);
    double value = AS_NUMBER(args[2]);

    uint64_t correctVal = (uint64_t)value;
    if (!writeInternal(
            buffer, index,
            swap((uint8_t *)&correctVal, sizeof(correctVal), buffer->bigEndian),
            sizeof(correctVal)))
        return newResultError(vm, "index must be smaller than buffer size - 8");
    return newResultSuccess(vm, NUMBER_VAL(correctVal));
}

static Value bufferWriteint64LE(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 2) {
        runtimeError(vm, buffer->bigEndian ? "writeInt64BE() takes 2 argument"
                                           : "writeInt64LE() takes 2 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "writeInt64BE() index argument must be a number"
                         : "writeInt64LE() index argument must be a number");
        return EMPTY_VAL;
    }
    if (!IS_NUMBER(args[2])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "writeInt64BE() value argument must be a number"
                         : "writeInt64LE() value argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);
    double value = AS_NUMBER(args[2]);
    int64_t correctVal = (int64_t)value;
    if (!writeInternal(
            buffer, index,
            swap((uint8_t *)&correctVal, sizeof(correctVal), buffer->bigEndian),
            sizeof(correctVal)))
        return newResultError(vm, "index must be smaller than buffer size - 8");
    return newResultSuccess(vm, NUMBER_VAL(correctVal));
}

static Value bufferWriteint32LE(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 2) {
        runtimeError(vm, buffer->bigEndian ? "writeInt32BE() takes 2 argument"
                                           : "writeInt32LE() takes 2 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "writeInt32BE() index argument must be a number"
                         : "writeInt32LE() index argument must be a number");
        return EMPTY_VAL;
    }
    if (!IS_NUMBER(args[2])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "writeInt32BE() value argument must be a number"
                         : "writeInt32LE() value argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);
    double value = AS_NUMBER(args[2]);

    int32_t correctVal = (int32_t)value;
    if (!writeInternal(
            buffer, index,
            swap((uint8_t *)&correctVal, sizeof(correctVal), buffer->bigEndian),
            sizeof(correctVal)))
        return newResultError(vm, "index must be smaller than buffer size - 4");
    return newResultSuccess(vm, NUMBER_VAL(correctVal));
}

static Value bufferWriteint16LE(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 2) {
        runtimeError(vm, buffer->bigEndian ? "writeInt16BE() takes 2 argument"
                                           : "writeInt16LE() takes 2 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "writeInt16BE() index argument must be a number"
                         : "writeInt16LE() index argument must be a number");
        return EMPTY_VAL;
    }
    if (!IS_NUMBER(args[2])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "writeInt16BE() value argument must be a number"
                         : "writeInt16LE() value argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);
    double value = AS_NUMBER(args[2]);

    int16_t correctVal = (int16_t)value;
    if (!writeInternal(
            buffer, index,
            swap((uint8_t *)&correctVal, sizeof(correctVal), buffer->bigEndian),
            sizeof(correctVal)))
        return newResultError(vm, "index must be smaller than buffer size - 2");
    return newResultSuccess(vm, NUMBER_VAL(correctVal));
}

static Value bufferWritefloat32LE(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 2) {
        runtimeError(vm, buffer->bigEndian ? "writeFloatBE() takes 2 argument"
                                           : "writeFloatLE() takes 2 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "writeFloatBE() index argument must be a number"
                         : "writeFloatLE() index argument must be a number");
        return EMPTY_VAL;
    }
    if (!IS_NUMBER(args[2])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "writeFloatBE() value argument must be a number"
                         : "writeFloatLE() value argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);
    double value = AS_NUMBER(args[2]);

    float correctVal = (float)value;
    if (!writeInternal(
            buffer, index,
            swap((uint8_t *)&correctVal, sizeof(correctVal), buffer->bigEndian),
            sizeof(correctVal)))
        return newResultError(vm, "index must be smaller than buffer size - 4");
    return newResultSuccess(vm, NUMBER_VAL(correctVal));
}

static Value bufferWritefloat64LE(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 2) {
        runtimeError(vm, buffer->bigEndian
                             ? "writeDoubleBE() takes 2 argument"
                             : "writeDoubleLE() takes 2 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "writeDoubleBE() index argument must be a number"
                         : "writeDoubleLE() index argument must be a number");
        return EMPTY_VAL;
    }
    if (!IS_NUMBER(args[2])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "writeDoubleBE() value argument must be a number"
                         : "writeDoubleLE() value argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);
    double value = AS_NUMBER(args[2]);

    double correctVal = value;

    if (!writeInternal(
            buffer, index,
            swap((uint8_t *)&correctVal, sizeof(correctVal), buffer->bigEndian),
            sizeof(correctVal)))
        return newResultError(vm, "index must be smaller than buffer size - 8");
    return newResultSuccess(vm, NUMBER_VAL(correctVal));
}

static Value bufferReadfloat64LE(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 1) {
        runtimeError(vm, buffer->bigEndian ? "readDoubleBE() takes 1 argument"
                                           : "readDoubleLE() takes 1 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "readDoubleBE() index argument must be a number"
                         : "readDoubleLE() index argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);
    double value;

    uint8_t *ptr = getReadPtr(buffer, index, sizeof(value));
    if (ptr == NULL)
        return newResultError(vm, "index must be smaller than buffer size - 8");
    memcpy(&value, ptr, sizeof(value));
    swap((uint8_t *)&value, sizeof(value), buffer->bigEndian);

    return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferReadfloat32LE(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 1) {
        runtimeError(vm, buffer->bigEndian ? "readFloatBE() takes 1 argument"
                                           : "readFloatLE() takes 1 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm, buffer->bigEndian
                             ? "readFloatBE() index argument must be a number"
                             : "readFloatLE() index argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);
    float value;

    uint8_t *ptr = getReadPtr(buffer, index, sizeof(value));
    if (ptr == NULL)
        return newResultError(vm, "index must be smaller than buffer size - 4");
    memcpy(&value, ptr, sizeof(value));
    swap((uint8_t *)&value, sizeof(value), buffer->bigEndian);
    return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferReadUint64LE(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 1) {
        runtimeError(vm, buffer->bigEndian ? "readUInt64BE() takes 1 argument"
                                           : "readUInt64LE() takes 1 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "readUInt64BE() index argument must be a number"
                         : "readUInt64LE() index argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);
    uint64_t value;

    uint8_t *ptr = getReadPtr(buffer, index, sizeof(value));
    if (ptr == NULL)
        return newResultError(vm, "index must be smaller than buffer size - 8");
    memcpy(&value, ptr, sizeof(value));
    swap((uint8_t *)&value, sizeof(value), buffer->bigEndian);

    // Above this value theres no guarantee that the integer value is correctly represented,
    // so if are above that we don't allow it, 
    const uint64_t MAX_VALUE = 9007199254740992;
    if (value > MAX_VALUE){
        return newResultError(vm,
                              "value too large for internal representation");
    }
    return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferReadUint32LE(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 1) {
        runtimeError(vm, buffer->bigEndian ? "readUInt32BE() takes 1 argument"
                                           : "readUInt32LE() takes 1 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "readUInt32BE() index argument must be a number"
                         : "readUInt32LE() index argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);
    uint32_t value;
    uint8_t *ptr = getReadPtr(buffer, index, sizeof(value));
    if (ptr == NULL)
        return newResultError(vm, "index must be smaller than buffer size - 4");
    memcpy(&value, ptr, sizeof(value));
    swap((uint8_t *)&value, sizeof(value), buffer->bigEndian);
    return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferReadUint16LE(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 1) {
        runtimeError(vm, buffer->bigEndian ? "readUInt16BE() takes 1 argument"
                                           : "readUInt16LE() takes 1 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm,
                     buffer->bigEndian
                         ? "readUInt16BE() index argument must be a number"
                         : "readUInt16LE() index argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);

    uint16_t value;
    uint8_t *ptr = getReadPtr(buffer, index, sizeof(value));
    if (ptr == NULL)
        return newResultError(vm, "index must be smaller than buffer size - 2");
    memcpy(&value, ptr, sizeof(value));
    swap((uint8_t *)&value, sizeof(value), buffer->bigEndian);
    return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferReadint64LE(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 1) {
        runtimeError(vm, buffer->bigEndian ? "readInt64BE() takes 1 argument"
                                           : "readInt64LE() takes 1 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm, buffer->bigEndian
                             ? "readInt64BE() index argument must be a number"
                             : "readInt64LE() index argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);
    int64_t value;
    uint8_t *ptr = getReadPtr(buffer, index, sizeof(value));
    if (ptr == NULL)
        return newResultError(vm, "index must be smaller than buffer size - 8");
    memcpy(&value, ptr, sizeof(value));

    swap((uint8_t *)&value, sizeof(value), buffer->bigEndian);
    // Above this value theres no guarantee that the integer value is correctly represented,
    // so if are above that we don't allow it, 
    const int64_t MAX_VALUE = 9007199254740992;
    if (value > MAX_VALUE || value < -MAX_VALUE){
        return newResultError(vm,
                              "value too large for internal representation");
    }
    return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferReadint32LE(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 1) {
        runtimeError(vm, buffer->bigEndian ? "readInt32BE() takes 1 argument"
                                           : "readInt32LE() takes 1 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm, buffer->bigEndian
                             ? "readInt32BE() index argument must be a number"
                             : "readInt32LE() index argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);

    int32_t value;
    uint8_t *ptr = getReadPtr(buffer, index, sizeof(value));
    if (ptr == NULL)
        return newResultError(vm, "index must be smaller than buffer size - 4");
    memcpy(&value, ptr, sizeof(value));
    swap((uint8_t *)&value, sizeof(value), buffer->bigEndian);
    return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferReadint16LE(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 1) {
        runtimeError(vm, buffer->bigEndian ? "readInt16BE() takes 1 argument"
                                           : "readInt16LE() takes 1 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm, buffer->bigEndian
                             ? "readInt16BE() index argument must be a number"
                             : "readInt16LE() index argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);

    int16_t value;
    uint8_t *ptr = getReadPtr(buffer, index, sizeof(value));
    if (ptr == NULL)
        return newResultError(vm, "index must be smaller than buffer size - 2");
    memcpy(&value, ptr, sizeof(value));
    swap((uint8_t *)&value, sizeof(value), buffer->bigEndian);
    return newResultSuccess(vm, NUMBER_VAL(value));
}

static Value bufferReadint8(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    if (argCount != 1) {
        runtimeError(vm, "readInt8() takes 1 argument");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm, "readInt8() index argument must be a number");
        return EMPTY_VAL;
    }
    double index = AS_NUMBER(args[1]);

    int8_t value;
    uint8_t *ptr = getReadPtr(buffer, index, sizeof(value));
    if (ptr == NULL)
        return newResultError(vm, "index must be smaller than buffer size - 1");
    memcpy(&value, ptr, sizeof(value));
    swap((uint8_t *)&value, sizeof(value), buffer->bigEndian);
    return newResultSuccess(vm, NUMBER_VAL(value));
}

typedef Value buffer_func_t(DictuVM *vm, int argCount, Value *args);
// is this hacky?
static Value runBigEndian(DictuVM *vm, int argCount, Value *args,
                          buffer_func_t *f) {
    Buffer *buffer = AS_BUFFER(args[0]);
    buffer->bigEndian = true;
    Value result = f(vm, argCount, args);
    buffer->bigEndian = false;
    return result;
}

static Value bufferReadUint64BE(DictuVM *vm, int argCount, Value *args) {
    return runBigEndian(vm, argCount, args, &bufferReadUint64LE);
}

static Value bufferReadUint32BE(DictuVM *vm, int argCount, Value *args) {
    return runBigEndian(vm, argCount, args, &bufferReadUint32LE);
}

static Value bufferReadUint16BE(DictuVM *vm, int argCount, Value *args) {
    return runBigEndian(vm, argCount, args, &bufferReadUint16LE);
}

static Value bufferReadint64BE(DictuVM *vm, int argCount, Value *args) {
    return runBigEndian(vm, argCount, args, &bufferReadint64LE);
}

static Value bufferReadint32BE(DictuVM *vm, int argCount, Value *args) {
    return runBigEndian(vm, argCount, args, &bufferReadint32LE);
}

static Value bufferReadint16BE(DictuVM *vm, int argCount, Value *args) {
    return runBigEndian(vm, argCount, args, &bufferReadint16LE);
}

static Value bufferReadfloat32BE(DictuVM *vm, int argCount, Value *args) {
    return runBigEndian(vm, argCount, args, &bufferReadfloat32LE);
}

static Value bufferReadfloat64BE(DictuVM *vm, int argCount, Value *args) {
    return runBigEndian(vm, argCount, args, &bufferReadfloat64LE);
}

static Value bufferWriteUint64BE(DictuVM *vm, int argCount, Value *args) {
    return runBigEndian(vm, argCount, args, &bufferWriteUint64LE);
}

static Value bufferWriteUint32BE(DictuVM *vm, int argCount, Value *args) {
    return runBigEndian(vm, argCount, args, &bufferWriteUint32LE);
}

static Value bufferWriteUint16BE(DictuVM *vm, int argCount, Value *args) {
    return runBigEndian(vm, argCount, args, &bufferWriteUint16LE);
}

static Value bufferWriteint64BE(DictuVM *vm, int argCount, Value *args) {
    return runBigEndian(vm, argCount, args, &bufferWriteint64LE);
}

static Value bufferWriteint32BE(DictuVM *vm, int argCount, Value *args) {
    return runBigEndian(vm, argCount, args, &bufferWriteint32LE);
}

static Value bufferWriteint16BE(DictuVM *vm, int argCount, Value *args) {
    return runBigEndian(vm, argCount, args, &bufferWriteint16LE);
}

static Value bufferWritefloat32BE(DictuVM *vm, int argCount, Value *args) {
    return runBigEndian(vm, argCount, args, &bufferWritefloat32LE);
}

static Value bufferWritefloat64BE(DictuVM *vm, int argCount, Value *args) {
    return runBigEndian(vm, argCount, args, &bufferWritefloat64LE);
}

static Value bufferGet(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "get() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm, "get() argument must be a number");
        return EMPTY_VAL;
    }

    double index = AS_NUMBER(args[1]);
    if (index < 0) {
        return newResultError(vm, "index must be greater than -1");
    }
    Buffer *buffer = AS_BUFFER(args[0]);
    if (index >= buffer->size) {
        return newResultError(vm, "index must be smaller than buffer size");
    }

    return newResultSuccess(vm, NUMBER_VAL(buffer->bytes[(size_t)index]));
}

static Value bufferSet(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "set() takes 2 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm, "set() index argument must be a number");
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[2])) {
        runtimeError(vm, "set() value argument must be a number");
        return EMPTY_VAL;
    }

    double index = AS_NUMBER(args[1]);
    double value = AS_NUMBER(args[2]);
    if (index < 0) {
        return newResultError(vm, "index must be greater than -1");
    }
    Buffer *buffer = AS_BUFFER(args[0]);
    if (index >= buffer->size) {
        return newResultError(vm, "index must be smaller than buffer size");
    }

    buffer->bytes[(size_t)index] = (uint8_t)value;

    return newResultSuccess(vm, NUMBER_VAL(buffer->bytes[(size_t)index]));
}

static Value bufferWriteString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError(vm, "writeString() takes 2 argument (%d given).",
                     argCount);
        return EMPTY_VAL;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError(vm, "writeString() index argument must be a number");
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[2])) {
        runtimeError(vm, "writeString() value argument must be a string");
        return EMPTY_VAL;
    }

    double index = AS_NUMBER(args[1]);
    ObjString *str = AS_STRING(args[2]);
    if (index < 0) {
        return newResultError(vm, "index must be greater than -1");
    }
    Buffer *buffer = AS_BUFFER(args[0]);
    if (index >= buffer->size) {
        return newResultError(vm, "index must be smaller than buffer size");
    }

    if (buffer->size - index < str->length) {
        return newResultError(vm,
                              "buffer is not large enough to fit the string");
    }
    memcpy(buffer->bytes + (size_t)index, str->chars, str->length);
    return newResultSuccess(vm, NIL_VAL);
}

static Value bufferReadString(DictuVM *vm, int argCount, Value *args) {
    Buffer *buffer = AS_BUFFER(args[0]);
    size_t start = 0;
    size_t end = buffer->size;
    int length = buffer->size;
    if (argCount > 0) {
        if (!IS_NUMBER(args[1])) {
            runtimeError(vm, "readString() start argument must be a number");
            return EMPTY_VAL;
        }
        double startParam = AS_NUMBER(args[1]);
        if (startParam >= buffer->size) {
            return newResultError(vm,
                                  "start greater or equals than buffer length");
        } else {
            start = startParam;
            length = end - start;
        }
    }
    if (argCount == 2) {
        if (!IS_NUMBER(args[2])) {
            runtimeError(vm, "readString() end argument must be a number");
            return EMPTY_VAL;
        }
        double endParam = AS_NUMBER(args[2]);
        if (endParam > buffer->size) {
            return newResultError(vm, "end greater than buffer length");
        } else {
            end = endParam;
            length = end - start;
        }
    }
    if (length <= 0) {
        return newResultError(vm, "string length is 0");
    }
    return newResultSuccess(
        vm,
        OBJ_VAL(copyString(vm, (const char *)buffer->bytes + start, length)));
}

static Value bufferSubArray(DictuVM *vm, int argCount, Value *args) {
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
            return newResultError(vm,
                                  "start greater or equals than buffer length");
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
        if (endParam > buffer->size) {
            return newResultError(vm, "end greater than buffer length");
        } else {
            end = endParam;
            length = end - start;
        }
    }
    if (length <= 0) {
        return newResultError(vm, "array length is 0");
    }
    ObjAbstract *newBuffer = newBufferObj(vm, length);
    Buffer *nb = (Buffer *)newBuffer->data;
    for (int i = 0; i < length; i++) {
        nb->bytes[i] = buffer->bytes[start + i];
    }
    return newResultSuccess(vm, OBJ_VAL(newBuffer));
}

ObjAbstract *newBufferObj(DictuVM *vm, double capacity) {
    ObjAbstract *abstract = newAbstract(vm, freeBuffer, bufferToString);
    push(vm, OBJ_VAL(abstract));

    Buffer *buffer = ALLOCATE(vm, Buffer, 1);
    buffer->bigEndian = false;
    buffer->bytes = ALLOCATE(vm, uint8_t, capacity);
    memset(buffer->bytes, 0, capacity);
    buffer->size = capacity;

    /**
     * Setup Buffer object methods
     */
    defineNative(vm, &abstract->values, "resize", bufferResize);
    defineNative(vm, &abstract->values, "set", bufferSet);
    defineNative(vm, &abstract->values, "get", bufferGet);
    defineNative(vm, &abstract->values, "subarray", bufferSubArray);
    defineNative(vm, &abstract->values, "string", bufferString);
    defineNative(vm, &abstract->values, "len", bufferLen);
    defineNative(vm, &abstract->values, "values", bufferValues);

    defineNative(vm, &abstract->values, "writeString", bufferWriteString);
    defineNative(vm, &abstract->values, "readString", bufferReadString);

    defineNative(vm, &abstract->values, "readUInt64LE", bufferReadUint64LE);
    defineNative(vm, &abstract->values, "readUInt32LE", bufferReadUint32LE);
    defineNative(vm, &abstract->values, "readUInt16LE", bufferReadUint16LE);
    defineNative(vm, &abstract->values, "readInt64LE", bufferReadint64LE);
    defineNative(vm, &abstract->values, "readInt32LE", bufferReadint32LE);
    defineNative(vm, &abstract->values, "readInt16LE", bufferReadint16LE);
    defineNative(vm, &abstract->values, "readInt8", bufferReadint8);

    defineNative(vm, &abstract->values, "readFloatLE", bufferReadfloat32LE);
    defineNative(vm, &abstract->values, "readDoubleLE", bufferReadfloat64LE);

    defineNative(vm, &abstract->values, "writeUInt64LE", bufferWriteUint64LE);
    defineNative(vm, &abstract->values, "writeUInt32LE", bufferWriteUint32LE);
    defineNative(vm, &abstract->values, "writeUInt16LE", bufferWriteUint16LE);
    defineNative(vm, &abstract->values, "writeInt64LE", bufferWriteint64LE);
    defineNative(vm, &abstract->values, "writeInt32LE", bufferWriteint32LE);
    defineNative(vm, &abstract->values, "writeInt16LE", bufferWriteint16LE);
    defineNative(vm, &abstract->values, "writeInt8", bufferWriteint8);

    defineNative(vm, &abstract->values, "writeFloatLE", bufferWritefloat32LE);
    defineNative(vm, &abstract->values, "writeDoubleLE", bufferWritefloat64LE);

    defineNative(vm, &abstract->values, "readUInt64BE", bufferReadUint64BE);
    defineNative(vm, &abstract->values, "readUInt32BE", bufferReadUint32BE);
    defineNative(vm, &abstract->values, "readUInt16BE", bufferReadUint16BE);
    defineNative(vm, &abstract->values, "readInt64BE", bufferReadint64BE);
    defineNative(vm, &abstract->values, "readInt32BE", bufferReadint32BE);
    defineNative(vm, &abstract->values, "readInt16BE", bufferReadint16BE);

    defineNative(vm, &abstract->values, "readFloatBE", bufferReadfloat32BE);
    defineNative(vm, &abstract->values, "readDoubleBE", bufferReadfloat64BE);

    defineNative(vm, &abstract->values, "writeUInt64BE", bufferWriteUint64BE);
    defineNative(vm, &abstract->values, "writeUInt32BE", bufferWriteUint32BE);
    defineNative(vm, &abstract->values, "writeUInt16BE", bufferWriteUint16BE);
    defineNative(vm, &abstract->values, "writeInt64BE", bufferWriteint64BE);
    defineNative(vm, &abstract->values, "writeInt32BE", bufferWriteint32BE);
    defineNative(vm, &abstract->values, "writeInt16BE", bufferWriteint16BE);

    defineNative(vm, &abstract->values, "writeFloatBE", bufferWritefloat32BE);
    defineNative(vm, &abstract->values, "writeDoubleBE", bufferWritefloat64BE);

    abstract->data = buffer;
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
    if (capacity <= 0 || capacity >= BUFFER_SIZE_MAX) {
        return newResultError(
            vm, "capacity must be greater than 0 and less than 2147483647");
    }

    return newResultSuccess(vm, OBJ_VAL(newBufferObj(vm, capacity)));
}

static Value newBufferFromString(DictuVM *vm, int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError(vm, "fromString() takes 1 argument (%d given).", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError(vm, "fromString() argument must be a string");
        return EMPTY_VAL;
    }

    ObjString *str = AS_STRING(args[0]);
    if (str->length <= 0) {
        return newResultError(vm, "string length needs to be greater than 0");
    }
    ObjAbstract *b = newBufferObj(vm, str->length);
    Buffer *buffer = (Buffer *)b->data;
    memcpy(buffer->bytes, str->chars, str->length);
    return newResultSuccess(vm, OBJ_VAL(b));
}

Value createBufferModule(DictuVM *vm) {
    ObjString *name = copyString(vm, "Buffer", 6);
    push(vm, OBJ_VAL(name));
    ObjModule *module = newModule(vm, name);
    push(vm, OBJ_VAL(module));

    defineNative(vm, &module->values, "new", newBuffer);
    defineNative(vm, &module->values, "fromString", newBufferFromString);

    pop(vm);
    pop(vm);

    return OBJ_VAL(module);
}