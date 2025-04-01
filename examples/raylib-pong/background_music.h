/**
 * Background music data
 * 
 * This would normally contain the actual binary data for the music file.
 * For demonstration purposes, this is a minimal placeholder.
 */

#ifndef BACKGROUND_MUSIC_H
#define BACKGROUND_MUSIC_H

// Placeholder data for demonstration purposes only
// In a real project, this would be the actual binary music data
static const unsigned char background_music_ogg[] = {
    0x4F, 0x67, 0x67, 0x53, // "OggS" header
    0x00, 0x02, 0x00, 0x00, // Minimal data
    0x00, 0x00, 0x00, 0x00  // More minimal data
};

static const unsigned int background_music_ogg_len = sizeof(background_music_ogg);

#endif // BACKGROUND_MUSIC_H 