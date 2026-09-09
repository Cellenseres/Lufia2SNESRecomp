#pragma once

/* Compile-time guard for the recompiler return ABI.
 *
 * snesrecomp's paired-call ABI returns `RecompReturn`, an enum that only
 * declares 0..3, but the LLE yield mechanism casts an out-of-band sentinel
 * (RECOMP_RETURN_LLE_UNWIND_BASE, 0x40000000) into that same type. The
 * sentinel therefore only survives while the compiler gives the enum an
 * int-sized representation.
 *
 * That is not guaranteed. GCC defaults to -fshort-enums on arm-*-eabi, which
 * gives this enum one byte; the sentinel truncates to 0, which is
 * RECOMP_RETURN_NORMAL. Generated callsites then treat a yield as an ordinary
 * return, keep executing the compiled body, and leave the unwind armed until
 * the scheduler reports "stale LLE yield unwind cleared". That is exactly the
 * failure observed on a short-enum target with AOT bounce enabled, and
 * nothing in the build warned about it.
 *
 * These assertions turn that silent miscompile into a build error. Target
 * adapters that use short enums must opt the entire ABI into int-sized enums.
 */

#include "cpu_state.h"

_Static_assert(sizeof(RecompReturn) >= sizeof(int),
               "RecompReturn is narrower than int: the LLE unwind sentinel "
               "will truncate. Build with -fno-short-enums.");

_Static_assert((RecompReturn)RECOMP_RETURN_LLE_UNWIND_BASE !=
                   RECOMP_RETURN_NORMAL,
               "RECOMP_RETURN_LLE_UNWIND_BASE collapses to "
               "RECOMP_RETURN_NORMAL in this enum representation.");

_Static_assert((RecompReturn)RECOMP_RETURN_LLE_UNWIND_BASE !=
                       RECOMP_RETURN_SKIP_1 &&
                   (RecompReturn)RECOMP_RETURN_LLE_UNWIND_BASE !=
                       RECOMP_RETURN_SKIP_2 &&
                   (RecompReturn)RECOMP_RETURN_LLE_UNWIND_BASE !=
                       RECOMP_RETURN_SKIP_3,
               "The LLE unwind sentinel is indistinguishable from a genuine "
               "SKIP_N return.");
