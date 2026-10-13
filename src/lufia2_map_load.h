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

/* True only while the loader is at work on bank $7F. Before that, with the
 * next map already selected, the bank still holds the map on the screen. */
bool Lufia2MapLoadRewriting(void);

/* Increments once per completed load. Consumers cache map-derived state per
 * value and re-read bank $7F when it changes. */
uint32_t Lufia2MapLoadGeneration(void);

/* Discard observations derived from the abandoned host timeline. */
void Lufia2MapLoadStateChanged(void);
