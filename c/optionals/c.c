#include "c.h"

Value strerrorGeneric(VM *vm, int error) {
    if (error <= 0) {
        runtimeError(vm, "strerror() argument should be > 0");
        return EMPTY_VAL;
    }

    char buf[MAX_ERROR_LEN];

#ifdef POSIX_STRERROR
    int retval = strerror_r(error, buf, sizeof(buf));

    if (!retval) {
        return OBJ_VAL(copyString(vm, buf, strlen(buf)));
    }

    if (retval == EINVAL) {
        runtimeError(vm, "strerror() argument should be <= %d", LAST_ERROR);
        return EMPTY_VAL;
    }

    /* it is safe to assume that we do not reach here */
    return OBJ_VAL(copyString(vm, "Buffer is too small", 19));
#else
    char *tmp;
    tmp = strerror_r(error, buf, sizeof(buf));
    return OBJ_VAL(copyString(vm, buf, strlen(buf)));
#endif
}

Value strerrorNative(VM *vm, int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError(vm, "strerror() takes either 0 or 1 arguments (%d given)", argCount);
        return EMPTY_VAL;
    }

    int error;
    if (argCount == 1) {
      error = AS_NUMBER(args[0]);
    } else {
        error = AS_NUMBER(GET_ERRNO(GET_SELF_CLASS));
    }

    return strerrorGeneric(vm, error);
}

void createCClass(VM *vm) {
    ObjString *name = copyString(vm, "C", 1);
    push(vm, OBJ_VAL(name));
    ObjClassNative *klass = newClassNative(vm, name);
    push(vm, OBJ_VAL(klass));

    /**
     * Define C methods
     */
    defineNative(vm, &klass->methods, "strerror", strerrorNative);

    /**
     * Define C properties
     */
    defineNativeProperty(vm, &klass->properties, "EPERM",  NUMBER_VAL(EPERM));
    defineNativeProperty(vm, &klass->properties, "ENOENT", NUMBER_VAL(ENOENT));
    defineNativeProperty(vm, &klass->properties, "ESRCH",  NUMBER_VAL(ESRCH));
    defineNativeProperty(vm, &klass->properties, "EINTR",  NUMBER_VAL(EINTR));
    defineNativeProperty(vm, &klass->properties, "EIO",    NUMBER_VAL(EIO));
    defineNativeProperty(vm, &klass->properties, "ENXIO",  NUMBER_VAL(ENXIO));
    defineNativeProperty(vm, &klass->properties, "E2BIG",  NUMBER_VAL(E2BIG));
    defineNativeProperty(vm, &klass->properties, "ENOEXEC",NUMBER_VAL(ENOEXEC));
    defineNativeProperty(vm, &klass->properties, "EAGAIN", NUMBER_VAL(EAGAIN));
    defineNativeProperty(vm, &klass->properties, "ENOMEM", NUMBER_VAL(ENOMEM));
    defineNativeProperty(vm, &klass->properties, "EACCES", NUMBER_VAL(EACCES));
    defineNativeProperty(vm, &klass->properties, "EFAULT", NUMBER_VAL(EFAULT));
#ifdef ENOTBLK
    defineNativeProperty(vm, &klass->properties, "ENOTBLK", NUMBER_VAL(ENOTBLK));
#endif
    defineNativeProperty(vm, &klass->properties, "EBUSY",  NUMBER_VAL(EBUSY));
    defineNativeProperty(vm, &klass->properties, "EEXIST", NUMBER_VAL(EEXIST));
    defineNativeProperty(vm, &klass->properties, "EXDEV",  NUMBER_VAL(EXDEV));
    defineNativeProperty(vm, &klass->properties, "ENODEV", NUMBER_VAL(ENODEV));
    defineNativeProperty(vm, &klass->properties, "ENOTDIR",NUMBER_VAL(ENOTDIR));
    defineNativeProperty(vm, &klass->properties, "EISDIR", NUMBER_VAL(EISDIR));
    defineNativeProperty(vm, &klass->properties, "EINVAL", NUMBER_VAL(EINVAL));
    defineNativeProperty(vm, &klass->properties, "ENFILE", NUMBER_VAL(ENFILE));
    defineNativeProperty(vm, &klass->properties, "EMFILE", NUMBER_VAL(EMFILE));
    defineNativeProperty(vm, &klass->properties, "ENOTTY", NUMBER_VAL(ENOTTY));
    defineNativeProperty(vm, &klass->properties, "ETXTBSY",NUMBER_VAL(ETXTBSY));
    defineNativeProperty(vm, &klass->properties, "EFBIG",  NUMBER_VAL(EFBIG));
    defineNativeProperty(vm, &klass->properties, "ENOSPC", NUMBER_VAL(ENOSPC));
    defineNativeProperty(vm, &klass->properties, "ESPIPE", NUMBER_VAL(ESPIPE));
    defineNativeProperty(vm, &klass->properties, "EROFS",  NUMBER_VAL(EROFS));
    defineNativeProperty(vm, &klass->properties, "EMLINK", NUMBER_VAL(EMLINK));
    defineNativeProperty(vm, &klass->properties, "EPIPE",  NUMBER_VAL(EPIPE));
    defineNativeProperty(vm, &klass->properties, "EDOM",   NUMBER_VAL(EDOM));
    defineNativeProperty(vm, &klass->properties, "ERANGE", NUMBER_VAL(ERANGE));
    defineNativeProperty(vm, &klass->properties, "EDEADLK",NUMBER_VAL(EDEADLK));
    defineNativeProperty(vm, &klass->properties, "ENAMETOOLONG", NUMBER_VAL(ENAMETOOLONG));
    defineNativeProperty(vm, &klass->properties, "ENOLCK", NUMBER_VAL(ENOLCK));
    defineNativeProperty(vm, &klass->properties, "ENOSYS", NUMBER_VAL(ENOSYS));
    defineNativeProperty(vm, &klass->properties, "ENOTEMPTY", NUMBER_VAL(ENOTEMPTY));
    defineNativeProperty(vm, &klass->properties, "ELOOP",  NUMBER_VAL(ELOOP));
    defineNativeProperty(vm, &klass->properties, "EWOULDBLOCK", NUMBER_VAL(EWOULDBLOCK));
    defineNativeProperty(vm, &klass->properties, "ENOMSG", NUMBER_VAL(ENOMSG));
    defineNativeProperty(vm, &klass->properties, "EIDRM", NUMBER_VAL(EIDRM));
#ifdef ECHRNG
    defineNativeProperty(vm, &klass->properties, "ECHRNG", NUMBER_VAL(ECHRNG));
#endif
#ifdef EL2NSYNC
    defineNativeProperty(vm, &klass->properties, "EL2NSYNC", NUMBER_VAL(EL2NSYNC));
#endif
#ifdef EL3HLT
    defineNativeProperty(vm, &klass->properties, "EL3HLT", NUMBER_VAL(EL3HLT));
#endif
#ifdef EL3RST
    defineNativeProperty(vm, &klass->properties, "EL3RST", NUMBER_VAL(EL3RST));
#endif
#ifdef ELNRNG
    defineNativeProperty(vm, &klass->properties, "ELNRNG", NUMBER_VAL(ELNRNG));
#endif
#ifdef EUNATCH
    defineNativeProperty(vm, &klass->properties, "EUNATCH", NUMBER_VAL(EUNATCH));
#endif
#ifdef ENOCSI
    defineNativeProperty(vm, &klass->properties, "ENOCSI", NUMBER_VAL(ENOCSI));
#endif
#ifdef EL2HLT
    defineNativeProperty(vm, &klass->properties, "EL2HLT", NUMBER_VAL(EL2HLT));
#endif
#ifdef EBADE
    defineNativeProperty(vm, &klass->properties, "EBADE", NUMBER_VAL(EBADE));
#endif
#ifdef EBADR
    defineNativeProperty(vm, &klass->properties, "EBADR", NUMBER_VAL(EBADR));
#endif
#ifdef EXFULL
    defineNativeProperty(vm, &klass->properties, "EXFULL", NUMBER_VAL(EXFULL));
#endif
#ifdef ENOANO
    defineNativeProperty(vm, &klass->properties, "ENOANO", NUMBER_VAL(ENOANO));
#endif
#ifdef EBADRQC
    defineNativeProperty(vm, &klass->properties, "EBADRQC", NUMBER_VAL(EBADRQC));
#endif
#ifdef EBADSLT
    defineNativeProperty(vm, &klass->properties, "EBADSLT", NUMBER_VAL(EBADSLT));
#endif
#ifdef EDEADLOCK
    defineNativeProperty(vm, &klass->properties, "EDEADLOCK", NUMBER_VAL(EDEADLOCK));
#endif
#ifdef EBFONT
    defineNativeProperty(vm, &klass->properties, "EBFONT", NUMBER_VAL(EBFONT));
#endif
    defineNativeProperty(vm, &klass->properties, "ENOSTR", NUMBER_VAL(ENOSTR));
    defineNativeProperty(vm, &klass->properties, "ENODATA", NUMBER_VAL(ENODATA));
    defineNativeProperty(vm, &klass->properties, "ETIME", NUMBER_VAL(ETIME));
    defineNativeProperty(vm, &klass->properties, "ENOSR", NUMBER_VAL(ENOSR));
#ifdef ENONET
    defineNativeProperty(vm, &klass->properties, "ENONET", NUMBER_VAL(ENONET));
#endif
#ifdef ENOPKG
    defineNativeProperty(vm, &klass->properties, "ENOPKG", NUMBER_VAL(ENOPKG));
#endif
#ifdef EREMOTE
    defineNativeProperty(vm, &klass->properties, "EREMOTE", NUMBER_VAL(EREMOTE));
#endif
    defineNativeProperty(vm, &klass->properties, "ENOLINK", NUMBER_VAL(ENOLINK));
#ifdef EADV
    defineNativeProperty(vm, &klass->properties, "EADV", NUMBER_VAL(EADV));
#endif
#ifdef ESRMNT
    defineNativeProperty(vm, &klass->properties, "ESRMNT", NUMBER_VAL(ESRMNT));
#endif
#ifdef ECOMM
    defineNativeProperty(vm, &klass->properties, "ECOMM", NUMBER_VAL(ECOMM));
#endif
    defineNativeProperty(vm, &klass->properties, "EPROTO", NUMBER_VAL(EPROTO));
    defineNativeProperty(vm, &klass->properties, "EMULTIHOP", NUMBER_VAL(EMULTIHOP));
#ifdef EDOTDOT
    defineNativeProperty(vm, &klass->properties, "EDOTDOT", NUMBER_VAL(EDOTDOT));
#endif
    defineNativeProperty(vm, &klass->properties, "EBADMSG", NUMBER_VAL(EBADMSG));
    defineNativeProperty(vm, &klass->properties, "EOVERFLOW", NUMBER_VAL(EOVERFLOW));
#ifdef ENOTUNIQ
    defineNativeProperty(vm, &klass->properties, "ENOTUNIQ", NUMBER_VAL(ENOTUNIQ));
#endif
#ifdef EBADFD
    defineNativeProperty(vm, &klass->properties, "EBADFD", NUMBER_VAL(EBADFD));
#endif
#ifdef EREMCHG
    defineNativeProperty(vm, &klass->properties, "EREMCHG", NUMBER_VAL(EREMCHG));
#endif
#ifdef ELIBACC
    defineNativeProperty(vm, &klass->properties, "ELIBACC", NUMBER_VAL(ELIBACC));
#endif
#ifdef ELIBBAD
    defineNativeProperty(vm, &klass->properties, "ELIBBAD", NUMBER_VAL(ELIBBAD));
#endif
#ifdef ELIBSCN
    defineNativeProperty(vm, &klass->properties, "ELIBSCN", NUMBER_VAL(ELIBSCN));
#endif
#ifdef ELIBMAX
    defineNativeProperty(vm, &klass->properties, "ELIBMAX", NUMBER_VAL(ELIBMAX));
#endif
#ifdef ELIBEXEC
    defineNativeProperty(vm, &klass->properties, "ELIBEXEC", NUMBER_VAL(ELIBEXEC));
#endif
    defineNativeProperty(vm, &klass->properties, "EILSEQ", NUMBER_VAL(EILSEQ));
#ifdef ERESTART
    defineNativeProperty(vm, &klass->properties, "ERESTART", NUMBER_VAL(ERESTART));
#endif
#ifdef ESTRPIPE
    defineNativeProperty(vm, &klass->properties, "ESTRPIPE", NUMBER_VAL(ESTRPIPE));
#endif
#ifdef EUSERS
    defineNativeProperty(vm, &klass->properties, "EUSERS", NUMBER_VAL(EUSERS));
#endif
    defineNativeProperty(vm, &klass->properties, "ENOTSOCK", NUMBER_VAL(ENOTSOCK));
    defineNativeProperty(vm, &klass->properties, "EDESTADDRREQ", NUMBER_VAL(EDESTADDRREQ));
    defineNativeProperty(vm, &klass->properties, "EMSGSIZE", NUMBER_VAL(EMSGSIZE));
    defineNativeProperty(vm, &klass->properties, "EPROTOTYPE", NUMBER_VAL(EPROTOTYPE));
    defineNativeProperty(vm, &klass->properties, "ENOPROTOOPT", NUMBER_VAL(ENOPROTOOPT));
    defineNativeProperty(vm, &klass->properties, "EPROTONOSUPPORT", NUMBER_VAL(EPROTONOSUPPORT));
#ifdef ESOCKTNOSUPPORT
    defineNativeProperty(vm, &klass->properties, "ESOCKTNOSUPPORT", NUMBER_VAL(ESOCKTNOSUPPORT));
#endif
    defineNativeProperty(vm, &klass->properties, "EOPNOTSUPP", NUMBER_VAL(EOPNOTSUPP));
#ifdef EPFNOSUPPORT
    defineNativeProperty(vm, &klass->properties, "EPFNOSUPPORT", NUMBER_VAL(EPFNOSUPPORT));
#endif
    defineNativeProperty(vm, &klass->properties, "EAFNOSUPPORT", NUMBER_VAL(EAFNOSUPPORT));
    defineNativeProperty(vm, &klass->properties, "EADDRINUSE", NUMBER_VAL(EADDRINUSE));
    defineNativeProperty(vm, &klass->properties, "EADDRNOTAVAIL", NUMBER_VAL(EADDRNOTAVAIL));
    defineNativeProperty(vm, &klass->properties, "ENETDOWN", NUMBER_VAL(ENETDOWN));
    defineNativeProperty(vm, &klass->properties, "ENETUNREACH", NUMBER_VAL(ENETUNREACH));
    defineNativeProperty(vm, &klass->properties, "ENETRESET", NUMBER_VAL(ENETRESET));
    defineNativeProperty(vm, &klass->properties, "ECONNABORTED", NUMBER_VAL(ECONNABORTED));
    defineNativeProperty(vm, &klass->properties, "ECONNRESET", NUMBER_VAL(ECONNRESET));
    defineNativeProperty(vm, &klass->properties, "ENOBUFS", NUMBER_VAL(ENOBUFS));
    defineNativeProperty(vm, &klass->properties, "EISCONN", NUMBER_VAL(EISCONN));
    defineNativeProperty(vm, &klass->properties, "ENOTCONN", NUMBER_VAL(ENOTCONN));
#ifdef ESHUTDOWN
    defineNativeProperty(vm, &klass->properties, "ESHUTDOWN", NUMBER_VAL(ESHUTDOWN));
#endif
#ifdef ETOOMANYREFS
    defineNativeProperty(vm, &klass->properties, "ETOOMANYREFS", NUMBER_VAL(ETOOMANYREFS));
#endif
    defineNativeProperty(vm, &klass->properties, "ETIMEDOUT", NUMBER_VAL(ETIMEDOUT));
    defineNativeProperty(vm, &klass->properties, "ECONNREFUSED", NUMBER_VAL(ECONNREFUSED));
#ifdef EHOSTDOWN
    defineNativeProperty(vm, &klass->properties, "EHOSTDOWN", NUMBER_VAL(EHOSTDOWN));
#endif
    defineNativeProperty(vm, &klass->properties, "EHOSTUNREACH", NUMBER_VAL(EHOSTUNREACH));
    defineNativeProperty(vm, &klass->properties, "EALREADY", NUMBER_VAL(EALREADY));
    defineNativeProperty(vm, &klass->properties, "EINPROGRESS", NUMBER_VAL(EINPROGRESS));
    defineNativeProperty(vm, &klass->properties, "ESTALE", NUMBER_VAL(ESTALE));
#ifdef EUCLEAN
    defineNativeProperty(vm, &klass->properties, "EUCLEAN", NUMBER_VAL(EUCLEAN));
#endif
#ifdef ENOTNAM
    defineNativeProperty(vm, &klass->properties, "ENOTNAM", NUMBER_VAL(ENOTNAM));
#endif
#ifdef ENAVAIL
    defineNativeProperty(vm, &klass->properties, "ENAVAIL", NUMBER_VAL(ENAVAIL));
#endif
#ifdef EISNAM
    defineNativeProperty(vm, &klass->properties, "EISNAM", NUMBER_VAL(EISNAM));
#endif
#ifdef EREMOTEIO
    defineNativeProperty(vm, &klass->properties, "EREMOTEIO", NUMBER_VAL(EREMOTEIO));
#endif
    defineNativeProperty(vm, &klass->properties, "EDQUOT", NUMBER_VAL(EDQUOT));
#ifdef ENOMEDIUM
    defineNativeProperty(vm, &klass->properties, "ENOMEDIUM", NUMBER_VAL(ENOMEDIUM));
#endif
#ifdef EMEDIUMTYPE
    defineNativeProperty(vm, &klass->properties, "EMEDIUMTYPE", NUMBER_VAL(EMEDIUMTYPE));
#endif
    defineNativeProperty(vm, &klass->properties, "ECANCELED", NUMBER_VAL(ECANCELED));
#ifdef ENOKEY
    defineNativeProperty(vm, &klass->properties, "ENOKEY", NUMBER_VAL(ENOKEY));
#endif
#ifdef EKEYEXPIRED
    defineNativeProperty(vm, &klass->properties, "EKEYEXPIRED", NUMBER_VAL(EKEYEXPIRED));
#endif
#ifdef EKEYREVOKED
    defineNativeProperty(vm, &klass->properties, "EKEYREVOKED", NUMBER_VAL(EKEYREVOKED));
#endif
#ifdef EKEYREJECTED
    defineNativeProperty(vm, &klass->properties, "EKEYREJECTED", NUMBER_VAL(EKEYREJECTED));
#endif
    defineNativeProperty(vm, &klass->properties, "EOWNERDEAD", NUMBER_VAL(EOWNERDEAD));
    defineNativeProperty(vm, &klass->properties, "ENOTRECOVERABLE", NUMBER_VAL(ENOTRECOVERABLE));
#ifdef ERFKILL
    defineNativeProperty(vm, &klass->properties, "ERFKILL", NUMBER_VAL(ERFKILL));
#endif
#ifdef EHWPOISON
    defineNativeProperty(vm, &klass->properties, "EHWPOISON", NUMBER_VAL(EHWPOISON));
#endif

    tableSet(vm, &vm->globals, name, OBJ_VAL(klass));
    pop(vm);
    pop(vm);
}

