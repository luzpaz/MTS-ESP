/*
Copyright (C) 2021 by ODDSound Ltd. info@oddsound.com

Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
*/

#ifndef libMTSClient_h
#define libMTSClient_h

#ifdef __cplusplus
extern "C" {
#endif
    
    /*
     How to use libMTSClient:

     On startup in the constructor:

        MTSClient *client = MTS_RegisterClient();

     On shutdown in the destructor:

        MTS_DeregisterClient(client);

     When given a note:

        double f = MTS_NoteToFrequency(client, midinote, midichannel);
     OR
        double retune_semitones = MTS_RetuningInSemitones(client, midinote, midichannel);
     OR
        double retune_ratio = MTS_RetuningAsRatio(client, midinote, midichannel);

     If you don’t have the midi channel, use -1, but supplying the channel allows support for microtonal
     MIDI controllers with more than 128 keys that use multi-channel tuning tables.

     ***NOTE***: Querying retune whilst a note is playing allows the tuning to change along the flight of the note,
     which is the ideal, so do this if you can and as often as possible. Ideally at the same time as processing
     any other pitch modulation sources (envelopes, MIDI controllers, LFOs etc.).

     The Scala .kbm keyboard mapping file format allows for MIDI keys to be unmapped i.e. no frequency
     is specified for them. The MTS-ESP library supports this. You can query whether a note
     is unmapped and should be ignored with:

        bool should_ignore_note = MTS_ShouldFilterNote(client, midinote, midichannel);

     If this returns true, ignore the noteOn and don’t play anything. Calling this function is encouraged but
     optional and a valid value for the frequency/retuning will still be returned for an unmapped note. Once again
     if you don’t have the midi channel, use -1, however supplying it allows a master to dedicate notes on specific
     channels for e.g. key switches to change tunings.
     
     Helper functions are available which return the MIDI note whose pitch is nearest a given frequency.
     The MIDI note returned is guaranteed to be mapped. If you intend to generate a note-on message using the
     returned note number, you may already know which MIDI channel you will send it on, in which case you can specify this in
     the call, else the client library can prescribe a channel for you. This is done so that multi-channel mapping
     and note filtering can be respected. See below for further details.

     To add support for MIDI Tuning System (or MTS, from the MIDI specification) SysEx messages to your plugin,
     implement the above and, when given SysEx, call:

        MTS_ParseMIDIData(client, buffer, len); // if buffer is signed char *
     OR
        MTS_ParseMIDIDataU(client, buffer, len); // if buffer is unsigned char *

     If you want to display to the user whether you can see a Master in the session, call:

        bool has_master = MTS_HasMaster(client);

     It is possible to query the name of the current scale.  This function is necessarily supplied for the case
     where a client is sending MTS SysEx messages, however it can be used to display the current scale name
     to the user on your UI too:

        const char *name = MTS_GetScaleName(client);
     
     */
    
    // Opaque datatype for MTSClient.
    typedef struct MTSClient MTSClient;

    // Register/deregister as a client.  Call from the plugin constuctor and destructor.
    extern MTSClient *MTS_RegisterClient();
    extern void MTS_DeregisterClient(MTSClient *client);

    // Check if the client is currently connected to a master plugin.
    extern bool MTS_HasMaster(MTSClient *client);

    // Returns true if note should not be played. MIDI channel argument is optional but should be included if possible (0-15), else set to -1.
    extern bool MTS_ShouldFilterNote(MTSClient *client, char midinote, char midichannel);

    // Retuning a midi note. Pick the version that makes your life easiest! MIDI channel argument is optional but should be included if possible (0-15), else set to -1.
    extern double MTS_NoteToFrequency(MTSClient *client, char midinote, char midichannel);
    extern double MTS_RetuningInSemitones(MTSClient *client, char midinote, char midichannel);
    extern double MTS_RetuningAsRatio(MTSClient *client, char midinote, char midichannel);
    
    // MTS_FrequencyToNote() is a helper function returning the note number whose pitch is closest to the supplied frequency. Two versions are provided:
    // The first is for the simplest case: supply a frequency and get a note number back.
    // If you intend to use the returned note number to generate a note-on message on a specific, pre-determined MIDI channel, set the midichannel argument to the destination channel (0-15), else set to -1.
    // If a MIDI channel is supplied, the corresponding multi-channel tuning table will be queried if in use, else multi-channel tables are ignored.
    extern char MTS_FrequencyToNote(MTSClient *client, double freq, char midichannel);
    // Use the second version if you intend to use the returned note number to generate a note-on message and where you have the possibility to send it on any MIDI channel.
    // The midichannel argument is a pointer to a char which will receive the MIDI channel on which the note message should be sent (0-15).
    // Multi-channel tuning tables are queried if in use.
    extern char MTS_FrequencyToNoteAndChannel(MTSClient *client, double freq, char *midichannel);
    
    // Returns the name of the current scale.
    extern const char *MTS_GetScaleName(MTSClient *client);

    // Parse incoming MIDI data to update local retuning.  All formats of MTS sysex message accepted.
    extern void MTS_ParseMIDIDataU(MTSClient *client, const unsigned char *buffer, int len);
    extern void MTS_ParseMIDIData(MTSClient *client, const char *buffer, int len);

#ifdef __cplusplus
}
#endif

#endif

