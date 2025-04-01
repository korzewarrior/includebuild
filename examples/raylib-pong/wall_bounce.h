/**
 * Wall bounce sound effect data
 * 
 * This would normally contain the actual binary data for the sound file.
 * For demonstration purposes, this is a minimal placeholder.
 */

#ifndef WALL_BOUNCE_H
#define WALL_BOUNCE_H

// Placeholder data for demonstration purposes only
// In a real project, this would be the actual binary sound data
static const unsigned char wall_bounce_wav[] = {
    0x52, 0x49, 0x46, 0x46, // "RIFF" header
    0x08, 0x00, 0x00, 0x00, // Minimal size
    0x57, 0x41, 0x56, 0x45  // "WAVE" format
};

static const unsigned int wall_bounce_wav_len = sizeof(wall_bounce_wav);

#endif // WALL_BOUNCE_H 