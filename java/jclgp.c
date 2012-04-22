/* Copyright (c) 2009-2012 Matthieu Volat and Remi Emonet. All rights
 * reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <assert.h>

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif
#include <clgp/clgp.h>

#include <jni.h>
#include <CLGP.java.h>

jclass NativePointerObject_class;
jfieldID NativePointerObject_nativePointer;
jfieldID NativePointerObject_byteOffset;
jmethodID NativePointerObject_Init;
jclass cl_kernel_class;
jmethodID cl_kernel_Init;

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    JNIEnv *env = NULL;

    reserved = reserved;

    (*jvm)->GetEnv(jvm, (void**)&env, JNI_VERSION_1_4);
    assert(env != NULL);

    NativePointerObject_class = 
        (*env)->FindClass(env, "org/jocl/NativePointerObject");
    assert(NativePointerObject_class != NULL);
    NativePointerObject_nativePointer = 
        (*env)->GetFieldID(
                 env, 
                 NativePointerObject_class, 
                 "nativePointer", 
                 "J");
    assert(NativePointerObject_nativePointer != NULL);
    NativePointerObject_byteOffset = 
        (*env)->GetFieldID(
                 env, 
                 NativePointerObject_class, 
                 "byteOffset", 
                 "J");
    assert(NativePointerObject_byteOffset != NULL);
    NativePointerObject_Init = 
        (*env)->GetMethodID(env, NativePointerObject_class, "<init>", "()V");
    assert(NativePointerObject_Init != NULL);

    cl_kernel_class = (*env)->FindClass(env, "org/jocl/cl_kernel");
    assert(cl_kernel_class != NULL);

    return JNI_VERSION_1_4;
}

JNIEXPORT jobjectArray JNICALL
Java_clgp_CLGP_clgpInit(
        JNIEnv *env,
        jclass cls,
        jobject context,
        jintArray errcode)
{
    cl_int _err = CL_SUCCESS;
    cl_context _context = NULL;
    cl_kernel *_kernels = NULL;
    
    jobject kernels = NULL;

    cls = cls;

    _context = (cl_context)
        (*env)->GetLongField(env, context, NativePointerObject_nativePointer);

    _kernels = clgpInit(_context, &_err); 
    assert(_kernels != NULL);

    kernels = 
        (*env)->NewObject(
                env, 
                NativePointerObject_class, 
                NativePointerObject_Init);
    assert(kernels != NULL);
    (*env)->SetLongField(
            env, 
            kernels, 
            NativePointerObject_nativePointer,
            (jlong)_kernels);
    (*env)->SetLongField(
            env, 
            kernels, 
            NativePointerObject_byteOffset,
            0);

    (*env)->SetIntArrayRegion(env, errcode, 0, 1, &_err);

    return kernels;
}

JNIEXPORT void JNICALL
Java_clgp_CLGP_clgpRelease(
        JNIEnv *env,
        jclass cls,
        jobject kernels)
{
    cl_kernel *_kernels = NULL;

    cls = cls;

    _kernels = (cl_kernel *)
        (*env)->GetLongField(
                env, 
                kernels,
                NativePointerObject_nativePointer);
    clgpRelease(_kernels);
}

JNIEXPORT jlong JNICALL 
Java_clgp_CLGP_clgpMaxlevel(
        JNIEnv *env, 
        jclass cls, 
        jlong width, 
        jlong height)
{
    env = env;
    cls = cls;
    return clgpMaxlevel(width, height);
}

JNIEXPORT jint JNICALL 
Java_clgp_CLGP_clgpEnqueuePyramid(
        JNIEnv *env, 
        jclass cls, 
        jobject command_queue, 
        jobjectArray kernels, 
        jobjectArray pyramid_images, 
        jobject input_image, 
        jlong maxlevel)
{
    cl_int _err = CL_SUCCESS;
    cl_command_queue _queue = NULL;
    cl_kernel *_kernels = NULL;
    cl_mem _input_image = NULL;
    cl_mem *_pyramid_images = NULL;

    cls = cls;

    _queue = (cl_command_queue)
        (*env)->GetLongField(
                env, 
                command_queue,
                NativePointerObject_nativePointer);
    _kernels = (cl_kernel *)
        (*env)->GetLongField(
                env, 
                kernels,
                NativePointerObject_nativePointer);
    _input_image = (cl_mem)
        (*env)->GetLongField(
                env, 
                input_image,
                NativePointerObject_nativePointer);
    _pyramid_images = malloc(maxlevel*sizeof(cl_mem));
    assert(_pyramid_images != NULL);
    for (size_t l = 0; l < (size_t)maxlevel; l++) {
        jobject pyramid_image = NULL;
        pyramid_image = (*env)->GetObjectArrayElement(env, pyramid_images, l),
        _pyramid_images[l] = (cl_mem)
             (*env)->GetLongField(
                    env, 
                    pyramid_image,
                    NativePointerObject_nativePointer);
        (*env)->DeleteLocalRef(env, pyramid_image);
    }

    _err = 
        clgpEnqueuePyramid(
                _queue,
                _kernels,
                _pyramid_images,
                _input_image,
                maxlevel);

    free(_pyramid_images);

    return _err;
}

JNIEXPORT jlong JNICALL
Java_clgp_CLGP_clgpMaxlevelHalfOctave(
        JNIEnv *env, 
        jclass cls, 
        jlong width, 
        jlong height)
{
    env = env;
    cls = cls;
    return clgpMaxlevelHalfOctave(width, height);
}
  
