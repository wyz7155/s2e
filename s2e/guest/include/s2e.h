/*
 * S2E Selective Symbolic Execution Framework
 *
 * Copyright (c) 2010, Dependable Systems Laboratory, EPFL
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Dependable Systems Laboratory, EPFL nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE DEPENDABLE SYSTEMS LABORATORY, EPFL BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Currently maintained by:
 *    Volodymyr Kuznetsov <vova.kuznetsov@epfl.ch>
 *    Vitaly Chipounov <vitaly.chipounov@epfl.ch>
 *
 * All contributors are listed in S2E-AUTHORS file.
 *
 */

#ifndef S2E_MAIN_HEADER_H // MJR
#define S2E_MAIN_HEADER_H // MJR

//#include <stdio.h> // MJR
//#include <string.h> // MJR
//#include <inttypes.h> // MJR

// MJR:
#define ALWAYS_INLINE static __attribute__((always_inline)) inline
// #define ALWAYS_INLINE __attribute__((always_inline))
// #define ALWAYS_INLINE inline

/** Forces the read of every byte of the specified string.
  * This makes sure the memory pages occupied by the string are paged in
  * before passing them to S2E, which can't page in memory by itself. */
ALWAYS_INLINE void __s2e_touch_string(volatile const char *string)
{
    while(*string) {
        ++string;
    }
}

ALWAYS_INLINE void __s2e_touch_buffer(volatile void *buffer, unsigned size)
{
    unsigned i;
    volatile char *b = (volatile char *)buffer;
    for (i=0; i<size; ++i) {
        *b; ++b;
    }
}

/** Get S2E version or 0 when running without S2E. */
ALWAYS_INLINE int s2e_version(void) // MJR
{
    int version;
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : "=a" (version)  : "a" (0)
    );
    return version;
}

/** Print message to the S2E log. */
ALWAYS_INLINE void s2e_message(const char* message)
{
    __s2e_touch_string(message);
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x10, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (message)
    );
}

/** Print warning to the S2E log and S2E stdout. */
ALWAYS_INLINE void s2e_warning(const char* message)
{
    __s2e_touch_string(message);
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x10, 0x01, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (message)
    );
}

/** Print symbolic expression to the S2E log. */
ALWAYS_INLINE void s2e_print_expression(const char* name, int expression)
{
    __s2e_touch_string(name);
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x07, 0x01, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (expression), "c" (name)
    );
}

/** Enable forking on symbolic conditions. */
ALWAYS_INLINE void s2e_enable_forking(void)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x09, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
    );
}

/** Disable forking on symbolic conditions. */
ALWAYS_INLINE void s2e_disable_forking(void)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x0a, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
    );
}

/** Yield the current state */
static inline void s2e_yield(void)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x0F, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
    );
}

/** Get the current execution path/state id. */
ALWAYS_INLINE unsigned s2e_get_path_id(void)
{
    unsigned id;
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x05, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : "=a" (id)
    );
    return id;
}

/** Fill buffer with unconstrained symbolic values. */
ALWAYS_INLINE void s2e_make_symbolic(void* buf, int size, const char* name)
{
    __s2e_touch_string(name);
    __s2e_touch_buffer(buf, size);
    __asm__ __volatile__(
        "pushl %%ebx\n"
        "movl %%edx, %%ebx\n"
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x03, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        "popl %%ebx\n"
        : : "a" (buf), "d" (size), "c" (name) : "memory"
    );
}

////////////////// MJR -------------------------------------------->
// MJR 23 in all.

/** Fill buffer with unconstrained symbolic values. */
/** Equivalent to I/O memory:  writes are discarded */
/* MJR added this, requires SymbolicHardware */
ALWAYS_INLINE void s2e_make_dma_symbolic(void* buf, int size, const char* name)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xB1, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (buf), "b" (size), "c" (name) : "memory"
                         );
}

/** Remove symbolic mapping. */
/* Does not leave memory as symbolic afterward. Reads revert to underlying concrete value */
/* MJR added this, requires SymbolicHardware */
ALWAYS_INLINE void s2e_free_dma_symbolic(void* buf, int size, const char* name)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xB2, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (buf), "b" (size), "c" (name) : "memory"
                         );
}

// MJR added this, requires SymDriveSearcher...
ALWAYS_INLINE void s2e_prioritize(int line)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xB3, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line)
                         );
}

// MJR added this, requires SymDriveSearcher...
ALWAYS_INLINE void s2e_deprioritize(int line)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xB4, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line)
                         );
}

// MJR added this, requires SymDriveSearcher...
ALWAYS_INLINE void s2e_loop_before(int line, int call_id)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xB5, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line), "b" (call_id)
                         );
}

// MJR added this, requires SymDriveSearcher...
ALWAYS_INLINE void s2e_loop_body(int line, int call_id)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xB6, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line), "b" (call_id)
                         );
}

// MJR added this, requires SymDriveSearcher...
ALWAYS_INLINE void s2e_loop_after(int line, int call_id)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xB7, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line), "b" (call_id)
                         );
}

// MJR Added this, requires SymDriveSearcher
ALWAYS_INLINE void s2e_concretize_kill(int line)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xB8, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line)
                         );
}

ALWAYS_INLINE void s2e_concretize_all(int line) {
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xB9, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line)
                         );
}

ALWAYS_INLINE void s2e_kill_all_others(int line) {
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xBA, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line)
                         );
}

// MJR Added this, requires SymDriveSearcher
ALWAYS_INLINE void s2e_driver_call_stack(int line, int depth)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xBB, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line), "b" (depth)
                         );
}

// MJR added this, requires SymDriveSearcher...
ALWAYS_INLINE void s2e_favor_successful(int line, int successful)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xBC, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line), "b" (successful)
                         );
}

// MJR opcode hole

// MJR added this, requires SymDriveSearcher...
ALWAYS_INLINE void s2e_reset_priorities(int line)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xBE, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line)
                         );
}

// MJR opcode hole

// MJR added this, requires SymDriveSearcher...
ALWAYS_INLINE void s2e_enable_tracing(int line)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xC0, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line)
    );
}

// MJR added this, requires SymDriveSearcher...
ALWAYS_INLINE void s2e_disable_tracing(int line)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xC1, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line)
    );
}

// MJR added this, requires SymDriveSearcher...
/** Notify S2E that we've entered a driver function. */
ALWAYS_INLINE void s2e_enter_function(int line, const char* message, int wrapper_type)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xC2, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line), "b" (message), "c" (wrapper_type)
    );
}

// MJR added this, requires SymDriveSearcher...
/** Notify S2E that we've exited a driver function. */
ALWAYS_INLINE void s2e_exit_function(int line, const char* message, int wrapper_type)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xC3, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line), "b" (message), "c" (wrapper_type)
    );
}

// MJR added this, requires SymDriveSearcher...
ALWAYS_INLINE int s2e_is_symbolic_symdrive(int line, int expr)
{
    int retval;
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xC4, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : "=a" (retval)  : "a" (line), "b" (expr)
    );
    return retval;
}

// MJR added this, requires SymDriveSearcher...
ALWAYS_INLINE void s2e_success_path(int line, int success)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xC5, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line), "b" (success)
    );
}

// MJR added this, requires SymDriveSearcher...
ALWAYS_INLINE void s2e_enter_block(int line, const char *fn, int total_blocks, int cur_block)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xC6, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line), "b" (fn), "c" (total_blocks), "d" (cur_block)
    );
}

// MJR added this, requires SymDriveSearcher...
ALWAYS_INLINE void s2e_primary_fn(int line, const char *fn)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xC7, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line), "b" (fn)
                         );
}

// If changing this:
// Update SymDriveSearcher.h
// Update MJR_dump_trackperf function
// Update MTbTrace
#define SYMDRIVE_PAUSE_PP 11
#define SYMDRIVE_CONTINUE_PP 12

#define SYMDRIVE_PAUSE_STUB 21
#define SYMDRIVE_CONTINUE_STUB 22

#define SYMDRIVE_PAUSE_IRQ 31
#define SYMDRIVE_CONTINUE_IRQ 32

#define SYMDRIVE_START_AUTO 40
#define SYMDRIVE_PAUSE_AUTO 41
#define SYMDRIVE_CONTINUE_AUTO 42
#define SYMDRIVE_STOP_AUTO 43
#define SYMDRIVE_DISCARD_AUTO 44

#define SYMDRIVE_START_MANUAL 50
#define SYMDRIVE_PAUSE_MANUAL 51
#define SYMDRIVE_CONTINUE_MANUAL 52
#define SYMDRIVE_STOP_MANUAL 53
#define SYMDRIVE_DISCARD_MANUAL 54

#define SYMDRIVE_START_FN 60
#define SYMDRIVE_STOP_FN 61

// MJR added this, requires SymDriveSearcher...
ALWAYS_INLINE void s2e_enable_trackperf(int line, int flags)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xC8, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line), "b" (flags)
                         );
}

// MJR added this, requires SymDriveSearcher...
ALWAYS_INLINE void s2e_disable_trackperf(int line, int flags)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xC9, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line), "b" (flags)
                         );
}

// MJR added this, requires SymDriveSearcher...
#define TRACKPERF_NONTRANSITIVE 0
#define TRACKPERF_TRANSITIVE 1
ALWAYS_INLINE void s2e_trackperf_fn(int line, const char *fn, int flags)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xCA, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (line), "b" (fn), "c" (flags)
                         );
}

/////////////////////////////////////// <-------------- MJR

/** Fill buffer with unconstrained symbolic values without discarding concrete data. */
ALWAYS_INLINE void s2e_make_concolic(void* buf, int size, const char* name)
{
    __s2e_touch_string(name);
    __s2e_touch_buffer(buf, size);
    __asm__ __volatile__(
        "pushl %%ebx\n"
        "movl %%edx, %%ebx\n"
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x11, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        "popl %%ebx\n"
        : : "a" (buf), "d" (size), "c" (name) : "memory"
    );
}

/** Returns true if ptr points to symbolic memory */
ALWAYS_INLINE inline int s2e_is_symbolic(void* ptr, size_t size)
{
    int result;
    __s2e_touch_buffer(ptr, 1);
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x04, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : "=a" (result) : "a" (size), "c" (ptr)
    );
    return result;
}

/** Concretize the expression. */
ALWAYS_INLINE void s2e_concretize(void* buf, int size)
{
    __s2e_touch_buffer(buf, size);
    __asm__ __volatile__(
        "pushl %%ebx\n"
        "movl %%edx, %%ebx\n"
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x20, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        "popl %%ebx\n"
        : : "a" (buf), "d" (size) : "memory"
    );
}

/** Get example value for expression (without adding state constraints). */
ALWAYS_INLINE void s2e_get_example(void* buf, int size)
{
    __s2e_touch_buffer(buf, size);
    __asm__ __volatile__(
        "pushl %%ebx\n"
        "movl %%edx, %%ebx\n"
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x21, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        "popl %%ebx\n"
        : : "a" (buf), "d" (size) : "memory"
    );
}

/** Get example value for expression (without adding state constraints). */
/** Convenience function to be used in printfs */
ALWAYS_INLINE unsigned s2e_get_example_uint(unsigned val)
{
    unsigned buf = val;
    __asm__ __volatile__(
        "pushl %%ebx\n"
        "movl %%edx, %%ebx\n"
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x21, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        "popl %%ebx\n"
        : : "a" (&buf), "d" (sizeof(buf)) : "memory"
    );
    return buf;
}

/** Terminate current state. */
// MJR changed implementation:
// ALWAYS_INLINE void s2e_kill_state(int status, const char* message)
ALWAYS_INLINE void s2e_kill_state(int kill_all, int status, const char* message)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x06, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "a" (kill_all), "d" (status), "c" (message)
    );
/*
    __s2e_touch_string(message);
    __asm__ __volatile__(
        "pushl %%ebx\n"
        "movl %%edx, %%ebx\n"
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x06, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        "popl %%ebx\n"
        : : "a" (status), "d" (message)
    );
*/
}

/** Disable timer interrupt in the guest. */
ALWAYS_INLINE void s2e_disable_timer_interrupt(void) // MJR
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x50, 0x01, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
    );
}

/** Enable timer interrupt in the guest. */
ALWAYS_INLINE void s2e_enable_timer_interrupt(void) // MJR
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x50, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
    );
}

/** Disable all APIC interrupts in the guest. */
ALWAYS_INLINE void s2e_disable_all_apic_interrupts(void) // MJR
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x51, 0x01, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
    );
}

/** Enable all APIC interrupts in the guest. */
ALWAYS_INLINE void s2e_enable_all_apic_interrupts(void) // MJR
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x51, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
    );
}

/** Get the current S2E_RAM_OBJECT_BITS configuration macro */
ALWAYS_INLINE int s2e_get_ram_object_bits(void) // MJR
{
    int bits;
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x52, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : "=a" (bits)  : "a" (0)
    );
    return bits;
}

/** Declare a merge point: S2E will try to merge
 *  all states when they reach this point.
 *
 * NOTE: This requires merge searcher to be enabled. */
ALWAYS_INLINE void s2e_merge_point(void) // MJR
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0x70, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
    );
}

/** Open file from the guest.
 *
 * NOTE: This require HostFiles plugin. */
ALWAYS_INLINE int s2e_open(const char* fname)
{
    int fd;
    __s2e_touch_string(fname);
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xEE, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : "=a" (fd) : "a"(-1), "b" (fname), "c" (0)
    );
    return fd;
}

/** Close file from the guest.
 *
 * NOTE: This require HostFiles plugin. */
ALWAYS_INLINE int s2e_close(int fd)
{
    int res;
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xEE, 0x01, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : "=a" (res) : "a" (-1), "b" (fd)
    );
    return res;
}

/** Read file content from the guest.
 *
 * NOTE: This require HostFiles plugin. */
ALWAYS_INLINE int s2e_read(int fd, char* buf, int count)
{
    int res;
    __s2e_touch_buffer(buf, count);
    __asm__ __volatile__(
        "pushl %%ebx\n"
        "movl %%esi, %%ebx\n"
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xEE, 0x02, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        "popl %%ebx\n"
        : "=a" (res) : "a" (-1), "S" (fd), "c" (buf), "d" (count)
    );
    return res;
}

/** Enable memory tracing */
static inline void s2e_memtracer_enable()
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xac, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
    );
}

/** Disable memory tracing */
static inline void s2e_memtracer_disable()
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xac, 0x01, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
    );
}

/** Raw monitor plugin */
/** Communicates to S2E the coordinates of loaded modules. Useful when there is
    no plugin to automatically parse OS data structures */
ALWAYS_INLINE void s2e_rawmon_loadmodule(const char *name, unsigned loadbase, unsigned size)
{
    __s2e_touch_string(name);
    __asm__ __volatile__(
        "pushl %%ebx\n"
        "movl %%edx, %%ebx\n"
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xAA, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        "popl %%ebx\n"
        : : "a" (name), "d" (loadbase), "c" (size)
    );
}

#if 0
//  MJR commented out
typedef struct _s2e_opcode_module_config_t {
    uint32_t name;
    uint64_t nativeBase;
    uint64_t loadBase;
    uint64_t entryPoint;
    uint64_t size;
    uint32_t kernelMode;
} __attribute__((packed)) s2e_opcode_module_config_t;

/** Raw monitor plugin */
/** Communicates to S2E the coordinates of loaded modules. Useful when there is
    no plugin to automatically parse OS data structures */
ALWAYS_INLINE void s2e_rawmon_loadmodule2(const char *name,
                                         uint64_t nativebase,
                                         uint64_t loadbase,
                                         uint64_t entrypoint,
                                         uint64_t size, unsigned kernelMode)
{
    s2e_opcode_module_config_t cfg;
    cfg.name = (uint32_t) name;
    cfg.nativeBase = nativebase;
    cfg.loadBase = loadbase;
    cfg.entryPoint = entrypoint;
    cfg.size = size;
    cfg.kernelMode = kernelMode;

    __s2e_touch_string(name);

    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xAA, 0x02, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "c" (&cfg)
    );
}
#endif

#if 0
// MJR commented out
/** CodeSelector plugin */
/** Enable forking in the current process (entire address space or user mode only) */
ALWAYS_INLINE void s2e_codeselector_enable_address_space(unsigned user_mode_only)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xAE, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "c" (user_mode_only)
    );
}

/** Disable forking in the specified process (represented by its page directory).
    If pagedir is 0, disable forking in the current process. */
ALWAYS_INLINE void s2e_codeselector_disable_address_space(uint64_t pagedir)
{
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xAE, 0x01, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "c" (pagedir)
    );
}
#endif

ALWAYS_INLINE void s2e_codeselector_select_module(const char *moduleId)
{
    __s2e_touch_string(moduleId);
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xAE, 0x02, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
        : : "c" (moduleId)
    );
}

/** Programmatically add a new configuration entry to the ModuleExecutionDetector plugin */
ALWAYS_INLINE void s2e_moduleexec_add_module(const char *moduleId, const char *moduleName, int kernelMode)
   
{
    __s2e_touch_string(moduleId);
    __s2e_touch_string(moduleName);
    __asm__ __volatile__(
        ".byte 0x0f, 0x3f\n"
        ".byte 0x00, 0xAF, 0x00, 0x00\n"
        ".byte 0x00, 0x00, 0x00, 0x00\n"
            : : "c" (moduleId), "a" (moduleName), "d" (kernelMode)
    );
}




/**
 * If processToRetrive == null,  get the name, load address, and
 * the size of the currently running process.
 *
 * If processToRetrive != null,  get the name, load address, and
 * the size of the specified module in the currently running process.
 *
 * The returned name is an absolute path to the program file.
 */
#if 0
//  MJR Commented out
ALWAYS_INLINE int s2e_get_module_info(const char *moduleToRetrieve,
                                char *name, size_t maxNameLength,
                                uint64_t *loadBase, uint64_t *size)
{
    const char* modName = NULL;
    int result = -1;

    if (moduleToRetrieve) {
        modName = strrchr(moduleToRetrieve, '/');
        if (modName == NULL) {
            modName = name;
        } else {
            ++modName;  // point to the first char after the slash
        }
    }

    FILE* maps = fopen("/proc/self/maps", "r");
    if (!maps) {
        return result;
    }

    size_t start, end;
    char executable;

    char line[256], path[128];
    while (fgets(line, sizeof(line), maps)) {
        if (sscanf(line, "%x-%x %*c%*c%c%*c %*x %*s %*d %127[^\n]", &start, &end, &executable, path) == 4) {
            if (modName) {
                if (executable == 'x' && strstr(path, modName) != NULL) {
                    *loadBase = start;
                    *size = end - start;
                    strncpy(name, path, maxNameLength);
                    result = 0;
                    break;
                }
            } else {
                if (!executable) {
                    continue;
                }

                //Found the module, get its data
                *loadBase = start;
                *size = end - start;
                strncpy(name, path, maxNameLength);
                result = 0;
                break;
            }
        }
    }

    fclose(maps);
    return result;
}
#endif

/* Kills the current state if b is zero */
ALWAYS_INLINE void _s2e_assert(int b, const char *expression )
{
   if (!b) {
       s2e_kill_state(1, 0, expression); // MJR added 1
   }
}

#define s2e_assert(expression) _s2e_assert(expression, "Assertion failed: "  #expression)

/** Returns a symbolic value in [start, end) */
ALWAYS_INLINE int s2e_range(int start, int end, const char* name)
{
  int x = -1;

  if (start >= end) {
      s2e_kill_state(1, 1, "s2e_range: invalid range"); // MJR added 1
  }

  if (start+1==end) {
    return start;
  } else {
    s2e_make_symbolic(&x, sizeof x, name);

    /* Make nicer constraint when simple... */
    if (start==0) {
      if ((unsigned) x >= (unsigned) end) {
          s2e_kill_state(1, 0, "s2e_range creating a constraint..."); // MJR added 1
      }
    } else {
      if (x < start || x >= end) {
          s2e_kill_state(1, 0, "s2e_range creating a constraint..."); // MJR added 1
      }
    }

    return x;
  }
}

#endif
