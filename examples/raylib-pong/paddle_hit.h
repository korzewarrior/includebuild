/**
 * Paddle hit sound effect data
 * 
 * This would normally contain the actual binary data for the sound file.
 * For demonstration purposes, this is a minimal placeholder.
 */

#ifndef PADDLE_HIT_H
#define PADDLE_HIT_H

// Placeholder data for demonstration purposes only
// In a real project, this would be the actual binary sound data
static const unsigned char paddle_hit_wav[] = {
    0x52, 0x49, 0x46, 0x46, // "RIFF" header
    0x08, 0x00, 0x00, 0x00, // Minimal size
    0x57, 0x41, 0x56, 0x45  // "WAVE" format
};

static const unsigned int paddle_hit_wav_len = sizeof(paddle_hit_wav);

#endif // PADDLE_HIT_H 