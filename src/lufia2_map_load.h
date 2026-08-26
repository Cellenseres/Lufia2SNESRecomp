#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Readiness of the processed map in bank $7F.
 *
 * The guest writes the current-map field at $7E:05AC before bank $7F holds
 * the matching map, so that field alone never proves the buffer is readable.
 * These hooks observe the original loader instead; they do not change guest
 * behavior. */
void Lufia2MapLoadInstallHooks(void);

/* Call once per prepared video frame, before the readiness query. */
void Lufia2MapLoadFrame(void);

/* True while the guest is replacing the processed map, or while the selected
 * map has not been committed by the loader yet. */
bool Lufia2MapLoadInProgress(void);

/* Increments once per completed load. Consumers cache map-derived state per
 * value and re-read bank $7F when it changes. */
uint32_t Lufia2MapLoadGeneration(void);
